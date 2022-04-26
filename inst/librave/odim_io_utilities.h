/* --------------------------------------------------------------------
Copyright (C) 2011 Swedish Meteorological and Hydrological Institute, SMHI,

This file is part of RAVE.

RAVE is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RAVE is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with RAVE.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------*/
/**
 * Utilities when working with ODIM H5 files.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2011-09-30
 */
#ifndef ODIM_IO_UTILITIES_H
#define ODIM_IO_UTILITIES_H

#include <lazy_nodelist_reader.h>
#include "rave_field.h"
#include "rave_object.h"
#include "rave_types.h"
#include "raveobject_list.h"
#include "raveobject_hashtable.h"
#include "hlhdf.h"
#include <stdarg.h>

/**
 * Struct that can be used when passing around objects and associated nodelist
 * between the writing and loading functions.
 *
 */
typedef struct OdimIoUtilityArg {
  LazyNodeListReader_t* lazyReader; /**< the lazy node list reader */
  HL_NodeList* nodelist;  /**< the nodelist */
  RaveCoreObject* object; /**< the object */
  RaveIO_ODIM_Version version; /**< the version */
} OdimIoUtilityArg;

/**
 * Converts the gain and offset so that quantity is adjusted for ODIM version to be written.
 * @param[in] quantity - the quantity that is affected
 * @param[in] version - ODIM version that should be written
 * @param[in,out] gain - the gain. Will be adjusted according to needs
 * @param[in,out] offset - the offset. Will be adjusted according to needs
 * @return 1 on success
 */
int OdimIoUtilities_convertGainOffsetFromInternalRave(const char* quantity, RaveIO_ODIM_Version version, double* gain, double* offset);

/**
 * Converts the gain and offset so that quantity is adjusted for the internally used quantity.
 * @param[in] quantity - the quantity that is affected
 * @param[in] version - ODIM version that should be written
 * @param[in,out] gain - the gain. Will be adjusted according to needs
 * @param[in,out] offset - the offset. Will be adjusted according to needs
 * @return 1 on success
 */
int OdimIoUtilities_convertGainOffsetToInternalRave(const char* quantity, RaveIO_ODIM_Version version, double* gain, double* offset);

/**
 * Adds a rave field to a nodelist.
 *
 * @param[in] field - the field
 * @param[in] nodelist - the hlhdf node list
 * @param[in] outversion - the version of file to be written
 * @param[in] fmt - the varargs format string
 * @param[in] ... - the varargs
 * @return 1 on success otherwise 0
 */
int OdimIoUtilities_addRaveField(RaveField_t* field, HL_NodeList* nodelist, RaveIO_ODIM_Version outversion, const char* fmt, ...);

/**
 * Adds a list of quality fields (RaveField_t) to a nodelist.
 *
 * @param[in] fields - the list of fields
 * @param[in] nodelist - the hlhdf node list
 * @param[in] outversion - the version of file to be written
 * @param[in] fmt - the varargs format string
 * @param[in] ... - the varargs
 * @return 1 on success otherwise 0
 */
int OdimIoUtilities_addQualityFields(RaveObjectList_t* fields, HL_NodeList* nodelist, RaveIO_ODIM_Version outversion, const char* fmt, ...);

/**
 * Loads a rave field. A rave field can be just about anything with a mapping
 * between attributes and a dataset.
 * @param[in] lazyReader - the wrapper around the hlhdf node list
 * @param[in] version - version of the file read
 * @param[in] fmt - the variable argument list string format
 * @param[in] ... - the variable argument list
 * @return a rave field on success otherwise NULL
 */
RaveField_t* OdimIoUtilities_loadField(LazyNodeListReader_t* lazyReader, RaveIO_ODIM_Version version, const char* fmt, ...);

/**
 * Gets the specified id from the source string, for example. If source string contains CMT:abc,NOD:selek,RAD:se50 then if id = NOD: buf will be filled with selek
 * @param[in] source - the what/source string
 * @param[in] id - the id to search for ended with a :, for example NOD:
 * @param[in,out] buf - the buffer filled with the found id
 * @param[in] buflen - the length of buf
 * @returns 1 if id was found and did fit into buf, otherwise 0
 */
int OdimIoUtilities_getIdFromSource(const char* source, const char* id, char* buf, size_t buflen);

/**
 * Like \ref OdimIoUtilities_getIdFromSource but will first check NOD: and then CMT:
 * @param[in] source - the what/source string
 * @param[in] id - the id to search for ended with a :, for example NOD:
 * @param[in,out] buf - the buffer filled with the found id
 * @param[in] buflen - the length of buf
 * @returns 1 if id was found and did fit into buf, otherwise 0
 */
int OdimIoUtilities_getNodOrCmtFromSource(const char* source, char* buf, size_t buflen);

#endif /* ODIM_IO_UTILITIES_H */
