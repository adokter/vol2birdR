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
 * Common functions for working with an HDF5 file through the HL-HDF API.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-06-15
 */
#include "hlhdf.h"
#include "hlhdf_private.h"
#include "hlhdf_debug.h"
#include "hlhdf_alloc.h"
#include "hlhdf_defines_private.h"
#include <string.h>
#include <stdlib.h>

/*For internal use*/
static int errorReportingOn=1;
static void *edata;
static herr_t (*errorFunction)(hid_t estack, void *client_data);

static int initialized = 0;

/** Flag toggling the debugging */
static int _debug_hdf;

static const char HLHDF_UNDEFINED_STR[]= "UNDEFINED"; /**< 'UNDEFINED' */
static const char HLHDF_CHAR_STR[]     = "char";      /**< 'char' */
static const char HLHDF_SCHAR_STR[]    = "schar";     /**< 'schar' */
static const char HLHDF_UCHAR_STR[]    = "uchar";     /**< 'uchar' */
static const char HLHDF_SHORT_STR[]    = "short";     /**< 'short' */
static const char HLHDF_USHORT_STR[]   = "ushort";    /**< 'ushort' */
static const char HLHDF_INT_STR[]      = "int";       /**< 'int' */
static const char HLHDF_UINT_STR[]     = "uint";      /**< 'uint' */
static const char HLHDF_LONG_STR[]     = "long";      /**< 'long' */
static const char HLHDF_ULONG_STR[]    = "ulong";     /**< 'ulong' */
static const char HLHDF_LLONG_STR[]    = "llong";     /**< 'llong' */
static const char HLHDF_ULLONG_STR[]   = "ullong";    /**< 'ullong' */
static const char HLHDF_FLOAT_STR[]    = "float";     /**< 'float' */
static const char HLHDF_DOUBLE_STR[]   = "double";    /**< 'double' */
static const char HLHDF_LDOUBLE_STR[]  = "ldouble";   /**< 'ldouble' */
static const char HLHDF_HSIZE_STR[]    = "hsize";     /**< 'hsize' */
static const char HLHDF_HSSIZE_STR[]   = "hssize";    /**< 'hssize' */
static const char HLHDF_HERR_STR[]     = "herr";      /**< 'herr' */
static const char HLHDF_HBOOL_STR[]    = "hbool";     /**< 'hbool' */
static const char HLHDF_STRING_STR[]   = "string";    /**< 'string' */
static const char HLHDF_COMPOUND_STR[] = "compound";  /**< 'compound' */
static const char HLHDF_ARRAY_STR[]    = "array";     /**< 'array' */

static char HLHDF_HDF5_VERSION_STRING[64];      /**< keeps the version string */

static const char* VALID_FORMAT_SPECIFIERS[] = {
  HLHDF_UNDEFINED_STR,
  HLHDF_CHAR_STR,
  HLHDF_SCHAR_STR,
  HLHDF_UCHAR_STR,
  HLHDF_SHORT_STR,
  HLHDF_USHORT_STR,
  HLHDF_INT_STR,
  HLHDF_UINT_STR,
  HLHDF_LONG_STR,
  HLHDF_ULONG_STR,
  HLHDF_LLONG_STR,
  HLHDF_ULLONG_STR,
  HLHDF_FLOAT_STR,
  HLHDF_DOUBLE_STR,
  HLHDF_LDOUBLE_STR,
  HLHDF_HSIZE_STR,
  HLHDF_HSSIZE_STR,
  HLHDF_HERR_STR,
  HLHDF_HBOOL_STR,
  HLHDF_STRING_STR,
  HLHDF_COMPOUND_STR,
  HLHDF_ARRAY_STR,
  NULL,
};

/*@{ Interface functions */
/************************************************
 * disableErrorReporting
 ***********************************************/
void HL_disableErrorReporting(void)
{
  /*Disable error reporting*/
  if (errorReportingOn == 1) {
    H5Eget_auto2(H5E_DEFAULT, &errorFunction, &edata);
    H5Eset_auto2(H5E_DEFAULT, NULL, NULL);
    errorReportingOn = 0;
  }
}

/************************************************
 * enableErrorReporting
 ***********************************************/
void HL_enableErrorReporting(void)
{
  if (errorReportingOn == 0) {
    H5Eset_auto2(H5E_DEFAULT, errorFunction, edata);
    errorReportingOn = 1;
  }
}

int HL_isErrorReportingEnabled(void)
{
  return errorReportingOn;
}

#ifdef HLHDF_MEMORY_DEBUG
static void hlhdf_dump_memory_information(void)
{
  hlhdf_alloc_dump_heap();
  hlhdf_alloc_print_statistics();
}
#endif

/************************************************
 * initHlHdf
 ***********************************************/
void HL_init(void)
{
  if (initialized == 0) {
    initialized = 1;
    _debug_hdf = 0;
    H5Eset_auto2(H5E_DEFAULT, HL_hdf5_debug_function, NULL); /* Force logging to always goto HLHDFs handler */
    HL_InitializeDebugger();
    HL_enableHdf5ErrorReporting();
    HL_disableErrorReporting();
#ifdef HLHDF_MEMORY_DEBUG
    if (atexit(hlhdf_dump_memory_information) != 0) {
      HL_printf("Could not set atexit function");
    }
#endif
  }
}

/************************************************
 * debugHlHdf
 ***********************************************/
void HL_setDebugMode(int flag)
{
  if (flag == 0) {
    /*Don't debug anything*/
    _debug_hdf = 0;
    HL_setDebugLevel(HLHDF_SILENT);
    HL_disableErrorReporting();
  } else if (flag == 1) {
    /*Only debug HLHDF stuff*/
    _debug_hdf = 1;
    HL_setDebugLevel(HLHDF_DEBUG);
    HL_disableErrorReporting();
  } else {
    /*Debug everything*/
    _debug_hdf = 1;
    HL_setDebugLevel(HLHDF_DEBUG);
    HL_enableErrorReporting();
  }
}

/************************************************
 * isHdf5File
 ***********************************************/
int HL_isHDF5File(const char* filename)
{
  htri_t checkValue = H5Fis_hdf5(filename);
  HL_DEBUG0("isHdf5File");
  if (checkValue > 0)
    return TRUE;
  else
    return FALSE;
}
/************************************************
 * createHlHdfFileCreationProperty
 ***********************************************/
HL_FileCreationProperty* HLFileCreationProperty_new(void)
{
  HL_FileCreationProperty* retv = NULL;
  hid_t theHid = -1;

  HL_DEBUG0("ENTER: createHlHdfFileCreationProperty");

  if ((retv = (HL_FileCreationProperty*) HLHDF_MALLOC(sizeof(HL_FileCreationProperty))) == NULL) {
    HL_ERROR0("Failure when allocating memory for HL_FileCreationProperty");
    return NULL;
  }

  if ((theHid = H5Pcreate(H5P_FILE_CREATE)) < 0) {
    HL_ERROR0("Failure when creating the property list");
    HLHDF_FREE(retv);
    return NULL;
  }

  /* Fetch default information */
  if (H5Pget_version(theHid, &retv->version.super, &retv->version.freelist,
                     &retv->version.stab, &retv->version.shhdr) < 0) {
    HL_ERROR0("Failure while getting version for property");
    goto fail;
  }

  if (H5Pget_userblock(theHid, &retv->userblock) < 0) {
    HL_ERROR0("Failure while getting the userblock for property");
    goto fail;
  }

  if (H5Pget_sizes(theHid, &retv->sizes.sizeof_addr, &retv->sizes.sizeof_size)
      < 0) {
    HL_ERROR0("Failure while getting the sizes for property");
    goto fail;
  }

  if (H5Pget_sym_k(theHid, &retv->sym_k.ik, &retv->sym_k.lk) < 0) {
    HL_ERROR0("Failure while getting the sym_k for property");
    goto fail;
  }

  if (H5Pget_istore_k(theHid, &retv->istore_k) < 0) {
    HL_ERROR0("Failure while getting the istore_k for property");
    goto fail;
  }

  HL_H5P_CLOSE(theHid);
  if ((theHid = H5Pcreate(H5P_FILE_ACCESS)) < 0) {
    HL_ERROR0("Failure when creating the file access property list");
    goto fail;
  }
  if (H5Pget_meta_block_size(theHid, &retv->meta_block_size) < 0) {
    HL_ERROR0("Failure while getting the meta_block_size for property");
    goto fail;
  }
  HL_H5P_CLOSE(theHid);

  return retv;
fail:
  HL_H5P_CLOSE(theHid);
  HLFileCreationProperty_free(retv);
  return NULL;
}

/************************************************
 * freeHL_fileCreationProperty
 ***********************************************/
void HLFileCreationProperty_free(HL_FileCreationProperty* prop)
{
  HL_DEBUG0("ENTER: freeHL_fileCreationProperty");
  if (prop == NULL) {
    return;
  }
  HLHDF_FREE(prop);
}

/**********************************************************
 *Function: whatSizeIsHdfFormat
 **********************************************************/
int HL_sizeOfFormat(const char* format)
{
  hid_t tmpType;
  int size = -1;
  HL_DEBUG0("ENTER: whatSizeIsHdfFormat");
  tmpType = HL_translateFormatStringToDatatype(format);
  if (tmpType < 0) {
    HL_ERROR1("There is no type called %s",format);
    return -1;
  }
  size = H5Tget_size(tmpType);
  H5Tclose(tmpType);
  return size;
}

/**********************************************************
 *Function: isFormatSupported
 **********************************************************/
int HL_isFormatSupported(const char* format)
{
  hid_t tmpHid = -1;
  int retv = 1;
  HL_DEBUG0("ENTER: isFormatSupported");
  if ((tmpHid = HL_translateFormatStringToDatatype(format)) < 0) {
    retv = 0;
  }
  HL_H5T_CLOSE(tmpHid);
  return retv;
}

HL_FormatSpecifier HL_getFormatSpecifier(const char* format)
{
  int i = 0;
  if (format == NULL) {
    HL_ERROR0("format NULL");
    return HLHDF_UNDEFINED;
  }

  for ( i = HLHDF_UNDEFINED; i < HLHDF_END_OF_SPECIFIERS; i++) {
    if (strcmp(format, VALID_FORMAT_SPECIFIERS[i]) == 0) {
      return i;
    }
  }

  return HLHDF_UNDEFINED;
}

const char* HL_getFormatSpecifierString(HL_FormatSpecifier specifier)
{
  if (specifier < HLHDF_UNDEFINED || specifier >= HLHDF_END_OF_SPECIFIERS) {
    return NULL;
  }
  return VALID_FORMAT_SPECIFIERS[specifier];
}

const char* HL_getHDF5Version(void) {
  if (strcmp("", H5_VERS_SUBRELEASE)==0) {
    sprintf(HLHDF_HDF5_VERSION_STRING, "%d.%d.%d", H5_VERS_MAJOR, H5_VERS_MINOR, H5_VERS_RELEASE);
  } else {
    sprintf(HLHDF_HDF5_VERSION_STRING, "%d.%d.%d-%s", H5_VERS_MAJOR, H5_VERS_MINOR, H5_VERS_RELEASE, H5_VERS_SUBRELEASE);
  }
  return HLHDF_HDF5_VERSION_STRING;
}

/**********************************************************
 *Function: newHL_Compression
 **********************************************************/
HL_Compression* HLCompression_new(HL_CompressionType aType)
{
  HL_Compression* retv = NULL;
  HL_DEBUG0("ENTER: newHL_Compression");
  if (!(retv = (HL_Compression*) HLHDF_MALLOC(sizeof(HL_Compression)))) {
    HL_ERROR0("Failed to allocate memory for HL_Compression");
    return NULL;
  }
  HLCompression_init(retv, aType);
  return retv;
}

/**********************************************************
 *Function: dupHL_Compression
 **********************************************************/
HL_Compression* HLCompression_clone(HL_Compression* inv)
{
  HL_Compression* retv = NULL;
  HL_SPEWDEBUG0("ENTER: dupHL_Compression");
  if (!inv) {
    HL_SPEWDEBUG0("dupHL_Compression: compression object NULL");
    goto fail;
  }
  if (!(retv = (HL_Compression*) HLHDF_MALLOC(sizeof(HL_Compression)))) {
    HL_ERROR0("Failed to allocate memory for HL_Compression");
    goto fail;
  }
  retv->type = inv->type;
  retv->level = inv->level;
  retv->szlib_mask = inv->szlib_mask;
  retv->szlib_px_per_block = inv->szlib_px_per_block;
fail:
  HL_SPEWDEBUG0("EXIT: dupHL_Compression");
  return retv;
}

/**********************************************************
 *Function: initHL_Compression
 **********************************************************/
void HLCompression_init(HL_Compression* inv, HL_CompressionType aType)
{
  HL_DEBUG0("ENTER: initHL_Compression");
  if (!inv) {
    HL_ERROR0("Trying to initialize NULL");
    return;
  }
  inv->type = aType;
  inv->level = 6;
  inv->szlib_mask = H5_SZIP_ALLOW_K13_OPTION_MASK | H5_SZIP_EC_OPTION_MASK;
  inv->szlib_px_per_block = 16;
}

/**********************************************************
 *Function: freeHL_Compression
 **********************************************************/
void HLCompression_free(HL_Compression* inv)
{
  HL_SPEWDEBUG0("ENTER: freeHL_Compression");
  HLHDF_FREE(inv);
}

/*@} End of Interface functions */

/*@{ Private functions */

/************************************************
 * openHlHdfFile
 ***********************************************/
hid_t openHlHdfFile(const char* filename, const char* how)
{
  unsigned flags = H5F_ACC_RDWR;
  HL_DEBUG2("ENTER: openHlHdfFile(%s,%s)", filename, how);

  if (strcmp(how, "r") == 0) {
    flags = H5F_ACC_RDONLY;
  } else if (strcmp(how, "w") == 0 || strcmp(how, "rw") == 0 || strcmp(how, "wr") == 0) {
    flags = H5F_ACC_RDWR;
  } else {
    HL_ERROR0("Illegal mode given when opening file, should be (r|w|rw)");
    return (hid_t) -1;
  }
  HL_DEBUG0("EXIT: openHlHdfFile");
  return H5Fopen(filename, flags, H5P_DEFAULT);
}

/************************************************
 * createHlHdfFile
 ***********************************************/
hid_t createHlHdfFile(const char* filename,
  HL_FileCreationProperty* property)
{
  hid_t propId = -1;
  hid_t fileId = -1;
  hid_t fileaccesspropertyId = -1;

  HL_DEBUG0("ENTER: createHlHdfFile");
  if (property == NULL) {
    HL_DEBUG0("Using default properties");
    fileId = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  } else {
    HL_DEBUG0("Using specific properties");
    if ((propId = H5Pcreate(H5P_FILE_CREATE)) < 0) {
      HL_ERROR0("Failed to create the property");
      goto done;
    }

    HL_DEBUG1("Setting userblock property to %d",property->userblock);
    if (H5Pset_userblock(propId, property->userblock) < 0) {
      HL_ERROR0("Failed to set the userblock property");
      goto done;
    }

    HL_DEBUG2("Setting sizes to %d, %d",property->sizes.sizeof_addr,property->sizes.sizeof_size);
    if (H5Pset_sizes(propId, property->sizes.sizeof_addr,
                     property->sizes.sizeof_size) < 0) {
      HL_ERROR0("Failed to set the sizes property");
      goto done;
    }

    HL_DEBUG2("Setting sym_k to %d, %d",property->sym_k.ik,property->sym_k.lk);
    if (H5Pset_sym_k(propId, property->sym_k.ik, property->sym_k.lk) < 0) {
      HL_ERROR0("Failed to set the sym_k property");
      goto done;
    }

    HL_DEBUG1("Setting istore_k to %d",property->istore_k);
    if (H5Pset_istore_k(propId, property->istore_k) < 0) {
      HL_ERROR0("Failed to set the istore_k property");
      goto done;
    }

    if (property->meta_block_size != 2048) {
      if ((fileaccesspropertyId = H5Pcreate(H5P_FILE_ACCESS)) < 0) {
        HL_ERROR0("Failed to create the H5P_FILE_ACCESS property");
        goto done;
      }
      if (H5Pset_meta_block_size(fileaccesspropertyId,
                                 property->meta_block_size) < 0) {
        HL_ERROR0("Failed to set the meta block size");
        goto done;
      }
      fileId = H5Fcreate(filename, H5F_ACC_TRUNC, propId, fileaccesspropertyId);
    } else {
      fileId = H5Fcreate(filename, H5F_ACC_TRUNC, propId, H5P_DEFAULT);
      return fileId;
    }
  }

done:
  HL_H5P_CLOSE(propId);
  HL_H5P_CLOSE(fileaccesspropertyId);
  HL_DEBUG0("EXIT: createHlHdfFile");
  return fileId;
}

/************************************************
 * getFixedType
 ***********************************************/
hid_t getFixedType(hid_t type)
{
  size_t size;
  hid_t mtype = -1;
  H5T_str_t strpad;
  hid_t* memb = NULL;
  char** name = NULL;
  int nmembs = 0, i, j, *ndims = NULL;
  size_t offset, nelmts;
  hsize_t *dims141 = NULL;
  hid_t f_memb = -1;
  hid_t member_type;
  hid_t tmpt = -1;
  HL_SPEWDEBUG0("ENTER: getFixedType, version > 1.4");

  size = H5Tget_size(type);

  switch (H5Tget_class(type)) {
  case H5T_INTEGER:
    HL_SPEWDEBUG0("This is of type H5T_INTEGER");
    if (size <= sizeof(char))
      mtype = H5Tcopy(H5T_NATIVE_SCHAR);
    else if (size <= sizeof(short))
      mtype = H5Tcopy(H5T_NATIVE_SHORT);
    else if (size <= sizeof(int))
      mtype = H5Tcopy(H5T_NATIVE_INT);
    else if (size <= sizeof(long))
      mtype = H5Tcopy(H5T_NATIVE_LONG);
    else
      mtype = H5Tcopy(H5T_NATIVE_LLONG);
    H5Tset_sign(mtype, H5Tget_sign(type));
    break;
  case H5T_FLOAT:
    HL_SPEWDEBUG0("This is of type H5T_FLOAT");
    if (size <= sizeof(float))
      mtype = H5Tcopy(H5T_NATIVE_FLOAT);
    else if (size <= sizeof(double))
      mtype = H5Tcopy(H5T_NATIVE_DOUBLE);
    else
      mtype = H5Tcopy(H5T_NATIVE_LDOUBLE);
    break;
  case H5T_STRING:
    HL_SPEWDEBUG0("This is of type H5T_STRING");
    mtype = H5Tcopy(H5T_C_S1);
    if (H5Tis_variable_str(type)) {
      HL_H5T_CLOSE(mtype);
      mtype = H5Tcopy(type);
    } else {
      H5Tset_size(mtype, size);
    }
    strpad = H5Tget_strpad(type);
    H5Tset_strpad(mtype, strpad);
    if (H5Tequal(mtype, type) < 0) {
      HL_H5T_CLOSE(mtype);
      mtype = H5Tcopy(H5T_FORTRAN_S1);
      H5Tset_size(mtype, size);
      H5Tset_strpad(mtype, strpad);
      if (mtype >= 0 && H5Tequal(mtype, type) < 0) {
        HL_H5T_CLOSE(mtype);
      }
    }
    if (mtype >= 0 &&
        (H5Tget_strpad(mtype) == H5T_STR_NULLPAD ||
         H5Tget_strpad(mtype) == H5T_STR_SPACEPAD)) {
      size_t msize = H5Tget_size(mtype);
      if (H5Tset_strpad(mtype, H5T_STR_NULLTERM) < 0) {
        HL_ERROR0("Failed to change strpad to NULLTERMINATION");
        HL_H5T_CLOSE(mtype);
      }
      if (mtype>=0 && H5Tset_size(mtype, msize+1)<0) {
        HL_ERROR0("Failed to modify size to contain 1 more character");
        HL_H5T_CLOSE(mtype);
      }
    }

    break;
  case H5T_COMPOUND:
    HL_SPEWDEBUG0("This is of type H5T_COMPOUND");
    nmembs = H5Tget_nmembers(type);
    memb = HLHDF_CALLOC(nmembs, sizeof(hid_t));
    name = HLHDF_CALLOC(nmembs, sizeof(char*));
    ndims = HLHDF_CALLOC(nmembs, sizeof(int));
    dims141 = HLHDF_CALLOC(nmembs * 4, sizeof(hsize_t));

    if (!memb || !name || !ndims || !dims141) {
      HL_ERROR0("Failed to allocate memory");
      goto done;
    }

    for (i = 0; i < nmembs; i++) {
      name[i] = NULL;
    }

    for (i = 0, size = 0; i < nmembs; i++) {
      f_memb = H5Tget_member_type(type, i);
      memb[i] = getFixedType(f_memb);

      if (memb[i] < 0) {
        HL_H5T_CLOSE(f_memb);
        goto done;
      }

      if (H5T_ARRAY == H5Tget_member_class(type, i)) {
        hsize_t tmpdims[4];
        H5Tget_array_dims2(f_memb, tmpdims);
        ndims[i] = H5Tget_array_ndims(f_memb);
        if (ndims[i] < 0 || ndims[i] > 4) {
          HL_ERROR0("Number of dims (the rank) not between 0-4");
          if (f_memb >= 0)
            H5Tclose(f_memb);
          goto done;
        }
        for (j = 0, nelmts = 1; j < ndims[i]; j++) {
          dims141[i * 4 + j] = tmpdims[j];
          nelmts *= tmpdims[j];
        }
      } else {
        nelmts = 1;
      }

      H5Tclose(f_memb);

      name[i] = H5Tget_member_name(type, i);
      if (name[i] == NULL) {
        goto done;
      }
      size = HLHDF_ALIGN(size, H5Tget_size(memb[i])) + nelmts * H5Tget_size(memb[i]);
    }

    mtype = H5Tcreate(H5T_COMPOUND, size);
    for (i = 0, offset = 0; i < nmembs; i++) {
      for (j = 0, nelmts = 1; j < ndims[i]; j++) {
        nelmts *= dims141[i * 4 + j];
      }

      if (H5T_ARRAY == H5Tget_member_class(type, i)) {
        f_memb = H5Tget_member_type(type, i);

        member_type = H5Tarray_create2(memb[i], ndims[i], dims141 + i * 4);
        H5Tinsert(mtype, name[i], offset, member_type);
        H5Tclose(member_type);
        offset = HLHDF_ALIGN(offset, H5Tget_size(memb[i])) + H5Tget_size(f_memb);
        if (f_memb >= 0)
          H5Tclose(f_memb);
      } else {
        H5Tinsert(mtype, name[i], offset, memb[i]);
        offset = HLHDF_ALIGN(offset, H5Tget_size(memb[i])) + nelmts
            * H5Tget_size(memb[i]);
      }
    }

    break;
  case H5T_ENUM:
    HL_SPEWDEBUG0("This is of type H5T_ENUM");
    break;
  case H5T_REFERENCE:
    HL_SPEWDEBUG0("This is of type H5T_REFERENCE");
    break;
  case H5T_OPAQUE:
    HL_SPEWDEBUG0("This is of type H5T_OPAQUE");
    mtype = H5Tcopy(type);
    break;
  case H5T_BITFIELD:
    HL_SPEWDEBUG0("This is of type H5T_BITFIELD");
    mtype = H5Tcopy(type);
    H5Tset_offset(mtype, 0);
    H5Tset_order(mtype, H5T_ORDER_LE);
    break;
  case H5T_TIME:
    HL_SPEWDEBUG0("This is of type H5T_TIME");
    break;
  case H5T_ARRAY:
    HL_SPEWDEBUG0("This is of type H5T_ARRAY");
    if ((mtype = H5Tget_super(type)) < 0) {
      HL_ERROR0("Failed getting the type id for the type that the array is made up of...");
      goto done;
    }
    tmpt = mtype;
    mtype = getFixedType(mtype);
    if (H5Tclose(tmpt) < 0) {
      HL_ERROR0("Failed closing super type");
    }
    break;

  default:
    HL_INFO0("HRRM, I don't recognize this datatype");
    break;
  }

done:
  for (i = 0; i < nmembs; i++) {
    if (memb != NULL) {
      HL_H5T_CLOSE(memb[i]);
    }
    if (name != NULL) {
      free(name[i]); /* Not allocated by HLHDF */
    }
  }
  HLHDF_FREE(memb);
  HLHDF_FREE(name);
  HLHDF_FREE(ndims);
  HLHDF_FREE(dims141);

  return mtype;
}

hid_t HL_translateFormatSpecifierToType(HL_FormatSpecifier specifier)
{
  hid_t retv = -1;
  if (specifier <= HLHDF_UNDEFINED || specifier == HLHDF_STRING ||
      specifier == HLHDF_COMPOUND || specifier == HLHDF_ARRAY) {
    HL_ERROR0("Can not translate format=%d into a hdf5 datatype");
    return -1;
  }
  switch (specifier) {
  case   HLHDF_CHAR:
    retv = H5Tcopy(H5T_NATIVE_CHAR);
    break;
  case HLHDF_SCHAR:
    retv = H5Tcopy(H5T_NATIVE_SCHAR);
    break;
  case HLHDF_UCHAR:
    retv = H5Tcopy(H5T_NATIVE_UCHAR);
    break;
  case HLHDF_SHORT:
    retv = H5Tcopy(H5T_NATIVE_SHORT);
    break;
  case HLHDF_USHORT:
    retv = H5Tcopy(H5T_NATIVE_USHORT);
    break;
  case HLHDF_INT:
    retv = H5Tcopy(H5T_NATIVE_INT);
    break;
  case HLHDF_UINT:
    retv = H5Tcopy(H5T_NATIVE_UINT);
    break;
  case HLHDF_LONG:
    retv = H5Tcopy(H5T_NATIVE_LONG);
    break;
  case HLHDF_ULONG:
    retv = H5Tcopy(H5T_NATIVE_ULONG);
    break;
  case HLHDF_LLONG:
    retv = H5Tcopy(H5T_NATIVE_LLONG);
    break;
  case HLHDF_ULLONG:
    retv = H5Tcopy(H5T_NATIVE_ULLONG);
    break;
  case HLHDF_FLOAT:
    retv = H5Tcopy(H5T_NATIVE_FLOAT);
    break;
  case HLHDF_DOUBLE:
    retv = H5Tcopy(H5T_NATIVE_DOUBLE);
    break;
  case HLHDF_LDOUBLE:
    retv = H5Tcopy(H5T_NATIVE_LDOUBLE);
    break;
  case HLHDF_HSIZE:
    retv = H5Tcopy(H5T_NATIVE_HSIZE);
    break;
  case HLHDF_HSSIZE:
    retv = H5Tcopy(H5T_NATIVE_HSSIZE);
    break;
  case HLHDF_HERR:
    retv = H5Tcopy(H5T_NATIVE_HERR);
    break;
  case HLHDF_HBOOL:
    retv = H5Tcopy(H5T_NATIVE_HBOOL);
    break;
  case HLHDF_END_OF_SPECIFIERS:
  default:
    break;
  }
  if (retv == -1) {
    HL_ERROR1("Could not determine hdf5 datatype from %d",specifier);
  }
  return retv;
}

/************************************************
 * HL_translateFormatStringToDatatype
 ***********************************************/
hid_t HL_translateFormatStringToDatatype(const char* dataType)
{
  HL_FormatSpecifier specifier = HLHDF_UNDEFINED;

  if (dataType == NULL) {
    HL_ERROR0("Atempting to translate NULL into a HDF5 datatype.");
    return -1;
  }
  specifier = HL_getFormatSpecifier(dataType);

  return HL_translateFormatSpecifierToType(specifier);
}

/************************************************
 * getTypeNameString
 ***********************************************/
char* getTypeNameString(hid_t type)
{
  char* retv = NULL;
  HL_DEBUG0("ENTER: getTypeNameString");

  switch (H5Tget_class(type)) {
  case H5T_INTEGER:
    if (H5Tequal(type, H5T_STD_I8BE))
      retv = strdup("H5T_STD_I8BE");
    else if (H5Tequal(type, H5T_STD_I8LE))
      retv = strdup("H5T_STD_I8LE");
    else if (H5Tequal(type, H5T_STD_I16BE))
      retv = strdup("H5T_STD_I16BE");
    else if (H5Tequal(type, H5T_STD_I16LE))
      retv = strdup("H5T_STD_I16LE");
    else if (H5Tequal(type, H5T_STD_I32BE))
      retv = strdup("H5T_STD_I32BE");
    else if (H5Tequal(type, H5T_STD_I32LE))
      retv = strdup("H5T_STD_I32LE");
    else if (H5Tequal(type, H5T_STD_I64BE))
      retv = strdup("H5T_STD_I64BE");
    else if (H5Tequal(type, H5T_STD_I64LE))
      retv = strdup("H5T_STD_I64LE");
    else if (H5Tequal(type, H5T_STD_U8BE))
      retv = strdup("H5T_STD_U8BE");
    else if (H5Tequal(type, H5T_STD_U8LE))
      retv = strdup("H5T_STD_U8LE");
    else if (H5Tequal(type, H5T_STD_U16BE))
      retv = strdup("H5T_STD_U16BE");
    else if (H5Tequal(type, H5T_STD_U16LE))
      retv = strdup("H5T_STD_U16LE");
    else if (H5Tequal(type, H5T_STD_U32BE))
      retv = strdup("H5T_STD_U32BE");
    else if (H5Tequal(type, H5T_STD_U32LE))
      retv = strdup("H5T_STD_U32LE");
    else if (H5Tequal(type, H5T_STD_U64BE))
      retv = strdup("H5T_STD_U64BE");
    else if (H5Tequal(type, H5T_STD_U64LE))
      retv = strdup("H5T_STD_U64LE");
    else if (H5Tequal(type, H5T_NATIVE_SCHAR))
      retv = strdup("H5T_NATIVE_SCHAR");
    else if (H5Tequal(type, H5T_NATIVE_UCHAR))
      retv = strdup("H5T_NATIVE_UCHAR");
    else if (H5Tequal(type, H5T_NATIVE_SHORT))
      retv = strdup("H5T_NATIVE_SHORT");
    else if (H5Tequal(type, H5T_NATIVE_USHORT))
      retv = strdup("H5T_NATIVE_USHORT");
    else if (H5Tequal(type, H5T_NATIVE_INT))
      retv = strdup("H5T_NATIVE_INT");
    else if (H5Tequal(type, H5T_NATIVE_UINT))
      retv = strdup("H5T_NATIVE_UINT");
    else if (H5Tequal(type, H5T_NATIVE_LONG))
      retv = strdup("H5T_NATIVE_LONG");
    else if (H5Tequal(type, H5T_NATIVE_ULONG))
      retv = strdup("H5T_NATIVE_ULONG");
    else if (H5Tequal(type, H5T_NATIVE_LLONG))
      retv = strdup("H5T_NATIVE_LLONG");
    else if (H5Tequal(type, H5T_NATIVE_ULLONG))
      retv = strdup("H5T_NATIVE_ULLONG");
    else {
      HL_ERROR0("Undefined integer type");
    }
    break;
  case H5T_FLOAT:
    if (H5Tequal(type, H5T_IEEE_F32BE))
      retv = strdup("H5T_IEEE_F32BE");
    else if (H5Tequal(type, H5T_IEEE_F32LE))
      retv = strdup("H5T_IEEE_F32LE");
    else if (H5Tequal(type, H5T_IEEE_F64BE))
      retv = strdup("H5T_IEEE_F64BE");
    else if (H5Tequal(type, H5T_IEEE_F64LE))
      retv = strdup("H5T_IEEE_F64LE");
    else if (H5Tequal(type, H5T_NATIVE_FLOAT))
      retv = strdup("H5T_NATIVE_FLOAT");
    else if (H5Tequal(type, H5T_NATIVE_DOUBLE))
      retv = strdup("H5T_NATIVE_DOUBLE");
    else if (H5Tequal(type, H5T_NATIVE_LDOUBLE))
      retv = strdup("H5T_NATIVE_LDOUBLE");
    else {
      HL_ERROR0("Undefined float type");
    }
    break;
  case H5T_TIME:
    HL_INFO0("Unsupported type: H5T_TIME");
    break;
  case H5T_STRING:
    retv = strdup("H5T_STRING");
    break;
  case H5T_COMPOUND:
    retv = strdup("H5T_COMPOUND");
    break;
  case H5T_ARRAY:
    retv = strdup("H5T_ARRAY");
    break;
  default:
    HL_INFO0("Unsupported class");
    break;
  }

  return retv;
}

/************************************************
 * HL_getFormatSpecifierFromType
 ***********************************************/
HL_FormatSpecifier HL_getFormatSpecifierFromType(hid_t type)
{
  HL_SPEWDEBUG0("ENTER: getFormatNameString");

  if (H5Tget_class(type) == H5T_STRING) {
    return HLHDF_STRING;
  }

  if (H5Tequal(type, H5T_NATIVE_SCHAR))
    return HLHDF_SCHAR;
  else if (H5Tequal(type, H5T_NATIVE_UCHAR))
    return HLHDF_UCHAR;
  else if (H5Tequal(type, H5T_NATIVE_CHAR))
    return HLHDF_CHAR;
  else if (H5Tequal(type, H5T_NATIVE_SHORT))
    return HLHDF_SHORT;
  else if (H5Tequal(type, H5T_NATIVE_USHORT))
    return HLHDF_USHORT;
  else if (H5Tequal(type, H5T_NATIVE_INT))
    return HLHDF_INT;
  else if (H5Tequal(type, H5T_NATIVE_UINT))
    return HLHDF_UINT;
  else if (H5Tequal(type, H5T_NATIVE_LONG))
    return HLHDF_LONG;
  else if (H5Tequal(type, H5T_NATIVE_ULONG))
    return HLHDF_ULONG;
  else if (H5Tequal(type, H5T_NATIVE_LLONG))
    return HLHDF_LLONG;
  else if (H5Tequal(type, H5T_NATIVE_ULLONG))
    return HLHDF_ULLONG;
  else if (H5Tequal(type, H5T_NATIVE_FLOAT))
    return HLHDF_FLOAT;
  else if (H5Tequal(type, H5T_NATIVE_DOUBLE))
    return HLHDF_DOUBLE;
  else if (H5Tequal(type, H5T_NATIVE_LDOUBLE))
    return HLHDF_LDOUBLE;
  else if (H5Tequal(type, H5T_NATIVE_HSIZE))
    return HLHDF_HSIZE;
  else if (H5Tequal(type, H5T_NATIVE_HSSIZE))
    return HLHDF_HSSIZE;
  else if (H5Tequal(type, H5T_NATIVE_HERR))
    return HLHDF_HERR;
  else if (H5Tequal(type, H5T_NATIVE_HBOOL))
    return HLHDF_HBOOL;
  else if (H5Tget_class(type) == H5T_COMPOUND)
    return HLHDF_COMPOUND;
  else if (H5Tget_class(type) == H5T_ARRAY)
    return HLHDF_ARRAY;
  else {
    HL_INFO0("Not possible to translate from given type to string");
  }
  return HLHDF_UNDEFINED;
}

/************************************************
 * getFormatNameString
 ***********************************************/
const char* getFormatNameString(hid_t type)
{
  HL_SPEWDEBUG0("ENTER: getFormatNameString");
  HL_FormatSpecifier specifier = HL_getFormatSpecifierFromType(type);
  return HL_getFormatSpecifierString(specifier);
}

/************************************************
 * getStringPadName
 ***********************************************/
char* getStringPadName(hid_t type)
{
  H5T_str_t strpad = H5Tget_strpad(type);
  char* retv = NULL;
  HL_DEBUG0("ENTER: getStringPadName");

  if (strpad == H5T_STR_NULLTERM)
    retv = strdup("H5T_STR_NULLTERM");
  else if (strpad == H5T_STR_NULLPAD)
    retv = strdup("H5T_STR_NULLPAD");
  else if (strpad == H5T_STR_SPACEPAD)
    retv = strdup("H5T_STR_SPACEPAD");
  else
    retv = strdup("ILLEGAL STRPAD");

  return retv;
}

/************************************************
 * getStringCsetName
 ***********************************************/
char* getStringCsetName(hid_t type)
{
  H5T_cset_t cset = H5Tget_cset(type);
  char* retv = NULL;
  HL_DEBUG0("ENTER: getStringCsetName");
  if (cset == H5T_CSET_ASCII)
    retv = strdup("H5T_CSET_ASCII");
  else
    retv = strdup("UNKNOWN CHARACTER SET");

  return retv;
}

/************************************************
 * getStringCtypeName
 ***********************************************/
char* getStringCtypeName(hid_t type)
{
  hid_t strtype = H5Tcopy(H5T_C_S1);
  char* retv = NULL;
  H5T_cset_t cset = H5Tget_cset(type);
  H5T_str_t strpad = H5Tget_strpad(type);
  size_t size = H5Tget_size(type);
  H5Tset_cset(strtype, cset);
  H5Tset_size(strtype, size);
  H5Tset_strpad(strtype, strpad);

  HL_DEBUG0("ENTER: getStringCtypeName");
  if (H5Tequal(type, strtype)) {
    retv = strdup("H5T_C_S1");
    H5Tclose(strtype);
  } else {
    H5Tclose(strtype);
    strtype = H5Tcopy(H5T_FORTRAN_S1);
    H5Tset_cset(strtype, cset);
    H5Tset_size(strtype, size);
    H5Tset_strpad(strtype, strpad);
    if (H5Tequal(type, strtype))
      retv = strdup("H5T_FORTRAN_S1");
    else
      retv = strdup("UNKNOWN CHARACTER TYPE");
    HL_H5T_CLOSE(strtype);
  }
  return retv;
}

int extractParentChildName(HL_Node* node, char** parent, char** child)
{
  char* tmpStr = NULL;
  char* tmpPtr = NULL;
  int status = 0;

  //char* tmpPtr;
  HL_SPEWDEBUG0("ENTER: extractParentChildName");
  if (parent == NULL || child == NULL) {
    HL_ERROR0("When extracting ParentChild name, both parent and child must be specified");
    return 0;
  }

  *parent = NULL;
  *child = NULL;
  if (HLNode_getName(node) != NULL) {
    tmpStr = HLHDF_STRDUP(HLNode_getName(node));
  }
  if (tmpStr == NULL) {
    HL_ERROR0("Could not allocate memory for node name");
    goto fail;
  }

  if ((tmpPtr = strrchr(tmpStr, '/')) == NULL) {
    HL_ERROR1("Could not extract '/' from node name %s", tmpStr);
    goto fail;
  }

  tmpPtr[0] = '\0';
  tmpPtr++;
  *parent = HLHDF_STRDUP(tmpStr);
  *child = HLHDF_STRDUP(tmpPtr);
  if (*parent == NULL || *child == NULL) {
    HL_ERROR0("Failed to allocate memory for parent and/or child");
    goto fail;
  }
  status = 1;
fail:
  if (status == 0) {
    HLHDF_FREE(*parent);
    HLHDF_FREE(*child);
  }
  HLHDF_FREE(tmpStr);
  return status;
}

int openGroupOrDataset(hid_t file_id, const char* name, hid_t* lid, HL_Type* type)
{
  int status = 0;

  if (name == NULL || lid == NULL || type == NULL) {
    HL_ERROR0("Inparameters NULL");
    goto fail;
  }
  *lid = -1;
  *type = UNDEFINED_ID;
  if (strcmp(name, "") != 0) {
    H5O_info_t objectInfo;
    herr_t infoStatus = -1;
    int enableReporting = HL_isErrorReportingEnabled();
    HL_disableErrorReporting(); /*Bypass the error reporting, if failed to open a dataset/or group*/
#ifdef USE_HDF5_1_12_API    
    infoStatus = H5Oget_info_by_name(file_id, name, &objectInfo, H5O_INFO_ALL, H5P_DEFAULT);
#else
    infoStatus = H5Oget_info_by_name(file_id, name, &objectInfo, H5P_DEFAULT);
#endif    
    if (enableReporting) {
      HL_enableErrorReporting();
    }
    if (infoStatus >= 0) {
      if (objectInfo.type == H5O_TYPE_GROUP) {
        *type = GROUP_ID;
      } else if (objectInfo.type == H5O_TYPE_DATASET) {
        *type = DATASET_ID;
      } else {
        infoStatus = -1;
        *type = UNDEFINED_ID;
      }
    }
    if (infoStatus < 0) {
      HL_ERROR0("name needs to be a dataset or group.");
      goto fail;
    }
    if ((*lid = H5Oopen(file_id, name, H5P_DEFAULT))<0) {
      HL_ERROR1("Node '%s' could not be opened", name);
      goto fail;
    }
  } else {
    if ((*lid = H5Gopen(file_id, "/", H5P_DEFAULT)) < 0) {
      HL_ERROR0("Could not open root group");
      goto fail;
    }
    *type = GROUP_ID;
  }
  status = 1;
fail:
  if (status == 0) {
    HL_H5O_CLOSE(*lid);
    *type = UNDEFINED_ID;
  }
  return status;
}

/*@} End of Private functions */

