/* --------------------------------------------------------------------
Copyright (C) 2012 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Implementation of the Precipitation accumulation - ACRR algorithm
 * This object does NOT support \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2012-05-31
 */
#include "rave_acrr.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "raveutil.h"
#include <string.h>

/** The resolution to use for scaling the distance from pixel to used radar. */
/** By multiplying the values in the distance field by 1000, we get the value in unit meters. */
#define ACRR_DISTANCE_TO_RADAR_RESOLUTION 1000.0

/**
 * Represents the acrr generator
 */
struct _RaveAcrr_t {
  RAVE_OBJECT_HEAD /** Always on top */
  int initialized; /**< if the acrr generator has been initialized (first call to sum has been made) */
  double nodata; /**< the nodata value that will be used in the accumulation */
  double undetect; /**< the undetect value that will be used in the accumulation */
  char* quantity;  /**< Quantity we are working on */
  char* howtaskfieldname; /**< the name of the how task field to be used in the parameters, default se.smhi.composite.distance.radar */
  RaveField_t* nd; /**< nodata hits */
  RaveField_t* dd; /**< Distances */
  RaveField_t* cd; /**< Distance counts */
  RaveField_t* sd; /**< Accumulations */
  int nracc; /**< number of accumulations */
};

/*@{ Private functions */
/* Forward declaration of RaveAccrInternal_setQuantity.
 * Sets the quantity that is beeing processed, this should be taken
 * from the first parameter inserted into sum.
 *
 * @param[in] self - self
 * @param[in] quantity - the quantity (or NULL to reset)
 * @return 1 on success otherwise 0
 */
static int RaveAcrrInternal_setQuantity(RaveAcrr_t* self, const char* quantity);

/**
 * Constructor
 */
static int RaveAcrr_constructor(RaveCoreObject* obj)
{
  RaveAcrr_t* self = (RaveAcrr_t*)obj;
  self->initialized = 0;
  self->nodata = -1.0;
  self->undetect = 0.0;
  self->quantity = NULL;
  self->howtaskfieldname = NULL;
  self->nd = NULL;
  self->dd = NULL;
  self->cd = NULL;
  self->sd = NULL;
  self->nracc = 0;
  self->howtaskfieldname = RAVE_STRDUP("se.smhi.composite.distance.radar");
  if (self->howtaskfieldname == NULL) {
    RAVE_ERROR0("Could not intialized howtaskfieldname");
    goto fail;
  }
  return 1;
fail:
  RAVE_FREE(self->howtaskfieldname);
  return 0;
}

/**
 * Copy constructor
 */
static int RaveAcrr_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  RaveAcrr_t* self = (RaveAcrr_t*)obj;
  RaveAcrr_t* src = (RaveAcrr_t*)obj;
  self->initialized = src->initialized;
  self->nodata = src->nodata;
  self->undetect = src->undetect;
  self->quantity = NULL;
  self->howtaskfieldname = NULL;
  self->nd = RAVE_OBJECT_CLONE(src->nd);
  self->dd = RAVE_OBJECT_CLONE(src->dd);
  self->cd = RAVE_OBJECT_CLONE(src->cd);
  self->sd = RAVE_OBJECT_CLONE(src->sd);
  self->nracc = src->nracc;
  if (self->nd == NULL || self->dd == NULL ||
      self->cd == NULL || self->sd == NULL ||
      !RaveAcrrInternal_setQuantity(self, src->quantity) ||
      !RaveAcrr_setQualityFieldName(self, RaveAcrr_getQualityFieldName(src))) {
    goto fail;
  }

  return 1;
fail:
  RAVE_OBJECT_RELEASE(self->nd);
  RAVE_OBJECT_RELEASE(self->dd);
  RAVE_OBJECT_RELEASE(self->cd);
  RAVE_OBJECT_RELEASE(self->sd);
  RAVE_FREE(self->quantity);
  RAVE_FREE(self->howtaskfieldname);
  return 0;
}

/**
 * Destructor
 */
static void RaveAcrr_destructor(RaveCoreObject* obj)
{
  RaveAcrr_t* self = (RaveAcrr_t*)obj;
  RAVE_OBJECT_RELEASE(self->nd);
  RAVE_OBJECT_RELEASE(self->dd);
  RAVE_OBJECT_RELEASE(self->cd);
  RAVE_OBJECT_RELEASE(self->sd);
  RAVE_FREE(self->quantity);
  RAVE_FREE(self->howtaskfieldname);
}

static int RaveAcrrInternal_setQuantity(RaveAcrr_t* self, const char* quantity)
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


static int RaveAcrrInternal_initialize(RaveAcrr_t* self, CartesianParam_t* param)
{
  long xsize = CartesianParam_getXSize(param);
  long ysize = CartesianParam_getYSize(param);
  self->nd = RAVE_OBJECT_NEW(&RaveField_TYPE);
  self->dd = RAVE_OBJECT_NEW(&RaveField_TYPE);
  self->cd = RAVE_OBJECT_NEW(&RaveField_TYPE);
  self->sd = RAVE_OBJECT_NEW(&RaveField_TYPE);
  self->nracc = 0;

  if (self->nd == NULL || self->dd == NULL || self->cd == NULL || self->sd == NULL ||
      !RaveField_createData(self->nd, xsize, ysize, RaveDataType_SHORT) ||
      !RaveField_createData(self->dd, xsize, ysize, RaveDataType_DOUBLE) ||
      !RaveField_createData(self->cd, xsize, ysize, RaveDataType_SHORT) ||
      !RaveField_createData(self->sd, xsize, ysize, RaveDataType_DOUBLE)) {
    RAVE_ERROR0("Failed to initialize memory");
    goto fail;
  }
  if (CartesianParam_getQuantity(param) == NULL ||
      !RaveAcrrInternal_setQuantity(self, CartesianParam_getQuantity(param))) {
    RAVE_ERROR0("Problems initializing quantity");
    goto fail;
  }

  self->initialized = 1;
  return 1;
fail:
  RAVE_OBJECT_RELEASE(self->nd);
  RAVE_OBJECT_RELEASE(self->dd);
  RAVE_OBJECT_RELEASE(self->cd);
  RAVE_OBJECT_RELEASE(self->sd);
  return 0;
}

static int RaveAcrrInternal_verify(RaveAcrr_t* self, CartesianParam_t* param)
{
  const char* pquantity = CartesianParam_getQuantity(param);
  long xsize = CartesianParam_getXSize(param);
  long ysize = CartesianParam_getYSize(param);

  if (self->initialized == 0 ||
      pquantity == NULL ||
      strcmp(pquantity, self->quantity) != 0 ||
      xsize != RaveField_getXsize(self->sd) ||
      ysize != RaveField_getYsize(self->sd)) {
    RAVE_ERROR0("Not same dimensions, quantity of previous data and provided data");
    goto fail;
  }

  return 1;
fail:
  return 0;
}

static int RaveAcrrInternal_getDoubleAttributeValueFromField(RaveField_t* field, const char* name, double* v)
{
  RaveAttribute_t* attr = NULL;
  int result = 0;
  RAVE_ASSERT((field != NULL), "field == NULL");
  RAVE_ASSERT((v != NULL), "v == NULL");
  attr = RaveField_getAttribute(field, name);
  if (attr != NULL && RaveAttribute_getFormat(attr) == RaveAttribute_Format_Double) {
    result = RaveAttribute_getDouble(attr, v);
  }
  RAVE_OBJECT_RELEASE(attr);
  return result;
}

static int RaveAcrrInternal_addDoubleAttributeToField(RaveField_t* field, const char* name, double value)
{
  RaveAttribute_t* attr = NULL;
  int result = 0;

  RAVE_ASSERT((field != NULL), "field == NULL");

  attr = RaveAttributeHelp_createDouble(name, value);
  if (attr != NULL) {
    result = RaveField_addAttribute(field, attr);
  }
  RAVE_OBJECT_RELEASE(attr);
  return result;
}

static int RaveAcrrInternal_addStringAttributeToField(RaveField_t* field, const char* name, const char* value)
{
  RaveAttribute_t* attr = NULL;
  int result = 0;

  RAVE_ASSERT((field != NULL), "field == NULL");

  attr = RaveAttributeHelp_createString(name, value);
  if (attr != NULL) {
    result = RaveField_addAttribute(field, attr);
  }
  RAVE_OBJECT_RELEASE(attr);
  return result;
}

static int RaveAcrrInternal_addDoubleAttributeToParam(CartesianParam_t* param, const char* name, double value)
{
  RaveAttribute_t* attr = NULL;
  int result = 0;

  RAVE_ASSERT((param != NULL), "param == NULL");

  attr = RaveAttributeHelp_createDouble(name, value);
  if (attr != NULL) {
    result = CartesianParam_addAttribute(param, attr);
  }
  RAVE_OBJECT_RELEASE(attr);
  return result;
}

/*@} End of Private functions */

/*@{ Interface functions */
int RaveAcrr_sum(RaveAcrr_t* self, CartesianParam_t* param, double zr_a, double zr_b)
{
  int result = 0;
  long xsize = 0, ysize = 0;
  long x = 0, y = 0;
  RaveField_t* dfield = NULL;
  double dgain = 1.0;
  double doffset = 0.0;
  RAVE_ASSERT((self != NULL), "self == NULL");

  dfield = CartesianParam_getQualityFieldByHowTask(param, RaveAcrr_getQualityFieldName(self));
  if (dfield == NULL) {
    RAVE_ERROR1("Could not find quality field '%s'", RaveAcrr_getQualityFieldName(self));
    goto done;
  }

  if (self->initialized == 0) {
    if (!RaveAcrrInternal_initialize(self, param)) {
      goto done;
    }
  } else {
    if (!RaveAcrrInternal_verify(self, param)) {
      goto done;
    }
  }

  if (!RaveAcrrInternal_getDoubleAttributeValueFromField(dfield, "what/offset", &doffset)) {
    RAVE_INFO0("Could not find what/offset in quality field, defaulting to 0.0");
    doffset = 0.0;
  }

  if (!RaveAcrrInternal_getDoubleAttributeValueFromField(dfield, "what/gain", &dgain)) {
    RAVE_INFO0("Could not find what/gain in quality field, defaulting to 1.0");
    dgain = 1.0;
  }

  xsize = CartesianParam_getXSize(param);
  ysize = CartesianParam_getYSize(param);

  self->nracc += 1;

  for (y = 0; y < ysize; y++) {
    for (x = 0; x < xsize; x++) {
      double v = 0.0, acrr = 0.0;
      RaveValueType rvt = CartesianParam_getConvertedValue(param, x, y, &v);
      RaveField_getValue(self->sd, x, y, &acrr);

      if (rvt == RaveValueType_DATA || rvt == RaveValueType_UNDETECT) {
        double dist = 0.0, dist_sum = 0.0, ndist = 0.0;

        RaveField_getValue(dfield, x, y, &dist);
        RaveField_getValue(self->dd, x, y, &dist_sum);
        dist_sum += ((dist*dgain + doffset) / 1000.0);  /* km */
        RaveField_setValue(self->dd, x, y, dist_sum);
        RaveField_getValue(self->cd, x, y, &ndist);
        ndist += 1;
        RaveField_setValue(self->cd, x, y, ndist);

        if (rvt == RaveValueType_DATA) {
          double rr = dBZ2R(v, zr_a, zr_b);
          acrr += rr;
          RaveField_setValue(self->sd, x, y, acrr);
        }

      } else if (rvt == RaveValueType_NODATA) {
        double nval = 0.0;
        RaveField_getValue(self->nd, x, y, &nval);
        nval += 1;
        RaveField_setValue(self->nd, x, y, nval);
      }
    }
  }

  result = 1;
done:
  RAVE_OBJECT_RELEASE(dfield);
  return result;
}

CartesianParam_t* RaveAcrr_accumulate(RaveAcrr_t* self, double acpt, long N, double hours)
{
  CartesianParam_t* result = NULL;
  CartesianParam_t* param = NULL;
  RaveField_t* qfield = NULL;

  long xsize = 0, ysize = 0;
  long x = 0, y = 0;
  long acceptN = (long)(acpt * (double)N);

  RAVE_ASSERT((self != NULL), "self == NULL");

  if (self->initialized == 0) {
    RAVE_ERROR0("acrr has not got any data to perform accumulation on");
    goto done;
  }
  if (acpt < 0.0 || acpt > 1.0) {
    RAVE_ERROR0("ACCEPT not >= 0 and <= 1.0");
    goto done;
  }
  xsize = RaveField_getXsize(self->sd);
  ysize = RaveField_getYsize(self->sd);

  param = RAVE_OBJECT_NEW(&CartesianParam_TYPE);
  qfield = RAVE_OBJECT_NEW(&RaveField_TYPE);
  if (param == NULL ||
      qfield == NULL ||
      !CartesianParam_createData(param, xsize, ysize, RaveDataType_DOUBLE, 0) ||
      !RaveField_createData(qfield, xsize, ysize, RaveDataType_DOUBLE) ||
      !CartesianParam_setQuantity(param, "ACRR") ||
      !RaveAcrrInternal_addStringAttributeToField(qfield, "how/task", "se.smhi.composite.distance.radar") ||
      !RaveAcrrInternal_addDoubleAttributeToField(qfield, "what/gain", ACRR_DISTANCE_TO_RADAR_RESOLUTION) ||
      !RaveAcrrInternal_addDoubleAttributeToField(qfield, "what/offset", 0.0) ||
      !RaveAcrrInternal_addDoubleAttributeToParam(param, "what/prodpar", hours)) {
    RAVE_ERROR0("Failed to create cartesian parameter");
    goto done;
  }
  CartesianParam_setNodata(param, self->nodata);
  CartesianParam_setUndetect(param, self->undetect);

  for (y = 0; y < ysize; y++) {
    for (x = 0; x < xsize; x++) {
      double dnval = 0.0, acrr = 0.0;
      long nval = 0;
      RaveField_getValue(self->nd, x, y, &dnval);
      CartesianParam_setValue(param, x, y, self->nodata);
      RaveField_setValue(qfield, x, y, self->nodata);
      nval = (long)dnval;

      if ((long)self->nracc < N) {
        nval = nval + (N - self->nracc);
      }
      if ((long)nval <= acceptN) {

        double dist_sum = 0.0, ndist = 0.0;
        RaveField_getValue(self->dd, x, y, &dist_sum); // dist_sum is in km.
        RaveField_getValue(self->cd, x, y, &ndist);
        if (ndist != 0.0) {
          // store value in unit km. Multiplied with the gain value of 1000.0 it will give unit meters.
          RaveField_setValue(qfield, x, y, dist_sum/ndist);
        } else {
          RAVE_INFO0("ndist == 0.0 => Division by zero");
          RaveField_setValue(qfield, x, y, 0.0);
        }

        RaveField_getValue(self->sd, x, y, &acrr);
        if (acrr <= 0.0) {
          CartesianParam_setValue(param, x, y, self->undetect);
        } else if (N != nval) {
          acrr /= (double)((double)N-nval);
          acrr *= hours;
          CartesianParam_setValue(param, x, y, acrr);
        } else {
          RAVE_INFO0("N == nval => Division by zero");
          CartesianParam_setValue(param, x, y, self->nodata);
          RaveField_setValue(qfield, x, y, self->nodata);
        }

      }
    }
  }

  if (!CartesianParam_addQualityField(param, qfield)) {
    goto done;
  }

  result = RAVE_OBJECT_COPY(param);
done:
  RAVE_OBJECT_RELEASE(param);
  RAVE_OBJECT_RELEASE(qfield);
  return result;
}

int RaveAcrr_isInitialized(RaveAcrr_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->initialized;
}

void RaveAcrr_setNodata(RaveAcrr_t* self, double nodata)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->nodata = nodata;
}

double RaveAcrr_getNodata(RaveAcrr_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->nodata;
}

void RaveAcrr_setUndetect(RaveAcrr_t* self, double undetect)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->undetect = undetect;
}

double RaveAcrr_getUndetect(RaveAcrr_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->undetect;
}

const char* RaveAcrr_getQuantity(RaveAcrr_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return (const char*)self->quantity;
}

int RaveAcrr_setQualityFieldName(RaveAcrr_t* self, const char* fieldname)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (fieldname != NULL) {
    char* tmp = RAVE_STRDUP(fieldname);
    if (tmp != NULL) {
      RAVE_FREE(self->howtaskfieldname);
      self->howtaskfieldname = tmp;
      result = 1;
    }
  } else {
    RAVE_ERROR0("You must specify a quality field name");
  }

  return result;
}

const char* RaveAcrr_getQualityFieldName(RaveAcrr_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return (const char*)self->howtaskfieldname;
}

/*@} End of Interface functions */

RaveCoreObjectType RaveAcrr_TYPE = {
    "RaveAcrr",
    sizeof(RaveAcrr_t),
    RaveAcrr_constructor,
    RaveAcrr_destructor,
    RaveAcrr_copyconstructor
};
