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
 * Defines the functions available when working with cartesian products
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-10-16
 */
#include "cartesian.h"
#include "area.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "rave_datetime.h"
#include "rave_data2d.h"
#include "raveobject_hashtable.h"
#include "rave_utilities.h"
#include <string.h>
#include <stdio.h>
#include "projection_pipeline.h"
#include "rave_attribute_table.h"

/**
 * Represents the cartesian product.
 */
struct _Cartesian_t {
  RAVE_OBJECT_HEAD /** Always on top */

  // Where
  double xscale;     /**< xscale */
  double yscale;     /**< yscale */

  // x / ysize to use for parameters
  long xsize;        /**< xsize to use */
  long ysize;        /**< ysize to use */

  Rave_ProductType product;   /**< product */
  Rave_ObjectType objectType; /**< object type */

  double llX;        /**< lower left x-coordinate */
  double llY;        /**< lower left y-coordinate */
  double urX;        /**< upper right x-coordinate */
  double urY;        /**< upper right x-coordinate */

  // What
  RaveDateTime_t* datetime;  /**< the date and time */
  RaveDateTime_t* startdatetime;  /**< the start date and time */
  RaveDateTime_t* enddatetime;  /**< the end date and time */

  char* source;              /**< where does this data come from */

  char* prodname;    /**< Product name */

  RaveDataType datatype;     /**< the datatype to use */
  Projection_t* projection; /**< the projection */
  ProjectionPipeline_t* pipeline; /**< Used for transforming between lon/lat and cartesian coordinate. First entry is lonlat, second is projection */

  RaveAttributeTable_t* attrs; /**< attributes */

  RaveObjectList_t* qualityfields; /**< quality fields */

  char* defaultParameter;                     /**< the default parameter */
  CartesianParam_t* currentParameter; /**< the current parameter */
  RaveObjectHashTable_t* parameters;  /**< the cartesian data fields */
};

/*@{ Private functions */
/**
 * Constructor.
 */
static int Cartesian_constructor(RaveCoreObject* obj)
{
  Cartesian_t* this = (Cartesian_t*)obj;
  this->xsize = 0;
  this->ysize = 0;
  this->xscale = 0.0;
  this->yscale = 0.0;
  this->llX = 0.0;
  this->llY = 0.0;
  this->urX = 0.0;
  this->urY = 0.0;
  this->datetime = NULL;
  this->product = Rave_ProductType_UNDEFINED;
  this->objectType = Rave_ObjectType_IMAGE;
  this->source = NULL;
  this->prodname = NULL;
  this->datatype = RaveDataType_UCHAR;
  this->projection = NULL;
  this->pipeline = NULL;

  this->currentParameter = NULL;
  this->defaultParameter = RAVE_STRDUP("DBZH");
  this->datetime = RAVE_OBJECT_NEW(&RaveDateTime_TYPE);
  this->startdatetime = RAVE_OBJECT_NEW(&RaveDateTime_TYPE);
  this->enddatetime = RAVE_OBJECT_NEW(&RaveDateTime_TYPE);
  this->attrs = RAVE_OBJECT_NEW(&RaveAttributeTable_TYPE);
  this->qualityfields = RAVE_OBJECT_NEW(&RaveObjectList_TYPE);
  this->parameters = RAVE_OBJECT_NEW(&RaveObjectHashTable_TYPE);
  if (this->datetime == NULL || this->defaultParameter == NULL || this->attrs == NULL ||
      this->startdatetime == NULL || this->enddatetime == NULL || this->qualityfields == NULL ||
      this->parameters == NULL) {
    goto fail;
  }

  return 1;
fail:
  RAVE_OBJECT_RELEASE(this->currentParameter);
  RAVE_OBJECT_RELEASE(this->datetime);
  RAVE_OBJECT_RELEASE(this->attrs);
  RAVE_OBJECT_RELEASE(this->startdatetime);
  RAVE_OBJECT_RELEASE(this->enddatetime);
  RAVE_OBJECT_RELEASE(this->qualityfields);
  RAVE_OBJECT_RELEASE(this->parameters);
  return 0;
}

/**
 * Copy constructor.
 */
static int Cartesian_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  Cartesian_t* this = (Cartesian_t*)obj;
  Cartesian_t* src = (Cartesian_t*)srcobj;
  this->xscale = src->xscale;
  this->yscale = src->yscale;
  this->xsize = src->xsize;
  this->ysize = src->ysize;
  this->llX = src->llX;
  this->llY = src->llY;
  this->urX = src->urX;
  this->urY = src->urY;
  this->product = src->product;
  this->objectType = src->objectType;
  this->datatype = src->datatype;
  this->source = NULL;
  this->prodname = NULL;
  this->projection = NULL;
  this->pipeline = NULL;

  this->datetime = NULL;
  this->startdatetime = NULL;
  this->enddatetime = NULL;
  this->currentParameter = NULL;
  this->defaultParameter = NULL;

  this->datetime = RAVE_OBJECT_CLONE(src->datetime);
  this->startdatetime = RAVE_OBJECT_CLONE(src->startdatetime);
  this->enddatetime = RAVE_OBJECT_CLONE(src->enddatetime);
  this->currentParameter = RAVE_OBJECT_CLONE(src->currentParameter);
  this->attrs = RAVE_OBJECT_CLONE(src->attrs);
  this->qualityfields = RAVE_OBJECT_CLONE(src->qualityfields);
  this->parameters = RAVE_OBJECT_CLONE(src->parameters);

  if (this->datetime == NULL || (src->currentParameter != NULL && this->currentParameter == NULL) || this->attrs == NULL ||
      this->startdatetime == NULL || this->enddatetime == NULL || this->qualityfields == NULL ||
      this->parameters == NULL || !Cartesian_setDefaultParameter(this, Cartesian_getDefaultParameter(src))) {
    goto fail;
  }

  Cartesian_setSource(this, Cartesian_getSource(src));
  if (!Cartesian_setProdname(this, Cartesian_getProdname(src))) {
    goto fail;
  }
  if (src->projection != NULL) {
    this->projection = RAVE_OBJECT_CLONE(src->projection);
    if (this->projection == NULL) {
      goto fail;
    }
  }
  if (src->pipeline != NULL) {
    this->pipeline = RAVE_OBJECT_CLONE(src->pipeline);
    if (this->pipeline == NULL) {
      goto fail;
    }
  }

  return 1;
fail:
  RAVE_FREE(this->source);
  RAVE_FREE(this->prodname);
  RAVE_OBJECT_RELEASE(this->currentParameter);
  RAVE_OBJECT_RELEASE(this->datetime);
  RAVE_OBJECT_RELEASE(this->startdatetime);
  RAVE_OBJECT_RELEASE(this->enddatetime);
  RAVE_OBJECT_RELEASE(this->attrs);
  RAVE_OBJECT_RELEASE(this->projection);
  RAVE_OBJECT_RELEASE(this->pipeline);
  RAVE_OBJECT_RELEASE(this->qualityfields);
  RAVE_OBJECT_RELEASE(this->parameters);
  RAVE_FREE(this->defaultParameter);
  return 0;
}

/**
 * Destroys the cartesian product
 * @param[in] scan - the cartesian product to destroy
 */
static void Cartesian_destructor(RaveCoreObject* obj)
{
  Cartesian_t* cartesian = (Cartesian_t*)obj;
  if (cartesian != NULL) {
    RAVE_OBJECT_RELEASE(cartesian->projection);
    RAVE_OBJECT_RELEASE(cartesian->pipeline);

    RAVE_OBJECT_RELEASE(cartesian->datetime);
    RAVE_OBJECT_RELEASE(cartesian->startdatetime);
    RAVE_OBJECT_RELEASE(cartesian->enddatetime);
    RAVE_FREE(cartesian->source);
    RAVE_FREE(cartesian->prodname);
    RAVE_OBJECT_RELEASE(cartesian->currentParameter);
    RAVE_OBJECT_RELEASE(cartesian->attrs);
    RAVE_OBJECT_RELEASE(cartesian->qualityfields);
    RAVE_OBJECT_RELEASE(cartesian->parameters);
    RAVE_FREE(cartesian->defaultParameter);
  }
}

static int CartesianInternal_getLonLatFromXY(Cartesian_t* self, int x, int y, double* lon, double* lat)
{
  double xpos=self->llX + self->xscale * (double)x;
  double ypos=self->urY - self->yscale * (double)y;
  if (self->pipeline == NULL) {
    return 0;
  }
  return ProjectionPipeline_inv(self->pipeline, xpos, ypos, lon, lat);
}

static int CartesianInternal_getXYFromLonLat(Cartesian_t* self, double lon, double lat, int* x, int* y)
{
  double xpos = 0.0, ypos = 0.0;
  int result = 0;
  if (self->pipeline == NULL) {
    return 0;
  }
  result = ProjectionPipeline_fwd(self->pipeline, lon, lat, &xpos, &ypos);

  *x = (int)((xpos - self->llX) / self->xscale);
  *y = (int)((self->urY - ypos)/self->yscale);

  return result;
}

/*@} End of Private functions */

/*@{ Interface functions */
int Cartesian_setTime(Cartesian_t* cartesian, const char* value)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveDateTime_setTime(cartesian->datetime, value);
}

const char* Cartesian_getTime(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveDateTime_getTime(cartesian->datetime);
}

int Cartesian_setDate(Cartesian_t* cartesian, const char* value)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveDateTime_setDate(cartesian->datetime, value);
}

const char* Cartesian_getDate(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveDateTime_getDate(cartesian->datetime);
}

int Cartesian_setStartTime(Cartesian_t* cartesian, const char* value)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveDateTime_setTime(cartesian->startdatetime, value);
}

const char* Cartesian_getStartTime(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (RaveDateTime_getTime(cartesian->startdatetime) == NULL) {
    return RaveDateTime_getTime(cartesian->datetime);
  }
  return RaveDateTime_getTime(cartesian->startdatetime);
}

int Cartesian_setStartDate(Cartesian_t* cartesian, const char* value)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveDateTime_setDate(cartesian->startdatetime, value);
}

const char* Cartesian_getStartDate(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (RaveDateTime_getDate(cartesian->startdatetime) == NULL) {
    return RaveDateTime_getDate(cartesian->datetime);
  }
  return RaveDateTime_getDate(cartesian->startdatetime);
}

int Cartesian_setEndTime(Cartesian_t* cartesian, const char* value)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveDateTime_setTime(cartesian->enddatetime, value);
}

const char* Cartesian_getEndTime(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (RaveDateTime_getTime(cartesian->enddatetime) == NULL) {
    return RaveDateTime_getTime(cartesian->datetime);
  }
  return RaveDateTime_getTime(cartesian->enddatetime);
}

int Cartesian_setEndDate(Cartesian_t* cartesian, const char* value)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveDateTime_setDate(cartesian->enddatetime, value);
}

const char* Cartesian_getEndDate(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (RaveDateTime_getDate(cartesian->enddatetime) == NULL) {
    return RaveDateTime_getDate(cartesian->datetime);
  }
  return RaveDateTime_getDate(cartesian->enddatetime);
}

int Cartesian_setSource(Cartesian_t* cartesian, const char* value)
{
  char* tmp = NULL;
  int result = 0;
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  if (value != NULL) {
    tmp = RAVE_STRDUP(value);
    if (tmp != NULL) {
      RAVE_FREE(cartesian->source);
      cartesian->source = tmp;
      tmp = NULL;
      result = 1;
    }
  } else {
    RAVE_FREE(cartesian->source);
    result = 1;
  }
  return result;
}

const char* Cartesian_getSource(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  return (const char*)cartesian->source;
}

int Cartesian_setProdname(Cartesian_t* cartesian, const char* value)
{
  char* tmp = NULL;
  int result = 0;
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  if (value != NULL) {
    tmp = RAVE_STRDUP(value);
    if (tmp != NULL) {
      RAVE_FREE(cartesian->prodname);
      cartesian->prodname = tmp;
      tmp = NULL;
      result = 1;
    }
  } else {
    RAVE_FREE(cartesian->prodname);
    result = 1;
  }
  return result;
}

const char* Cartesian_getProdname(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  return (const char*)cartesian->prodname;
}

int Cartesian_setObjectType(Cartesian_t* self, Rave_ObjectType type)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (type == Rave_ObjectType_IMAGE || type == Rave_ObjectType_COMP) {
    self->objectType = type;
    return 1;
  }
  return 0;
}

Rave_ObjectType Cartesian_getObjectType(Cartesian_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->objectType;
}

void Cartesian_setXSize(Cartesian_t* self, long xsize)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->xsize = xsize;

}

void Cartesian_setYSize(Cartesian_t* self, long ysize)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->ysize = ysize;
}

long Cartesian_getXSize(Cartesian_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->xsize;
}

long Cartesian_getYSize(Cartesian_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->ysize;
}

void Cartesian_setAreaExtent(Cartesian_t* cartesian, double llX, double llY, double urX, double urY)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  cartesian->llX = llX;
  cartesian->llY = llY;
  cartesian->urX = urX;
  cartesian->urY = urY;
}

void Cartesian_getAreaExtent(Cartesian_t* cartesian, double* llX, double* llY, double* urX, double* urY)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  if (llX != NULL) {
    *llX = cartesian->llX;
  }
  if (llY != NULL) {
    *llY = cartesian->llY;
  }
  if (urX != NULL) {
    *urX = cartesian->urX;
  }
  if (urY != NULL) {
    *urY = cartesian->urY;
  }
}

int Cartesian_getExtremeLonLatBoundaries(Cartesian_t* self, double* ulLon, double* ulLat, double* lrLon, double* lrLat)
{
  double ulX = 0.0, ulY = 0.0, lrX = 0.0, lrY = 0.0;
  int x = 0, y = 0;

  RAVE_ASSERT((self != NULL), "self == NULL");

  if (!CartesianInternal_getLonLatFromXY(self, 0, 0, &ulX, &ulY) ||
      !CartesianInternal_getLonLatFromXY(self, self->xsize-1, self->ysize-1, &lrX, &lrY)) {
    return 0;
  }

  /* First we travel along the upper and lower boundaries */
  for (x = 0; x < self->xsize; x++) {
    double tmpLon = 0.0, tmpLat = 0.0;
    /* top boundary */
    CartesianInternal_getLonLatFromXY(self, x, 0, &tmpLon, &tmpLat);
    if (ulX > tmpLon) {/* Lon goes towards -180 the more west we travel */
      ulX = tmpLon;
    }
    if (lrX < tmpLon) { /* Lon goes towards +180 the more east we travel */
      lrX = tmpLon;
    }

    /* lower boundary */
    CartesianInternal_getLonLatFromXY(self, x, self->ysize-1, &tmpLon, &tmpLat);
    if (ulX > tmpLon) { /* Lon goes towards -180 the more west we travel */
      ulX = tmpLon;
    }
    if (lrX < tmpLon) { /* Lon goes towards +180 the more east we travel */
      lrX = tmpLon;
    }
  }

  /* After that we travel along the east and west boundaries */
  for (y = 0; y < self->ysize; y++) {
    double tmpLon = 0.0, tmpLat = 0.0;
    /* west boundary */
    CartesianInternal_getLonLatFromXY(self, 0, y, &tmpLon, &tmpLat);
    if (ulY < tmpLat) {/* Lat goes towards +90 the more north we travel */
      ulY = tmpLat;
    }
    if (lrY > tmpLat) { /* Lat goes towards -90 the more south we travel */
      lrY = tmpLat;
    }

    /* east boundary */
    CartesianInternal_getLonLatFromXY(self, self->xsize-1, y, &tmpLon, &tmpLat);
    if (ulY < tmpLat) { /* Lat goes towards +90 the more north we travel */
      ulY = tmpLat;
    }
    if (lrY > tmpLat) { /* Lat goes towards -90 the more south we travel */
      lrY = tmpLat;
    }
  }

  if (ulLon != NULL) {
    *ulLon = ulX;
  }
  if (ulLat != NULL) {
    *ulLat = ulY;
  }
  if (lrLon != NULL) {
    *lrLon = lrX;
  }
  if (lrLat != NULL) {
    *lrLat = lrY;
  }

  return 1;
}


void Cartesian_setXScale(Cartesian_t* cartesian, double xscale)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  cartesian->xscale = xscale;
}

double Cartesian_getXScale(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  return cartesian->xscale;
}

void Cartesian_setYScale(Cartesian_t* cartesian, double yscale)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  cartesian->yscale = yscale;
}

double Cartesian_getYScale(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  return cartesian->yscale;
}

int Cartesian_setProduct(Cartesian_t* cartesian, Rave_ProductType type)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  cartesian->product = type;
  return 1;
}

Rave_ProductType Cartesian_getProduct(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  return cartesian->product;
}

double Cartesian_getNodata(Cartesian_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->currentParameter != NULL) {
    return CartesianParam_getNodata(self->currentParameter);
  }
  return 0.0;
}

double Cartesian_getUndetect(Cartesian_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->currentParameter != NULL) {
    return CartesianParam_getUndetect(self->currentParameter);
  }
  return 0.0;
}

double Cartesian_getLocationX(Cartesian_t* cartesian, long x)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  return cartesian->llX + cartesian->xscale * (double)x;
}

double Cartesian_getLocationY(Cartesian_t* cartesian, long y)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  return cartesian->urY - cartesian->yscale * (double)y;
}

long Cartesian_getIndexX(Cartesian_t* cartesian, double x)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  RAVE_ASSERT((cartesian->xscale != 0.0), "xcale == 0.0, would result in Division by zero");
  return (long)((x - cartesian->llX) / cartesian->xscale);
}

long Cartesian_getIndexY(Cartesian_t* cartesian, double y)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  RAVE_ASSERT((cartesian->yscale != 0.0), "ycale == 0.0, would result in Division by zero");
  return (long)((cartesian->urY - y)/cartesian->yscale);
}

int Cartesian_setDefaultParameter(Cartesian_t* cartesian, const char* name)
{
  int result = 0;
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  if (name != NULL) {
    char* tmp = RAVE_STRDUP(name);
    if (tmp == NULL) {
      RAVE_CRITICAL0("Failed to allocate memory");
      goto done;
    }
    RAVE_FREE(cartesian->defaultParameter);
    cartesian->defaultParameter = tmp;
    RAVE_OBJECT_RELEASE(cartesian->currentParameter);
    if (RaveObjectHashTable_exists(cartesian->parameters, name)) {
      cartesian->currentParameter = (CartesianParam_t*)RaveObjectHashTable_get(cartesian->parameters, name);
    }
    result = 1;
  } else {
    RAVE_WARNING0("Not supported parameter name");
  }

done:
  return result;
}

const char* Cartesian_getDefaultParameter(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  return (const char*)cartesian->defaultParameter;
}

int Cartesian_setProjection(Cartesian_t* cartesian, Projection_t* projection)
{
  int result = 0;
  ProjectionPipeline_t *pipeline = NULL;
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");

  if (projection != NULL) {
    pipeline = ProjectionPipeline_createDefaultLonLatPipeline(projection);
    if (pipeline == NULL) {
      RAVE_ERROR0("Could not create default lon/lat pipeline");
      goto done;
    }

    RAVE_OBJECT_RELEASE(cartesian->projection);
    RAVE_OBJECT_RELEASE(cartesian->pipeline);
    cartesian->projection = RAVE_OBJECT_COPY(projection);
    cartesian->pipeline = RAVE_OBJECT_COPY(pipeline);
  } else {
    RAVE_OBJECT_RELEASE(cartesian->projection);
    RAVE_OBJECT_RELEASE(cartesian->pipeline);
  }
  result = 1;
done:
  RAVE_OBJECT_RELEASE(pipeline);
  return result;
}

Projection_t* Cartesian_getProjection(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  if (cartesian->projection != NULL) {
    return RAVE_OBJECT_COPY(cartesian->projection);
  }
  return NULL;
}

const char* Cartesian_getProjectionString(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  if (cartesian->projection != NULL) {
    return Projection_getDefinition(cartesian->projection);
  }
  return NULL;
}

int Cartesian_setValue(Cartesian_t* cartesian, long x, long y, double v)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  if (cartesian->currentParameter != NULL) {
    return CartesianParam_setValue(cartesian->currentParameter, x, y, v);
  }
  return 0;
}

int Cartesian_setConvertedValue(Cartesian_t* cartesian, long x, long y, double v)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  if (cartesian->currentParameter != NULL) {
    return CartesianParam_setConvertedValue(cartesian->currentParameter, x, y, v, RaveValueType_DATA);
  }
  return 0;
}

RaveValueType Cartesian_getValue(Cartesian_t* cartesian, long x, long y, double* v)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (cartesian->currentParameter != NULL) {
    return CartesianParam_getValue(cartesian->currentParameter, x, y, v);
  }
  return RaveValueType_UNDEFINED;
}

RaveValueType Cartesian_getConvertedValue(Cartesian_t* cartesian, long x, long y, double* v)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (cartesian->currentParameter != NULL) {
    return CartesianParam_getConvertedValue(cartesian->currentParameter, x, y, v);
  }
  return RaveValueType_UNDEFINED;
}

RaveValueType Cartesian_getValueAtLocation(Cartesian_t* cartesian, double lx, double ly, double* v)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (cartesian->currentParameter != NULL) {
    int x = 0, y = 0;
    x = Cartesian_getIndexX(cartesian, lx);
    y = Cartesian_getIndexY(cartesian, ly);
    return CartesianParam_getValue(cartesian->currentParameter, x, y, v);
  }
  return RaveValueType_UNDEFINED;
}

RaveValueType Cartesian_getConvertedValueAtLocation(Cartesian_t* cartesian, double lx, double ly, double* v)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (cartesian->currentParameter != NULL) {
    int x = 0, y = 0;
    x = Cartesian_getIndexX(cartesian, lx);
    y = Cartesian_getIndexY(cartesian, ly);
    return CartesianParam_getConvertedValue(cartesian->currentParameter, x, y, v);
  }
  return RaveValueType_UNDEFINED;
}

RaveValueType Cartesian_getConvertedValueAtLonLat(Cartesian_t* cartesian, double lon, double lat, double* v)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (cartesian->currentParameter != NULL) {
    int x = 0, y = 0;
    if (!CartesianInternal_getXYFromLonLat(cartesian, lon, lat, &x, &y)) {
      return RaveValueType_UNDEFINED;
    }
    return Cartesian_getConvertedValue(cartesian, x, y, v);
  }
  return RaveValueType_UNDEFINED;
}

int Cartesian_getQualityValueAtLocation(Cartesian_t* cartesian, double lx, double ly, const char* name, double *v)
{
  RaveField_t* field = NULL;
  int result = 0;
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");

  field = Cartesian_findQualityFieldByHowTask(cartesian, name);

  if (field != NULL) {
    int x = 0, y = 0;
    x = Cartesian_getIndexX(cartesian, lx);
    y = Cartesian_getIndexY(cartesian, ly);
    result = RaveField_getValue(field, x, y, v);
  }
  RAVE_OBJECT_RELEASE(field);

  return result;
}

int Cartesian_getConvertedQualityValueAtLocation(Cartesian_t* cartesian, double lx, double ly, const char* name, double *v)
{
  RaveField_t* field = NULL;
  int result = 0;
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");

  field = Cartesian_findQualityFieldByHowTask(cartesian, name);

  if (field != NULL) {
    int x = 0, y = 0;
    x = Cartesian_getIndexX(cartesian, lx);
    y = Cartesian_getIndexY(cartesian, ly);
    result = RaveField_getConvertedValue(field, x, y, v);
  }
  RAVE_OBJECT_RELEASE(field);

  return result;
}

int Cartesian_getQualityValueAtLonLat(Cartesian_t* cartesian, double lon, double lat, const char* name, double *v)
{
  RaveField_t* field = NULL;
  int result = 0;
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");

  field = Cartesian_findQualityFieldByHowTask(cartesian, name);

  if (field != NULL) {
    int x = 0, y = 0;
    CartesianInternal_getXYFromLonLat(cartesian, lon, lat, &x, &y);
    result = RaveField_getValue(field, x, y, v);
  }
  RAVE_OBJECT_RELEASE(field);

  return result;
}

int Cartesian_getConvertedQualityValueAtLonLat(Cartesian_t* cartesian, double lon, double lat, const char* name, double *v)
{
  RaveField_t* field = NULL;
  int result = 0;
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");

  field = Cartesian_findQualityFieldByHowTask(cartesian, name);

  if (field != NULL) {
    int x = 0, y = 0;
    CartesianInternal_getXYFromLonLat(cartesian, lon, lat, &x, &y);
    result = RaveField_getConvertedValue(field, x, y, v);
  }
  RAVE_OBJECT_RELEASE(field);

  return result;
}

void Cartesian_init(Cartesian_t* self, Area_t* area)
{
  double llX = 0.0L, llY = 0.0L, urX = 0.0L, urY = 0.0L;
  Projection_t* projection = NULL;
  RAVE_ASSERT((self != NULL), "self == NULL");
  RAVE_ASSERT((area != NULL), "area == NULL");

  Cartesian_setXScale(self, Area_getXScale(area));
  Cartesian_setYScale(self, Area_getYScale(area));
  Cartesian_setXSize(self, Area_getXSize(area));
  Cartesian_setYSize(self, Area_getYSize(area));
  projection = Area_getProjection(area);
  Cartesian_setProjection(self, projection);
  Area_getExtent(area, &llX, &llY, &urX, &urY);
  Cartesian_setAreaExtent(self, llX, llY, urX, urY);
  RAVE_OBJECT_RELEASE(projection);
}

RaveValueType Cartesian_getMean(Cartesian_t* cartesian, long x, long y, int N, double* v)
{
  RaveValueType xytype = RaveValueType_NODATA;
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");

  xytype = Cartesian_getValue(cartesian, x, y, v);
  if (xytype == RaveValueType_DATA) {
    long xk = 0, yk = 0;
    double sum = 0.0L;
    int pts = 0;
    int k = N/2;
    double value = 0.0L;

    for (yk = -k; yk < k; yk++) {
      for (xk = -k; xk < k; xk++) {
        xytype = Cartesian_getValue(cartesian, xk + x, yk + y, &value);
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

int Cartesian_isTransformable(Cartesian_t* cartesian)
{
  int result = 0;
  int ncount = 0;
  int i = 0;
  RaveObjectList_t* params = NULL;

  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");

  params = RaveObjectHashTable_values(cartesian->parameters);
  if (params == NULL) {
    goto done;
  }
  ncount = RaveObjectList_size(params);
  if (ncount <= 0 || cartesian->xscale <= 0.0 || cartesian->yscale <= 0.0 || cartesian->projection == NULL) {
    goto done;
  }

  result = 1;
  for (i = 0; result == 1 && i < ncount; i++) {
    CartesianParam_t* param = (CartesianParam_t*)RaveObjectList_get(params, i);
    if (param != NULL) {
      result = CartesianParam_isTransformable(param);
    } else {
      result = 0;
    }
    RAVE_OBJECT_RELEASE(param);
  }

done:
  RAVE_OBJECT_RELEASE(params);
  return result;
}

int Cartesian_addAttribute(Cartesian_t* cartesian, RaveAttribute_t* attribute)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return Cartesian_addAttributeVersion(cartesian, attribute, RAVEIO_API_ODIM_VERSION);
}

int Cartesian_addAttributeVersion(Cartesian_t* cartesian, RaveAttribute_t* attribute, RaveIO_ODIM_Version version)
{
  const char* name = NULL;
  char* aname = NULL;
  char* gname = NULL;
  int result = 0;
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  RAVE_ASSERT((attribute != NULL), "attribute == NULL");

  name = RaveAttribute_getName(attribute);
  if (name != NULL) {
    if (!RaveAttributeHelp_extractGroupAndName(name, &gname, &aname)) {
      RAVE_ERROR1("Failed to extract group and name from %s", name);
      goto done;
    }
    if ((strcasecmp("how", gname)==0) &&RaveAttributeHelp_validateHowGroupAttributeName(gname, aname)) {
      result = RaveAttributeTable_addAttributeVersion(cartesian->attrs, attribute, version, NULL);
    } else if (strcasecmp("what/prodpar", name)==0) {
      result = RaveAttributeTable_addAttributeVersion(cartesian->attrs, attribute, version, NULL);
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

RaveAttribute_t* Cartesian_getAttribute(Cartesian_t* cartesian, const char* name)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return Cartesian_getAttributeVersion(cartesian, name, RAVEIO_API_ODIM_VERSION);
}

RaveAttribute_t* Cartesian_getAttributeVersion(Cartesian_t* cartesian, const char* name, RaveIO_ODIM_Version version)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (name == NULL) {
    RAVE_ERROR0("Trying to get an attribute with NULL name");
    return NULL;
  }
  return (RaveAttribute_t*)RaveAttributeTable_getAttributeVersion(cartesian->attrs, name, version);
}

RaveList_t* Cartesian_getAttributeNames(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return Cartesian_getAttributeNamesVersion(cartesian, RAVEIO_API_ODIM_VERSION);
}

RaveList_t* Cartesian_getAttributeNamesVersion(Cartesian_t* cartesian, RaveIO_ODIM_Version version)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveAttributeTable_getAttributeNamesVersion(cartesian->attrs, version);
}

RaveObjectList_t* Cartesian_getAttributeValues(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return Cartesian_getAttributeValuesVersion(cartesian, RAVEIO_API_ODIM_VERSION);
}

RaveObjectList_t* Cartesian_getAttributeValuesVersion(Cartesian_t* cartesian, RaveIO_ODIM_Version version)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveAttributeTable_getValuesVersion(cartesian->attrs, version);
}

int Cartesian_hasAttribute(Cartesian_t* cartesian, const char* name)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveAttributeTable_hasAttribute(cartesian->attrs, name);
}

int Cartesian_addQualityField(Cartesian_t* cartesian, RaveField_t* field)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveObjectList_add(cartesian->qualityfields, (RaveCoreObject*)field);
}

RaveField_t* Cartesian_getQualityField(Cartesian_t* cartesian, int index)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return (RaveField_t*)RaveObjectList_get(cartesian->qualityfields, index);
}

int Cartesian_getNumberOfQualityFields(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveObjectList_size(cartesian->qualityfields);
}

void Cartesian_removeQualityField(Cartesian_t* cartesian, int index)
{
  RaveField_t* field = NULL;
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  field = (RaveField_t*)RaveObjectList_remove(cartesian->qualityfields, index);
  RAVE_OBJECT_RELEASE(field);
}

RaveObjectList_t* Cartesian_getQualityFields(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return (RaveObjectList_t*)RAVE_OBJECT_COPY(cartesian->qualityfields);
}

RaveField_t* Cartesian_getQualityFieldByHowTask(Cartesian_t* cartesian, const char* value)
{
  int nfields = 0, i = 0;
  RaveField_t* result = NULL;

  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (value == NULL) {
    RAVE_WARNING0("Trying to use Cartesian_getQualityFieldByHowTask without a how/task value");
    return NULL;
  }
  nfields = RaveObjectList_size(cartesian->qualityfields);
  for (i = 0; result == NULL && i < nfields; i++) {
    RaveField_t* field = (RaveField_t*)RaveObjectList_get(cartesian->qualityfields, i);
    if (field != NULL && RaveField_hasAttributeStringValue(field, "how/task", value)) {
      result = RAVE_OBJECT_COPY(field);
    }
    RAVE_OBJECT_RELEASE(field);
  }

  return result;
}

RaveField_t* Cartesian_findQualityFieldByHowTask(Cartesian_t* self, const char* value)
{
  RaveField_t* result = NULL;

  RAVE_ASSERT((self != NULL), "self == NULL");

  if (self->currentParameter != NULL) {
    result = CartesianParam_getQualityFieldByHowTask(self->currentParameter, value);
  }

  if (result == NULL) {
    result = Cartesian_getQualityFieldByHowTask(self, value);
  }

  return result;
}

int Cartesian_addParameter(Cartesian_t* self, CartesianParam_t* param)
{
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (param != NULL) {
    const char* p = CartesianParam_getQuantity(param);
    if (p == NULL) {
      RAVE_ERROR0("Parameter does not contain any quantity");
      goto done;
    }
    if (RaveObjectHashTable_size(self->parameters) == 0) {
      self->xsize = CartesianParam_getXSize(param);
      self->ysize = CartesianParam_getYSize(param);
    }

    if (CartesianParam_getXSize(param) != self->xsize ||
        CartesianParam_getYSize(param) != self->ysize) {
      RAVE_ERROR0("Inconsistent x/y size between parameters");
      goto done;
    }

    if (!RaveObjectHashTable_put(self->parameters, p, (RaveCoreObject*)param)) {
      RAVE_ERROR0("Could not add parameter to cartesian");
      goto done;
    }

    if (strcmp(self->defaultParameter, p) == 0) {
      RAVE_OBJECT_RELEASE(self->currentParameter);
      self->currentParameter = RAVE_OBJECT_COPY(param);
    }
    result = 1;
  }

done:
  return result;
}

CartesianParam_t* Cartesian_getParameter(Cartesian_t* self, const char* name)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (RaveObjectHashTable_exists(self->parameters, name)) {
    return (CartesianParam_t*)RaveObjectHashTable_get(self->parameters, name);
  }
  return NULL;
}

int Cartesian_hasParameter(Cartesian_t* self, const char* quantity)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveObjectHashTable_exists(self->parameters, quantity);
}


void Cartesian_removeParameter(Cartesian_t* self, const char* name)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  CartesianParam_t* param = (CartesianParam_t*)RaveObjectHashTable_remove(self->parameters, name);
  RAVE_OBJECT_RELEASE(param);
}

int Cartesian_getParameterCount(Cartesian_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveObjectHashTable_size(self->parameters);
}

RaveList_t* Cartesian_getParameterNames(Cartesian_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveObjectHashTable_keys(self->parameters);
}

CartesianParam_t* Cartesian_createParameter(Cartesian_t* self, const char* quantity, RaveDataType type, double datavalue)
{
  CartesianParam_t* result = NULL;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->xsize > 0 && self->ysize > 0 && quantity != NULL && type != RaveDataType_UNDEFINED) {
    result = RAVE_OBJECT_NEW(&CartesianParam_TYPE);
    if (result == NULL ||
        !CartesianParam_createData(result, self->xsize, self->ysize, type, datavalue) ||
        !CartesianParam_setQuantity(result, quantity) ||
        !Cartesian_addParameter(self, result)) {
      RAVE_OBJECT_RELEASE(result);
    }
  }
  return result;
}

/*@} End of Interface functions */

RaveCoreObjectType Cartesian_TYPE = {
    "Cartesian",
    sizeof(Cartesian_t),
    Cartesian_constructor,
    Cartesian_destructor,
    Cartesian_copyconstructor
};

