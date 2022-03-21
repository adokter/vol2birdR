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
 * Used for keeping track on attributes.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-06-03
 */
#ifndef RAVE_ATTRIBUTE_H
#define RAVE_ATTRIBUTE_H
#include "rave_object.h"
#include "rave_types.h"

typedef enum RaveAttribute_Format {
  RaveAttribute_Format_Undefined = -1, /**< Undefined */
  RaveAttribute_Format_String = 0,     /**< String */
  RaveAttribute_Format_Long = 1,       /**< Long */
  RaveAttribute_Format_Double = 2,     /**< Double */
  RaveAttribute_Format_LongArray = 3,  /**< Simple 1-dimensional array of longs */
  RaveAttribute_Format_DoubleArray = 4 /**< Simple 1-dimensional array of doubles */
} RaveAttribute_Format;

/**
 * Defines a Geographical Area
 */
typedef struct _RaveAttribute_t RaveAttribute_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType RaveAttribute_TYPE;

/**
 * Sets the name of this attribute
 * @param[in] attr - self
 * @param[in] name - the name this attribute should have
 * @returns 1 on success otherwise 0
 */
int RaveAttribute_setName(RaveAttribute_t* attr, const char* name);

/**
 * Returns the name of this attribute
 * @param[in] attr - self
 * @returns the name of this attribute
 */
const char* RaveAttribute_getName(RaveAttribute_t* attr);

/**
 * Returns the format for this attribute
 * @param[in] attr - self
 * @returns the format
 */
RaveAttribute_Format RaveAttribute_getFormat(RaveAttribute_t* attr);

/**
 * Sets the value as a long.
 * @param[in] attr - self
 * @param[in] value - the value
 */
void RaveAttribute_setLong(RaveAttribute_t* attr, long value);

/**
 * Sets the value as a double.
 * @param[in] attr - self
 * @param[in] value - the value
 */
void RaveAttribute_setDouble(RaveAttribute_t* attr, double value);

/**
 * Sets the value as a string
 * @param[in] attr - self
 * @param[in] value - the value
 * @return 1 on success otherwise 0
 */
int RaveAttribute_setString(RaveAttribute_t* attr, const char* value);

/**
 * Sets the value as a simple 1-dimensional long array.
 * @param[in] attr - self
 * @param[in] value - the value
 * @param[in] len - the number of longs in the array
 * @returns 1 on success otherwise 0
 */
int RaveAttribute_setLongArray(RaveAttribute_t* attr, long* value, int len);

/**
 * Sets the value as a simple 1-dimensional double array.
 * @param[in] attr - self
 * @param[in] value - the value
 * @param[in] len - the number of doubles in the array
 * @returns 1 on success otherwise 0
 */
int RaveAttribute_setDoubleArray(RaveAttribute_t* attr, double* value, int len);

/**
 * Sets the attribute with the array from the provided data with the specified type and converts it into an
 * appropriate array type.
 * @param[in] attr - self
 * @param[in] name - the name of the attribute
 * @param[in] value - the data
 * @param[in] len - the number of items in the array
 * @param[in] type - the data type
 * @return 1 on success otherwise 0
 */
int RaveAttribute_setArrayFromData(RaveAttribute_t* attr, void* value, int len, RaveDataType type);

/**
 * Returns the value as a long.
 * @param[in] attr - self
 * @param[out] value - the long value
 * @returns 1 on success or 0 if format of the data not is a long
 */
int RaveAttribute_getLong(RaveAttribute_t* attr, long* value);

/**
 * Returns the value as a double.
 * @param[in] attr - self
 * @param[out] value - the double value
 * @returns 1 on success or 0 if format of the data not is a double
 */
int RaveAttribute_getDouble(RaveAttribute_t* attr, double* value);

/**
 * Returns the value as a string.
 * @param[in] attr - self
 * @param[out] value - the internal 0-terminated string, DO NOT RELEASE memory
 * @returns 1 on success or 0 if format of the data not is a string
 */
int RaveAttribute_getString(RaveAttribute_t* attr, char** value);

/**
 * Returns the value as a long array.
 * @param[in] attr - self
 * @param[out] value - the internal long array, DO NOT RELEASE memory
 * @param[out] len - the number of values in the array
 * @returns 1 on success or 0 if format of the data not is a long array
 */
int RaveAttribute_getLongArray(RaveAttribute_t* attr, long** value, int* len);

/**
 * Returns the value as a double array.
 * @param[in] attr - self
 * @param[out] value - the internal double array, DO NOT RELEASE memory
 * @param[out] len - the number of values in the array
 * @returns 1 on success or 0 if format of the data not is a double array
 */
int RaveAttribute_getDoubleArray(RaveAttribute_t* attr, double** value, int* len);

/**
 * Performs a circular shift of the array. if nx < 0, then shift is performed counter clockwise, if nx > 0, shift is performed clock wise, if 0, no shift is performed.
 * @param[in] attr - attribute to shift
 * @param[in] nx - number of positions to shift
 * return 1 if successful, 0 if trying to shift an attribute that isn't an array or an error occurs during shift.
 */
int RaveAttribute_shiftArray(RaveAttribute_t* attr, int nx);

/**
 * Helper function for extracting the group and name part from a
 * string with the format <group>/<name>.
 * @param[in] attrname - the string that should get group and name extracted
 * @param[out] group   - the group name (allocated memory so free it)
 * @param[out] name    - the attr name (allocated memory so free it)
 * @returns 1 on success otherwise 0
 */
int RaveAttributeHelp_extractGroupAndName(
  const char* attrname, char** group, char** name);

/**
 * Validates an attribute name that resides in a how-main group. This will validate
 * that it may exists sub groups and that this name is valid. gname should be how, then aname is the trailing part For example:
 * gname=how, aname=attribute
 * gname=how, aname=subgroup1/attribute
 * gname=how, aname=subgroup1/subgroup2/attribute
 * are all valid names.
 * @param[in] gname - should always be how
 * @param[in] aname - the rest of the name as described above.
 * @returns 1 on success otherwise 0
 */
int RaveAttributeHelp_validateHowGroupAttributeName(const char* gname, const char* aname);

/**
 * Creates a named rave attribute.
 * @param[in] name - the name of the attribute
 * @returns the attribute on success otherwise NULL
 */
RaveAttribute_t* RaveAttributeHelp_createNamedAttribute(const char* name);

/**
 * Creates a long rave attribute.
 * @param[in] name - the name of the attribute
 * @param[in] value - the long
 * @returns the attribute on success otherwise NULL
 */
RaveAttribute_t* RaveAttributeHelp_createLong(const char* name, long value);

/**
 * Creates a double rave attribute.
 * @param[in] name - the name of the attribute
 * @param[in] value - the double
 * @returns the attribute on success otherwise NULL
 */
RaveAttribute_t* RaveAttributeHelp_createDouble(const char* name, double value);

/**
 * Creates a double rave attribute from a string representation of the double value
 * @param[in] name - the name of the attribute
 * @param[in] value - the double
 * @returns the attribute on success otherwise NULL
 */
RaveAttribute_t* RaveAttributeHelp_createDoubleFromString(const char* name, const char* value);

/**
 * Creates a string rave attribute.
 * @param[in] name - the name of the attribute
 * @param[in] value - the string
 * @returns the attribute on success otherwise NULL
 */
RaveAttribute_t* RaveAttributeHelp_createString(const char* name, const char* value);

/**
 * Creates a long array rave attribute.
 * @param[in] name - the name of the attribute
 * @param[in] value - the long array
 * @param[in] len - the length of the array
 * @returns the attribute on success otherwise NULL
 */
RaveAttribute_t* RaveAttributeHelp_createLongArray(const char* name, long* value, int len);

/**
 * Creates a double array rave attribute.
 * @param[in] name - the name of the attribute
 * @param[in] value - the double array
 * @param[in] len - the length of the array
 * @returns the attribute on success otherwise NULL
 */
RaveAttribute_t* RaveAttributeHelp_createDoubleArray(const char* name, double* value, int len);

/**
 * Creates an array from the provided data with the specified type and converts it into an
 * appropriate array type.
 * @param[in] name - the name of the attribute
 * @param[in] value - the data
 * @param[in] len - the number of items in the array
 * @param[in] type - the data type
 * @return a rave attribute on success otherwise NULL
 */
RaveAttribute_t* RaveAttributeHelp_createArrayFromData(const char* name, void* value, int len, RaveDataType type);


#endif /* RAVE_ATTRIBUTE_H */

