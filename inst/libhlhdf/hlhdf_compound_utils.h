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
#ifndef HLHDF_COMPOUND_UTILS_H
#define HLHDF_COMPOUND_UTILS_H

/**
 * Creates a new compound type.
 * @ingroup hlhdf_c_apis
 * @param[in] size  The size of the compound type, usually sizeof(struct name).
 * @return <0 on failure, otherwise a reference to the created type.
 */
hid_t createCompoundType(size_t size);

/**
 * Adds one attribute to a compound type.
 * @ingroup hlhdf_c_apis
 * @param[in] loc_id The type the attribute should be inserted in.
 * @param[in] name  The name of the member attribute
 * @param[in] offset  Offset of this attribute
 * @param[in] type_id The type of the attribute
 * @return <0 on failure, otherwise ok.
 */
herr_t addAttributeToCompoundType(hid_t loc_id, const char* name, size_t offset,hid_t type_id);

/**
 * Same as @ref addAttributeToCompoundType, but instead of a type as hid_t a
 * @ref ValidFormatSpecifiers "format specifier" is provided.
 * @ingroup hlhdf_c_apis
 * @param[in] loc_id The type the attribute should be inserted in.
 * @param[in] name  The name of the member attribute
 * @param[in] offset  Offset of this attribute
 * @param[in] fmt The type of the attribute. See @ref ValidFormatSpecifiers "format specifiers".
 * @return <0 on failure, otherwise ok.
 */
herr_t addAttributeToCompoundType_fmt(hid_t loc_id, const char* name, size_t offset,const char* fmt);

/**
 * Adds an array attribute to a compound type.
 * @ingroup hlhdf_c_apis
 * @param[in] loc_id  The type this attribute should be inserted in
 * @param[in] name  The name of the attribute
 * @param[in] offset  Offset of this attribute
 * @param[in] ndims The rank of the array attribute (max 4)
 * @param[in] dims  The dimension of the array attribute
 * @param[in] type_id The type of the array attribute
 * @return <0 on failure, otherwise ok.
 */
herr_t addArrayToCompoundType(hid_t loc_id, const char* name, size_t offset, int ndims,size_t* dims,hid_t type_id);

/**
 * Same as @ref addArrayToCompoundType, but instead of a type as hid_t, the format
 * is specified with a @ref ValidFormatSpecifiers "format specifier".
 * @ingroup hlhdf_c_apis
 * @param[in] loc_id  The type this attribute should be inserted in
 * @param[in] name  The name of the attribute
 * @param[in] offset  Offset of this attribute
 * @param[in] ndims The rank of the array attribute (max 4)
 * @param[in] dims  The dimension of the array attribute
 * @param[in] fmt The type of the attribute. See @ref ValidFormatSpecifiers "format specifiers".
 * Returns: <0 is failure, otherwise ok.
 */
herr_t addArrayToCompoundType_fmt(hid_t loc_id, const char* name, size_t offset, \
          int ndims, size_t* dims, const char* fmt);

#endif
