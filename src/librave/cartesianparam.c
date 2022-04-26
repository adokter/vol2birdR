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
 * Defines the functions available when working with cartesian data products
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2012-01-05
 */
#include "cartesianparam.h"
#include "area.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "rave_datetime.h"
#include "rave_data2d.h"
#include "raveobject_hashtable.h"
#include "rave_utilities.h"
#include "rave_types.h"
#include <string.h>
#include "rave_attribute_table.h"

/**
 * Represents the cartesian field product.
 */
struct _CartesianParam_t {
  RAVE_OBJECT_HEAD /** Always on top */

  // What
  char* quantity;            /**< what does this data represent */

  double gain;       /**< gain when scaling, default 1 */
  double offset;     /**< offset when scaling, default 0 */
  double nodata;     /**< nodata */
  double undetect;   /**< undetect */

  RaveData2D_t* data;   /**< 2 dimensional data array */

  LazyDataset_t* lazyDataset; /**< the lazy dataset */

  RaveAttributeTable_t* attrs; /**< attributes */
  RaveObjectList_t* qualityfields; /**< quality fields */
};

/*@{ Private functions */
/**
 * Constructor.
 */
static int CartesianParam_constructor(RaveCoreObject* obj)
{
  CartesianParam_t* this = (CartesianParam_t*)obj;
  this->quantity = NULL;
  this->gain = 1.0;
  this->offset = 0.0;
  this->nodata = 0.0;
  this->undetect = 0.0;
  this->lazyDataset = NULL;
  this->data = RAVE_OBJECT_NEW(&RaveData2D_TYPE);
  this->attrs = RAVE_OBJECT_NEW(&RaveAttributeTable_TYPE);
  this->qualityfields = RAVE_OBJECT_NEW(&RaveObjectList_TYPE);
  if (this->data == NULL || this->attrs == NULL || this->qualityfields == NULL) {
    goto fail;
  }

  return 1;
fail:
  RAVE_OBJECT_RELEASE(this->data);
  RAVE_OBJECT_RELEASE(this->attrs);
  RAVE_OBJECT_RELEASE(this->qualityfields);
  return 0;
}

/**
 * Ensures that we have got a data 2d field to work with regardless
 * if it already has been set or if it supposed to be lazy loaded.
 * @param[in] field - the rave field
 * @returns a loaded rave data 2D field
 */
static RaveData2D_t* CartesianParamInternal_ensureData2D(CartesianParam_t* self)
{
  if (self->lazyDataset != NULL) {
    RaveData2D_t* loaded = LazyDataset_get(self->lazyDataset);
    if (loaded != NULL) {
      /*fprintf(stderr, "CartesianParamInternal_ensureData2D: LazyDataset fetched\n");*/
      RAVE_OBJECT_RELEASE(self->data);
      self->data = RAVE_OBJECT_COPY(loaded);
      RAVE_OBJECT_RELEASE(self->lazyDataset);
    }
    RAVE_OBJECT_RELEASE(loaded);
  }
  return self->data;
}

/**
 * Copy constructor.
 */
static int CartesianParam_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  CartesianParam_t* this = (CartesianParam_t*)obj;
  CartesianParam_t* src = (CartesianParam_t*)srcobj;
  this->gain = src->gain;
  this->offset = src->offset;
  this->nodata = src->nodata;
  this->undetect = src->undetect;
  this->quantity = NULL;
  this->data = NULL;
  this->lazyDataset = NULL;

  CartesianParam_setQuantity(this, CartesianParam_getQuantity(src));

  this->data = RAVE_OBJECT_CLONE(CartesianParamInternal_ensureData2D(src));
  this->attrs = RAVE_OBJECT_CLONE(src->attrs);
  this->qualityfields = RAVE_OBJECT_CLONE(src->qualityfields);

  if (this->data == NULL || this->attrs == NULL || this->qualityfields == NULL) {
    goto fail;
  }

  return 1;
fail:
  RAVE_FREE(this->quantity);
  RAVE_OBJECT_RELEASE(this->data);
  RAVE_OBJECT_RELEASE(this->attrs);
  RAVE_OBJECT_RELEASE(this->qualityfields);
  return 0;
}


/**
 * Destroys the cartesian product
 * @param[in] scan - the cartesian product to destroy
 */
static void CartesianParam_destructor(RaveCoreObject* obj)
{
  CartesianParam_t* cartesian = (CartesianParam_t*)obj;
  if (cartesian != NULL) {
    RAVE_FREE(cartesian->quantity);
    RAVE_OBJECT_RELEASE(cartesian->data);
    RAVE_OBJECT_RELEASE(cartesian->lazyDataset);
    RAVE_OBJECT_RELEASE(cartesian->attrs);
    RAVE_OBJECT_RELEASE(cartesian->qualityfields);
  }
}

/*@} End of Private functions */

/*@{ Interface functions */
long CartesianParam_getXSize(CartesianParam_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->lazyDataset != NULL) {
    return LazyDataset_getXsize(self->lazyDataset);
  }
  return RaveData2D_getXsize(self->data);
}

long CartesianParam_getYSize(CartesianParam_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->lazyDataset != NULL) {
    return LazyDataset_getYsize(self->lazyDataset);
  }
  return RaveData2D_getYsize(self->data);
}

RaveDataType CartesianParam_getDataType(CartesianParam_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->lazyDataset != NULL) {
    return LazyDataset_getDataType(self->lazyDataset);
  }
  return RaveData2D_getType(self->data);
}

int CartesianParam_setQuantity(CartesianParam_t* self, const char* quantity)
{
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (quantity != NULL) {
    char* tmp = RAVE_STRDUP(quantity);
    if (tmp != NULL) {
      RAVE_FREE(self->quantity);
      self->quantity = tmp;
      result = 1;
    }
  } else {
    RAVE_FREE(self->quantity);
    result = 1;
  }
  return result;
}

const char* CartesianParam_getQuantity(CartesianParam_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return (const char*)self->quantity;
}

void CartesianParam_setGain(CartesianParam_t* self, double gain)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (gain != 0.0) {
    self->gain = gain;
  }
}

double CartesianParam_getGain(CartesianParam_t* self)
{
  RAVE_ASSERT((self != NULL), "cartesian == NULL");
  return self->gain;
}

void CartesianParam_setOffset(CartesianParam_t* self, double offset)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->offset = offset;
}

double CartesianParam_getOffset(CartesianParam_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->offset;
}

void CartesianParam_setNodata(CartesianParam_t* self, double nodata)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->nodata = nodata;
}

double CartesianParam_getNodata(CartesianParam_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->nodata;
}

void CartesianParam_setUndetect(CartesianParam_t* self, double undetect)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->undetect = undetect;
}

double CartesianParam_getUndetect(CartesianParam_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->undetect;
}

int CartesianParam_isTransformable(CartesianParam_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveData2D_hasData(CartesianParamInternal_ensureData2D(self));
}

int CartesianParam_setData(CartesianParam_t* self, long xsize, long ysize, void* data, RaveDataType type)
{
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  result = RaveData2D_setData(self->data, xsize, ysize, data, type);
  if (result) {
    RAVE_OBJECT_RELEASE(self->lazyDataset);
  }
  return result;
}

int CartesianParam_setLazyDataset(CartesianParam_t* self, LazyDataset_t* lazyDataset)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (RaveData2D_getData(self->data) == NULL) {
    self->lazyDataset = RAVE_OBJECT_COPY(lazyDataset);
    return 1;
  } else {
    RAVE_ERROR0("Trying to set lazy dataset loader when data exists");
    return 0;
  }
}

int CartesianParam_createData(CartesianParam_t* self, long xsize, long ysize, RaveDataType type, double value)
{
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  result = RaveData2D_createData(self->data, xsize, ysize, type, value);
  if (result) {
    RAVE_OBJECT_RELEASE(self->lazyDataset);
  }
  return result;
}

void* CartesianParam_getData(CartesianParam_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveData2D_getData(CartesianParamInternal_ensureData2D(self));
}

RaveDataType CartesianParam_getType(CartesianParam_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->lazyDataset != NULL) {
    return LazyDataset_getDataType(self->lazyDataset);
  }
  return RaveData2D_getType(self->data);
}

int CartesianParam_setValue(CartesianParam_t* self, long x, long y, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveData2D_setValue(CartesianParamInternal_ensureData2D(self), x, y, v);
}

int CartesianParam_setConvertedValue(CartesianParam_t* self, long x, long y, double v, RaveValueType vtype)
{
  double value = v;
  RAVE_ASSERT((self != NULL), "self == NULL");

  if (vtype == RaveValueType_NODATA) {
    value = self->nodata;
  } else if (vtype == RaveValueType_UNDETECT) {
    value = self->undetect;
  } else {
    if (self->gain != 0.0) {
      value = (v - self->offset)/self->gain;
    } else {
      RAVE_ERROR0("gain is 0.0 => division by zero error");
      return 0;
    }
  }

  return RaveData2D_setValue(CartesianParamInternal_ensureData2D(self), x, y, value);
}

RaveValueType CartesianParam_getValue(CartesianParam_t* self, long x, long y, double* v)
{
  RaveValueType result = RaveValueType_NODATA;
  double value = 0.0;
  RAVE_ASSERT((self != NULL), "self == NULL");

  value = self->nodata;

  if (RaveData2D_getValue(CartesianParamInternal_ensureData2D(self), x, y, &value)) {
    result = RaveValueType_DATA;
    if (value == self->nodata) {
      result = RaveValueType_NODATA;
    } else if (value == self->undetect) {
      result = RaveValueType_UNDETECT;
    }
  }

  if (v != NULL) {
    *v = value;
  }

  return result;
}

RaveValueType CartesianParam_getConvertedValue(CartesianParam_t* self, long x, long y, double* v)
{
  RaveValueType result = RaveValueType_NODATA;
  RAVE_ASSERT((self != NULL), "self == NULL");

  result = CartesianParam_getValue(self, x, y, v);
  if (result == RaveValueType_DATA && v != NULL) {
    *v = (*v) * self->gain + self->offset;
  }
  return result;
}

RaveValueType CartesianParam_getMean(CartesianParam_t* self, long x, long y, int N, double* v)
{
  RaveValueType xytype = RaveValueType_NODATA;
  RAVE_ASSERT((self != NULL), "self == NULL");

  xytype = CartesianParam_getValue(self, x, y, v);
  if (xytype == RaveValueType_DATA) {
    long xk = 0, yk = 0;
    double sum = 0.0L;
    int pts = 0;
    int k = N/2;
    double value = 0.0L;

    for (yk = -k; yk < k; yk++) {
      for (xk = -k; xk < k; xk++) {
        xytype = CartesianParam_getValue(self, xk + x, yk + y, &value);
        if (xytype == RaveValueType_DATA) {
          sum += value;
          pts++;
        }
      }
    }
    *v = sum / (double)pts; // we have at least 1 at pts so division by zero will not occur
  }

  return xytype;
}

int CartesianParam_addAttribute(CartesianParam_t* self, RaveAttribute_t* attribute)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return CartesianParam_addAttributeVersion(self, attribute, RAVEIO_API_ODIM_VERSION);
}

int CartesianParam_addAttributeVersion(CartesianParam_t* self, RaveAttribute_t* attribute, RaveIO_ODIM_Version version)
{
  const char* name = NULL;
  char* aname = NULL;
  char* gname = NULL;
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  RAVE_ASSERT((attribute != NULL), "attribute == NULL");

  name = RaveAttribute_getName(attribute);
  if (name != NULL) {
    if (!RaveAttributeHelp_extractGroupAndName(name, &gname, &aname)) {
      RAVE_ERROR1("Failed to extract group and name from %s", name);
      goto done;
    }
    if ((strcasecmp("how", gname)==0) &&RaveAttributeHelp_validateHowGroupAttributeName(gname, aname)) {
      result = RaveAttributeTable_addAttributeVersion(self->attrs, attribute, version, NULL);
    } else if (strcasecmp("what/prodpar", name)==0) {
      result = RaveAttributeTable_addAttributeVersion(self->attrs, attribute, version, NULL);
    } else {
      RAVE_WARNING1("You are not allowed to add dynamic attributes in other groups than 'how': '%s'", name);
      goto done;
    }
  }

done:
  RAVE_FREE(aname);
  RAVE_FREE(gname);
  return result;
}

RaveAttribute_t* CartesianParam_getAttribute(CartesianParam_t* self, const char* name)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return CartesianParam_getAttributeVersion(self, name, RAVEIO_API_ODIM_VERSION);
}

RaveAttribute_t* CartesianParam_getAttributeVersion(CartesianParam_t* self, const char* name, RaveIO_ODIM_Version version)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (name == NULL) {
    RAVE_ERROR0("Trying to get an attribute with NULL name");
    return NULL;
  }
  return RaveAttributeTable_getAttributeVersion(self->attrs, name, version);
}

RaveList_t* CartesianParam_getAttributeNames(CartesianParam_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return CartesianParam_getAttributeNamesVersion(self, RAVEIO_API_ODIM_VERSION);
}

RaveList_t* CartesianParam_getAttributeNamesVersion(CartesianParam_t* self, RaveIO_ODIM_Version version)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveAttributeTable_getAttributeNamesVersion(self->attrs, version);
}

RaveObjectList_t* CartesianParam_getAttributeValues(CartesianParam_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return CartesianParam_getAttributeValuesVersion(self, RAVEIO_API_ODIM_VERSION);
}

RaveObjectList_t* CartesianParam_getAttributeValuesVersion(CartesianParam_t* self, RaveIO_ODIM_Version version)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveAttributeTable_getValuesVersion(self->attrs, version);
}

int CartesianParam_hasAttribute(CartesianParam_t* self, const char* name)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveAttributeTable_hasAttribute(self->attrs, name);
}

int CartesianParam_addQualityField(CartesianParam_t* self, RaveField_t* field)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveObjectList_add(self->qualityfields, (RaveCoreObject*)field);
}

RaveField_t* CartesianParam_getQualityField(CartesianParam_t* self, int index)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return (RaveField_t*)RaveObjectList_get(self->qualityfields, index);
}

int CartesianParam_getNumberOfQualityFields(CartesianParam_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveObjectList_size(self->qualityfields);
}

void CartesianParam_removeQualityField(CartesianParam_t* self, int index)
{
  RaveField_t* field = NULL;
  RAVE_ASSERT((self != NULL), "self == NULL");
  field = (RaveField_t*)RaveObjectList_remove(self->qualityfields, index);
  RAVE_OBJECT_RELEASE(field);
}

RaveObjectList_t* CartesianParam_getQualityFields(CartesianParam_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return (RaveObjectList_t*)RAVE_OBJECT_COPY(self->qualityfields);
}

RaveField_t* CartesianParam_getQualityFieldByHowTask(CartesianParam_t* self, const char* value)
{
  int nfields = 0, i = 0;
  RaveField_t* result = NULL;

  RAVE_ASSERT((self != NULL), "scan == NULL");
  if (value == NULL) {
    RAVE_WARNING0("Trying to use CartesianParam_getQualityFieldByHowTask without a how/task value");
    return NULL;
  }
  nfields = RaveObjectList_size(self->qualityfields);
  for (i = 0; result == NULL && i < nfields; i++) {
    RaveField_t* field = (RaveField_t*)RaveObjectList_get(self->qualityfields, i);
    if (field != NULL && RaveField_hasAttributeStringValue(field, "how/task", value)) {
      result = RAVE_OBJECT_COPY(field);
    }
    RAVE_OBJECT_RELEASE(field);
  }
  return result;
}

/*@} End of Interface functions */

RaveCoreObjectType CartesianParam_TYPE = {
    "CartesianParam",
    sizeof(CartesianParam_t),
    CartesianParam_constructor,
    CartesianParam_destructor,
    CartesianParam_copyconstructor
};

