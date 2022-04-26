/* --------------------------------------------------------------------
Copyright (C) 2009-2010 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Generic field that only provides a 2-dim data field and a number of dynamic
 * attributes. This object supports cloning.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-07-05
 */
#ifndef RAVE_FIELD_H
#define RAVE_FIELD_H
#include "rave_object.h"
#include "rave_types.h"
#include "rave_attribute.h"
#include "rave_list.h"
#include "raveobject_list.h"
#include "rave_data2d.h"
#include "lazy_dataset.h"
/**
 * Defines a Rave field
 */
typedef struct _RaveField_t RaveField_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType RaveField_TYPE;

/**
 * Sets the data in the rave field.
 * @param[in] field - self
 * @param[in] xsize - the xsize
 * @param[in] ysize - the ysize
 * @param[in] data - the data
 * @param[in] type - the data type
 * @returns 1 on success otherwise 0
 */
int RaveField_setData(RaveField_t* field, long xsize, long ysize, void* data, RaveDataType type);

/**
 * Sets a lazy dataset as data member. On any requests to receive data, the lazy dataset will be used to populate
 * the internal data field.
 * @param[in]
 */
int RaveField_setLazyDataset(RaveField_t* field, LazyDataset_t* lazyDataset);

/**
 * Creates a empty data field
 * @param[in] field - self
 * @param[in] xsize - the xsize
 * @param[in] ysize - the ysize
 * @param[in] type - the data type
 * @returns 1 on success otherwise 0
 */
int RaveField_createData(RaveField_t* field, long xsize, long ysize, RaveDataType type);

/**
 * Sets the rave data 2d field. This will create a clone from the provided data field.
 * @param[in] field - self
 * @param[in] datafield - the data field to use (MAY NOT BE NULL)
 * @return 1 on success otherwise 0
 */
int RaveField_setDatafield(RaveField_t* field, RaveData2D_t* datafield);

/**
 * Returns a pointer to the internal data storage.
 * @param[in] field - self
 * @return the internal data pointer (NOTE! Do not release this pointer)
 */
void* RaveField_getData(RaveField_t* field);

/**
 * Returns the 2d field associated with this rave field. Note, it is a
 * clone so don't expect that any modifications will modify the rave fields
 * data array.
 * @param[in] field - self
 * @returns a clone of the internal data array on success otherwise NULL
 */
RaveData2D_t* RaveField_getDatafield(RaveField_t* field);

/**
 * Returns the value at the specified index.
 * @param[in] field - self
 * @param[in] x - the x-pos / bin index
 * @param[in] y - the y-pos / ray index
 * @param[out] v - the data at the specified index
 * @return 1 on success, 0 otherwise
 */
int RaveField_getValue(RaveField_t* field, long x, long y, double* v);

/**
 * Sets the value at specified position
 * @param[in] field - self
 * @param[in] x - x coordinate
 * @param[in] y - y coordinate
 * @param[in] value - the value to be set at specified coordinate
 */
int RaveField_setValue(RaveField_t* field, long x, long y, double value);

/**
 * Returns the converted value at the specified index. Corresponds to getAttribute("what/offset") + getAttribute("what/gain") * getValue(x,y).
 * @param[in] field - self
 * @param[in] x - the x-pos / bin index
 * @param[in] y - the y-pos / ray index
 * @param[out] v - the data at the specified index
 * @return 1 on success, 0 otherwise
 */
int RaveField_getConvertedValue(RaveField_t* field, long x, long y, double* v);

/**
 * Returns the xsize / number of bins
 * @param[in] field - self
 * @return the xsize / number of bins
 */
long RaveField_getXsize(RaveField_t* field);

/**
 * Returns the ysize / number of rays
 * @param[in] field - self
 * @return the ysize / number of rays
 */
long RaveField_getYsize(RaveField_t* field);

/**
 * Returns the data type
 * @param[in] field - self
 * @return the data type
 */
RaveDataType RaveField_getDataType(RaveField_t* field);

/**
 * Adds a rave attribute to the parameter.
 * @param[in] field - self
 * @param[in] attribute - the attribute
 * @return 1 on success otherwise 0
 */
int RaveField_addAttribute(RaveField_t* field,  RaveAttribute_t* attribute);

/**
 * Adds a rave attribute to the parameter.
 * NOTE! This method is usually only used internally.
 * @param[in] field - self
 * @param[in] attribute - the attribute
 * @param[in] version - the version of the attribute added
 * @return 1 on success otherwise 0
 */
int RaveField_addAttributeVersion(RaveField_t* field,  RaveAttribute_t* attribute, RaveIO_ODIM_Version version);

/**
 * Returns the rave attribute that is named accordingly.
 * @param[in] field - self
 * @param[in] name - the name of the attribute
 * @returns the attribute if found otherwise NULL
 */
RaveAttribute_t* RaveField_getAttribute(RaveField_t* field, const char* name);

/**
 * Returns if the specified attribute exists.
 * @param[in] field - self
 * @param[in] name - the name of the attribute
 * @returns 1 if attribute exists, otherwise 0
 */
int RaveField_hasAttribute(RaveField_t* field, const char* name);

/**
 * Returns a list of attribute names. Release with \@ref #RaveList_freeAndDestroy.
 * @param[in] field - self
 * @returns a list of attribute names
 */
RaveList_t* RaveField_getAttributeNames(RaveField_t* field);

/**
 * Returns a list of attribute names. Release with \@ref #RaveList_freeAndDestroy.
 * NOTE! This method is usually only used internally.
 * @param[in] field - self
 * @param[in] version - version requested
 * @returns a list of attribute names
 */
RaveList_t* RaveField_getAttributeNamesVersion(RaveField_t* field, RaveIO_ODIM_Version version);

/**
 * Returns a list of attribute values that has been set for this field
 * @param[in] field - self
 * @returns a list of RaveAttributes.
 */
RaveObjectList_t* RaveField_getAttributeValues(RaveField_t* field);

/**
 * Returns a list of attribute values that has been set for this field and version.
 * NOTE! This method is usually only used internally.
 * @param[in] field - self
 * @param[in] version - version requested
 * @returns a list of RaveAttributes.
 */
RaveObjectList_t* RaveField_getAttributeValuesVersion(RaveField_t* field, RaveIO_ODIM_Version version);

/**
 * Returns a reference to the internally stored attributes.
 * NOTE! This method is usually only used internally.
 * @param[in] field - self
 * @returns a list of RaveAttributes.
 */
RaveObjectList_t* RaveField_getInternalAttributeValues(RaveField_t* field);

/**
 * Removes all attributes from the field.
 */
void RaveField_removeAttributes(RaveField_t* field);

/**
 * Checks if the field has the attribute named name that is of type string
 * and has a matching value.
 * @param[in] field - the field
 * @param[in] name - the name of the attribute
 * @param[in] value - the value of the attribute
 * @return 1 if there is such a match, otherwise 0
 */
int RaveField_hasAttributeStringValue(RaveField_t* field, const char* name, const char* value);

/**
 * Concatenates field with other horizontally and returns the new field.
 * The fields and others y-dimension must be the same as well as the data
 * type.
 * @param[in] field - self
 * @param[in] other - the field to contatenate
 * @returns the concatenated field on success otherwise NULL
 */
RaveField_t* RaveField_concatX(RaveField_t* field, RaveField_t* other);

/**
 * Circular shift of the internal field in x & y dimension.
 * @param[in] field - the field to be shifted
 * @param[in] nx - the number of steps to be shifted in x-direction. Can be both positive and negative
 * @param[in] ny - the number of steps to be shifted in y-direction. Can be both positive and negative
 * @returns 1 if shift was successful otherwise 0
 */
int RaveField_circshiftData(RaveField_t* field, int nx, int ny);

#endif /* RAVE_FIELD_H */
