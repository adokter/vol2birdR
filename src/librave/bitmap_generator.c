/* --------------------------------------------------------------------
Copyright (C) 2016 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Provides functionallity for creating a surrounding bitmap on a composite.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2016-12-20
 */
#include <bitmap_generator.h>
#include "rave_debug.h"
#include "rave_alloc.h"
#include "raveutil.h"
#include <string.h>

/**
 * Represents the functionality for creating bitmaps
 */
struct _BitmapGenerator_t {
  RAVE_OBJECT_HEAD /** Always on top */
};

/*@{ Private functions */
/**
 * Constructor
 */
static int BitmapGenerator_constructor(RaveCoreObject* obj)
{
  /*SurroundingBitmap_t* self = (SurroundingBitmap_t*)obj;*/
  return 1;
}

/**
 * Copy constructor
 */
static int BitmapGenerator_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  /*SurroundingBitmap_t* self = (SurroundingBitmap_t*)obj;
  SurroundingBitmap_t* src = (SurroundingBitmap_t*)srcobj;*/
  return 1;
}

/**
 * Destructor
 */
static void BitmapGenerator_destructor(RaveCoreObject* obj)
{
  /*
  RaveVprCorrection_t* self = (RaveVprCorrection_t*)obj;
  */
}

/*@} End of Private functions */

/*@{ Interface functions */
RaveField_t* BitmapGenerator_create_surrounding(BitmapGenerator_t* self, CartesianParam_t* param)
{
  RaveField_t *field = NULL, *result = NULL;
  long x = 0, y = 0, xsize = 0, ysize = 0;

  RAVE_ASSERT((self != NULL), "self == NULL");
  RAVE_ASSERT((param != NULL), "param == NULL");

  xsize = CartesianParam_getXSize(param);
  ysize = CartesianParam_getYSize(param);

  field = RAVE_OBJECT_NEW(&RaveField_TYPE);
  if (field == NULL || !RaveField_createData(field, xsize, ysize, RaveDataType_UCHAR)) {
    goto done;
  }

  /** First we work from left to right */
  for (y = 0; y < ysize; y++) {
    double v = 0.0;
    int insideradar = 0;
    RaveValueType vt = CartesianParam_getValue(param, 0, y, &v);
    if (vt == RaveValueType_DATA || vt == RaveValueType_UNDETECT) {
      insideradar = 1;
    }
    for (x = 1; x < xsize; x++) {
      vt = CartesianParam_getValue(param, x, y, &v);
      if (insideradar == 1 && vt == RaveValueType_NODATA) {
        RaveField_setValue(field, x, y, 1.0);
        insideradar = 0;
      } else if (insideradar == 0 && (vt == RaveValueType_DATA || vt == RaveValueType_UNDETECT)) {
        RaveField_setValue(field, x-1, y, 1.0);
        insideradar = 1;
      }
    }
  }

  /** And then up to down */
  for (x = 0; x < xsize; x++) {
    double v = 0.0;
    int insideradar = 0;
    RaveValueType vt = CartesianParam_getValue(param, x, 0, &v);
    if (vt == RaveValueType_DATA || vt == RaveValueType_UNDETECT) {
      insideradar = 1;
    }
    for (y = 1; y < ysize; y++) {
      vt = CartesianParam_getValue(param, x, y, &v);
      if (insideradar == 1 && vt == RaveValueType_NODATA) {
        RaveField_setValue(field, x, y, 1.0);
        insideradar = 0;
      } else if (insideradar == 0 && (vt == RaveValueType_DATA || vt == RaveValueType_UNDETECT)) {
        RaveField_setValue(field, x, y-1, 1.0);
        insideradar = 1;
      }
    }
  }

  result = RAVE_OBJECT_COPY(field);
done:
  RAVE_OBJECT_RELEASE(field);
  return result;
}

RaveField_t* BitmapGenerator_create_intersect(BitmapGenerator_t* self, CartesianParam_t* param, const char* qualityFieldName)
{
  RaveField_t *field = NULL, *result = NULL;
  RaveField_t *qualityField = NULL;
  long x = 0, y = 0, xsize = 0, ysize = 0;

  RAVE_ASSERT((self != NULL), "self == NULL");
  RAVE_ASSERT((param != NULL), "param == NULL");
  RAVE_ASSERT((qualityFieldName != NULL), "qualityFieldName == NULL");

  xsize = CartesianParam_getXSize(param);
  ysize = CartesianParam_getYSize(param);

  qualityField = CartesianParam_getQualityFieldByHowTask(param, qualityFieldName);
  if (qualityField == NULL) {
    RAVE_ERROR1("Could not find any quality field with how/task = %s", qualityFieldName);
    goto done;
  }

  if (xsize != RaveField_getXsize(qualityField) || ysize != RaveField_getYsize(qualityField)) {
    RAVE_ERROR0("Different xsize/ysize between parameter and how/task");
    goto done;

  }

  field = RAVE_OBJECT_NEW(&RaveField_TYPE);
  if (field == NULL || !RaveField_createData(field, xsize, ysize, RaveDataType_UCHAR)) {
    goto done;
  }

  /** First we work from left to right */
  for (y = 0; y < ysize; y++) {
    double v = 0.0;
    double lastvalue = 0.0;
    RaveField_getValue(qualityField, 0, y, &lastvalue);
    for (x = 1; x < xsize; x++) {
      RaveField_getValue(qualityField, x, y, &v);
      if (v != lastvalue && v != 0.0 && lastvalue != 0.0) {
        RaveField_setValue(field, x, y, 1.0);
      }
      lastvalue = v;
    }
  }

  /** And then from up and downwards */
  for (x = 0; x < xsize; x++) {
    double v = 0.0;
    double lastvalue = 0.0;
    RaveField_getValue(qualityField, x, 0, &lastvalue);
    for (y = 1; y < ysize; y++) {
      RaveField_getValue(qualityField, x, y, &v);
      if (v != lastvalue && v != 0.0 && lastvalue != 0.0) {
        RaveField_setValue(field, x, y, 1.0);
      }
      lastvalue = v;
    }
  }

  result = RAVE_OBJECT_COPY(field);
done:
  RAVE_OBJECT_RELEASE(field);
  RAVE_OBJECT_RELEASE(qualityField);
  return result;
}


/*@} End of Interface functions */

RaveCoreObjectType BitmapGenerator_TYPE = {
    "BitmapGenerator",
    sizeof(BitmapGenerator_t),
    BitmapGenerator_constructor,
    BitmapGenerator_destructor,
    BitmapGenerator_copyconstructor
};
