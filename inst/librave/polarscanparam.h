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
 * Defines the functions available when working with one parameter in a polar scan.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-01-22
 */
#ifndef POLARSCANPARAM_H
#define POLARSCANPARAM_H
#include "polarnav.h"
#include "projection.h"
#include "rave_object.h"
#include "rave_types.h"
#include "rave_attribute.h"
#include "rave_list.h"
#include "raveobject_list.h"
#include "rave_field.h"
#include "lazy_dataset.h"

/**
 * Defines a Polar Scan Parameter
 */
typedef struct _PolarScanParam_t PolarScanParam_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType PolarScanParam_TYPE;

/**
 * Sets the quantity
 * @param[in] scanparam - self
 * @param[in] quantity - the quantity, e.g. DBZH
 * @returns 1 on success, otherwise 0
 */
int PolarScanParam_setQuantity(PolarScanParam_t* scanparam, const char* quantity);

/**
 * Returns the quantity
 * @param[in] scanparam - self
 * @return the quantity or NULL if not set
 */
const char* PolarScanParam_getQuantity(PolarScanParam_t* scanparam);

/**
 * Sets the gain
 * @param[in] scanparam - self
 * @param[in] gain - the gain
 */
void PolarScanParam_setGain(PolarScanParam_t* scanparam, double gain);

/**
 * Returns the gain
 * @param[in] scanparam - self
 * @return the gain
 */
double PolarScanParam_getGain(PolarScanParam_t* scanparam);

/**
 * Sets the offset
 * @param[in] scanparam - self
 * @param[in] offset - the offset
 */
void PolarScanParam_setOffset(PolarScanParam_t* scanparam, double offset);

/**
 * Returns the offset
 * @param[in] scanparam - self
 * @return the offset
 */
double PolarScanParam_getOffset(PolarScanParam_t* scanparam);

/**
 * Sets the nodata
 * @param[in] scanparam - self
 * @param[in] nodata - the nodata
 */
void PolarScanParam_setNodata(PolarScanParam_t* scanparam, double nodata);

/**
 * Returns the nodata
 * @param[in] scanparam - self
 * @return the nodata
 */
double PolarScanParam_getNodata(PolarScanParam_t* scanparam);

/**
 * Sets the undetect
 * @param[in] scanparam - self
 * @param[in] undetect - the undetect
 */
void PolarScanParam_setUndetect(PolarScanParam_t* scanparam, double undetect);

/**
 * Returns the undetect
 * @param[in] scanparam - self
 * @return the undetect
 */
double PolarScanParam_getUndetect(PolarScanParam_t* scanparam);

/**
 * Sets the data
 * @param[in] scanparam  - self
 * @param[in] nbins - number of bins
 * @param[in] nrays - number of rays
 * @param[in] data  - the data
 * @param[in] type  - the data type
 * @return 1 on success otherwise 0
 */
int PolarScanParam_setData(PolarScanParam_t* scanparam, long nbins, long nrays, void* data, RaveDataType type);

/**
 * Sets a lazy dataset as data member. On any requests to receive data, the lazy dataset will be used to populate
 * the internal data field.
 * @param[in]
 */
int PolarScanParam_setLazyDataset(PolarScanParam_t* scanparam, LazyDataset_t* lazyDataset);

/**
 * Sets the data from a rave data 2d object
 * @param[in] scanparam - self
 * @param[in] data2d - the data 2d field
 * @return 1 on success otherwise 0
 */
int PolarScanParam_setData2D(PolarScanParam_t* scanparam, RaveData2D_t* data2d);

/**
 * Creates a data field with the specified dimensions and type. The data till be initialized to 0.
 * @param[in] scanparam - self
 * @param[in] nbins - number of bins
 * @param[in] nrays - number of rays
 * @param[in] type - the type of the data
 * @returns 1 on success otherwise 0
 */
int PolarScanParam_createData(PolarScanParam_t* scanparam, long nbins, long nrays, RaveDataType type);

/**
 * Returns a pointer to the internal data storage.
 * @param[in] scanparam - self
 * @return the internal data pointer (NOTE! Do not release this pointer)
 */
void* PolarScanParam_getData(PolarScanParam_t* scanparam);

/**
 * Returns a copy of the internal 2d data field.
 * @param[in] scanparam - self
 * @return a copy of the internal 2d data field
 */
RaveData2D_t* PolarScanParam_getData2D(PolarScanParam_t* scanparam);

/**
 * Returns the number of bins
 * @param[in] scanparam - self
 * @return the number of bins
 */
long PolarScanParam_getNbins(PolarScanParam_t* scanparam);

/**
 * Returns the number of rays/scan
 * @param[in] scanparam - self
 * @return the number of rays
 */
long PolarScanParam_getNrays(PolarScanParam_t* scanparam);

/**
 * Returns the data type
 * @param[in] scanparam - self
 * @return the data type
 */
RaveDataType PolarScanParam_getDataType(PolarScanParam_t* scan);

/**
 * Returns the value at the specified index.
 * @param[in] scanparam - self
 * @param[in] bin - the bin index
 * @param[in] ray - the ray index
 * @param[out] v - the data at the specified index
 * @return the type of data
 */
RaveValueType PolarScanParam_getValue(PolarScanParam_t* scanparam, int bin, int ray, double* v);

/**
 * Returns the linear converted value at the specified index. That is,
 * offset + gain * value;
 * @param[in] scanparam - self
 * @param[in] bin - the bin index
 * @param[in] ray - the ray index
 * @param[out] v - the data at the specified index
 * @return the type of data
 */
RaveValueType PolarScanParam_getConvertedValue(PolarScanParam_t* scanparam, int bin, int ray, double* v);

/**
 * Sets the value
 * @param[in] scanparam - self
 * @param[in] bin - the bin index
 * @param[in] ray - the ray index
 * @param[in] v - the value (converted)
 * @return 1 on success or 0 on failure
 */
int PolarScanParam_setValue(PolarScanParam_t* scanparam, int bin, int ray, double v);

/**
 * Adds a rave attribute to the parameter.
 * @param[in] scanparam - self
 * @param[in] attribute - the attribute
 * @return 1 on success otherwise 0
 */
int PolarScanParam_addAttribute(PolarScanParam_t* scanparam,
  RaveAttribute_t* attribute);

/**
 * Adds a rave attribute to the parameter.
 * NOTE! This method is usually only used internally.
 * @param[in] scanparam - self
 * @param[in] attribute - the attribute
 * @param[in] version - the version of the attribute added
 * @return 1 on success otherwise 0
 */
int PolarScanParam_addAttributeVersion(PolarScanParam_t* scanparam, RaveAttribute_t* attribute, RaveIO_ODIM_Version version);

/**
 * Returns the rave attribute that is named accordingly.
 * @param[in] scanparam - self
 * @param[in] name - the name of the attribute
 * @returns the attribute if found otherwise NULL
 */
RaveAttribute_t* PolarScanParam_getAttribute(PolarScanParam_t* scanparam,  const char* name);

/**
 * Returns the rave attribute that is named accordingly and version.
 * NOTE! This method is usually only used internally.
 * @param[in] scanparam - self
 * @param[in] name - the name of the attribute
 * @param[in] version - the version of the attribute
 * @returns the attribute if found otherwise NULL
 */
RaveAttribute_t* PolarScanParam_getAttributeVersion(PolarScanParam_t* scanparam,  const char* name, RaveIO_ODIM_Version version);

/**
 * Returns if the specified attribute exists.
 * @param[in] scan - self
 * @param[in] name - the name of the attribute
 * @returns 1 if attribute exists, otherwise 0
 */
int PolarScanParam_hasAttribute(PolarScanParam_t* scanparam, const char* name);

/**
 * Returns a list of attribute names. Release with \@ref #RaveList_freeAndDestroy.
 * @param[in] scanparam - self
 * @returns a list of attribute names
 */
RaveList_t* PolarScanParam_getAttributeNames(PolarScanParam_t* scanparam);

/**
 * Returns a list of attribute names for specified version. Release with \@ref #RaveList_freeAndDestroy.
 * NOTE! This method is usually only used internally.
 * @param[in] scanparam - self
 * @param[in] version - the version of the attribute
 * @returns a list of attribute names
 */
RaveList_t* PolarScanParam_getAttributeNamesVersion(PolarScanParam_t* scanparam, RaveIO_ODIM_Version version);

/**
 * Returns a list of attribute values that should be stored for this parameter. Corresponding
 * members will also be added as attribute values. E.g. gain will be stored
 * as a double with name what/gain.
 * @param[in] scanparam - self
 * @returns a list of RaveAttributes.
 */
RaveObjectList_t* PolarScanParam_getAttributeValues(PolarScanParam_t* scanparam);

/**
 * Returns a list of attribute values in specified version that has been added to this parameter. Corresponding members will also be added as attribute values. E.g. gain will be stored
 * NOTE! This method is usually only used internally.
 * as a double with name what/gain.
 * @param[in] scanparam - self
 * @param[in] version - the version of the attribute
 * @returns a list of RaveAttributes.
 */
RaveObjectList_t* PolarScanParam_getAttributeValuesVersion(PolarScanParam_t* scanparam, RaveIO_ODIM_Version version);

/**
 * Adds a quality field to this scan.
 * @param[in] param - self
 * @param[in] field - the field to add
 * @returns 1 on success otherwise 0
 */
int PolarScanParam_addQualityField(PolarScanParam_t* param, RaveField_t* field);

/**
 * Returns the quality field at the specified location.
 * @param[in] param - self
 * @param[in] index - the index
 * @returns the quality field if found, otherwise NULL
 */
RaveField_t* PolarScanParam_getQualityField(PolarScanParam_t* param, int index);

/**
 * Returns the number of quality fields
 * @param[in] param - self
 * @returns the number of quality fields
 */
int PolarScanParam_getNumberOfQualityFields(PolarScanParam_t* param);

/**
 * Removes the quality field at the specified location
 * @param[in] param - self
 * @param[in] index - the index
 */
void PolarScanParam_removeQualityField(PolarScanParam_t* param, int index);

/**
 * Returns all quality fields belonging to this scan parameter. The returned
 * object is only a reference so do not modify it.
 * @param[in] param - self
 * @returns a list of 0 or more quality fields or NULL on error
 */
RaveObjectList_t* PolarScanParam_getQualityFields(PolarScanParam_t* param);

/**
 * Returns a quality field based on the value of how/task that should be a
 * string.
 * @param[in] param - self
 * @param[in] value - the value of the how/task attribute
 * @return the field if found otherwise NULL
 */
RaveField_t* PolarScanParam_getQualityFieldByHowTask(PolarScanParam_t* param, const char* value);

/**
 * Converts a polar scan parameter into a rave field. I.e. this function will only
 * take the actual parameter data and convert it into a field. It will not consider
 * any quality field.
 * @param[in] param - self
 * @returns a rave field
 */
RaveField_t* PolarScanParam_toField(PolarScanParam_t* param);

/**
 * Translates a rave field into a polar scan parameter. It will only atempt to find
 * to use what/gain, what/offset, what/nodata and what/undetect. If these doesn't
 * exist, default values will be used.
 * @param[in] field - the field to convert into a polar scan parameter
 * @returns a polar scan parameter on success otherwise NULL
 */
PolarScanParam_t* PolarScanParam_fromField(RaveField_t* field);

/**
 * Converter for 64-bit float (from BUFR) to 8-bit uint,
 * primarily for reverting reflectivity data back to what they once were.
 * @param[in] param - the quantity to convert
 * @return 1 on success otherwise 0
 */
int PolarScanParam_convertDataDoubleToUchar(PolarScanParam_t* param);

/**
 * Performs a circular shift of the dataset and the attributes that are associated with the rays. It can be negative for counter clock wise
 * and positive for clock wise rotation.
 * @param[in] param - the parameter
 * @param[in] nrays - the number of rays to shift
 * @return 1 if successful otherwise 0
 */
int PolarScanParam_shiftData(PolarScanParam_t* param, int nrays);

#endif
