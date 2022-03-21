/* --------------------------------------------------------------------
Copyright (C) 2009 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Utilities for working with H5 files
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-09-10
 */
#ifndef RAVE_HLHDF_UTILITIES_H
#define RAVE_HLHDF_UTILITIES_H
#include "hlhdf.h"
#include "rave_utilities.h"
#include "rave_types.h"
#include <stdarg.h>

/**
 * ODIM Version string 2.0
 */
#define RAVE_ODIM_VERSION_2_0_STR "ODIM_H5/V2_0"

/**
 * ODIM Version string 2.1
 */
#define RAVE_ODIM_VERSION_2_1_STR "ODIM_H5/V2_1"

/**
 * ODIM Version string 2.2
 */
#define RAVE_ODIM_VERSION_2_2_STR "ODIM_H5/V2_2"

/**
 * ODIM Version string 2.3
 */
#define RAVE_ODIM_VERSION_2_3_STR "ODIM_H5/V2_3"

/**
 * ODIM Version string 2.4
 */
#define RAVE_ODIM_VERSION_2_4_STR "ODIM_H5/V2_4"

/**
 * ODIM H5 rad version string 2.0
 */
#define RAVE_ODIM_H5RAD_VERSION_2_0_STR "H5rad 2.0"

/**
 * ODIM H5 rad version string 2.1
 */
#define RAVE_ODIM_H5RAD_VERSION_2_1_STR "H5rad 2.1"

/**
 * ODIM H5 rad version string 2.2
 */
#define RAVE_ODIM_H5RAD_VERSION_2_2_STR "H5rad 2.2"

/**
 * ODIM H5 rad version string 2.3
 */
#define RAVE_ODIM_H5RAD_VERSION_2_3_STR "H5rad 2.3"

/**
 * ODIM H5 rad version string 2.4
 */
#define RAVE_ODIM_H5RAD_VERSION_2_4_STR "H5rad 2.4"

/**
 * Attribute function called when an attribute is found.
 * @param[in] object - the object passed on from the loading functions
 * @param[in] attr - the attribute
 * @return 1 on success or 0 at failure
 */
typedef int (*RaveHL_attr_f)(void* object, RaveAttribute_t* attr);

/**
 * Data function called when an dataset is found.
 * @param[in] object - the object passed on from the loading functions
 * @param[in] xsize - the xsize
 * @param[in] ysize - the ysize
 * @param[in] data - the dataset data
 * @param[in] dtype - the data type
 * @param[in] nodeName - the name of the data type
 * @return 1 on success or 0 at failure
 */
typedef int (*RaveHL_data_f)(void* object, hsize_t xsize, hsize_t ysize, void* data, RaveDataType dtype, const char* nodeName);

/**
 * Group function called when a group is found
 * @param[in] object - the object passed on from the loading functions
 * @param[in] groupname - the name of the group
 * @param[in] name - the attribute name
 * @return 1 on success or 0 at failure
 */
typedef int (*RaveHL_group_f)(void* object, const char* groupname, const char* name);

/**
 * Translates an attribute name from 2.0/2.1 ODIM into 2.2 ODIM. For example how/TXloss => how/TXlossH.
 * @param[in] name - name of current attribute
 * @returns the original name if no conversion can be done otherwise the translated name
 */
const char* RaveHL_convertAttributeName(const char* name);

/**
 * Translates a quantity from 2.0/2.1 ODIM into 2.2 ODIM. For example VRAD => VRADH.
 * @param[in] name - name of current quantity
 * @returns the original name if no conversion can be done otherwise the translated name
 */
const char* RaveHL_convertQuantity(const char* name);

/**
 * Returns the string representation of the specified odim version
 * @param[in] version - the ODIM version
 * @returns the string representation or UNDEFINED if not found
 */
const char* RaveHL_getOdimVersionString(RaveIO_ODIM_Version version);

/**
 * Returns the odim version for the string representation of the ODIM version
 * @param[in] str - the string representation
 * @returns the odim version or undefined if not found
 */
RaveIO_ODIM_Version RaveHL_getOdimVersionFromString(const char* str);

/**
 * Returns the h5rad string representation of the specified odim version since we can assume that
 * the h5rad and odim version will follow each other
 * @param[in] version - the ODIM version
 * @returns the string representation or UNDEFINED if not found
 */
const char* RaveHL_getH5RadVersionStringFromOdimVersion(RaveIO_ODIM_Version version);

/**
 * Creates a rave attribute from a HLHDF node value.
 * Node must contain data that can be translated to long, double or strings otherwise
 * NULL will be returned. Note, the name will not be set on the attribute and has to
 * be set after this function has been called.
 * @param[in] node - the HLHDF node
 * @returns the rave attribute on success, otherwise NULL.
 */
RaveAttribute_t* RaveHL_createAttribute(HL_Node* node);

/**
 * Tries to find the node as defined by the varargs string and then create
 * a rave attribute from it.
 * @param[in] nodelist - the node list
 * @param[in] fmt - the varargs format
 * @param[in] ... - the varargs
 */
RaveAttribute_t* RaveHL_getAttribute(HL_NodeList* nodelist, const char* fmt, ...);

/**
 * Verifies if the file contains a node with the name as specified by the variable
 * argument list.
 * @param[in] nodelist - the hlhdf nodelist
 * @param[in] fmt    - the variable argument format specifier
 * @param[in] ...    - the variable argument list
 * @returns 1 if the node could be found, otherwise 0
 */
int RaveHL_hasNodeByName(HL_NodeList* nodelist, const char* fmt, ...);

/**
 * Gets a string value from a nodelist that is represented by a node named
 * according to the varargs formatter string. The returned string is
 * pointing to internal memory in the nodelist so DO NOT FREE!!
 * @param[in] nodelist - the node list
 * @param[in|out] value - the value (Pointing to internal memory, DO NOT FREE)
 * @param[in] fmt - the varargs formatter string
 * @param[in] ... - the varargs
 * @returns 1 on success otherwise 0
 */
int RaveHL_getStringValue(HL_NodeList* nodelist, char** value, const char* fmt, ...);

/**
 * Creates a group node in the node list.
 * @param[in] nodelist - the node list
 * @param[in] fmt - the variable argument format
 * @param[in] ... - the arguments.
 */
int RaveHL_createGroup(HL_NodeList* nodelist, const char* fmt, ...);

/**
 * Creates a group node in the node list unless it already exists.
 * @param[in] nodelist - the node list
 * @param[in] fmt - the variable argument format
 * @param[in] ... - the arguments.
 */
int RaveHL_createGroupUnlessExists(HL_NodeList* nodelist, const char* fmt, ...);


/**
 * Adds a string value to a nodelist
 * @param[in] nodelist - the hlhdf node list
 * @param[in] value - the string value
 * @param[in] fmt - the varargs format string
 * @returns 1 on success otherwise 0
 */
int RaveHL_createStringValue(HL_NodeList* nodelist, const char* value, const char* fmt, ...);

/**
 * Puts an attribute in the nodelist as a hlhdf node.
 * @param[in] nodelist - the node list
 * @param[in] attribute - the attribute, the name of the attribute will be used as attr-member
 * @param[in] fmt - the root name, specified as a varargs
 * @param[in] ... - the varargs list
 * @returns 1 on success otherwise 0
 */
int RaveHL_addAttribute(HL_NodeList* nodelist, RaveAttribute_t* attribute, const char* fmt, ...);

/**
 * Stores the attributes from the object into the nodelist
 * name/how/..., name/where/... and name/what/... If the groups
 * name/how, name/where, name/what does not exist they will be created
 * for you. All other groups you will have to add your self.
 *
 * @param[in] nodelist - the hlhdf list
 * @param[in] name - the name of the object
 * @param[in] object - the object to fill
 */
int RaveHL_addAttributes(HL_NodeList* nodelist, RaveObjectList_t* attributes, const char* name);

/**
 * Creates a dataset with the provided 2-dimensional array.
 * @param[in] nodelist - the node list
 * @param[in] data - the data
 * @param[in] xsize - the xsize
 * @param[in] ysize - the ysize
 * @param[in] dataType - the type of data
 * @param[in] fmt - the variable argument format
 * @param[in] ... - the arguments.
 * @returns 1 on success, otherwise 0
 */
int RaveHL_createDataset(HL_NodeList* nodelist, void* data, long xsize, long ysize, RaveDataType dataType, const char* fmt, ...);

/**
 * Adds a data field to the node list according to ODIM H5. If data type is
 * UCHAR, the nessecary attributes for viewing in HdfView will also be added.
 * The name will always be <root>/data since that is according to ODIM H5
 * as well.
 * @param[in] nodelist - the node list that should get nodes added
 * @param[in] data - the array data
 * @param[in] xsize - the xsize
 * @param[in] ysize - the ysize
 * @param[in] dataType - type of data
 * @param[in] fmt - the varargs format
 * @param[in] ... - the vararg list
 * @returns 1 on success otherwise 0
 */
int RaveHL_addData(
  HL_NodeList* nodelist,
  void* data,
  long xsize,
  long ysize,
  RaveDataType dataType,
  const char* fmt, ...);


/**
 * Translates a rave data type into a hlhdf format specifier
 * @param[in] format - the rave data type
 * @returns the hlhdf format specifier
 */
HL_FormatSpecifier RaveHL_raveToHlhdfType(RaveDataType format);

/**
 * Translates a hlhdf format specified into a rave data type.
 * @param[in] format - the hlhdf format specified
 * @returns the RaveDataType
 */
RaveDataType RaveHL_hlhdfToRaveType(HL_FormatSpecifier format);

/**
 * Loads the attributes from the nodelist name and calls the
 * attrf and dataf respectively depending on what type is found.
 * E.g.
 * name/how/..., name/where/... and name/what/...
 * @param[in] nodelist - the hlhdf list
 * @param[in] object - the object to fill
 * @param[in] fmt - the varargs name of the object
 * @param[in] ... - the varargs
 * @return 1 on success otherwise 0
 */
int RaveHL_loadAttributesAndData(HL_NodeList* nodelist, void* object, RaveHL_attr_f attrf, RaveHL_data_f dataf, const char* fmt, ...);

#endif /* RAVE_HLHDF_UTILITIES_H */
