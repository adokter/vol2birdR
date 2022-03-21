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
 * Defines the functions available when working with a cartesian field.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 *
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2012-01-04
 */
#ifndef CARTESIANPARAM_H
#define CARTESIANPARAM_H
#include "rave_proj.h"
#include "projection.h"
#include "area.h"
#include "rave_object.h"
#include "rave_types.h"
#include "rave_list.h"
#include "raveobject_list.h"
#include "rave_attribute.h"
#include "rave_field.h"

/**
 * Defines a Cartesian product
 */
typedef struct _CartesianParam_t CartesianParam_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType CartesianParam_TYPE;

/**
 * Returns the xsize
 * @param[in] self - the cartesian product
 * @return the xsize
 */
long CartesianParam_getXSize(CartesianParam_t* self);

/**
 * Returns the ysize
 * @param[in] self - the cartesian product
 * @return the ysize
 */
long CartesianParam_getYSize(CartesianParam_t* self);
/**
 * Sets the data type of the data that is worked with
 * @param[in] self - the cartesian product
 * @param[in] type - the data type
 * @return 0 if type is not known, otherwise the type was set
 */
int CartesianParam_setDataType(CartesianParam_t* self, RaveDataType type);

/**
 * Returns the data type
 * @param[in] self - the cartesian product
 * @return the data type
 */
RaveDataType CartesianParam_getDataType(CartesianParam_t* self);

/**
 * Sets the quantity
 * @param[in] self - the cartesian product
 * @param[in] quantity - the quantity, e.g. DBZH
 * @returns 1 on success, otherwise 0
 */
int CartesianParam_setQuantity(CartesianParam_t* self, const char* quantity);

/**
 * Returns the quantity
 * @param[in] self - the cartesian product
 * @return the quantity
 */
const char* CartesianParam_getQuantity(CartesianParam_t* self);

/**
 * Sets the gain.
 * @param[in] self - the cartesian product
 * @param[in] gain - the gain (MAY NOT BE 0.0)
 */
void CartesianParam_setGain(CartesianParam_t* self, double gain);

/**
 * Returns the gain
 * @param[in] self - the cartesian product
 * @return the gain
 */
double CartesianParam_getGain(CartesianParam_t* self);

/**
 * Sets the offset
 * @param[in] self - the cartesian product
 * @param[in] offset - the offset
 */
void CartesianParam_setOffset(CartesianParam_t* self, double offset);

/**
 * Returns the offset
 * @param[in] self - the cartesian product
 * @return the offset
 */
double CartesianParam_getOffset(CartesianParam_t* self);

/**
 * Sets the nodata
 * @param[in] self - the cartesian product
 * @param[in] nodata - the nodata
 */
void CartesianParam_setNodata(CartesianParam_t* self, double nodata);

/**
 * Returns the nodata
 * @param[in] self - the cartesian product
 * @return the nodata
 */
double CartesianParam_getNodata(CartesianParam_t* self);

/**
 * Sets the undetect
 * @param[in] self - the cartesian product
 * @param[in] undetect - the undetect
 */
void CartesianParam_setUndetect(CartesianParam_t* self, double undetect);

/**
 * Returns the undetect
 * @param[in] self - the cartesian product
 * @return the undetect
 */
double CartesianParam_getUndetect(CartesianParam_t* self);

/**
 * Returns if this parameter is transformable. I.e. has data.
 * @param[in] self - self
 * @return 1 if transformable otherwise 0
 */
int CartesianParam_isTransformable(CartesianParam_t* self);

/**
 * Sets the data
 * @param[in] self  - the cartesian product
 * @param[in] xsize - x-size
 * @param[in] ysize - y-size
 * @param[in] data  - the data
 * @param[in] type  - the data type
 * @return 1 on success otherwise 0
 */
int CartesianParam_setData(CartesianParam_t* self, long xsize, long ysize, void* data, RaveDataType type);

/**
 * Sets a lazy dataset as data member. On any requests to receive data, the lazy dataset will be used to populate
 * the internal data field.
 * @param[in]
 */
int CartesianParam_setLazyDataset(CartesianParam_t* self, LazyDataset_t* lazyDataset);

/**
 * Creates data with the provided specification
 * @param[in] self - self
 * @param[in] xsize - x size
 * @param[in] ysize - y size
 * @param[in] type - the data type
 * @param[in] value - initial value to set for all positions in the data field
 * @return 1 on success otherwise 0
 */
int CartesianParam_createData(CartesianParam_t* self, long xsize, long ysize, RaveDataType type, double value);

/**
 * Returns a pointer to the internal data storage.
 * @param[in] self - self
 * @return the internal data pointer (NOTE! Do not release this pointer)
 */
void* CartesianParam_getData(CartesianParam_t* self);

/**
 * Returns the data type
 * @param[in] self - self
 * @return the data type
 */
RaveDataType CartesianParam_getType(CartesianParam_t* self);

/**
 * Sets the value at the specified coordinates.
 * @param[in] self - self
 * @param[in] x - the x-position
 * @param[in] y - the y-position
 * @param[in] v - the value to set
 * @return 1 on success, otherwise 0
 */
int CartesianParam_setValue(CartesianParam_t* self, long x, long y, double v);

/**
 * Scales the value v according to gain and offset before setting it.
 * I.e. same as CartesianParam_setValue(cartesian, x, y, (v - offset)/gain)
 */
int CartesianParam_setConvertedValue(CartesianParam_t* self, long x, long y, double v, RaveValueType vtype);

/**
 * Returns the value at the specified x and y position.
 * @param[in] self - the cartesian product
 * @param[in] x - the x index
 * @param[in] y - the y index
 * @param[out] v - the data at the specified index
 * @return the type of data
 */
RaveValueType CartesianParam_getValue(CartesianParam_t* self, long x, long y, double* v);

/**
 * Returns the converted value at the specified x and y position.
 * @param[in] self - the cartesian product
 * @param[in] x - the x index
 * @param[in] y - the y index
 * @param[out] v - the data at the specified index
 * @return the type of data
 */
RaveValueType CartesianParam_getConvertedValue(CartesianParam_t* self, long x, long y, double* v);

/**
 * Returns the mean value over a NxN square around the specified x and y position.
 * @param[in] self - the cartesian product
 * @param[in] x - the x index
 * @param[in] y - the y index
 * @param[in] N - the N size
 * @param[out] v - the data at the specified index
 * @return the type of data
 */
RaveValueType CartesianParam_getMean(CartesianParam_t* self, long x, long y, int N, double* v);

/**
 * Adds a rave attribute to the cartesian product. If attribute maps to the
 * member attributes it will be used to set the specific member
 * instead.
 * @param[in] self - self
 * @param[in] attribute - the attribute
 * @return 1 on success otherwise 0
 */
int CartesianParam_addAttribute(CartesianParam_t* self, RaveAttribute_t* attribute);

/**
 * Returns the rave attribute that is named accordingly.
 * @param[in] self - self
 * @param[in] name - the name of the attribute
 * @returns the attribute if found otherwise NULL
 */
RaveAttribute_t* CartesianParam_getAttribute(CartesianParam_t* self, const char* name);

/**
 * Returns a list of attribute names. Release with \@ref #RaveList_freeAndDestroy.
 * @param[in] self - self
 * @returns a list of attribute names
 */
RaveList_t* CartesianParam_getAttributeNames(CartesianParam_t* self);

/**
 * Returns a list of attribute values that should be stored for this cartesian product.
 * Corresponding members will also be added as attribute values.
 * @param[in] self - self
 * @param[in] otype - what type of attributes that should be returned, if it is for a cartesian image or a image
 * belonging to a cartesian volume
 * @returns a list of RaveAttributes.
 */
RaveObjectList_t* CartesianParam_getAttributeValues(CartesianParam_t* self);

/**
 * Returns if the cartesian product has got the specified attribute.
 * @param[in] self - self
 * @param[in] name - what to look for
 * @returns 1 if the attribute exists, otherwise 0
 */
int CartesianParam_hasAttribute(CartesianParam_t* self, const char* name);

/**
 * Adds a quality field to this cartesian parameter.
 * @param[in] self - self
 * @param[in] field - the field to add
 * @returns 1 on success otherwise 0
 */
int CartesianParam_addQualityField(CartesianParam_t* self, RaveField_t* field);

/**
 * Returns the quality field at the specified location.
 * @param[in] self - self
 * @param[in] index - the index
 * @returns the quality field if found, otherwise NULL
 */
RaveField_t* CartesianParam_getQualityField(CartesianParam_t* self, int index);

/**
 * Returns the number of quality fields
 * @param[in] self - self
 * @returns the number of quality fields
 */
int CartesianParam_getNumberOfQualityFields(CartesianParam_t* self);

/**
 * Removes the quality field at the specified location
 * @param[in] self - self
 * @param[in] index - the index
 */
void CartesianParam_removeQualityField(CartesianParam_t* self, int index);

/**
 * Returns all quality fields belonging to this cartesian. The returned
 * object is only a reference so do not modify it.
 * @param[in] self - self
 * @returns a list of 0 or more quality fields or NULL on error.
 */
RaveObjectList_t* CartesianParam_getQualityFields(CartesianParam_t* self);

/**
 * Returns a quality field based on the value of how/task that should be a
 * string.
 * @param[in] self - self
 * @param[in] value - the value of the how/task attribute
 * @return the field if found otherwise NULL
 */
RaveField_t* CartesianParam_getQualityFieldByHowTask(CartesianParam_t* self, const char* value);

#endif
