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
 * Represents a 2-dimensional data array.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-12-17
 */
#include "rave_data2d.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "raveutil.h"
#include <limits.h>
#include <float.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/**
 * Represents a date time instance
 */
struct _RaveData2D_t {
  RAVE_OBJECT_HEAD /** Always on top */
  long xsize;        /**< xsize */
  long ysize;        /**< ysize */
  int useNodata;     /**< using nodata */
  double nodata;     /**< the nodata value */
  RaveDataType type; /**< data type */
  void* data;        /**< data ptr */
};

/**
 * Function pointer used by the element wise operations
 * @param[in] v1 - the first value
 * @param[in] v2 - the second value
 */
typedef double (*rave_eoperation)(double v1,double v2);

/*@{ Private functions */
/**
 * Constructor.
 */
static int RaveData2D_constructor(RaveCoreObject* obj)
{
  RaveData2D_t* data = (RaveData2D_t*)obj;
  data->xsize = 0;
  data->ysize = 0;
  data->useNodata = 0;
  data->nodata = 255;
  data->type = RaveDataType_UNDEFINED;
  data->data = NULL;
  return 1;
}

/**
 * Copy constructor.
 */
static int RaveData2D_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  RaveData2D_t* data = (RaveData2D_t*)obj;
  RaveData2D_t* srcdata = (RaveData2D_t*)srcobj;
  int result = 1;
  data->xsize = srcdata->xsize;
  data->ysize = srcdata->ysize;
  data->useNodata = srcdata->useNodata;
  data->nodata = srcdata->nodata;
  data->type = srcdata->type;
  data->data = NULL;
  if (srcdata->data != NULL && srcdata->type > RaveDataType_UNDEFINED && srcdata->type < RaveDataType_LAST) {
    result = RaveData2D_setData(data, srcdata->xsize, srcdata->ysize, srcdata->data, srcdata->type);
  }
  return result;
}

/**
 * Destructor.
 */
static void RaveData2D_destructor(RaveCoreObject* obj)
{
  RaveData2D_t* data = (RaveData2D_t*)obj;
  if (data != NULL) {
    RAVE_FREE(data->data);
  }
}

/**
 * Performs a element wise matrix operation on field.
 * @param[in] field - self
 * @param[in] other - the other
 * @param[in] eoperation - the operation to execute on the values
 * @returns a new field or NULL on error
 */
static RaveData2D_t* RaveData2DInternal_eoperation(RaveData2D_t* field, RaveData2D_t* other, rave_eoperation eoperation)
{
  RaveData2D_t *result = NULL, *newfield = NULL;
  long x = 0, y = 0;

  RAVE_ASSERT((field != NULL), "field == NULL");
  if (other == NULL) {
    RAVE_CRITICAL0("other == NULL");
    return NULL;
  }
  if (other->xsize != 1 && field->xsize != other->xsize) {
    RAVE_ERROR0("other must either have xsize=1 or same xsize as frame to be possible to use for substraction");
    return NULL;
  }
  if (other->ysize != 1 && field->ysize != other->ysize) {
    RAVE_ERROR0("other must either have ysize=1 or same ysize as frame to be possible to use for substraction");
    return NULL;
  }

  newfield = RaveData2D_zeros(field->xsize, field->ysize, RaveDataType_DOUBLE);
  if (newfield == NULL) {
    return NULL;
  }

  if (field->useNodata) {
    newfield->useNodata = 1;
    newfield->nodata = field->nodata;
  } else if (other->useNodata) {
    newfield->useNodata = 1;
    newfield->nodata = other->nodata;
  }

  for (x = 0; x < field->xsize; x++) {
    long ox = 0;
    if (other->xsize > 1) {
      ox = x;
    }
    for (y = 0; y < field->ysize; y++) {
      double v = 0.0, ov = 0.0;
      long oy = 0;
      if (other->ysize > 1) {
        oy = y;
      }
      RaveData2D_getValueUnchecked(field, x, y, &v);
      RaveData2D_getValueUnchecked(other, ox, oy, &ov);

      if ((field->useNodata && field->nodata == v) || (other->useNodata && other->nodata == ov)) {
        RaveData2D_setValueUnchecked(newfield, x, y, newfield->nodata);
      } else {
        RaveData2D_setValueUnchecked(newfield, x, y, eoperation(v, ov));
      }
    }
  }

  result = RAVE_OBJECT_COPY(newfield);
  RAVE_OBJECT_RELEASE(newfield);
  return result;
}

/**
 * Executes the eoperation for each item in field.
 * @param[in] field - self
 * @param[in] v - the second value provided to the eoperation
 * @returns a rave data 2d field on success or NULL on error
 */
static RaveData2D_t* RaveData2D_eoperationNumber(RaveData2D_t* field, double v, rave_eoperation eoperation)
{
  RaveData2D_t *result = NULL, *newfield = NULL;
  long x = 0, y = 0;

  RAVE_ASSERT((field != NULL), "field == NULL");
  newfield = RaveData2D_zeros(field->xsize, field->ysize, RaveDataType_DOUBLE);
  newfield->useNodata = field->useNodata;
  newfield->nodata = field->nodata;
  if (newfield == NULL) {
    return NULL;
  }
  for (x = 0; x < newfield->xsize; x++) {
    for (y = 0; y < newfield->ysize; y++) {
      double nfv = 0.0;
      RaveData2D_getValueUnchecked(field, x, y, &nfv);
      if (field->useNodata && field->nodata == nfv) {
        RaveData2D_setValueUnchecked(newfield, x, y, field->nodata);
      } else {
        RaveData2D_setValueUnchecked(newfield, x, y, eoperation(nfv, v));
      }
    }
  }
  result = RAVE_OBJECT_COPY(newfield);
  RAVE_OBJECT_RELEASE(newfield);
  return result;
}

/*@} End of Private functions */

/*@{ Interface functions */
void RaveData2D_useNodata(RaveData2D_t* self, int use)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (use == 1) {
    self->useNodata = use;
  } else {
    self->useNodata = 0;
  }
}

int RaveData2D_usingNodata(RaveData2D_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->useNodata;
}

void RaveData2D_setNodata(RaveData2D_t* self, double nodata)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->nodata = nodata;
}

double RaveData2D_getNodata(RaveData2D_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->nodata;
}


long RaveData2D_getXsize(RaveData2D_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->xsize;
}

long RaveData2D_getYsize(RaveData2D_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->ysize;

}

RaveDataType RaveData2D_getType(RaveData2D_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->type;

}

void* RaveData2D_getData(RaveData2D_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->data;
}

int RaveData2D_setData(RaveData2D_t* self, long xsize, long ysize, void* data, RaveDataType type)
{
  int result = 0;

  RAVE_ASSERT((self != NULL), "self == NULL");

  result = RaveData2D_createData(self, xsize, ysize, type, 0);
  if (result == 1 && data != NULL) {
    long sz = 0;
    long nbytes = 0;
    sz = get_ravetype_size(type);
    nbytes = xsize*ysize*sz;
    memcpy(self->data, data, nbytes);
  }

  return result;
}

int RaveData2D_createData(RaveData2D_t* self, long xsize, long ysize, RaveDataType type, double value)
{
  long sz = 0;
  long nbytes = 0;
  void* ptr = NULL;
  int result = 0;

  RAVE_ASSERT((self != NULL), "self == NULL");

  if (type <= RaveDataType_UNDEFINED || type >= RaveDataType_LAST) {
    RAVE_ERROR1("RaveData2D does not support the data type %d", type);
    return 0;
  }

  sz = get_ravetype_size(type);
  nbytes = xsize*ysize*sz;
  ptr = RAVE_MALLOC(nbytes);

  if (ptr == NULL) {
    RAVE_CRITICAL1("Failed to allocate memory (%d bytes)", (int)nbytes);
    goto fail;
  }
  memset(ptr, value, nbytes);
  RAVE_FREE(self->data);
  self->data = ptr;
  self->xsize = xsize;
  self->ysize = ysize;
  self->type = type;
  result = 1;
fail:
  return result;
}

int RaveData2D_fill(RaveData2D_t* self, double v)
{
  int x = 0, y = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->data == NULL) {
    RAVE_ERROR0("Atempting to set value when there is no data array");
    return 0;
  }
  for (x = 0; x < self->xsize; x++) {
    for (y = 0; y < self->ysize; y++) {
      RaveData2D_setValueUnchecked(self, x, y, v);
    }
  }

  return 1;
}

int RaveData2D_setValue(RaveData2D_t* self, long x, long y, double v)
{
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->data == NULL) {
    RAVE_ERROR0("Atempting to set value when there is no data array");
    return 0;
  }
  if (x >= 0 && x < self->xsize && y >= 0 && y < self->ysize) {
    result = RaveData2D_setValueUnchecked(self, x, y, v);
  }
  return result;
}

int RaveData2D_setValueUnchecked(RaveData2D_t* self, long x, long y, double v)
{
  int result = 1;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->data == NULL) {
    RAVE_ERROR0("Atempting to set value when there is no data array");
    return 0;
  }

  switch (self->type) {
  case RaveDataType_CHAR: {
    int c = myround_int(v, -128, 127);
    char *a = (char *) self->data;
    a[y * self->xsize + x] = c;
    break;
  }
  case RaveDataType_UCHAR: {
    unsigned char c = (unsigned char)myround_int(v, 0, 255);
    unsigned char *a = (unsigned char *) self->data;
    a[y * self->xsize + x] = c;
    break;
  }
  case RaveDataType_SHORT: {
    int c = myround_int(v, SHRT_MIN, SHRT_MAX);
    short *a = (short *) self->data;
    a[y * self->xsize + x] = c;
    break;
  }
  case RaveDataType_USHORT: {
    int c = myround_int(v, 0, USHRT_MAX);
    unsigned short *a = (unsigned short *) self->data;
    a[y * self->xsize + x] = c;
    break;
  }
  case RaveDataType_INT: {
    int c = myround_int(v, INT_MIN, INT_MAX);
    int *a = (int *) self->data;
    a[y * self->xsize + x] = c;
    break;
  }
  case RaveDataType_UINT: {
    unsigned int *a = (unsigned int *) self->data;
    if (v < 0)
      v = 0;
    if (v > UINT_MAX)
      v = UINT_MAX;
    a[y * self->xsize + x] = (unsigned int)v;
    break;
  }
  case RaveDataType_LONG: {
    long *a = (long *) self->data;
    long c;
    if (v > LONG_MAX)
      v = LONG_MAX;
    if (v < LONG_MIN)
      v = LONG_MIN;
    c = round(v); /* Should work on 64bit boxes after above preparations. */
    a[y * self->xsize + x] = c;
    break;
  }
  case RaveDataType_ULONG: {
    unsigned long *a = (unsigned long *) self->data;
    if (v < 0)
      v = 0;
    if (v > ULONG_MAX)
      v = ULONG_MAX;
    a[y * self->xsize + x] = (unsigned long)round(v);
    break;
  }
  case RaveDataType_FLOAT: {
    float *a = (float *) self->data;
    if (v > FLT_MAX)
      v = FLT_MAX;
    if (v < FLT_MIN)
      v = FLT_MIN;
    a[y * self->xsize + x] = v;
    break;
  }
  case RaveDataType_DOUBLE: {
    double *a = (double *) self->data;
    a[y * self->xsize + x] = v;
    break;
  }
  default:
    RAVE_WARNING1("RaveData2D_setValue: Unsupported type: '%d'\n", self->type);
    result = 0;
  }

  return result;
}

int RaveData2D_getValue(RaveData2D_t* self, long x, long y, double* v)
{
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->data == NULL) {
    RAVE_ERROR0("Atempting to get value when there is no data array");
    return 0;
  }

  if (v == NULL) {
    RAVE_ERROR0("Atempting to get a value without providing a value pointer");
    return 0;
  }

  if (x >= 0 && x < self->xsize && y >= 0 && y < self->ysize) {
    result = RaveData2D_getValueUnchecked(self, x, y, v);
  }

  return result;
}

int RaveData2D_getValueUnchecked(RaveData2D_t* self, long x, long y, double* v)
{
  int result = 1;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->data == NULL) {
    RAVE_ERROR0("Atempting to get value when there is no data array");
    return 0;
  }

  if (v == NULL) {
    RAVE_ERROR0("Atempting to get a value without providing a value pointer");
    return 0;
  }

  switch (self->type) {
  case RaveDataType_CHAR: {
    char *a = (char *) self->data;
    *v = a[y * self->xsize + x];
    break;
  }
  case RaveDataType_UCHAR: {
    unsigned char *a = (unsigned char *) self->data;
    *v = a[y * self->xsize + x];
    break;
  }
  case RaveDataType_SHORT: {
    short *a = (short *) self->data;
    *v = a[y * self->xsize + x];
    break;
  }
  case RaveDataType_USHORT: {
    unsigned short *a = (unsigned short *) self->data;
    *v = a[y * self->xsize + x];
    break;
  }
  case RaveDataType_INT: {
    int *a = (int *) self->data;
    *v = a[y * self->xsize + x];
    break;
  }
  case RaveDataType_UINT: {
    unsigned int *a = (unsigned int *) self->data;
    *v = a[y * self->xsize + x];
    break;
  }
  case RaveDataType_LONG: {
    long *a = (long *) self->data;
    *v = a[y * self->xsize + x];
    break;
  }
  case RaveDataType_ULONG: {
    unsigned long *a = (unsigned long *) self->data;
    *v = a[y * self->xsize + x];
    break;
  }
  case RaveDataType_FLOAT: {
    float *a = (float *) self->data;
    *v = a[y * self->xsize + x];
    break;
  }
  case RaveDataType_DOUBLE: {
    double *a = (double *) self->data;
    *v = a[y * self->xsize + x];
    break;
  }
  default:
    RAVE_WARNING1("RaveData2D_getValue: Unsupported type: '%d'\n", self->type);
    result = 0;
  }

  return result;
}

int RaveData2D_hasData(RaveData2D_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");

  if (self->data != NULL && self->xsize > 0 && self->ysize > 0) {
    return 1;
  }

  return 0;
}

double RaveData2D_min(RaveData2D_t* self)
{
  long x = 0, y = 0;
  double result = DBL_MAX;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (!RaveData2D_hasData(self)) {
    RAVE_ERROR0("No data in field");
    return 0.0;
  }
  for (x = 0; x < self->xsize; x++) {
    for (y = 0; y < self->ysize; y++) {
      double v;
      RaveData2D_getValueUnchecked(self, x, y, &v);
      if (v < result && (self->useNodata == 0 || (self->useNodata == 1 && self->nodata != v)))
        result = v;
    }
  }

  return result;
}

double RaveData2D_max(RaveData2D_t* self)
{
  long x = 0, y = 0;
  double result = DBL_MIN;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (!RaveData2D_hasData(self)) {
    RAVE_ERROR0("No data in field");
    return 0.0;
  }
  for (x = 0; x < self->xsize; x++) {
    for (y = 0; y < self->ysize; y++) {
      double v;
      RaveData2D_getValueUnchecked(self, x, y, &v);
      if (v > result && (self->useNodata == 0 || (self->useNodata == 1 && self->nodata != v)))
        result = v;
    }
  }

  return result;
}

RaveData2D_t* RaveData2D_concatX(RaveData2D_t* field, RaveData2D_t* other)
{
  RaveData2D_t *result = NULL, *newfield = NULL;
  long xsize = 0, ysize = 0, y = 0;
  int typesize = 0;

  RAVE_ASSERT((field != NULL), "field == NULL");
  if (!RaveData2D_hasData(field)) {
    RAVE_ERROR0("No data in field");
    return NULL;
  }

  if (other == NULL) {
    return NULL;
  }
  if (field->ysize != other->ysize ||
      field->type != other->type) {
    RAVE_WARNING0("Cannot concatenate two fields that have different y-sizes and/or different data types");
    return NULL;
  }
  newfield = RAVE_OBJECT_NEW(&RaveData2D_TYPE);
  if (newfield == NULL) {
    goto done;
  }
  xsize = field->xsize + other->xsize;
  ysize = field->ysize;
  newfield->nodata = field->nodata;
  newfield->useNodata = field->useNodata;

  if (!RaveData2D_createData(newfield, xsize, ysize, field->type, 0)) {
    RAVE_ERROR0("Failed to create field data");
    goto done;
  }

  typesize = get_ravetype_size(field->type);

  for (y = 0; y < ysize; y++) {
    unsigned char* nfd = newfield->data;
    unsigned char* fd = field->data;
    unsigned char* od = other->data;
    memcpy(&nfd[typesize*y*xsize], &fd[typesize*y*field->xsize], typesize * field->xsize);
    memcpy(&nfd[typesize*y*xsize + typesize*field->xsize], &od[typesize*y*other->xsize], typesize * other->xsize);
  }

  result = RAVE_OBJECT_COPY(newfield);
done:
  RAVE_OBJECT_RELEASE(newfield);
  return result;
}

RaveData2D_t* RaveData2D_concatY(RaveData2D_t* field, RaveData2D_t* other)
{
  RaveData2D_t *result = NULL, *newfield = NULL;
  long xsize = 0, ysize = 0;
  int typesize = 0;

  RAVE_ASSERT((field != NULL), "field == NULL");

  if (!RaveData2D_hasData(field)) {
    RAVE_ERROR0("No data in field");
    return NULL;
  }

  if (other == NULL) {
    return NULL;
  }
  if (field->xsize != other->xsize ||
      field->type != other->type) {
    RAVE_WARNING0("Cannot concatenate two fields that have different x-sizes and/or different data types");
    return NULL;
  }
  newfield = RAVE_OBJECT_NEW(&RaveData2D_TYPE);
  if (newfield == NULL) {
    goto done;
  }
  xsize = field->xsize;
  ysize = field->ysize + other->ysize;
  newfield->nodata = field->nodata;
  newfield->useNodata = field->useNodata;

  if (!RaveData2D_createData(newfield, xsize, ysize, field->type, 0)) {
    RAVE_ERROR0("Failed to create field data");
    goto done;
  }

  typesize = get_ravetype_size(field->type);

  unsigned char* nfd = newfield->data;
  unsigned char* fd = field->data;
  unsigned char* od = other->data;
  memcpy(&nfd[0], &fd[0], typesize * field->xsize * field->ysize);
  memcpy(&nfd[typesize * field->xsize * field->ysize], &od[0], typesize * other->xsize * other->ysize);

  result = RAVE_OBJECT_COPY(newfield);
done:
  RAVE_OBJECT_RELEASE(newfield);
  return result;
}

RaveData2D_t* RaveData2D_circshift(RaveData2D_t* field, int nx, int ny)
{
  RaveData2D_t *result = NULL, *newfield = NULL;
  long x = 0, y = 0;

  RAVE_ASSERT((field != NULL), "field == NULL");
  if (!RaveData2D_hasData(field)) {
    RAVE_ERROR0("No data in field");
    return NULL;
  }

  newfield = RAVE_OBJECT_CLONE(field);
  if (newfield == NULL) {
    return NULL;
  }
  RaveData2D_fill(newfield, 0.0);
  for (x = 0; x < field->xsize; x++) {
    int shiftedx = x + nx;
    if (shiftedx >= field->xsize) {
      shiftedx = shiftedx % field->xsize;
    } else if (shiftedx < 0) {
      while (shiftedx < 0) {
        shiftedx = shiftedx + field->xsize;
      }
    }
    for (y = 0; y < field->ysize; y++) {
      double v = 0.0;
      int shiftedy = y + ny;
      if (shiftedy >= field->ysize) {
        shiftedy = shiftedy % field->ysize;
      } else if (shiftedy < 0) {
        while (shiftedy < 0) {
          shiftedy = shiftedy + field->ysize;
        }
      }
      RaveData2D_getValueUnchecked(field, x, y, &v);

      if (!RaveData2D_setValue(newfield, shiftedx, shiftedy, v)) {
        RAVE_CRITICAL0("Coding error. Bad indexing in circshift!!!!");
        goto done;
      }
    }
  }
  result = RAVE_OBJECT_COPY(newfield);
done:
  RAVE_OBJECT_RELEASE(newfield);
  return result;
}

int RaveData2D_circshiftData(RaveData2D_t* field, int nx, int ny)
{
  RaveData2D_t *newfield = NULL;
  int result = 1;

  RAVE_ASSERT((field != NULL), "field == NULL");
  if (!RaveData2D_hasData(field)) {
    RAVE_ERROR0("No data in field");
    goto fail;
  }

  newfield = RaveData2D_circshift(field, nx, ny);
  if (newfield != NULL) {
    long sz = get_ravetype_size(field->type);
    long nbytes = field->xsize*field->ysize*sz;
    memcpy(field->data, newfield->data, nbytes);
  }

  result = 1;
fail:
  RAVE_OBJECT_RELEASE(newfield);
  return result;
}

static double eoperation_add(double v1, double v2)
{
  return v1 + v2;
}

RaveData2D_t* RaveData2D_addNumber(RaveData2D_t* field, double v)
{
  return RaveData2D_eoperationNumber(field, v, eoperation_add);
}

RaveData2D_t* RaveData2D_add(RaveData2D_t* field, RaveData2D_t* other)
{
  return RaveData2DInternal_eoperation(field, other, eoperation_add);
}

static double eoperation_sub(double v1, double v2)
{
  return v1 - v2;
}

RaveData2D_t* RaveData2D_subNumber(RaveData2D_t* field, double v)
{
  return RaveData2D_eoperationNumber(field, v, eoperation_sub);
}

RaveData2D_t* RaveData2D_sub(RaveData2D_t* field, RaveData2D_t* other)
{
  return RaveData2DInternal_eoperation(field, other, eoperation_sub);
}

static double eoperation_emul(double v1, double v2)
{
  return v1*v2;
}

RaveData2D_t* RaveData2D_mulNumber(RaveData2D_t* field, double v)
{
  return RaveData2D_eoperationNumber(field, v, eoperation_emul);
}

RaveData2D_t* RaveData2D_emul(RaveData2D_t* field, RaveData2D_t* other)
{
  return RaveData2DInternal_eoperation(field, other, eoperation_emul);
}

double eoperation_epow(double v1, double v2)
{
  return pow(v1, v2);
}

RaveData2D_t* RaveData2D_powNumber(RaveData2D_t* field, double v)
{
  return RaveData2D_eoperationNumber(field, v, eoperation_epow);
}

RaveData2D_t* RaveData2D_epow(RaveData2D_t* field, RaveData2D_t* other)
{
  return RaveData2DInternal_eoperation(field, other, eoperation_epow);
}

/**
 * Fills winarr with the winXsize * winYSize window around pos ox,oy
 * @param[in] field - self
 * @param[in] ox - origin x
 * @param[in] oy - origin y
 * @param[in] winXSize - x window size
 * @param[in] winYSize - y window size
 * @param[in,out] winarr - the values within the window. Values arranged as y0: x0-xN, y1: x0-xN, y2: x0-XN and so on
 */
void RaveData2DInternal_fillWindow(RaveData2D_t* field, long ox, long oy, long winXsize, long winYsize, double* winarr)
{
  long x = 0, y = 0;
  int index = 0;
  long halfY = winYsize / 2;
  long halfX = winXsize / 2;
  for (y = oy - halfY; y <= oy + halfY; y++) {
    for (x = ox - halfX; x <= ox + halfX; x++) {
      winarr[index] = 0.0;
      if (y >= 0 && y < field->ysize && x >= 0 && x < field->xsize) {
        double v;
        RaveData2D_getValueUnchecked(field,  x,  y,  &v);
        winarr[index] = v;
      } else {
        winarr[index] = 0.0;
      }
      index++;
    }
  }
}

int doublesortfunc(const void * a, const void * b) {
   return ( *(double*)a - *(double*)b );
}

double RaveData2DInternal_computeMedian(double* arr, long len, int useNodata, double nodata)
{
  double result = 0.0;
  double* tmp = NULL;
  if (useNodata && len > 0) {
    tmp = RAVE_MALLOC(sizeof(double) * len);
    if (tmp == NULL) {
      RAVE_ERROR0("Failed to compute median");
    } else {
      int i = 0;
      int bctr = len - 1;
      int fctr = 0;
      for (i = 0; i < len; i++) {
        if (arr[i] == nodata) {
          tmp[bctr--] = nodata;
        } else {
          tmp[fctr++] = arr[i];
        }
      }
      memcpy(arr, tmp, sizeof(double)*len);
      len = fctr;
    }
    RAVE_FREE(tmp);
  }
  qsort(arr, len, sizeof(double), doublesortfunc);
  result = arr[(int)(len / 2)];
  return result;
}

RaveData2D_t* RaveData2D_medfilt2(RaveData2D_t* field, long winXsize, long winYsize)
{
  long x = 0, y = 0, totalsize = 0;
  long xsize = 0, ysize = 0;
  double* medarr = NULL;
  RaveData2D_t* result = NULL;

  RAVE_ASSERT((field != NULL), "field == NULL");

  if (!RaveData2D_hasData(field)) {
    RAVE_ERROR0("No data in field");
    return NULL;
  }

  if (winXsize % 2 == 0 || winYsize %2 == 0 || winXsize < 0 || winYsize < 0) {
    RAVE_ERROR2("medfilt2 only supports positive odd-size windows at the moment: (%ld, %ld)", winXsize, winYsize);
    return NULL;
  }
  xsize = RaveData2D_getXsize(field);
  ysize = RaveData2D_getYsize(field);

  result = RaveData2D_zeros(xsize, ysize, field->type);
  if (result == NULL) {
    return NULL;
  }
  result->useNodata = field->useNodata;
  result->nodata = field->nodata;

  totalsize = winXsize * winYsize;
  medarr = RAVE_MALLOC(sizeof(double)*totalsize);

  for (x = 0; x < xsize; x++) {
    for (y = 0; y < ysize; y++) {
      double v=0.0;
      RaveData2D_getValueUnchecked(field, x, y, &v);
      if (v == field->nodata)
        continue;
      RaveData2DInternal_fillWindow(field, x, y, winXsize, winYsize, medarr);
      RaveData2D_setValueUnchecked(result, x, y, RaveData2DInternal_computeMedian(medarr, totalsize, field->useNodata, field->nodata));
    }
  }

  RAVE_FREE(medarr);
  return result;
}

RaveData2D_t* RaveData2D_cumsum(RaveData2D_t* field, int dir)
{
	RaveData2D_t* result = NULL;
	long xsize, ysize, x, y;
	RAVE_ASSERT((field != NULL), "field == NULL");
	xsize = RaveData2D_getXsize(field);
	ysize = RaveData2D_getYsize(field);
	result = RaveData2D_zeros(xsize, ysize, RaveDataType_DOUBLE);
	if (result == NULL) {
		return NULL;
	}
  result->useNodata = field->useNodata;
  result->nodata = field->nodata;

  if (dir == 1) {
    /* column wise */
	  for (x = 0; x < xsize; x++) {
	    double sum = 0.0;
	    for(y = 0; y < ysize; y++) {
	      double v = 0;
	      RaveData2D_getValueUnchecked(field, x, y, &v);
	      sum += v;
	      RaveData2D_setValueUnchecked(result, x, y, sum);
	    }
	  }
	} else {
		/* row wise */
    for(y = 0; y < ysize; y++) {
      double sum = 0.0;
      for (x = 0; x < xsize; x++) {
        double v = 0;
        RaveData2D_getValueUnchecked(field, x, y, &v);
        sum += v;
        RaveData2D_setValueUnchecked(result, x, y, sum);
      }
    }
	}
	return result;
}

RaveData2D_t* RaveData2D_movingstd(RaveData2D_t* field, long nx, long ny)
{
  long x = 0, y = 0;
  long i = 0, j = 0;
  RaveData2D_t *mstd = NULL, *result = NULL;
  RaveData2D_t *weight = NULL;
  RAVE_ASSERT((field != NULL), "field == NULL");
  if (!field->useNodata) {
    RAVE_ERROR0("When creating movingstd nodata usage should be activated");
    return NULL;
  }
  mstd = RaveData2D_zeros(field->xsize, field->ysize, RaveDataType_DOUBLE);
  weight = RaveData2D_zeros(field->xsize, field->ysize, RaveDataType_DOUBLE);
  if (weight == NULL || mstd == NULL) {
    goto done;
  }
  field->useNodata = 0; /* Think it should be 1 but matlab, criteria says Weight(X>-900. | isnan(X)==0)=1.0 which basically says anything that is != nan is weight 1*/
  mstd->useNodata = 1; /* Should be active */
  mstd->nodata = field->nodata;
  for (x = 0; x < field->xsize; x++) {
    for (y = 0; y < field->ysize; y++) {
      double v = 0.0;
      RaveData2D_getValueUnchecked(field, x, y, &v);
      if (!field->useNodata || v != field->nodata) {
        RaveData2D_setValueUnchecked(weight, x, y, 1.0);
      }
    }
  }

  for (x = 0; x < field->xsize; x++) {
    for (y = 0; y < field->ysize; y++) {
      double valueMstd = 0.0;
      double valueSumWeight = 0.0;
      double valueWeight = 0.0;
      double valueX = 0.0;

      RaveData2D_getValueUnchecked(weight, x, y, &valueWeight);
      RaveData2D_getValueUnchecked(field, x, y, &valueX);

      for (j = ny; j >= -ny; j--) {
        for (i = nx; i >= -nx; i--) {
          long xi = (x + i)%field->xsize;
          long yj = (y + j)%field->ysize;
          double valueCircshiftWeight = 0.0;
          double valueCircshiftX = 0.0;

          if (i==0 && j==0) continue;

          while (xi < 0) {
            xi += field->xsize;
          }
          while (yj < 0) {
            yj += field->ysize;
          }

          RAVE_ASSERT((xi >= 0 && xi <field->xsize && yj >= 0 && yj < field->ysize), "BAD PROGRAMMING");

          RaveData2D_getValueUnchecked(weight, xi, yj, &valueCircshiftWeight);
          RaveData2D_getValueUnchecked(field, xi, yj, &valueCircshiftX);

          valueMstd = valueMstd + valueWeight * valueCircshiftWeight * (valueCircshiftX - valueX)*(valueCircshiftX - valueX);
          valueSumWeight = valueSumWeight + valueWeight * valueCircshiftWeight;
        }
      }

      if (valueSumWeight >= 3.0) {
        if (valueMstd >= 0) {
          RaveData2D_setValueUnchecked(mstd, x, y, sqrt(valueMstd) / valueSumWeight);
        } else {
          RaveData2D_setValueUnchecked(mstd, x, y, mstd->nodata);
        }
      } else {
        RaveData2D_setValueUnchecked(mstd, x, y, mstd->nodata);
      }
    }
  }

  result = RAVE_OBJECT_COPY(mstd);

done:
  RAVE_OBJECT_RELEASE(mstd);
  RAVE_OBJECT_RELEASE(weight);
  field->useNodata = 1;
  return result;
}

long* RaveData2D_hist(RaveData2D_t* field, int bins, long* nnodata)
{
  long* result = NULL;
  double max = 0.0, min = 0.0, dist = 0.0, scale = 0.0;
  long x = 0, y = 0;
  long histidx = 0;
  RAVE_ASSERT((field != NULL), "field == NULL");
  if (nnodata == NULL) {
    RAVE_ERROR0("No nodata ptr");
    return NULL;
  }
  *nnodata = 0;
  if (!RaveData2D_hasData(field)) {
    RAVE_ERROR0("No data in field");
    return NULL;
  }

  if (bins <= 0) {
    RAVE_ERROR0("bins must be greater than 0");
    return NULL;
  }
  result = RAVE_MALLOC(sizeof(long)*bins);
  if (result == NULL) {
    return NULL;
  }

  /**Calculate bin ranges as scale = (max - min) / nbins
   * Bin1: max >= x <= max + scale
   * Bin2: max + scale > x <= max + 2*scale
   * Bin3: max + 2*scale > x <= max + 3*scale
   * and so on.....
   *
   * Example: y=[-7  -5  -3  -2  -2  -1   0   1   2   3   3   4   5   5   6   7]
   *          nbins = 4
   *
   * scale = (7 - (-7)) / 4 = 3.5
   * =>
   * b1 = -7 => -3.5
   * b2 = -3.49 => 0
   * b3 = 0.01 => 3.5
   * b4 = 3.51 => 7
   */

  memset(result, 0, sizeof(long)*bins);
  min = RaveData2D_min(field);
  max = RaveData2D_max(field);
  dist = max - min;
  scale = dist / (double)bins;

  for (x = 0; x < field->xsize; x++) {
    for (y = 0; y < field->ysize; y++) {
      double v;
      RaveData2D_getValueUnchecked(field, x, y, &v);
      if (field->useNodata && v == field->nodata) {
        *nnodata = *nnodata + 1;
        continue;
      }

      if (v <= min + scale) {
        histidx = 0;
      } else {
        histidx = (long)((v - min) / scale);
        if (min + ((double)histidx)*scale - v == 0.0) { /* Exactly on v, need to adjust downwards since we want to have n >= x < n + 1 */
          histidx--;
        }
      }

      if (histidx < 0) {
        RAVE_CRITICAL0("Coding error in histogram coding");
        RAVE_FREE(result);
        return NULL;
      } else if (histidx >= bins) {
        histidx = bins - 1;
      }
      result[histidx] ++;
    }
  }

  return result;
}

int RaveData2D_entropy(RaveData2D_t* field, int bins, double* entropy)
{
  long* histogram = NULL;
  double* normCount = NULL;
  double sumEntropy = 0.0;
  int result = 0;
  int i = 0;
  long nnodata = 0;
  RAVE_ASSERT((field != NULL), "field == NULL");
  if (!RaveData2D_hasData(field)) {
    RAVE_ERROR0("No data in field");
    goto done;
  }

  if (bins <= 0) {
    RAVE_ERROR0("bins must be > 0");
    goto done;
  }

  if (entropy == NULL) {
    RAVE_ERROR0("entropy == NULL");
    goto done;
  }

  histogram = RaveData2D_hist(field, bins, &nnodata);
  if (histogram != NULL) {
    double totalCount = (double)field->xsize*field->ysize - nnodata; /* Since we are getting histogram we know that count must be same as number of entries in array, at least as long as we don't take into account NaN and stuff */
    normCount = RAVE_MALLOC(sizeof(double)*bins);
    if (normCount == NULL) {
      goto done;
    }
    for (i = 0; i < bins; i++) {
      normCount[i] = histogram[i] / totalCount;
    }
    for (i = 0; i < bins; i++) {
      if (normCount[i] > 0.0) { /* No real chance for negative values here since it's a count */
        sumEntropy += normCount[i] * log2(normCount[i]);
      }
    }
    *entropy = -sumEntropy;
  }
  result = 1;
done:
  RAVE_FREE(histogram);
  RAVE_FREE(normCount);
  return result;
}

void RaveData2D_disp(RaveData2D_t* field)
{
  long x, y;
  RAVE_ASSERT((field != NULL), "field == NULL");

  Rave_printf("RaveData2D=[ (%ld, %ld)\n", field->xsize, field->ysize);
  for (y = 0; y < field->ysize; y++) {
    Rave_printf("  [");
    for (x = 0; x < field->xsize; x++) {
      double v = 0.0;
      RaveData2D_getValueUnchecked(field,  x,  y,  &v);
      if (x > 0) {
        Rave_printf(", ");
      }
      Rave_printf("%0.3f", v);
    }
    Rave_printf("]");
    if (y==field->ysize-1) {
      Rave_printf("];\n");
    } else {
      Rave_printf(",\n");
    }
  }
}

const char* RaveData2D_str(RaveData2D_t* field)
{
  long x, y;
  static char* result = NULL;
  char buff[1024];
  size_t resultsize = 0;
  RAVE_ASSERT((field != NULL), "field == NULL");
  if (result == NULL) {
    result = malloc(sizeof(char) * 4096);
    resultsize = 4096;
  }
  snprintf(result, resultsize, "(%ld x %ld) [\n", field->xsize, field->ysize);
  for (y = 0; y < field->ysize; y++) {
    strcat(result, "   [");
    for (x = 0; x < field->xsize; x++) {
      double v = 0.0;
      size_t bs = strlen(result);
      if (bs > resultsize - 256) {
        size_t newsize = resultsize * 2;
        char* tmp = realloc(result, sizeof(char)*newsize);
        if (tmp != NULL) {
          result = tmp;
          resultsize = newsize;
        } else {
          RAVE_ERROR0("Failed to reallocate array, returning as much as possible");
          return result;
        }
      }
      RaveData2D_getValueUnchecked(field,  x,  y,  &v);
      if (x > 0) {
        snprintf(buff, 1024, ", %0.3f", v);
      } else {
        snprintf(buff, 1024, "%0.3f", v);
      }
      strcat(result, buff);
    }
    strcat(result, "]");
    if (y==field->ysize-1) {
      strcat(result, "];");
    } else {
      strcat(result, ",\n");
    }
  }
  return result;
}

void RaveData2D_replace(RaveData2D_t* field, double v, double v2)
{
  long x, y;
  RAVE_ASSERT((field != NULL), "field == NULL");
  for (y = 0; y < field->ysize; y++) {
    for (x = 0; x < field->xsize; x++) {
      double fv;
      RaveData2D_getValueUnchecked(field,  x,  y, &fv);
      if (fv == v) {
        RaveData2D_setValueUnchecked(field, x, y, v2);
      }
    }
  }
}

RaveData2D_t* RaveData2D_zeros(long xsize, long ysize, RaveDataType type)
{
  RaveData2D_t* result = RaveData2D_createObject(xsize, ysize, type);
  if (result != NULL) {
    RaveData2D_fill(result, 0.0);
  }
  return result;
}

RaveData2D_t* RaveData2D_ones(long xsize, long ysize, RaveDataType type)
{
  RaveData2D_t* result = RaveData2D_createObject(xsize, ysize, type);
  if (result != NULL) {
    RaveData2D_fill(result, 1.0);
  }
  return result;
}

RaveData2D_t* RaveData2D_createObject(long xsize, long ysize, RaveDataType type)
{
  RaveData2D_t* result = NULL;
  result = RAVE_OBJECT_NEW(&RaveData2D_TYPE);
  if (result != NULL) {
    if (!RaveData2D_createData(result, xsize, ysize, type, 0)) {
      RAVE_OBJECT_RELEASE(result);
    }
  }
  return result;
}

/*@} End of Interface functions */
RaveCoreObjectType RaveData2D_TYPE = {
    "RaveData2D",
    sizeof(RaveData2D_t),
    RaveData2D_constructor,
    RaveData2D_destructor,
    RaveData2D_copyconstructor
};
