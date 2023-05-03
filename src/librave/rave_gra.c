/* --------------------------------------------------------------------
Copyright (C) 2014 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Gauge radar adjustment (GRA). This class performs the actual gauge adjustment
 * using the derived coefficients. This can be done in two ways: one for ACRR products
 * (most common) and another for reflectivity (any of DBZH, DBZV, TH, TV).
 * In the case of ACRR, the parameter is already mm which is good.
 * In the case of reflectivity, the parameter needs to be converted to R (mm/hr), the correction applied, and
 * then the result converted back to reflectivity. This should be done in C. Functionality exists already in
 * raveutil.c/h: dBZ2R or raw2R and back.
 * Default Z-R coefficients are given in rave_defined.ZR_A and ZR_b. The C could look something like this (from N2):
 * F = A + B*DIST + C*pow(DIST, 2.0);
 * F = RAVEMIN(F, 2.0);    upper threshold 20 dBR
 * F = RAVEMAX(F, -0.25);  lower threshold -2.5 dBR
 * out = R*pow(10.0, F);
 *
 * final lower threhold on gauge-adjusted result
 *
 * if (out < lt) { out=0.0; }
 *
 * This object does support \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2014-03-28
 */
#include "rave_gra.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "raveutil.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

/**
 * Represents the gra applier
 */
struct _RaveGra_t {
  RAVE_OBJECT_HEAD /** Always on top */
  double A; /**< A - Coefficient */
  double B; /**< B - Coefficient */
  double C; /**< C - Coefficient */
  double upperThreshold; /**< in 10ths of dBR, default 2.0 (20 dBR) */
  double lowerThreshold; /**< in 10ths of dBR, default -0.25 (-2.5 dBR) */
  double zrA; /**< the ZR A coefficient when converting from reflectivity to MM/H */
  double zrb; /**< the ZR b coefficient when converting from reflectivity to MM/H */
};

/*@{ Private functions */

/**
 * Constructor
 */
static int RaveGra_constructor(RaveCoreObject* obj)
{
  RaveGra_t* self = (RaveGra_t*)obj;
  self->A = 0;
  self->B = 0;
  self->C = 0;
  self->upperThreshold = 2.0;
  self->lowerThreshold = -0.25;
  self->zrA = 200.0;
  self->zrb = 1.6;
  return 1;
}

/**
 * Copy constructor
 */
static int RaveGra_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  RaveGra_t* self = (RaveGra_t*)obj;
  RaveGra_t* src = (RaveGra_t*)obj;
  self->A = src->A;
  self->B = src->B;
  self->C = src->C;
  self->upperThreshold = src->upperThreshold;
  self->lowerThreshold = src->lowerThreshold;
  self->zrA = src->zrA;
  self->zrb = src->zrb;
  return 1;
}

/**
 * Destructor
 */
static void RaveGra_destructor(RaveCoreObject* obj)
{
  //RaveGra_t* self = (RaveGra_t*)obj;
}

static double RaveGraInternal_getAttributeDoubleValueFromField(RaveField_t* field, const char* name, double defaultValue)
{
  RaveAttribute_t* attr = RaveField_getAttribute(field, name);
  double result = defaultValue;
  if (attr != NULL) {
    RaveAttribute_getDouble(attr, &result);
  }
  RAVE_OBJECT_RELEASE(attr);
  return result;
}

/*@} End of Private functions */

/*@{ Interface functions */
void RaveGra_setA(RaveGra_t* self, double A)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->A = A;
}

double RaveGra_getA(RaveGra_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->A;
}

void RaveGra_setB(RaveGra_t* self, double B)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->B = B;
}

double RaveGra_getB(RaveGra_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->B;
}

void RaveGra_setC(RaveGra_t* self, double C)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->C = C;
}

double RaveGra_getC(RaveGra_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->C;
}

void RaveGra_setUpperThreshold(RaveGra_t* self, double threshold)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->upperThreshold = threshold;
}

double RaveGra_getUpperThreshold(RaveGra_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->upperThreshold;
}

void RaveGra_setLowerThreshold(RaveGra_t* self, double threshold)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->lowerThreshold = threshold;
}

double RaveGra_getLowerThreshold(RaveGra_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->lowerThreshold;
}

void RaveGra_setZRA(RaveGra_t* self, double zrA)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->zrA = zrA;
}

double RaveGra_getZRA(RaveGra_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->zrA;
}

void RaveGra_setZRB(RaveGra_t* self, double zrb)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->zrb = zrb;
}

double RaveGra_getZRB(RaveGra_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->zrb;
}

/**
 * Applies the gra coefficient on MM/H.
 * @param[in] self - self
 * @param[in] distance - the distance to the radar for this value. in unit km.
 * @param[in] value - the MM/H
 * @param[in] dtype - the data type
 * @returns the corrected value
 */
static double applyAcrr(RaveGra_t* self, double distance, double value, RaveValueType dtype)
{
  double result = value;

  if (dtype == RaveValueType_DATA) {
    double F = (self->A + self->B * distance + self->C * distance * distance) / 10.0;
    F = RAVEMIN(F, self->upperThreshold);
    F = RAVEMAX(F, self->lowerThreshold);
    result = value * pow(10.0, F);
    if (result < self->lowerThreshold) {
      result = 0.0;
    }
  }
  return result;
}

/**
 * Applies the gra coefficient on reflectivity by converting the dbz to mm/h using zrA and zrb.
 * @param[in] self - self
 * @param[in] distance - the distance to the radar for this value. in unit km.
 * @param[in] value - the MM/H
 * @param[in] dtype - the data type
 * @returns the corrected value
 */
static double applyReflectivity(RaveGra_t* self, double distance, double value, RaveValueType dtype)
{
  double result = value;

  if (dtype == RaveValueType_DATA) {
    double F = (self->A + self->B * distance + self->C * distance * distance) / 10.0;
    F = RAVEMIN(F, self->upperThreshold);
    F = RAVEMAX(F, self->lowerThreshold);
    result = dBZ2R(value, self->zrA, self->zrb) * pow(10.0, F);
    if (result < self->lowerThreshold) {
      result = 0.0;
    }
    result = R2dBZ(result, self->zrA, self->zrb);
  }
  return result;
}

CartesianParam_t* RaveGra_apply(RaveGra_t* self, RaveField_t* distance, CartesianParam_t* parameter)
{
  CartesianParam_t* result = NULL;
  CartesianParam_t* grafield = NULL;
  RaveAttribute_t* howTaskArgs = NULL;

  long x = 0, y = 0, xsize = 0, ysize = 0;
  const char* quantity;
  double dgain = 0.0, doffset = 0.0;
  double (*applyGra)(RaveGra_t*, double, double, RaveValueType) = applyReflectivity; /* FP using applyAcrr or applyReflectivity */
  char coeffs[256];

  RAVE_ASSERT((self != NULL), "self == NULL");

  if (distance == NULL || parameter == NULL) {
    RAVE_ERROR0("Neither distance field or cartesian parameter may be NULL");
    goto fail;
  }

  if (RaveField_getXsize(distance) != CartesianParam_getXSize(parameter) ||
      RaveField_getYsize(distance) != CartesianParam_getYSize(parameter)) {
    RAVE_ERROR0("Distance field and cartesian parameter should have the same x/y-dimensions");
    goto fail;
  }

  grafield = RAVE_OBJECT_CLONE(parameter);
  xsize = CartesianParam_getXSize(grafield);
  ysize = CartesianParam_getYSize(grafield);
  quantity = CartesianParam_getQuantity(grafield);
  if (quantity != NULL && strcmp("ACRR", quantity) == 0) {
    applyGra = applyAcrr;
  }
  doffset = RaveGraInternal_getAttributeDoubleValueFromField(distance, "what/offset", 0.0);
  dgain = RaveGraInternal_getAttributeDoubleValueFromField(distance, "what/gain", 1.0);

  for (y = 0; y < ysize; y++) {
    for (x = 0; x < xsize; x++) {
      RaveValueType dt = RaveValueType_UNDEFINED;
      double v = 0.0, dist = 0.0;
      dt = CartesianParam_getConvertedValue(grafield, x, y, &v);
      RaveField_getValue(distance, x, y, &dist);
      double distanceInMeters = doffset + dgain*dist;
      CartesianParam_setConvertedValue(grafield, x, y, applyGra(self, distanceInMeters/1000.0, v, dt), dt);
    }
  }
  snprintf(coeffs, 256, "GRA: A=%f, B=%f, C=%f, low_db=%f, high_db=%f",self->A, self->B, self->C, self->lowerThreshold, self->upperThreshold);
  howTaskArgs = RaveAttributeHelp_createString("how/task_args", coeffs);
  if (howTaskArgs == NULL || !CartesianParam_addAttribute(grafield, howTaskArgs)) {
    RAVE_ERROR0("Could not add how/task_args to gra field");
  }

  result = RAVE_OBJECT_COPY(grafield);
fail:
  RAVE_OBJECT_RELEASE(grafield);
  RAVE_OBJECT_RELEASE(howTaskArgs);
  return result;
}

/*@} End of Interface functions */

RaveCoreObjectType RaveGra_TYPE = {
    "RaveGra",
    sizeof(RaveGra_t),
    RaveGra_constructor,
    RaveGra_destructor,
    RaveGra_copyconstructor
};
