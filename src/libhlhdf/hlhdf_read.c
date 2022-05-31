/* --------------------------------------------------------------------
Copyright (C) 2009 Swedish Meteorological and Hydrological Institute, SMHI,

This file is part of HLHDF.

HLHDF is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HLHDF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with HLHDF.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------*/

/**
 * Functions for working with HL_Node's and HL_NodeList's.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-06-12
 */
#include "hlhdf.h"
#include "hlhdf_alloc.h"
#include "hlhdf_private.h"
#include "hlhdf_debug.h"
#include "hlhdf_defines_private.h"
#include "hlhdf_node_private.h"
#include <string.h>
#include <stdlib.h>

/*@{ Typedefs */

/**
 * Struct used internally for beeing able to do a reverse name lookup when
 * reading a file with references. We dont want to have the data in the object
 * referenced to, but instead we want to have the name, thats why this
 * has to be used.
 */
typedef struct ReferenceLookup
{
  hid_t file_id;                /**< the file id identifier */
  hobj_ref_t* ref;              /**< the reference */
  char tmp_name[512];           /**< tmp name used for generating paths */
  char found_name[512];         /**< the found name */
  int reffound;                 /**< if the reference has been found */
  struct ReferenceLookup* next; /**< next reference */
} ReferenceLookup;

/**
 * Used when traversing over the different nodes during reading.
 */
typedef struct VisitorStruct {
  char* path; /**< the root path initiating the visitor */
  HL_NodeList* nodelist; /**< the nodelist where to add nodes */
} VisitorStruct;

/*@} End of Typedefs */

/*@{ Private functions */
static HL_CompoundTypeDescription* buildTypeDescriptionFromTypeHid(hid_t type_id)
{
  HL_CompoundTypeDescription* typelist = NULL;
  HL_CompoundTypeAttribute* typenode;
  int i, j, nmembers;
  char* fname = NULL;
  hid_t mtype = -1;
  hid_t fixedType = -1;
  size_t* dims = NULL;
  hsize_t* dims_h514 = NULL;
  size_t dSize;
  int ndims;
  HL_FormatSpecifier format = HLHDF_UNDEFINED;

  HL_DEBUG0("ENTER: buildTypeDescriptionFromTypeHid");
  if (!(typelist = newHL_CompoundTypeDescription())) {
    HL_ERROR0("Failed to create datatype nodelist");
    goto fail;
  }

  typelist->size = H5Tget_size(type_id);
  nmembers = H5Tget_nmembers(type_id);
  for (i = 0; i < nmembers; i++) {
    size_t offset = H5Tget_member_offset(type_id, i);
    fname = H5Tget_member_name(type_id, i);
    mtype = H5Tget_member_type(type_id, i);

    if ((fixedType = getFixedType(mtype)) < 0) {
      HL_ERROR0("Failed to convert to fixed type");
      goto fail;
    }

    format = HL_getFormatSpecifierFromType(fixedType);
    if (format == HLHDF_UNDEFINED) {
      HL_ERROR0("Undefined format specifier");
      goto fail;
    }

    if (H5T_ARRAY == H5Tget_member_class(type_id, i)) {
      ndims = H5Tget_array_ndims(mtype);
      dims_h514 = (hsize_t*) HLHDF_MALLOC(sizeof(hsize_t) * ndims);
      if (H5Tget_array_dims(mtype, dims_h514) < 0) {
        HL_ERROR0("Failed to retrieve array dimensions");
      }
      dims = (size_t*) HLHDF_MALLOC(sizeof(size_t) * ndims);
      for (j = 0; j < ndims; j++) {
        dims[j] = (size_t) dims_h514[j];
      }
    } else {
      ndims = 1;
      dims = (size_t*) HLHDF_MALLOC(sizeof(size_t) * ndims);
      dims[0] = 1;
    }
    dSize = H5Tget_size(fixedType);

    if (fname == NULL) {
      HL_ERROR0("fname is NULL, cant use it to create CompoundTypeAttribute\n");
      goto fail;
    }
    typenode = newHL_CompoundTypeAttribute(fname,
                                           offset,
                                           HL_getFormatSpecifierString(format),
                                           dSize,
                                           ndims,
                                           dims);
    if (!addHL_CompoundTypeAttribute(typelist, typenode))
      goto fail;
    HLHDF_FREE(dims);
    HLHDF_FREE(dims_h514);
    HL_H5T_CLOSE(mtype);
    if (fname != NULL) { free(fname); fname = NULL; }
    HL_H5T_CLOSE(fixedType);
  }

  HLHDF_FREE(dims_h514);
  HLHDF_FREE(dims);
  HL_H5T_CLOSE(mtype);
  HL_H5T_CLOSE(fixedType);
  if (fname != NULL) { free(fname); fname = NULL; }
  return typelist;
fail:
  HLHDF_FREE(dims);
  HLHDF_FREE(dims_h514);
  HL_H5T_CLOSE(mtype);
  HL_H5T_CLOSE(fixedType);
  if (fname != NULL) { free(fname); fname = NULL; }
  freeHL_CompoundTypeDescription(typelist);
  return NULL;
}

static int checkIfReferenceMatch(hid_t loc_id, char* path, hobj_ref_t* ref)
{
  hobj_ref_t matchref;
  herr_t status;
  HL_DEBUG0("ENTER:  checkIfReferenceMatch");
  status = H5Rcreate(&matchref, loc_id, path, H5R_OBJECT, -1);
  if (status < 0) {
    HL_ERROR1("Could not create reference to '%s'",path);
    return 0;
  }
  if (memcmp(ref, &matchref, sizeof(hobj_ref_t)) == 0) {
    return 1;
  }
  return 0;
}

/* ---------------------------------------
 * REF_GROUP_LOCATION_ITERATOR
 * Iterator function for checking group
 * references.
 * ---------------------------------------*/
static herr_t refGroupLocationIterator(hid_t gid, const char* name,
  void* op_data)
{
  ReferenceLookup* lookup = (ReferenceLookup*) op_data;
  char tmp1[1024], tmp2[1024];
  H5G_stat_t statbuf;
  hid_t obj = -1;
  HL_DEBUG0("ENTER: refGroupLocationIterator");
  H5Gget_objinfo(gid, name, 0, &statbuf);

  strcpy(tmp1, lookup->tmp_name);

  switch (statbuf.type) {
  case H5G_GROUP:
    if ((obj = H5Gopen(gid, name, H5P_DEFAULT)) >= 0) {
      sprintf(tmp2, "%s/%s", lookup->tmp_name, name);
      strcpy(lookup->tmp_name, tmp2);
      if (checkIfReferenceMatch(lookup->file_id, lookup->tmp_name, lookup->ref) == 1) {
        strcpy(lookup->found_name, lookup->tmp_name);
        lookup->reffound = 1;
      }
      H5Giterate(obj, ".", NULL, refGroupLocationIterator, lookup);
      strcpy(lookup->tmp_name, tmp1);
      HL_H5G_CLOSE(obj);
    }
    break;
  case H5G_DATASET:
    if ((obj = H5Dopen(gid, name, H5P_DEFAULT)) >= 0) {
      sprintf(tmp2, "%s/%s", lookup->tmp_name, name);
      strcpy(lookup->tmp_name, tmp2);
      if (checkIfReferenceMatch(lookup->file_id, lookup->tmp_name, lookup->ref)
          == 1) {
        strcpy(lookup->found_name, lookup->tmp_name);
        lookup->reffound = 1;
      }
      /*H5Aiterate(obj,NULL,refAttributeLocationIterator,lookup);*/
      strcpy(lookup->tmp_name, tmp1);
      HL_H5D_CLOSE(obj);
    }
    break;
  case H5G_TYPE:
    sprintf(tmp2, "%s/%s", lookup->tmp_name, name);
    strcpy(lookup->tmp_name, tmp2);
    if (checkIfReferenceMatch(lookup->file_id, lookup->tmp_name, lookup->ref) == 1) {
      strcpy(lookup->found_name, lookup->tmp_name);
      lookup->reffound = 1;
    }
    strcpy(lookup->tmp_name, tmp1);
    break;
  default:
    HL_ERROR1("Undefined type for %s",name);
    HL_ERROR1("Name: %s",tmp1);
    HL_ERROR1("Type id %d",statbuf.type);
    break;
  }
  return 0;
}

/* ---------------------------------------
 * LOCATE_NAME_FOR_REFERENCE
 * --------------------------------------- */
static char* locateNameForReference(hid_t file_id, hobj_ref_t* ref)
{
  hid_t gid = -1;
  ReferenceLookup lookup;

  HL_DEBUG0("ENTER: locateNameForReference");

  lookup.file_id = file_id;
  lookup.ref = ref;
  strcpy(lookup.tmp_name, "");
  strcpy(lookup.found_name, "");
  lookup.reffound = 0;

  if ((gid = H5Gopen(file_id, ".", H5P_DEFAULT)) < 0) {
    HL_ERROR0("Failed to open root group");
    goto fail;
  }

  H5Giterate(file_id, ".", NULL, refGroupLocationIterator, &lookup);
  HL_H5G_CLOSE(gid);

  if (lookup.reffound == 0)
    goto fail;

  return HLHDF_STRDUP(lookup.found_name);
fail:
  return NULL;
}

static int hlhdf_read_readVariableString(hid_t obj, hid_t type, hsize_t npoints,
  size_t* dSize, unsigned char** dataptr)
{
  int status = 0;
  hid_t space = H5Dget_space (obj);
  hsize_t     dims[1] = {1};
  char* rdata = NULL;

  int ndims = H5Sget_simple_extent_dims (space, dims, NULL);
  if (ndims <= 0) { /** SCALAR */
    if (H5Aread(obj, type, &rdata) < 0) {
      HL_ERROR0("Failed to read string");
      goto fail;
    }
    *dataptr = (unsigned char*)HLHDF_STRDUP(rdata);
    *dSize = strlen((const char*)*dataptr);
  } else {
    HL_ERROR0("Variable string length reading currently not supporting arrays.");
    goto fail;
  }

  status = 1;
fail:
  if (status == 0) {
    *dSize = 0;
    HLHDF_FREE(*dataptr);
  }
  HLHDF_FREE(rdata);
  HL_H5S_CLOSE(space);
  return status;
}

static int hlhdf_read_readAttributeData(hid_t obj, hid_t type, hsize_t npoints,
  size_t* dSize, unsigned char** dataptr)
{
  int status = 0;
  if (dSize == NULL || dataptr == NULL) {
    HL_ERROR0("Inparameters NULL");
    return 0;
  }

  *dSize = H5Tget_size(type);
  if(H5Tget_class(type) == H5T_STRING && H5Tis_variable_str(type) == 1) {
    if (hlhdf_read_readVariableString(obj, type, npoints, dSize, dataptr) < 0) {
      HL_ERROR0("Failed to read variable length string");
      goto fail;
    }
  } else {
    if (!(*dataptr = (unsigned char*) HLHDF_MALLOC((*dSize) * npoints))) {
      HL_ERROR0("Could not allocate memory for attribute data");
      goto fail;
    }
    if (H5Aread(obj, type, *dataptr) < 0) {
      HL_ERROR0("Could not read attribute data\n");
      goto fail;
    }
  }

  // If string has been stored with bad nullterm, fix it.
  if (H5Tget_class(type) == H5T_STRING && *dSize > 0) {
    if (H5Tget_strpad(type) == H5T_STR_NULLTERM) {
      if (((char*)*dataptr)[*dSize - 1] != '\0') {
        unsigned char* nptr = (unsigned char*) HLHDF_REALLOC(*dataptr, ((*dSize) * npoints) + 1);
        if (nptr != NULL) {
          *dataptr = nptr;
          ((unsigned char*)*dataptr)[*dSize] = '\0';
          *dSize = *dSize + 1;
        } else {
          HL_ERROR0("Could not reallocate attribute data\n");
          goto fail;
        }
      }
    }
  }
  status = 1;
fail:
  if (status == 0) {
    *dSize = 0;
    HLHDF_FREE(*dataptr);
  }
  return status;
}

/**
 * Fills the attribute with the data or the rawdata depending on rawdata-attribute
 * @param[in] node the node
 * @param[in] obj the attribute hid
 * @param[in] type the data type
 * @param[in] npoints the number of values
 * @param[in] rawdata if 0, data will be set, if 1 rawdata will be set.
 */
static int hlhdf_read_fillAttributeNodeWithData(HL_Node* node, hid_t obj, hid_t type, int npoints, int rawdata)
{
  size_t dSize = 0;
  unsigned char* dataptr = NULL;
  int status = 0;

  if (!hlhdf_read_readAttributeData(obj, type, npoints, &dSize, &dataptr)) {
    HL_ERROR0("Failed to read attribute data");
    goto fail;
  }

  if (!rawdata) {
    HLNodePrivate_setData(node, dSize, dataptr);
  } else {
    HLNodePrivate_setRawdata(node,dSize,dataptr);
  }
  dataptr = NULL;

  status = 1;
fail:
  HLHDF_FREE(dataptr);
  return status;
}

/**
 * Gets the space dimension information.
 * @param[in] spaceid the space identifier
 * @param[out] ndims the rank
 * @param[out] npoints the number of values
 * @param[out] the dimensions
 * @return 1 on success, 0 on failure and in that case dims is guaranteed to be NULL
 */
static int hlhdf_read_getSpaceDimensions(hid_t spaceid, int* ndims, hsize_t* npoints, hsize_t** dims)
{
  int status = 0;

  HL_ASSERT((ndims != NULL && npoints != NULL && dims != NULL), "Inparameters NULL");

  *ndims = H5Sget_simple_extent_ndims(spaceid);
  *npoints = H5Sget_simple_extent_npoints(spaceid);
  *dims = NULL;
  if (*ndims > 0) {
    *dims = (hsize_t*)HLHDF_MALLOC(sizeof(hsize_t)* (*ndims));
    if (H5Sget_simple_extent_dims(spaceid, *dims, NULL) != *ndims) {
      HL_ERROR0("Could not get dimensions from space");
      goto fail;
    }
  }
  status = 1;
fail:
  if (status == 0) {
    *ndims = 0;
    *npoints = 0;
    HLHDF_FREE(*dims);
  }
  return status;
}

/**
 * Fills an attribute with data
 */
static int fillAttributeNode(hid_t file_id, HL_Node* node)
{
  hid_t obj = -1;
  hid_t loc_id = -1;
  hid_t type = -1, mtype = -1;
  hid_t f_space = -1;
  char* parent = NULL;
  char* child = NULL;
  HL_Type parentType = UNDEFINED_ID;
  H5G_stat_t statbuf;
  int result = 0;

  HL_SPEWDEBUG0("ENTER: fillAttributeNode");

  if (!extractParentChildName(node, &parent, &child)) {
    HL_ERROR0("Failed to extract parent/child");
    goto fail;
  }

  if (!openGroupOrDataset(file_id, parent, &loc_id, &parentType)) {
    HL_ERROR1("Failed to determine and open '%s'", parent);
    goto fail;
  }

  if ((obj = H5Aopen_name(loc_id, child)) < 0) {
    goto fail;
  }

  if ((type = H5Aget_type(obj)) < 0) {
    HL_ERROR0("Could not get attribute type");
    goto fail;
  }

  if ((mtype = getFixedType(type)) < 0) {
    HL_ERROR0("Could not create fixed attribute type");
    goto fail;
  }

  if (H5Tget_class(mtype) == H5T_COMPOUND) {
    HL_CompoundTypeDescription* descr = buildTypeDescriptionFromTypeHid(mtype);
    if (descr == NULL) {
      HL_ERROR0("Failed to create compound data description for attribute");
      goto fail;
    }

    if (H5Tcommitted(type) > 0) {
      H5Gget_objinfo(type, ".", TRUE, &statbuf);
      descr->objno[0] = statbuf.objno[0];
      descr->objno[1] = statbuf.objno[1];
    }

    HLNode_setCompoundDescription(node, descr);
  }

  if ((f_space = H5Aget_space(obj)) >= 0) {
    hsize_t* all_dims = NULL;
    hsize_t npoints;
    int ndims;

    if (!hlhdf_read_getSpaceDimensions(f_space, &ndims, &npoints, &all_dims)) {
      HL_ERROR0("Could not read space dimensions");
      goto fail;
    } else {
      if (!HLNode_setDimensions(node, ndims, all_dims)) {
        HL_ERROR0("Failed to set node dimensions");
        HLHDF_FREE(all_dims);
        goto fail;
      }
      HLHDF_FREE(all_dims);
    }

    if (H5Sis_simple(f_space) >= 0) {
      if (!hlhdf_read_fillAttributeNodeWithData(node, obj, mtype, npoints, 0)) {
        HL_ERROR0("Failed to read fixed attribute data");
        goto fail;
      }
      if (!hlhdf_read_fillAttributeNodeWithData(node, obj, type, npoints, 1)) {
        HL_ERROR0("Failed to read fixed attribute data");
        goto fail;
      }
    } else {
      HL_ERROR0("Attribute dataspace was not simple, can't handle");
      goto fail;
    }
  } else {
    HL_ERROR0("Could not get dataspace for attribute");
    goto fail;
  }

  if (!HLNodePrivate_setTypeIdAndDeriveFormat(node, mtype)) {
    HL_ERROR0("Failed to set type and format on node");
    goto fail;
  }
  HLNode_setMark(node, NMARK_ORIGINAL);
  HLNode_setFetched(node, 1);

  result = 1;
fail:
  HL_H5A_CLOSE(obj);
  HL_H5O_CLOSE(loc_id);
  HL_H5T_CLOSE(type);
  HL_H5T_CLOSE(mtype);
  HL_H5S_CLOSE(f_space);
  HLHDF_FREE(parent);
  HLHDF_FREE(child);

  return result;
}

/**
 * Fills a reference node
 */
static int fillReferenceNode(hid_t file_id, HL_Node* node)
{
  HL_Type parentType = UNDEFINED_ID;
  hobj_ref_t ref;
  hid_t obj = -1;
  hid_t loc_id = -1;
  char* parent = NULL;
  char* child = NULL;
  char* refername = NULL;
  int status = 0;
  hid_t strtype = -1;

  HL_DEBUG0("ENTER: fillReferenceNode");
  if (!extractParentChildName(node, &parent, &child)) {
    HL_ERROR0("Failed to extract parent/child");
    goto fail;
  }

  if (!openGroupOrDataset(file_id, parent, &loc_id, &parentType)) {
    HL_ERROR1("Failed to determine and open '%s'", parent);
    goto fail;
  }

  if ((obj = H5Aopen_name(loc_id, child)) < 0) {
    goto fail;
  }
  if (H5Aread(obj, H5T_STD_REF_OBJ, &ref) < 0) {
    HL_ERROR0("Could not read reference\n");
    goto fail;
  }

  if (!(refername = locateNameForReference(file_id, &ref))) {
    HL_INFO2("WARNING: Could not locate name of object referenced by: %s/%s"
             " will set referenced object to UNKNOWN.", parent, child);
    refername = strdup("UNKNOWN");
  }

  HLNodePrivate_setData(node, strlen(refername)+1, (unsigned char*)HLHDF_STRDUP(refername));
  HLNodePrivate_setRawdata(node, strlen(refername)+1, (unsigned char*)HLHDF_STRDUP(refername));
  HLNode_setDimensions(node, 0, NULL);
  HLNode_setMark(node, NMARK_ORIGINAL);
  HLNode_setFetched(node, 1);

  strtype = H5Tcopy(H5T_C_S1);
  H5Tset_size(strtype, strlen(refername)+1);
  if(!HLNodePrivate_setTypeIdAndDeriveFormat(node, strtype)) {
    HL_ERROR0("Failed to set type and format");
    goto fail;
  }

  status = 1;
fail:
  HL_H5A_CLOSE(obj);
  HL_H5O_CLOSE(loc_id);
  HLHDF_FREE(parent);
  HLHDF_FREE(child);
  HLHDF_FREE(refername);
  HL_H5T_CLOSE(strtype);

  return status;
}

/**
 * Fills a dataset node
 */
static int fillDatasetNode(hid_t file_id, HL_Node* node)
{
  hid_t obj = -1;
  hid_t type = -1;
  H5G_stat_t statbuf;
  hid_t f_space = -1;
  hid_t mtype = -1;
  int status = 0;

  HL_DEBUG0("ENTER: fillDatasetNode");

  if ((obj = H5Dopen(file_id, HLNode_getName(node), H5P_DEFAULT)) < 0) {
    goto fail;
  }

  /* What datatype was this dataset stored as? */
  if ((type = H5Dget_type(obj)) < 0) {
    HL_ERROR0("Failed to get type from dataset");
    goto fail;
  }

  /* What size does the type have? */
  if ((f_space = H5Dget_space(obj)) > 0) { /*Get the space description for the dataset */
    hsize_t* all_dims = NULL;
    hsize_t npoints;
    int ndims;

    if (!hlhdf_read_getSpaceDimensions(f_space, &ndims, &npoints, &all_dims)) {
      HL_ERROR0("Could not read space dimensions");
      goto fail;
    } else {
      if (!HLNode_setDimensions(node, ndims, all_dims)) {
        HL_ERROR0("Failed to set node dimensions");
        HLHDF_FREE(all_dims);
        goto fail;
      }
      HLHDF_FREE(all_dims);
    }

    /* Translate the type into a native dataspace */
    mtype = getFixedType(type);

    if (H5Tget_class(mtype) == H5T_COMPOUND) {
      HL_CompoundTypeDescription* descr = buildTypeDescriptionFromTypeHid(mtype);
      if (descr == NULL) {
        HL_ERROR0("Failed to create compound data description for attribute");
        goto fail;
      }

      if (H5Tcommitted(type) > 0) {
        H5Gget_objinfo(type, ".", TRUE, &statbuf);
        descr->objno[0] = statbuf.objno[0];
        descr->objno[1] = statbuf.objno[1];
      }

      HLNode_setCompoundDescription(node, descr);
    }

    if(!HLNodePrivate_setTypeIdAndDeriveFormat(node, mtype)) {
      HL_ERROR0("Failed to set type and format");
      goto fail;
    }

    /* If we are fetching dataset meta, we need to leave after type has been set. */
    if (HLNode_getMark(node) == NMARK_SELECTMETA) {
      HLNode_setMark(node, NMARK_ORIGINAL);
      HL_H5D_CLOSE(obj);
      HL_H5T_CLOSE(type);
      HL_H5S_CLOSE(f_space);
      HL_H5T_CLOSE(mtype);
      return 1;
    }

    if (H5Sis_simple(f_space) >= 0) { /*Only allow simple dataspace, nothing else supported by HDF5 anyway */
      unsigned char* dataptr = NULL;
      size_t dSize = H5Tget_size(mtype);
      dataptr = (unsigned char*) HLHDF_MALLOC(dSize * npoints);
      if (dataptr == NULL) {
        HL_ERROR0("Failed to allocate memory for dataset arrray");
        goto fail;
      }
      H5Sselect_all(f_space);
      if (H5Dread(obj, mtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, dataptr) < 0) {
        HL_ERROR0("Failed to read dataset");
        HLHDF_FREE(dataptr);
        goto fail;
      }

      HLNodePrivate_setData(node, dSize, dataptr);
    } else {
      HL_ERROR0("Dataspace for dataset was not simple, this is not supported");
      goto fail;
    }
  } else {
    HL_ERROR0("Failure getting space description for dataset");
    goto fail;
  }

  /* Mark the node as original */
  HLNode_setMark(node, NMARK_ORIGINAL);
  HLNode_setFetched(node, 1);

  status = 1;
fail:
  HL_H5D_CLOSE(obj);
  HL_H5T_CLOSE(type);
  HL_H5S_CLOSE(f_space);
  HL_H5T_CLOSE(mtype);
  return status;
}

/**
 * Fills a group node
 */
static int fillGroupNode(hid_t file_id, HL_Node* node)
{
  hid_t obj;

  if ((obj = H5Gopen(file_id, HLNode_getName(node), H5P_DEFAULT)) < 0) {
    return 0;
  }

  HLNode_setMark(node, NMARK_ORIGINAL);
  HLNode_setFetched(node, 1);

  HL_H5G_CLOSE(obj);
  return 1;
}

/* ---------------------------------------
 * FILL_TYPE_NODE
 * --------------------------------------- */
static int fillTypeNode(hid_t file_id, HL_Node* node)
{
  hid_t obj = -1;
  HL_CompoundTypeDescription* typelist = NULL;
  H5G_stat_t statbuf;

  if ((obj = H5Topen(file_id, HLNode_getName(node), H5P_DEFAULT)) < 0) {
    HL_ERROR1("Failed to open %s ", HLNode_getName(node));
    return 0;
  }
  H5Gget_objinfo(obj, ".", TRUE, &statbuf);

  if (!(typelist = buildTypeDescriptionFromTypeHid(obj))) {
    HL_ERROR0("Failed to create datatype nodelist");
    goto fail;
  }
  strcpy(typelist->hltypename, HLNode_getName(node));
  typelist->objno[0] = statbuf.objno[0];
  typelist->objno[1] = statbuf.objno[1];

  HLNode_setCompoundDescription(node, typelist);
  typelist = NULL; /* Ownership transfered. */

  HLNode_setMark(node, NMARK_ORIGINAL);
  HLNode_setFetched(node, 1);

  HLNodePrivate_setHdfID(node, obj); /*Save hdfid for later use, which means that obj not should be closed now. */
  //@todo This causes the file not to be closed when atempting to update a file.
  return 1;
fail:
  HL_H5T_CLOSE(obj);
  freeHL_CompoundTypeDescription(typelist);
  return 0;
}

/**
 * Fills the node with the appropriate data.
 */
static int fillNodeWithData(hid_t file_id, HL_Node* node)
{
  HL_SPEWDEBUG0("ENTER: fillNodeWithData");
  switch (HLNode_getType(node)) {
  case ATTRIBUTE_ID:
    return fillAttributeNode(file_id, node);
  case DATASET_ID:
    return fillDatasetNode(file_id, node);
  case GROUP_ID:
    return fillGroupNode(file_id, node);
  case TYPE_ID:
    return fillTypeNode(file_id, node);
  case REFERENCE_ID:
    return fillReferenceNode(file_id, node);
  default:
    HL_ERROR1("Can't handle other nodetypes but '%d'",HLNode_getName(node));
    break;
  }
  return 0;
}

/**
 * Creates an absolute path from <b>root</b> and <b>name</b> parts.
 * @param[in] root - the root path
 * @param[in] name - the name
 * @return a path in the form <b>root/name</b>
 */
static char* hlhdf_read_createPath(const char* root, const char* name)
{
  char* newpath = NULL;
  int status = FALSE;
  int len = 0;

  if (root == NULL || name == NULL) {
    HL_ERROR0("hlhdf_read_createPath: arguments NULL");
    goto fail;
  }

  newpath = HLHDF_MALLOC(sizeof(char)*(strlen(root) + strlen(name) + 2));
  if (newpath == NULL) {
    HL_ERROR0("Failed to allocate memory\n");
    goto fail;
  }

  if (strcmp(".", root) == 0 || strcmp("/", root) == 0) {
    strcpy(newpath, "");
  } else {
    int i = 0;
    sprintf(newpath, "%s", root);
    i = strlen(newpath) - 1;
    while (i>1 && newpath[i] == '/') {
      newpath[i] = '\0';
      i--;
    }
  }

  if (strcmp(name, ".") == 0) {
    int i = strlen(newpath);
    sprintf(&newpath[i], "/");
  } else {
    int i = strlen(newpath);
    sprintf(&newpath[i], "/%s", name);
  }

  len = strlen(newpath);
  if (len > 1 && newpath[len-1]=='/') {
    newpath[len-1]='\0';
  }

  status = TRUE;
fail:
  if (status == FALSE) {
    HLHDF_FREE(newpath);
  }
  return newpath;
}

/**
 * Called by H5Aiterate_by_name when iterating over all attributes in a group.
 * @param[in] location_id - the root group from where the iterator started
 * @param[in] name - the name of the attribute
 * @param[in] ainfo - the attribute info
 * @param[in] op_data - the \ref VisitorStruct
 * @return -1 on failure, otherwise 0.
 */
static herr_t hlhdf_node_attribute_visitor(hid_t location_id, const char *name, const H5A_info_t *ainfo, void *op_data)
{
  VisitorStruct* vsp = (VisitorStruct*)op_data;
  herr_t status = -1;
  char* path = hlhdf_read_createPath(vsp->path, name);
  hid_t attrid = -1;
  hid_t typeid = -1;

  if (path == NULL) {
    HL_ERROR0("Could not create path");
    goto fail;
  }

  if ((attrid = H5Aopen(location_id, name, H5P_DEFAULT))<0) {
    HL_ERROR1("Could not open attribute: %s", name);
    goto fail;
  }

  if ((typeid = H5Aget_type(attrid)) < 0) {
    HL_ERROR1("Could not get type for %s", name);
    goto fail;
  }

  if (H5Tget_class(typeid) == H5T_REFERENCE) {
    HLNodeList_addNode(vsp->nodelist, HLNode_newReference(path));
  } else {
    HLNodeList_addNode(vsp->nodelist, HLNode_newAttribute(path));
  }

  status = 0;
fail:
  HL_H5A_CLOSE(attrid);
  HL_H5T_CLOSE(typeid);
  HLHDF_FREE(path);
  return status;
}

/**
 * Called by H5Ovisit_by_name when iterating over all group/dataasets.
 * @param[in] location_id - the root group from where the iterator started
 * @param[in] name - the name of the attribute
 * @param[in] info - the object info
 * @param[in] op_data - the \ref VisitorStruct
 * @return -1 on failure, otherwise 0.
 */
static herr_t hlhdf_node_visitor(hid_t g_id, const char *name, const H5O_info_t *info, void *op_data)
{
  VisitorStruct* vsp = (VisitorStruct*)op_data;
  VisitorStruct vs;
  herr_t status = -1;
  char* path = hlhdf_read_createPath(vsp->path, name);

  if (path == NULL) {
    HL_ERROR0("Could not create path");
    goto fail;
  }

  vs.nodelist = vsp->nodelist;
  vs.path = path;
  switch (info->type) {
  case H5O_TYPE_GROUP: {
    hsize_t n=0;
    // The visitor also visits the root-node but that is not a valid
    // node to write since it always should exist.
    if (strcmp("/", path) != 0) {
      HLNodeList_addNode(vsp->nodelist, HLNode_newGroup(vs.path));
    }
    if (H5Aiterate_by_name(g_id, name, H5_INDEX_NAME, H5_ITER_INC, &n, hlhdf_node_attribute_visitor, &vs, H5P_DEFAULT) < 0) {
      HL_ERROR1("Failed to iterate over %s", vs.path);
      goto fail;
    }
    break;
  }
  case H5O_TYPE_DATASET: {
    hsize_t n=0;
    HLNodeList_addNode(vsp->nodelist, HLNode_newDataset(vs.path));
    if (H5Aiterate_by_name(g_id,  name, H5_INDEX_NAME, H5_ITER_INC, &n, hlhdf_node_attribute_visitor, &vs, H5P_DEFAULT) < 0) {
      HL_ERROR1("Failed to iterate over %s", vs.path);
      goto fail;
    }
    break;
  }
  case H5O_TYPE_NAMED_DATATYPE: {
    HLNodeList_addNode(vsp->nodelist, HLNode_newDatatype(vs.path));
    break;
  }
  default: {
    HL_printf("(%ld) UNKNOWN: %s\n", g_id, name);
    break;
  }
  }
  status = 0;
fail:
  HLHDF_FREE(path);
  return status;
}

/*@} End of Private functions */

/*@{ Interface functions */
HL_NodeList* HLNodeList_readFrom(const char* filename, const char* fromPath)
{
  hid_t file_id = -1, gid = -1;
  HL_NodeList* retv = NULL;
  VisitorStruct vs;
  H5O_info_t objectInfo;

  HL_DEBUG0("ENTER: readHL_NodeListFrom");

  if (fromPath == NULL) {
    HL_ERROR0("fromPath == NULL");
    goto fail;
  }

  if ((file_id = openHlHdfFile(filename, "r")) < 0) {
    HL_ERROR1("Failed to open file %s",filename);
    goto fail;
  }
#ifdef USE_HDF5_1_12_API    
  if (H5Oget_info_by_name(file_id, fromPath, &objectInfo, H5O_INFO_ALL, H5P_DEFAULT)<0) {
#else
  if (H5Oget_info_by_name(file_id, fromPath, &objectInfo, H5P_DEFAULT)<0) {
#endif  
    HL_ERROR0("fromPath needs to be a dataset or group when opening a file.");
    goto fail;
  }

  if (!(retv = HLNodeList_new())) {
    HL_ERROR0("Could not allocate NodeList\n");
    goto fail;
  }
  HLNodeList_setFileName(retv, filename);

  vs.path = (char*)fromPath;
  vs.nodelist = retv;

#ifdef USE_HDF5_1_12_API 
  if (H5Ovisit_by_name(file_id, fromPath, H5_INDEX_NAME, H5_ITER_INC, hlhdf_node_visitor, &vs, H5O_INFO_ALL, H5P_DEFAULT)<0) {
#else
  if (H5Ovisit_by_name(file_id, fromPath, H5_INDEX_NAME, H5_ITER_INC, hlhdf_node_visitor, &vs, H5P_DEFAULT)<0) {
#endif  
    HL_ERROR0("Could not iterate over file");
    goto fail;
  }

  HLNodeList_markNodes(retv, NMARK_ORIGINAL);

  HL_H5F_CLOSE(file_id);
  HL_H5G_CLOSE(gid);
  HL_DEBUG0("EXIT: readHL_NodeListFrom ");
  return retv;

fail:
  HL_H5F_CLOSE(file_id);
  HL_H5G_CLOSE(gid);
  HLNodeList_free(retv);
  HL_DEBUG0("EXIT: readHL_NodeListFrom with Error");
  return NULL;
}

/* ---------------------------------------
 * READ_HL_NODE_LIST
 * --------------------------------------- */
HL_NodeList* HLNodeList_read(const char* filename)
{
  HL_NodeList* retv = NULL;
  HL_DEBUG0("ENTER: readHL_NodeList");

  retv = HLNodeList_readFrom(filename, ".");

  HL_DEBUG0("EXIT: readHL_NodeList");
  return retv;
}

/* ---------------------------------------
 * SELECT_NODE
 * --------------------------------------- */
int HLNodeList_selectNode(HL_NodeList* nodelist, const char* name)
{
  HL_Node* node = NULL;

  HL_DEBUG0("ENTER: selectNode");
  if (!name) {
    HL_ERROR0("Can not select any node when name is NULL");
    return 0;
  }

  if ((node = HLNodeList_getNodeByName(nodelist, name)) != NULL) {
    HLNode_setMark(node, NMARK_SELECT);
    return 1;
  }

  HL_ERROR1("Could not find any node called '%s'",name);
  return 0;
}

/* ---------------------------------------
 * SELECT_ALL_NODES
 * --------------------------------------- */
int HLNodeList_selectAllNodes(HL_NodeList* nodelist)
{
  HLNodeList_markNodes(nodelist, NMARK_SELECT);

  return 1;
}

/* ---------------------------------------
 * SELECT_METADATA_NODES
 * VOLATILE: Do not attempt to access dataset arrays after calling this.
 * --------------------------------------- */
int HLNodeList_selectMetadataNodes(HL_NodeList* nodelist)
{
  int i = 0;
  int nNodes = -1;
  HL_DEBUG0("ENTER: selectMetadataNodes");
  nNodes = HLNodeList_getNumberOfNodes(nodelist);
  for (i = 0; i < nNodes; i++) {
    HL_Node* node = HLNodeList_getNodeByIndex(nodelist, i);
    if (HLNode_getType(node) != DATASET_ID && HLNode_getDataType(node) != HL_ARRAY) {
      HLNode_setMark(node, NMARK_SELECT);
    }
  }

  return 1;
}

/* ---------------------------------------
 * SELECT_ALL NODES EXCEPT DATASETS DATA
 * VOLATILE: Do not attempt to access dataset arrays after calling this.
 * --------------------------------------- */
int HLNodeList_selectAllMetadataNodes(HL_NodeList* nodelist)
{
  int i = 0;
  int nNodes = -1;
  HL_DEBUG0("ENTER: selectAllNodesExceptDatasets");
  nNodes = HLNodeList_getNumberOfNodes(nodelist);
  for (i = 0; i < nNodes; i++) {
    HL_Node* node = HLNodeList_getNodeByIndex(nodelist, i);
    if (HLNode_getType(node) != DATASET_ID) {
      HLNode_setMark(node, NMARK_SELECT);
    } else {
      HLNode_setMark(node, NMARK_SELECTMETA);
    }
  }

  return 1;
}

/* ---------------------------------------
 * SELECT_ALL DATASET NODES
 * VOLATILE: Do not attempt to access anything but dataset arrays after calling this.
 * --------------------------------------- */
int HLNodeList_selectOnlyDatasetNodes(HL_NodeList* nodelist)
{
  int i = 0;
  int nNodes = -1;
  HL_DEBUG0("ENTER: selectAllNodesExceptDatasets");
  nNodes = HLNodeList_getNumberOfNodes(nodelist);
  for (i = 0; i < nNodes; i++) {
    HL_Node* node = HLNodeList_getNodeByIndex(nodelist, i);
    if (HLNode_getType(node) == DATASET_ID) {
      HLNode_setMark(node, NMARK_SELECT);
    }
  }

  return 1;
}

/* ---------------------------------------
 * DE-SELECT_NODE
 * --------------------------------------- */
int HLNodeList_deselectNode(HL_NodeList* nodelist, const char* name)
{
  HL_Node* node = NULL;

  HL_DEBUG0("ENTER: deselectNode");
  if (!name) {
    HL_ERROR0("Can not deselect any node when name is NULL");
    return 0;
  }

  node = HLNodeList_getNodeByName(nodelist, name);
  if (node != NULL) {
    HLNode_setMark(node, NMARK_ORIGINAL);
    return 1;
  }

  HL_ERROR1("Could not find any node called '%s'",name);
  return 0;
}

/* ---------------------------------------
 * FETCH_MARKED_NODES
 * --------------------------------------- */
int HLNodeList_fetchMarkedNodes(HL_NodeList* nodelist)
{
  int i;
  hid_t file_id = -1;
  hid_t gid = -1;
  char* filename = NULL;
  int nNodes = 0;
  int result = 0;

  HL_DEBUG0("ENTER: fetchMarkedNodes");
  if (nodelist == NULL) {
    HL_ERROR0("Inparameters NULL");
    goto fail;
  }

  if ((filename = HLNodeList_getFileName(nodelist)) == NULL) {
    HL_ERROR0("Could not get filename from nodelist");
    goto fail;
  }

  if ((file_id = openHlHdfFile(filename, "r")) < 0) {
    HL_ERROR1("Could not open file '%s' when fetching data",filename);
    goto fail;
  }

  if ((gid = H5Gopen(file_id, ".", H5P_DEFAULT)) < 0) {
    HL_ERROR0("Could not open root group\n");
    goto fail;
  }

  if ((nNodes =  HLNodeList_getNumberOfNodes(nodelist)) < 0) {
    HL_ERROR0("Failed to get number of nodes");
    goto fail;
  }

  for (i = 0; i < nNodes; i++) {
    HL_Node* node = NULL;
    if ((node = HLNodeList_getNodeByIndex(nodelist, i)) == NULL) {
      HL_ERROR1("Error occured when fetching node at index %d", i);
      goto fail;
    }
    if (HLNode_getMark(node) == NMARK_SELECT || HLNode_getMark(node) == NMARK_SELECTMETA) {
      if (!fillNodeWithData(file_id, node)) {
        HL_ERROR1("Error occured when trying to fill node '%s'",HLNode_getName(node));
        goto fail;
      }
    }
  }
  result = 1;
fail:
  HL_H5F_CLOSE(file_id);
  HL_H5G_CLOSE(gid);
  HLHDF_FREE(filename);
  HL_DEBUG1("EXIT: fetchMarkedNodes with status = %d", result);
  return result;
}

/* ---------------------------------------
 * FETCH_NODE
 * --------------------------------------- */
HL_Node* HLNodeList_fetchNode(HL_NodeList* nodelist, const char* name)
{
  hid_t file_id = -1;
  HL_Node* result = NULL;
  HL_Node* foundnode = NULL;
  char* filename = NULL;

  HL_DEBUG0("ENTER: fetchNode");
  if (name == NULL || nodelist == NULL) {
    HL_ERROR0("Inparameters NULL");
    goto fail;
  }
  if ((filename = HLNodeList_getFileName(nodelist)) == NULL) {
    HL_ERROR0("Could not get filename from nodelist");
    goto fail;
  }

  if ((foundnode = HLNodeList_getNodeByName(nodelist, name))==NULL) {
    HL_ERROR1("No node: '%s' found", name);
    goto fail;
  }

  if ((file_id = openHlHdfFile(filename, "r")) < 0) {
    HL_ERROR1("Could not open file '%s' when fetching data",filename);
    goto fail;
  }

  if (!fillNodeWithData(file_id, foundnode)) {
    HL_ERROR1("Error occured when trying to fill node '%s'", name);
    goto fail;
  }

  result = foundnode;
fail:
  HL_H5F_CLOSE(file_id);
  HLHDF_FREE(filename);
  HL_DEBUG0("EXIT: fetchNode");
  return result;
}
/*@} End of Interface functions */


