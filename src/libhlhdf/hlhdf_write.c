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
 * Functions for writing and updating HDF5 files through the HLHDF-api.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-06-12
 */
#include "hlhdf.h"
#include "hlhdf_alloc.h"
#include "hlhdf_node_private.h"
#include "hlhdf_debug.h"
#include "hlhdf_private.h"
#include "hlhdf_defines_private.h"
#include <stdlib.h>
#include <string.h>

/*@{ Private functions */
/**
 * Turns a self defined type into a named type, i.e. gives it a name.
 * @param[in] loc_id  In what group or file, this type this should be placed.
 * @param[in] name  The name of this type
 * @param[in] type_id The reference to the self defined type.
 * @return <0 on failure, otherwise success.
 */
static herr_t commitType(hid_t loc_id, const char* name, hid_t type_id)
{
  HL_DEBUG0("ENTER: commitType");
  return H5Tcommit(loc_id, name, type_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
}


/**
 * Creates a reference from loc_id, name to the targetname.
 * @param[in] loc_id  The place the reference should be placed
 * @param[in] file_id The file id.
 * @param[in] name  The name of the reference
 * @param[in] targetname  Where the reference points.
 * Returns: <0 if error, otherwise ok.
 */
static herr_t createReference(hid_t loc_id, hid_t file_id, const char* name,
  const char* targetname)
{
  hid_t aid = -1;
  hid_t attr_id = -1;
  hid_t attr_type = -1;
  hobj_ref_t ref;
  herr_t status = -1;
  HL_DEBUG0("ENTER: createReference");
  if ((aid = H5Screate(H5S_SCALAR)) < 0) {
    HL_ERROR0("Failed to create scalar data space");
    goto fail;
  }

  if ((attr_type = H5Tcopy(H5T_STD_REF_OBJ)) < 0) {
    HL_ERROR0("Failed to copy H5T reference type");
    goto fail;
  }

  if ((attr_id = H5Acreate(loc_id, name, attr_type, aid, H5P_DEFAULT,
                           H5P_DEFAULT)) < 0) {
    HL_ERROR0("Failed to create scalar attribute");
    goto fail;
  }

  /* Of course I assume that the dataset is accurately specified
   * by targetname, otherwise well, implement a check here, will
   * probably require that the in argument is the actual dataset id :-/
   */
  if (H5Rcreate(&ref, file_id, targetname, H5R_OBJECT, -1) < 0) {
    HL_ERROR0("Failed to create reference object");
    goto fail;
  }

  if (H5Awrite(attr_id, attr_type, &ref) < 0) {
    HL_ERROR0("Failed to write scalar data to file");
    goto fail;
  }

  status = 0;

fail:
  HL_H5S_CLOSE(aid);
  HL_H5A_CLOSE(attr_id);
  HL_H5T_CLOSE(attr_type);
  return status;
}

/**
 * Writes a scalar attribute to the file.
 * @param[in] loc_id In what group or file or dataset this attribute should be written.
 * @param[in] type_id The type of what should be written.
 * @param[in] name The name of the attribute
 * @param[in] buf A pointer to the data that should be written.
 * @return <0 on failure, otherwise success
 */
static herr_t writeScalarDataAttribute(hid_t loc_id, hid_t type_id,
  const char* name, const void* buf)
{
  hid_t aid = -1;
  hid_t attr_id = -1;
  herr_t status = -1;
  HL_SPEWDEBUG0("ENTER: writeScalarDataAttribute");
  if ((aid = H5Screate(H5S_SCALAR)) < 0) {
    HL_ERROR0("Failed to create scalar data space");
    goto fail;
  }

  if ((attr_id = H5Acreate(loc_id, name, type_id, aid, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
    HL_ERROR0("Failed to create scalar attribute");
    goto fail;
  }

  if (H5Awrite(attr_id, type_id, buf) < 0) {
    HL_ERROR0("Failed to write scalar data to file");
    goto fail;
  }

  status = 0;
fail:
  HL_H5S_CLOSE(aid);
  HL_H5A_CLOSE(attr_id);
  return status;
}

/**
 * Writes a ndims-dimensional array to disk with the dimensions as specified in dims[0] .. dims[ndims-1].
 * @param[in] loc_id The location, the data should be written.
 * @param[in] type_id The type of the data that should be written
 * @param[in] name  The name of the attribute
 * @param[in] ndims The rank of the data
 * @param[in] dims  The dimensions of the data
 * @param[in] buf The data
 * @return <0 is failure, otherwise ok.
 */
static herr_t writeSimpleDataAttribute(hid_t loc_id, hid_t type_id, const char* name,
  int ndims, const hsize_t* dims, const void* buf)
{
  hid_t attr_id = -1;
  hid_t dataspace = -1;
  herr_t status = -1;

  HL_DEBUG0("ENTER: writeSimpleDataAttribute");
  if ((dataspace = H5Screate_simple(ndims, dims, NULL)) < 0) {
    HL_ERROR0("Failed to create simple dataspace for attribute");
    goto fail;
  }

  if ((attr_id = H5Acreate(loc_id, name, type_id, dataspace, H5P_DEFAULT,
                           H5P_DEFAULT)) < 0) {
    HL_ERROR0("Failed to create simple attribute");
    goto fail;
  }

  if (H5Awrite(attr_id, type_id, buf) < 0) {
    HL_ERROR0("Failed to write simple data attribute to file");
    goto fail;
  }
  status = 0;

fail:
  HL_H5S_CLOSE(dataspace);
  HL_H5A_CLOSE(attr_id);
  return status;
}

/**
 * Creates a simple dataset and if buf != NULL, the dataset will get the data filled in.
 * @param[in] loc_id  The location the dataset should be created in
 * @param[in] type_id The type of the data
 * @param[in] ndims The rank of the data
 * @param[in] dims  The dimensions of the data
 * @param[in] buf The data
 * @param[in] compress  The compression that should be used.
 * @return <0 on failure, otherwise success.
 */
static hid_t createSimpleDataset(hid_t loc_id, hid_t type_id, const char* name,
  int ndims, const hsize_t* dims, const void* buf, HL_Compression* compress)
{
  hid_t dataset = -1;
  hid_t dataspace = -1;
  hid_t props = -1;

  HL_SPEWDEBUG0("ENTER: createSimpleDataset");

  if ((dataspace = H5Screate_simple(ndims, dims, NULL)) < 0) {
    HL_ERROR0("Failed to create simple dataspace for dataset");
    goto done;
  }

  if (compress != NULL &&
      (compress->type == CT_SZLIB || (compress->type = CT_ZLIB && (compress->level > 0 && compress->level <= 9)))) {
    if ((props = H5Pcreate(H5P_DATASET_CREATE)) < 0) {
      HL_ERROR0("Failed to create the compression property");
      goto done;
    }

    if (H5Pset_chunk(props, ndims, dims) < 0) {
      HL_ERROR0("Failed to set chunk size");
      goto done;
    }
    if (compress->type == CT_ZLIB) {
      if (H5Pset_deflate(props, compress->level) < 0) {
        HL_ERROR1("Failed to set z compression to level %d", compress->level);
        goto done;
      }
    } else {
      if (H5Pset_szip(props, compress->szlib_mask, compress->szlib_px_per_block) < 0) {
        HL_ERROR2("Failed to set the szip compression, mask=%d, px_per_block=%d",
                  compress->szlib_mask, compress->szlib_px_per_block);
        goto done;
      }
    }

    if ((dataset = H5Dcreate(loc_id, name, type_id, dataspace, H5P_DEFAULT,
                             props, H5P_DEFAULT)) < 0) {
      HL_ERROR0("Failed to create the dataset");
      goto done;
    }
  } else {
    if ((dataset = H5Dcreate(loc_id, name, type_id, dataspace, H5P_DEFAULT,
                             H5P_DEFAULT, H5P_DEFAULT)) < 0) {
      HL_ERROR0("Failed to create the dataset");
      goto done;
    }
  }

  if (buf != NULL) {
    if (H5Dwrite(dataset, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf) < 0) {
      HL_ERROR0("Failed to write dataset");
      goto done;
    }
  }

done:
  HL_H5S_CLOSE(dataspace);
  HL_H5P_CLOSE(props);

  return dataset;
}

/**
 * Writes a HDF5 attribute.
 * @brief Writes a HDF5 attribute
 * @param[in] rootGrp The root group of the file
 * @param[in] parentNode - The parent node to the attribute that should be written
 * @param[in] parentName - The name of the parent node
 * @param[in] childNode - The node that should be written
 * @param[in] childName - The attributes name
 * @return 1 upon success, otherwise failure.
 */
static int doWriteHdf5Attribute(hid_t rootGrp, HL_Node* parentNode, char* parentName,
  HL_Node* childNode, char* childName)
{
  hid_t tmpLocId = -1;
  HL_SPEWDEBUG0("ENTER: doWriteHdf5Attribute");
  if (!parentName || !childName) {
    HL_ERROR0("Can't write HDF5 group since either parentName or childName is NULL");
    return 0;
  }

  if (strcmp(parentName, "") == 0) {
    tmpLocId = rootGrp;
  } else {
    tmpLocId = HLNodePrivate_getHdfID(parentNode);
  }

  if (HLNode_getRank(childNode) == 0) {
    if (writeScalarDataAttribute(tmpLocId,
                                 HLNodePrivate_getTypeId(childNode),
                                 childName,
                                 HLNode_getData(childNode)) < 0) {
      HL_ERROR1("Failed to write scalar data attribute '%s'",HLNode_getName(childNode));
      return 0;
    }
  } else {
    if (writeSimpleDataAttribute(tmpLocId,
                                 HLNodePrivate_getTypeId(childNode),
                                 childName,
                                 HLNode_getRank(childNode),
                                 HLNodePrivate_getDims(childNode),
                                 HLNode_getData(childNode)) < 0) {
      HL_ERROR1("Failed to write simple data attribute '%s'",HLNode_getName(childNode));
      return 0;
    }
  }

  return 1;
}

/**
 * Writes a HDF5 group.
 * @brief Writes a HDF5 group.
 * @param[in] rootGrp - The root group of the file
 * @param[in] parentNode - The parent node to the group that should be written
 * @param[in] parentName - The name of the parent node
 * @param[in] childNode - The node that should be written
 * @param[in] childName - The groups name
 * @return 1 upon success, otherwise failure.
 */
static int doWriteHdf5Group(hid_t rootGrp, HL_Node* parentNode, char* parentName,
  HL_Node* childNode, char* childName)
{
  HL_SPEWDEBUG0("ENTER: doWriteHdf5group");
  hid_t hdfid = -1;

  if (!parentName || !childName) {
    HL_ERROR0("Can't write HDF5 group since either parentName or childName is NULL");
    return 0;
  }

  if (strcmp(parentName, "") == 0) {
    hdfid = H5Gcreate(rootGrp, childName,H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  } else {
    hdfid = H5Gcreate(HLNodePrivate_getHdfID(parentNode), childName,H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  }

  if (hdfid < 0) {
    HL_ERROR1("Failed to create group %s",HLNode_getName(childNode));
    return 0;
  }
  HLNodePrivate_setHdfID(childNode, hdfid);
  return 1;
}

/**
 * Writes a HDF5 data set.
 * @brief Writes a HDF5 data set.
 * @param[in] rootGrp - The root group of the file
 * @param[in] parentNode - The parent node to the dataset that should be written
 * @param[in] parentName - The name of the parent node
 * @param[in] childNode - The node that should be written
 * @param[in] childName - The datasets name
 * @param[in] compression - the compression to be used
 * @return 1 upon success, otherwise failure.
 */
static int doWriteHdf5Dataset(hid_t rootGrp, HL_Node* parentNode, char* parentName,
  HL_Node* childNode, char* childName, HL_Compression* compression)
{
  hid_t tmpLocId = -1;
  hid_t hdfid = -1;

  HL_SPEWDEBUG0("ENTER: doWriteHdf5HLDataset");
  if (!parentName || !childName) {
    HL_ERROR0("Can't write HDF5 dataset since either parentName or childName is NULL");
    return 0;
  }
  if (strcmp(parentName, "") == 0) {
    tmpLocId = rootGrp;
  } else {
    tmpLocId = HLNodePrivate_getHdfID(parentNode);
  }

  hdfid = createSimpleDataset(tmpLocId,
                              HLNodePrivate_getTypeId(childNode),
                              childName,
                              HLNode_getRank(childNode),
                              HLNodePrivate_getDims(childNode),
                              HLNode_getData(childNode),
                              compression);
  if (hdfid < 0) {
    HL_ERROR1("Failed to create dataset %s",HLNode_getName(childNode));
    return 0;
  }
  HLNodePrivate_setHdfID(childNode, hdfid);

  return 1;
}

/**
 * Writes a named HDF5 datatype.
 * @brief Writes a named HDF5 data type.
 * @param[in] loc_id - The root group of the file
 * @param[in] parentNode - The parent node to the type that should be written
 * @param[in] parentName - The name of the parent node
 * @param[in] childNode - The node that should be written
 * @param[in] childName - The types name
 * @return 1 upon success, otherwise failure.
 */
static int doWriteHdf5Datatype(hid_t loc_id, HL_Node* parentNode, char* parentName,
  HL_Node* childNode, char* childName)
{
  HL_DEBUG0("ENTER: doCommitHdf5Datatype");
  if (loc_id < 0) {
    HL_ERROR0("Trying to create a committed datatype without setting typeid");
    return 0;
  }

  if ((commitType(loc_id, HLNode_getName(childNode), HLNodePrivate_getHdfID(childNode))) < 0)
    return 0;

  if (H5Tcommitted(HLNodePrivate_getHdfID(childNode)) <= 0) {
    HL_ERROR1("Failed to commit datatype'%s'",HLNode_getName(childNode));
  }

  return 1;
}

/**
 * Writes a HDF5 reference.
 * @brief Writes a HDF5 reference.
 * @param[in] rootGrp - The root group of the file
 * @param[in] file_id - The file pointer that we are working with
 * @param[in] parentNode - The parent node to the reference that should be written
 * @param[in] parentName - The name of the parent node
 * @param[in] childNode - The node that should be written
 * @param[in] childName - The references name
 * @return 1 upon success, otherwise failure.
 */
static int doWriteHdf5Reference(hid_t rootGrp, hid_t file_id, HL_Node* parentNode,
  char* parentName, HL_Node* childNode, char* childName)
{
  hid_t tmpLocId = -1;
  HL_DEBUG0("ENTER: doWriteHdf5Reference");
  if (!parentName || !childName) {
    HL_ERROR0("Can't write HDF5 dataset since either parentName or childName is NULL");
    return 0;
  }
  if (strcmp(parentName, "") == 0) {
    tmpLocId = rootGrp;
  } else {
    tmpLocId = HLNodePrivate_getHdfID(parentNode);
  }
  if (createReference(tmpLocId, file_id, childName, (char*) HLNode_getData(childNode)) < 0) {
    HL_ERROR3("Failed to create reference from '%s/%s' to '%s'",
        parentName,childName, (char*)HLNode_getData(childNode));
    return 0;
  }
  return 1;
}

/**
 * Appends an ATTRIBUTE node to the datastructure. The parentNode can be either
 * of type GROUP or DATASET.
 * @param[in] file_id The file reference
 * @param[in] parentNode The parent node of the datatype to be written.
 * @param[in] parentName The name of the parent node.
 * @param[in] childNode The node to be written.
 * @param[in] childName the attributes name.
 * @return 1 on success, otherwise 0
 */
static int doAppendHdf5Attribute(hid_t file_id, HL_Node* parentNode, char* parentName,
  HL_Node* childNode, char* childName)
{
  hid_t loc_id = -1;
  int status = 0;

  HL_Type parentType = UNDEFINED_ID;

  if (!parentName || !childName) {
    HL_ERROR0("Can't write HDF5 attribute since either parentName or childName is NULL\n");
    return 0;
  }

  if (!openGroupOrDataset(file_id, parentName, &loc_id, &parentType)) {
    HL_ERROR1("Failed to determine and open '%s'", parentName);
    goto fail;
  }

  if (HLNode_getRank(childNode) == 0) {
    if (writeScalarDataAttribute(loc_id,
                                 HLNodePrivate_getTypeId(childNode),
                                 childName,
                                 HLNode_getData(childNode)) < 0) {
      HL_ERROR1("Failed to write scalar data attribute '%s'\n",
                HLNode_getName(childNode));
      goto fail;
    }
  } else {
    if (writeSimpleDataAttribute(loc_id,
                                 HLNodePrivate_getTypeId(childNode),
                                 childName,
                                 HLNode_getRank(childNode),
                                 HLNodePrivate_getDims(childNode),
                                 HLNode_getData(childNode)) < 0) {
      HL_ERROR1("Failed to write simple data attribute '%s'\n",
                HLNode_getName(childNode));
      goto fail;
    }
  }
  status = 1;

  HLNode_setMark(childNode, NMARK_ORIGINAL);

fail:
  HL_H5O_CLOSE(loc_id);

  return status;
}

/**
 * Appends a new group node to the structure. The parentNode can only
 * be of type GROUP.
 * @param[in] file_id The file reference.
 * @param[in] parentNode The parent node of the group to be written.
 * @param[in] parentName The name of the parent node.
 * @param[in] childNode The node to be written.
 * @param[in] childName The groups name.
 * @return 1 upon success, otherwise 0.
 */
static int doAppendHdf5Group(hid_t file_id, HL_Node* parentNode, char* parentName,
  HL_Node* childNode, char* childName)
{
  hid_t loc_id = -1;
  hid_t new_id = -1;
  int status = 0;

  if (!parentName || !childName) {
    HL_ERROR0("Can't write HDF5 group since either parentName or childName is NULL\n");
    return 0;
  }
  if (strcmp(parentName, "") == 0) {
    if ((loc_id = H5Gopen(file_id, "/", H5P_DEFAULT)) < 0) {
      HL_ERROR1("Could not open root group when reading attr '%s'\n",
                childName);
      goto fail;
    }
  } else {
    if ((loc_id = H5Gopen(file_id, parentName, H5P_DEFAULT)) < 0) {
      HL_ERROR1("Could not open group '%s' when creating new group.\n",
                parentName);
      goto fail;
    }
  }

  if ((new_id = H5Gcreate(loc_id, childName, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
    HL_ERROR1("Failed to create new group %s\n", HLNode_getName(childNode));
    goto fail;
  }
  status = 1;
  HLNode_setMark(childNode, NMARK_ORIGINAL);
fail:
  HL_H5G_CLOSE(loc_id);
  HL_H5G_CLOSE(new_id);
  return status;
}

/**
 * Appends a new group node to the structure. The parentNode can only
 * be of type GROUP.
 * @param[in] file_id The file reference.
 * @param[in] parentNode The parent node of the dataset to be written.
 * @param[in] parentName The name of the parent node.
 * @param[in] childNode The node to be written.
 * @param[in] childName The datasets name.
 * @param[in] compression The compression level that is wanted.
 * @return 1 upon success, otherwise 0.
 */
static int doAppendHdf5Dataset(hid_t file_id, HL_Node* parentNode, char* parentName,
  HL_Node* childNode, char* childName, HL_Compression* compression)
{
  hid_t loc_id = -1;
  hid_t new_id = -1;
  int status = 0;

  if (!parentName || !childName) {
    HL_ERROR0("Can't write HDF5 dataset since either parentName or childName is NULL\n");
    return 0;
  }
  if (strcmp(parentName, "") == 0) {
    if ((loc_id = H5Gopen(file_id, "/", H5P_DEFAULT)) < 0) {
      HL_ERROR1("Could not open root group when creating new dataset '%s'\n",
                childName);
      goto fail;
    }
  } else {
    if ((loc_id = H5Gopen(file_id, parentName, H5P_DEFAULT)) < 0) {
      HL_ERROR1("Could not open group '%s' when creating new dataset.\n",
                parentName);
      goto fail;
    }
  }

  new_id = createSimpleDataset(loc_id,
                               HLNodePrivate_getTypeId(childNode),
                               childName,
                               HLNode_getRank(childNode),
                               HLNodePrivate_getDims(childNode),
                               HLNode_getData(childNode),
                               compression);
  if (new_id < 0) {
    HL_ERROR1("Failed to create dataset %s\n", HLNode_getName(childNode));
    goto fail;
  }

  HLNode_setMark(childNode, NMARK_ORIGINAL);
  status = 1;
fail:
  HL_H5G_CLOSE(loc_id);
  HL_H5D_CLOSE(new_id);
  return status;
}

/**
 * Appends an REFERENCE node to the data structure. The parentNode can be either
 * of type GROUP or DATASET.
 * @param[in] file_id The file reference
 * @param[in] parentNode The parent node of the datatype to be written.
 * @param[in] parentName The name of the parent node.
 * @param[in] childNode The node to be written.
 * @param[in] childName the attributes name.
 * @return 1 on success, otherwise 0
 */
static int doAppendHdf5Reference(hid_t rootGrp, hid_t file_id, HL_Node* parentNode,
  char* parentName, HL_Node* childNode, char* childName)
{
  hid_t tmpLocId = -1;
  HL_DEBUG0("ENTER: doWriteHdf5Reference");
  if (!parentName || !childName) {
    HL_ERROR0("Can't write HDF5 dataset since either parentName or childName is NULL");
    return 0;
  }
  if (strcmp(parentName, "") == 0) {
    tmpLocId = rootGrp;
  } else {
    tmpLocId = HLNodePrivate_getHdfID(parentNode);
  }
  if (createReference(tmpLocId, file_id, childName, (char*) HLNode_getData(childNode)) < 0) {
    HL_ERROR3("Failed to create reference from '%s/%s' to '%s'",
        parentName, childName, (char*)HLNode_getData(childNode));
    return 0;
  }
  return 1;
}

/*@} End of Private functions */

/*@{ Interface functions */
int HLNodeList_write(HL_NodeList* nodelist, HL_FileCreationProperty* property,
  HL_Compression* compression)
{
  int i;
  HL_Node* parentNode = NULL;
  char* parentName = NULL;
  char* childName = NULL;
  hid_t file_id = -1;
  hid_t gid = -1;
  int status = 0;
  char* filename = NULL;
  int nNodes = 0;

  HL_DEBUG0("ENTER: writeHL_NodeList");

  if (nodelist == NULL) {
    HL_ERROR0("Inparameters NULL");
    goto fail;
  }

  if ((filename = HLNodeList_getFileName(nodelist)) == NULL) {
    HL_ERROR0("Could not get filename from nodelist");
    goto fail;
  }

  if ((file_id = createHlHdfFile(filename, property)) < 0) {
    HL_DEBUG0("Failed to create HDF5 file");
    goto fail;
  }

  if ((gid = H5Gopen(file_id, ".", H5P_DEFAULT)) < 0) {
    HL_DEBUG0("Failed to open root group");
    goto fail;
  }

  if ((nNodes = HLNodeList_getNumberOfNodes(nodelist)) < 0) {
    HL_ERROR0("Failed to get number of nodes");
    goto fail;
  }

  for (i = 0; i < nNodes; i++) {
    HL_Node* node = HLNodeList_getNodeByIndex(nodelist, i);
    if (node == NULL) {
      HL_ERROR1("Failed to get node at index %d", i);
      goto fail;
    }
    HLHDF_FREE(parentName);
    HLHDF_FREE(childName);
    if (!extractParentChildName(node, &parentName, &childName)) {
      HL_ERROR0("Failed to extract parent, child name");
      goto fail;
    }
    if ((strcmp(parentName, "") != 0) &&
        !(parentNode = HLNodeList_getNodeByName(nodelist, parentName))) {
      HL_ERROR1("Failed to locate parent node '%s'",parentName);
      goto fail;
    }
    switch (HLNode_getType(node)) {
    case ATTRIBUTE_ID: {
      if (!doWriteHdf5Attribute(gid, parentNode, parentName,
                                node, childName)) {
        goto fail;
      }
      break;
    }
    case GROUP_ID: {
      if (!doWriteHdf5Group(gid, parentNode, parentName, node,
                            childName)) {
        goto fail;
      }
      break;
    }
    case DATASET_ID: {
      if (compression != NULL) {
        if (!doWriteHdf5Dataset(gid, parentNode, parentName,
                                node, childName, compression)) {
          goto fail;
        }
      } else {
        if (!doWriteHdf5Dataset(gid, parentNode, parentName,
                                node, childName,
                                HLNode_getCompression(node))) {
          goto fail;
        }
      }
      break;
    }
    case TYPE_ID: {
      if (!doWriteHdf5Datatype(file_id, parentNode, parentName,
                               node, childName))
        goto fail;
      break;
    }
    case REFERENCE_ID: {
      if (!doWriteHdf5Reference(gid, file_id, parentNode, parentName,
                                node, childName))
        goto fail;
      break;
    }
    default: {
      HL_ERROR0("Unrecognized type");
      break;
    }
    }
  }
  H5Fflush(file_id, H5F_SCOPE_LOCAL);
  status = 1;
fail:
  HLHDF_FREE(parentName);
  HLHDF_FREE(childName);
  HL_H5G_CLOSE(gid);
  HL_H5F_CLOSE(file_id);
  HLHDF_FREE(filename);
  HL_DEBUG1("EXIT: writeHL_NodeList with status %d", status);

  return status;
}

int HLNodeList_update(HL_NodeList* nodelist, HL_Compression* compression)
{
  int i;
  HL_Node* parentNode = NULL;
  char* parentName = NULL;
  char* childName = NULL;
  hid_t file_id = -1;
  hid_t gid = -1;
  int status = 0;
  char* filename = NULL;
  int nNodes = 0;

  HL_DEBUG0("ENTER: updateHL_NodeList");

  if (nodelist == NULL) {
    HL_ERROR0("Inparameters NULL");
    goto fail;
  }

  if ((filename = HLNodeList_getFileName(nodelist)) == NULL) {
    HL_ERROR0("Could not get filename from nodelist");
    goto fail;
  }

  if ((file_id = openHlHdfFile(filename, "rw")) < 0) {
    HL_ERROR1("Failed to open file %s\n", filename);
    goto fail;
  }

  if ((gid = H5Gopen(file_id, ".", H5P_DEFAULT)) < 0) {
    HL_ERROR0("Failed to open root group\n");
    goto fail;
  }

  if ((nNodes = HLNodeList_getNumberOfNodes(nodelist)) < 0) {
    HL_ERROR0("Failed to get number of nodes");
    goto fail;
  }

  for (i = 0; i < nNodes; i++) {
    HL_Node* node = HLNodeList_getNodeByIndex(nodelist, i);
    if (node == NULL) {
      HL_ERROR1("Failed to get node at index %d", i);
      goto fail;
    }
    if (HLNode_getMark(node) == NMARK_CREATED) {
      HLHDF_FREE(parentName);
      HLHDF_FREE(childName);
      if (!extractParentChildName(node, &parentName, &childName)) {
        HL_ERROR0("Failed to extract parent, child name\n");
        goto fail;
      }
      if ((strcmp(parentName, "") != 0) &&
           !(parentNode = HLNodeList_getNodeByName(nodelist, parentName))) {
        HL_ERROR1("Failed to locate parent node '%s'\n", parentName);
        goto fail;
      }
      switch (HLNode_getType(node)) {
      case ATTRIBUTE_ID: {
        if (!doAppendHdf5Attribute(file_id, parentNode, parentName,
                                   node, childName)) {
          goto fail;
        }
        break;
      }
      case GROUP_ID: {
        if (!doAppendHdf5Group(file_id, parentNode, parentName,
                               node, childName)) {
          goto fail;
        }
        break;
      }
      case DATASET_ID: {
        if (compression != NULL) {
          if (!doAppendHdf5Dataset(file_id, parentNode, parentName,
                                   node, childName, compression)) {
            goto fail;
          }
        } else {
          if (!doAppendHdf5Dataset(file_id, parentNode, parentName,
                                   node, childName,
                                   HLNode_getCompression(node))) {
            goto fail;
          }
        }
        break;
      }
      case TYPE_ID: {
        if (!doWriteHdf5Datatype(file_id, parentNode, parentName,
                                 node, childName))
          goto fail;
        break;
      }
      case REFERENCE_ID: {
        if (!doAppendHdf5Reference(gid, file_id, parentNode, parentName,
                                   node, childName))
          goto fail;
        break;
      }
      default: {
        HL_ERROR1("Unsupported node type for update '%d'\n", HLNode_getType(node));
        goto fail;
        break;
      }
      }
    }
  }

  status = 1;
fail:
  HLHDF_FREE(parentName);
  HLHDF_FREE(childName);
  HL_H5G_CLOSE(gid);
  HL_H5F_CLOSE(file_id);
  HLHDF_FREE(filename);
  HL_DEBUG1("EXIT: updateHL_NodeList with status = %d", status);
  return status;
}

/*@} End of Interface functions */
