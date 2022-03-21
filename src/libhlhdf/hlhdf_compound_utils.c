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
 * Utility functions for defining compound types through the HDF5-API.
 * These functions are going to change in the near future but are here for
 * backward compatibility.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-06-12
 */
#include "hlhdf.h"
#include "hlhdf_alloc.h"
#include "hlhdf_private.h"
#include "hlhdf_debug.h"
#include "hlhdf_defines_private.h"
#include "hlhdf_compound_utils.h"
#include <stdlib.h>
#include <string.h>

hid_t createCompoundType(size_t size)
{
  hid_t retv = -1;
  HL_SPEWDEBUG0("ENTER: createCompoundType");

  retv =  H5Tcreate(H5T_COMPOUND, size);

  HL_SPEWDEBUG0("EXIT: createCompoundType");
  return retv;
}

herr_t addAttributeToCompoundType(hid_t loc_id, const char* name,
  size_t offset, hid_t type_id)
{
  herr_t retv = -1;
  HL_SPEWDEBUG0("ENTER: addAttributeToCompoundType");

  retv = H5Tinsert(loc_id, name, offset, type_id);

  HL_SPEWDEBUG0("EXIT: addAttributeToCompoundType");
  return retv;
}

herr_t addAttributeToCompoundType_fmt(hid_t loc_id, const char* name,
  size_t offset, const char* fmt)
{
  herr_t status = -1;
  hid_t type_id = HL_translateFormatStringToDatatype(fmt);
  HL_SPEWDEBUG0("ENTER: addAttributeToCompoundType_fmt");
  if (type_id < 0) {
    goto fail;
  }
  status = addAttributeToCompoundType(loc_id, name, offset, type_id);

fail:
  HL_H5T_CLOSE(type_id);
  HL_SPEWDEBUG0("EXIT: addAttributeToCompoundType_fmt");
  return status;
}

herr_t addArrayToCompoundType(hid_t loc_id, const char* name, size_t offset,
  int ndims, size_t* dims, hid_t type_id)
{
  herr_t status = -1;
  hid_t array_type = -1;
  int i;
  hsize_t* dims_hsize_t = (hsize_t*) HLHDF_MALLOC(sizeof(hsize_t) * ndims);
  HL_SPEWDEBUG0("ENTER: addArrayToCompoundType");

  if (!dims_hsize_t) {
    HL_ERROR0("Failed to allocate memory for temporary hsize_t dims");
    goto fail;
  }

  for (i = 0; i < ndims; i++) {
    dims_hsize_t[i] = dims[i];
  }

  array_type = H5Tarray_create(type_id, ndims, dims_hsize_t);
  status = H5Tinsert(loc_id, name, offset, array_type);

fail:
  HL_H5T_CLOSE(array_type);
  HLHDF_FREE(dims_hsize_t);
  HL_SPEWDEBUG0("EXIT: addArrayToCompoundType");
  return status;
}

herr_t addArrayToCompoundType_fmt(hid_t loc_id, const char* name,
  size_t offset, int ndims, size_t* dims, const char* fmt)
{
  hid_t type_id = -1;
  herr_t status = -1;
  HL_SPEWDEBUG0("ENTER: addArrayToCompoundType_fmt");
  type_id = HL_translateFormatStringToDatatype(fmt);
  if (type_id < 0) {
    goto fail;
  }

  status = addArrayToCompoundType(loc_id, name, offset, ndims, dims, type_id);

fail:
  HL_H5T_CLOSE(type_id);
  HL_SPEWDEBUG0("EXIT: addArrayToCompoundType_fmt");
  return status;
}
