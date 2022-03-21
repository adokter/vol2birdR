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
 * Defines the functions available when working with cartesian products.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 *
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-10-16
 */
#ifndef CARTESIAN_H
#define CARTESIAN_H
#include "rave_proj.h"
#include "projection.h"
#include "area.h"
#include "rave_object.h"
#include "rave_types.h"
#include "rave_list.h"
#include "raveobject_list.h"
#include "rave_attribute.h"
#include "rave_field.h"
#include "cartesianparam.h"

/**
 * Defines a Cartesian product
 */
typedef struct _Cartesian_t Cartesian_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType Cartesian_TYPE;

/**
 * Sets the nominal time.
 * @param[in] cartesian - self
 * @param[in] value - the time in the format HHmmss
 * @returns 1 on success, otherwise 0
 */
int Cartesian_setTime(Cartesian_t* cartesian, const char* value);

/**
 * Returns the nominal time.
 * @param[in] cartesian - self
 * @returns the nominal time (or NULL if there is none)
 */
const char* Cartesian_getTime(Cartesian_t* cartesian);

/**
 * Sets the nominal date.
 * @param[in] cartesian - self
 * @param[in] value - the date in the format YYYYMMDD
 * @returns 1 on success, otherwise 0
 */
int Cartesian_setDate(Cartesian_t* cartesian, const char* value);

/**
 * Returns the nominal date.
 * @param[in] cartesian - self
 * @returns the nominal date (or NULL if there is none)
 */
const char* Cartesian_getDate(Cartesian_t* cartesian);

/**
 * Sets the start time.
 * @param[in] cartesian - self
 * @param[in] value - the time in the format HHmmss
 * @returns 1 on success, otherwise 0
 */
int Cartesian_setStartTime(Cartesian_t* cartesian, const char* value);

/**
 * Returns the start time.
 * @param[in] cartesian - self
 * @returns the start time (or NULL if there is none)
 */
const char* Cartesian_getStartTime(Cartesian_t* cartesian);

/**
 * Sets the start date.
 * @param[in] cartesian - self
 * @param[in] value - the date in the format YYYYMMDD
 * @returns 1 on success, otherwise 0
 */
int Cartesian_setStartDate(Cartesian_t* cartesian, const char* value);

/**
 * Returns the start date.
 * @param[in] cartesian - self
 * @returns the start date (or NULL if there is none)
 */
const char* Cartesian_getStartDate(Cartesian_t* cartesian);

/**
 * Sets the end time.
 * @param[in] cartesian - self
 * @param[in] value - the time in the format HHmmss
 * @returns 1 on success, otherwise 0
 */
int Cartesian_setEndTime(Cartesian_t* cartesian, const char* value);

/**
 * Returns the end time.
 * @param[in] cartesian - self
 * @returns the end time (or NULL if there is none)
 */
const char* Cartesian_getEndTime(Cartesian_t* cartesian);

/**
 * Sets the end date.
 * @param[in] cartesian - self
 * @param[in] value - the date in the format YYYYMMDD
 * @returns 1 on success, otherwise 0
 */
int Cartesian_setEndDate(Cartesian_t* cartesian, const char* value);

/**
 * Returns the end date.
 * @param[in] cartesian - self
 * @returns the end date (or NULL if there is none)
 */
const char* Cartesian_getEndDate(Cartesian_t* cartesian);

/**
 * Sets the source.
 * @param[in] cartesian - self
 * @param[in] value - the source
 * @returns 1 on success, otherwise 0
 */
int Cartesian_setSource(Cartesian_t* cartesian, const char* value);

/**
 * Returns the source.
 * @param[in] cartesian - self
 * @returns the source or NULL if there is none
 */
const char* Cartesian_getSource(Cartesian_t* cartesian);

/**
 * Sets the product name.
 * @param[in] cartesian - self
 * @param[in] value - the product name
 * @returns 1 on success, otherwise 0
 */
int Cartesian_setProdname(Cartesian_t* cartesian, const char* value);

/**
 * Returns the product name.
 * @param[in] cartesian - self
 * @returns the product name or NULL if there is none
 */
const char* Cartesian_getProdname(Cartesian_t* cartesian);

/**
 * Sets the object type this cartesian product should represent.
 * @param[in] self - self
 * @param[in] type - the object type
 * @returns 1 if the specified object type is supported, otherwise 0
 */
int Cartesian_setObjectType(Cartesian_t* self, Rave_ObjectType type);

/**
 * Returns the object type this cartesian product represents.
 * @param[in] self - self
 * @returns the object type
 */
Rave_ObjectType Cartesian_getObjectType(Cartesian_t* self);

/**
 * The xsize to use for the parameters
 * @param[in] self - self
 * @param[in] xsize - the xsize to use
 */
void Cartesian_setXSize(Cartesian_t* self, long xsize);

/**
 * The ysize to use for the parameters
 * @param[in] self - self
 * @param[in] ysize - the ysize to use
 */
void Cartesian_setYSize(Cartesian_t* self, long ysize);

/**
 * Returns the xsize
 * @param[in] cartesian - the cartesian product
 * @return the xsize
 */
long Cartesian_getXSize(Cartesian_t* cartesian);

/**
 * Returns the ysize
 * @param[in] cartesian - the cartesian product
 * @return the ysize
 */
long Cartesian_getYSize(Cartesian_t* cartesian);

/**
 * Sets the area extent for this cartesian product.
 * @param[in] cartesian - the cartesian product
 * @param[in] llX - lower left X position
 * @param[in] llY - lower left Y position
 * @param[in] urX - upper right X position
 * @param[in] urY - upper right Y position
 */
void Cartesian_setAreaExtent(Cartesian_t* cartesian, double llX, double llY, double urX, double urY);

/**
 * Gets the area extent for this cartesian product.
 * @param[in] cartesian - the cartesian product
 * @param[out] llX - lower left X position (may be NULL)
 * @param[out] llY - lower left Y position (may be NULL)
 * @param[out] urX - upper right X position (may be NULL)
 * @param[out] urY - upper right Y position (may be NULL)
 */
void Cartesian_getAreaExtent(Cartesian_t* cartesian, double* llX, double* llY, double* urX, double* urY);

/**
 * Determines the extreme lon lat boundaries for this area. I.e. the outer boundaries of this cartesian image
 * will be steped over until the absolute min/max lon/lat positions are found for this image.
 * Note, that the bounding box returned will be in a different setup than area extent
 * @param[in] self - self
 * @param[out] ulLon - Upper left longitude
 * @param[out] ulLat - Upper left latitude
 * @param[out] lrLon - Lower right longitude
 * @param[out] lrLat - Lower right latitude
 * @returns 1 if the boundaries could be determined otherwise 0
 *
 */
int Cartesian_getExtremeLonLatBoundaries(Cartesian_t* self, double* ulLon, double* ulLat, double* lrLon, double* lrLat);

/**
 * Sets the xscale
 * @param[in] cartesian - the cartesian product
 * @param[in] xscale - the xscale
 */
void Cartesian_setXScale(Cartesian_t* cartesian, double xscale);

/**
 * Returns the xscale
 * @param[in] cartesian - the cartesian product
 * @return the xscale
 */
double Cartesian_getXScale(Cartesian_t* cartesian);

/**
 * Sets the yscale
 * @param[in] cartesian - the cartesian product
 * @param[in] yscale - the yscale
 */
void Cartesian_setYScale(Cartesian_t* cartesian, double yscale);

/**
 * Returns the yscale
 * @param[in] cartesian - the cartesian product
 * @return the yscale
 */
double Cartesian_getYScale(Cartesian_t* cartesian);

/**
 * Sets the product this cartesian represents.
 * @param[in] cartesian  self
 * @param[in] type - the product type
 * @returns 1 if the operation was successful, otherwise 0
 */
int Cartesian_setProduct(Cartesian_t* cartesian, Rave_ProductType type);

/**
 * Returns the product this cartesian represents.
 * @param[in] cartesian - self
 * @returns the product type
 */
Rave_ProductType Cartesian_getProduct(Cartesian_t* cartesian);

/**
 * Returns the nodata value
 * @param[in] self - self
 * @return the nodata value
 */
double Cartesian_getNodata(Cartesian_t* self);

/**
 * Returns the undetect value
 * @param[in] self - self
 * @return the undetect value
 */
double Cartesian_getUndetect(Cartesian_t* self);

/**
 * Returns the location within the area as identified by a x-position.
 * Evaluated as: upperLeft.x + xscale * x
 * @param[in] cartesian - the cartesian product
 * @param[in] x - the x position in the area definition
 * @returns the x location
 */
double Cartesian_getLocationX(Cartesian_t* cartesian, long x);

/**
 * Returns the location within the area as identified by a y-position.
 * Evaluated as: upperLeft.y - yscale * y
 * @param[in] cartesian - the cartesian product
 * @param[in] y - the y position in the area definition
 * @returns the y location
 */
double Cartesian_getLocationY(Cartesian_t* cartesian, long y);

/**
 * Returns the x index
 * Evaluated as: (x - lowerLeft.x)/xscale
 * @param[in] cartesian - the cartesian product
 * @param[in] x - the x position in the area definition
 * @returns the x index
 */
long Cartesian_getIndexX(Cartesian_t* cartesian, double x);

/**
 * Returns the y index
 * Evaluated as: (upperRight.y - y)/yscale
 * @param[in] cartesian - the cartesian product
 * @param[in] y - the y position in the area definition
 * @returns the y index
 */
long Cartesian_getIndexY(Cartesian_t* cartesian, double y);

/**
 * Sets the default parameter
 * @param[in] self - self
 * @param[in] name - the quantity, e.g. DBZH
 * @returns 1 on success, otherwise 0
 */
int Cartesian_setDefaultParameter(Cartesian_t* self, const char* name);

/**
 * Returns the default parameter
 * @param[in] self - self
 * @return the default parameter
 */
const char* Cartesian_getDefaultParameter(Cartesian_t* self);

/**
 * Sets the projection that defines this cartesian product. Will also necessary
 * pipelines internally
 * @param[in] cartesian - the cartesian product
 * @param[in] projection - the projection
 * @return 1 if operation successful, otherwise 0
 */
int Cartesian_setProjection(Cartesian_t* cartesian, Projection_t* projection);

/**
 * Returns a copy of the projection that is used for this cartesian product.
 * I.e. remember to release it.
 * @param[in] cartesian - the cartesian product
 * @returns a projection (or NULL if none is set)
 */
Projection_t* Cartesian_getProjection(Cartesian_t* cartesian);

/**
 * Returns the projection string defining this cartesian product.
 * @param[in] cartesian - self
 * @return the projection string or NULL if none defined
 */
const char* Cartesian_getProjectionString(Cartesian_t* cartesian);

/**
 * Sets the value at the specified coordinates.
 * @param[in] cartesian - the cartesian product
 * @param[in] x - the x-position
 * @param[in] y - the y-position
 * @param[in] v - the value to set
 * @return 1 on success, otherwise 0
 */
int Cartesian_setValue(Cartesian_t* cartesian, long x, long y, double v);

/**
 * Scales the value v according to gain and offset before setting it.
 * I.e. same as Cartesian_setValue(cartesian, x, y, (v - offset)/gain)
 */
int Cartesian_setConvertedValue(Cartesian_t* cartesian, long x, long y, double v);

/**
 * Returns the value at the specified x and y position.
 * @param[in] cartesian - the cartesian product
 * @param[in] x - the x index
 * @param[in] y - the y index
 * @param[out] v - the data at the specified index
 * @return the type of data
 */
RaveValueType Cartesian_getValue(Cartesian_t* cartesian, long x, long y, double* v);

/**
 * Returns the converted value at the specified x and y position.
 * @param[in] cartesian - the cartesian product
 * @param[in] x - the x index
 * @param[in] y - the y index
 * @param[out] v - the data at the specified index
 * @return the type of data
 */
RaveValueType Cartesian_getConvertedValue(Cartesian_t* cartesian, long x, long y, double* v);

/**
 * Returns the value from the location as defined by the area definition. Same as calling
 * Cartesian_getValue(c, Cartesian_getIndexX(c), Cartesian_getIndexY(c), &v).
 * @param[in] cartesian - self
 * @param[in] lx - the position as defined in the area definition
 * @param[in] ly - the position as defined in the area definition
 * @param[out] v - the data at the specified position
 * @return the type of data
 */
RaveValueType Cartesian_getValueAtLocation(Cartesian_t* cartesian, double lx, double ly, double* v);

/**
 * Returns the converted value from the location as defined by the area definition. Same as calling
 * Cartesian_getConvertedValue(c, Cartesian_getIndexX(c), Cartesian_getIndexY(c), &v).
 * @param[in] cartesian - self
 * @param[in] lx - the position as defined in the area definition
 * @param[in] ly - the position as defined in the area definition
 * @param[out] v - the data at the specified position
 * @return the type of data
 */
RaveValueType Cartesian_getConvertedValueAtLocation(Cartesian_t* cartesian, double lx, double ly, double* v);

/**
 * Returns the converted value from the lon/lat position within the area.
 *
 * @param[in] cartesian - self
 * @param[in] lon - the longitude (in radians)
 * @param[in] lat - the latitude (in radians)
 * @param[out] v - the data at the specified position
 * @return the type of data
 */
RaveValueType Cartesian_getConvertedValueAtLonLat(Cartesian_t* cartesian, double lon, double lat, double* v);

/**
 * Returns the quality value at the specified location from the specified quality field. First the code
 * tests if the quality field exist in the default param. If not, it will check for the quality field
 * in self.
 * @param[in] cartesian - self
 * @param[in] lx - the position as defined in the area definition
 * @param[in] ly - the position as defined in the area definition
 * @param[in] name - the name of the quality field (how/task)
 * @param[out] v - the data at the specified position
 * @return 1 if value could be returned otherwise 0
 */
int Cartesian_getQualityValueAtLocation(Cartesian_t* cartesian, double lx, double ly, const char* name, double *v);

/**
 * Returns the scaled quality value at the specified location from the specified quality field. First the code
 * tests if the quality field exist in the default param. If not, it will check for the quality field
 * in self.
 * @param[in] cartesian - self
 * @param[in] lx - the position as defined in the area definition
 * @param[in] ly - the position as defined in the area definition
 * @param[in] name - the name of the quality field (how/task)
 * @param[out] v - the data at the specified position
 * @return 1 if value could be returned otherwise 0
 */
int Cartesian_getConvertedQualityValueAtLocation(Cartesian_t* cartesian, double lx, double ly, const char* name, double *v);

/**
 * Returns the quality value at the specified lon/lat from the specified quality field. First the code
 * tests if the quality field exist in the default param. If not, it will check for the quality field
 * in self.
 * @param[in] cartesian - self
 * @param[in] lon - the longitude in radians
 * @param[in] lat - the latitude in radians
 * @param[in] name - the name of the quality field (how/task)
 * @param[out] v - the data at the specified position
 * @return 1 if value could be returned otherwise 0
 */
int Cartesian_getQualityValueAtLonLat(Cartesian_t* cartesian, double lon, double lat, const char* name, double *v);

/**
 * Returns the scaled quality value at the specified lon/lat from the specified quality field. That is the value
 * got when applying gain and offset.
 * First the code tests if the quality field exist in the default param. If not, it will check for the quality field
 * in self.
 * @param[in] cartesian - self
 * @param[in] lon - the longitude in radians
 * @param[in] lat - the latitude in radians
 * @param[in] name - the name of the quality field (how/task)
 * @param[out] v - the data at the specified position
 * @return 1 if value could be returned otherwise 0
 */
int Cartesian_getConvertedQualityValueAtLonLat(Cartesian_t* cartesian, double lon, double lat, const char* name, double *v);

/**
 * Initializes this cartesian product with basic information. No parameter is created but
 * the dimensions and projection information is setup.
 * @param[in] cartesian - self
 * @param[in] area - the area
 * @returns 1 on success, otherwise 0
 */
void Cartesian_init(Cartesian_t* cartesian, Area_t* area);

/**
 * Returns the mean value over a NxN square around the specified x and y position.
 * @param[in] cartesian - the cartesian product
 * @param[in] x - the x index
 * @param[in] y - the y index
 * @param[in] N - the N size
 * @param[out] v - the data at the specified index
 * @return the type of data
 */
RaveValueType Cartesian_getMean(Cartesian_t* cartesian, long x, long y, int N, double* v);

/**
 * Verifies that all preconditions are met in order to perform
 * a transformation.
 * @param[in] cartesian - the cartesian product
 * @returns 1 if the cartesian product is ready, otherwise 0.
 */
int Cartesian_isTransformable(Cartesian_t* cartesian);

/**
 * Adds a rave attribute to the cartesian product. If attribute maps to the
 * member attributes it will be used to set the specific member
 * instead.
 * @param[in] cartesian - self
 * @param[in] attribute - the attribute
 * @return 1 on success otherwise 0
 */
int Cartesian_addAttribute(Cartesian_t* cartesian, RaveAttribute_t* attribute);

/**
 * Returns the rave attribute that is named accordingly.
 * @param[in] cartesian - self
 * @param[in] name - the name of the attribute
 * @returns the attribute if found otherwise NULL
 */
RaveAttribute_t* Cartesian_getAttribute(Cartesian_t* cartesian, const char* name);

/**
 * Returns a list of attribute names. Release with \@ref #RaveList_freeAndDestroy.
 * @param[in] cartesian - self
 * @returns a list of attribute names
 */
RaveList_t* Cartesian_getAttributeNames(Cartesian_t* cartesian);

/**
 * Returns a list of attribute values that should be stored for this cartesian product.
 * Corresponding members will also be added as attribute values.
 * @param[in] cartesian - self
 * @param[in] otype - what type of attributes that should be returned, if it is for a cartesian image or a image
 * belonging to a cartesian volume
 * @returns a list of RaveAttributes.
 */
RaveObjectList_t* Cartesian_getAttributeValues(Cartesian_t* cartesian);

/**
 * Returns if the cartesian product has got the specified attribute.
 * @param[in] cartesian - self
 * @param[in] name - what to look for
 * @returns 1 if the attribute exists, otherwise 0
 */
int Cartesian_hasAttribute(Cartesian_t* cartesian, const char* name);

/**
 * Adds a quality field to this cartesian product.
 * @param[in] cartesian - self
 * @param[in] field - the field to add
 * @returns 1 on success otherwise 0
 */
int Cartesian_addQualityField(Cartesian_t* cartesian, RaveField_t* field);

/**
 * Returns the quality field at the specified location.
 * @param[in] cartesian - self
 * @param[in] index - the index
 * @returns the quality field if found, otherwise NULL
 */
RaveField_t* Cartesian_getQualityField(Cartesian_t* cartesian, int index);

/**
 * Returns the number of quality fields
 * @param[in] cartesian - self
 * @returns the number of quality fields
 */
int Cartesian_getNumberOfQualityFields(Cartesian_t* cartesian);

/**
 * Removes the quality field at the specified location
 * @param[in] cartesian - self
 * @param[in] index - the index
 */
void Cartesian_removeQualityField(Cartesian_t* cartesian, int index);

/**
 * Returns all quality fields belonging to this cartesian. The returned
 * object is only a reference so do not modify it.
 * @param[in] cartesian - self
 * @returns a list of 0 or more quality fields or NULL on error.
 */
RaveObjectList_t* Cartesian_getQualityFields(Cartesian_t* cartesian);

/**
 * Returns a quality field based on the value of how/task that should be a
 * string.
 * @param[in] cartesian - cartesian
 * @param[in] value - the value of the how/task attribute
 * @return the field if found otherwise NULL
 */
RaveField_t* Cartesian_getQualityFieldByHowTask(Cartesian_t* cartesian, const char* name);

/**
 * Same as Cartesian_getQualityFieldByHowTask but it first tries if there are any quality
 * fields in the current parameter before checking self.
 * @param[in] self - self
 */
RaveField_t* Cartesian_findQualityFieldByHowTask(Cartesian_t* self, const char* value);

/**
 * Adds a parameter to the cartesian product. The quantity is used as unique
 * identifier which means that any existing one will be removed.
 * @param[in] self - self
 * @param[in] param - the parameter
 * @return 1 on success
 */
int Cartesian_addParameter(Cartesian_t* self, CartesianParam_t* param);

/**
 * Returns the parameter with the specified quantity.
 * @param[in] self - self
 * @param[in] name - the name of the parameter
 * @return the parameter or NULL
 */
CartesianParam_t* Cartesian_getParameter(Cartesian_t* self, const char* name);

/**
 * Returns if the product contains the specified parameter or not.
 * @param[in] self - self
 * @param[in] name - the quantity name
 * @returns 1 if the parameter exists, otherwise 0
 */
int Cartesian_hasParameter(Cartesian_t* self, const char* name);

/**
 * Removes the parameter with the specified quantity
 * @param[in] self - self
 * @param[in] name - the quantity name
 */
void Cartesian_removeParameter(Cartesian_t* self, const char* name);

/**
 * Return the number of parameters
 * @param[in] self - self
 * @return the number of parameters
 */
int Cartesian_getParameterCount(Cartesian_t* self);

/**
 * Returns a list of parameter names
 * @param[in] self - self
 * @returns a list of parameter names
 */
RaveList_t* Cartesian_getParameterNames(Cartesian_t* self);

/**
 * Creates a parameter. The created parameter will be added to the internal
 * list of parameters. So if you are just going to create one, wrap the call in
 * a RAVE_OBJECT_RELEASE. It is essential that _init has been called in order
 * for this to have any effect.
 * @param[in] self - self
 * @param[in] quantity - the quantity of the created parameter
 * @param[in] type - the data type
 * @param[in] datavalue - value to initialise the data field with
 * @return the created parameter
 */
CartesianParam_t* Cartesian_createParameter(Cartesian_t* self, const char* quantity, RaveDataType type, double datavalue);
#endif
