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
 * Functions for writing and updating HDF5 files.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-06-12
 */
#ifndef HLHDF_WRITE_H
#define HLHDF_WRITE_H
#include "hlhdf_types.h"

/**
 * Writes a HDF5 file from a nodelist with the specified file properties and compression level/type.
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the node list to write
 * @param[in] property the file creation properties
 * @param[in] compr the wanted compression type and level
 * @return TRUE on success otherwise failure.
 */
int HLNodeList_write(HL_NodeList* nodelist, HL_FileCreationProperty* property, HL_Compression* compr);

/**
 * Updates a HDF5 file from a nodelist.
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the node list to update
 * @param[in] compr the wanted compression type and level
 * @return TRUE on success otherwise failure.
 */
int HLNodeList_update(HL_NodeList* nodelist, HL_Compression* compr);

#endif
