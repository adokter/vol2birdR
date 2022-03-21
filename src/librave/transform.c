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
 * Defines the functions available when transforming between different
 * types of products
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-10-20
 */
#include "transform.h"
#include "projection.h"
#include "projection_pipeline.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "rave_utilities.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

/**
 * Represents one transformator
 */
struct _Transform_t {
  RAVE_OBJECT_HEAD /** Always on top */
  RaveTransformationMethod method;
};

/*@{ Private functions */
/**
 * Constructor
 */
static int Transform_constructor(RaveCoreObject* obj)
{
  Transform_t* transform = (Transform_t*)obj;
  transform->method = NEAREST;
  return 1;
}

/**
 * Destructor
 */
static void Transform_destructor(RaveCoreObject* obj)
{
}

/**
 * Internal routine to handle both cappis and pseudo-cappis since they are similar in behaviour.
 * @param[in] transform - the transformer instance
 * @param[in] pvol - the polar volume
 * @param[in] cartesian - the cartesian (resulting) product
 * @param[in] height - the altitude to create the cappi at
 * @param[in] insidee - the only difference between cappi and pseudo-cappi is if the range/height evaluates to an elevation that is inside or outside the min-max scan elevations.
 * @returns 1 on success otherwise 0
 */
static int Transform_cappis_internal(Transform_t* transform, PolarVolume_t* pvol, Cartesian_t* cartesian, double height, int insidee)
{
  int result = 0;
  long xsize = 0, ysize = 0, x = 0, y = 0;
  double cnodata = 0.0L, cundetect = 0.0L;
  Projection_t* sourcepj = NULL;
  Projection_t* targetpj = NULL;
  ProjectionPipeline_t* pipeline = NULL;

  RAVE_ASSERT((transform != NULL), "transform was NULL");
  RAVE_ASSERT((pvol != NULL), "pvol was NULL");
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");

  if (!Cartesian_isTransformable(cartesian)) {
    RAVE_ERROR0("Cartesian product is not possible to transform");
    goto done;
  }
  if (!PolarVolume_isTransformable(pvol)) {
    RAVE_ERROR0("Polar volume is not possible to transform");
    goto done;
  }

  sourcepj = Cartesian_getProjection(cartesian);
  targetpj = PolarVolume_getProjection(pvol);
  pipeline = ProjectionPipeline_createPipeline(sourcepj, targetpj);
  cnodata = Cartesian_getNodata(cartesian);
  cundetect = Cartesian_getUndetect(cartesian);
  xsize = Cartesian_getXSize(cartesian);
  ysize = Cartesian_getYSize(cartesian);

  if (pipeline == NULL) {
    RAVE_ERROR0("Failed to create pipeline");
    goto done;
  }

  for (y = 0; y < ysize; y++) {
    double herey = Cartesian_getLocationY(cartesian, y);
    double tmpy = herey;
    for (x = 0; x < xsize; x++) {
      double herex = Cartesian_getLocationX(cartesian, x);
      herey = tmpy; // So that we can use herey over and over again
      RaveValueType valid = RaveValueType_NODATA;
      double v = 0.0L;
      if (!ProjectionPipeline_fwd(pipeline, herex, herey, &herex, &herey)) {
        RAVE_ERROR0("Transform failed");
        goto done;
      }
      valid = PolarVolume_getNearest(pvol, herex, herey, height, insidee, &v);

      if (valid == RaveValueType_NODATA) {
        v = cnodata;
      } else if (valid == RaveValueType_UNDETECT) {
        v = cundetect;
      }
      Cartesian_setValue(cartesian, x, y, v);
    }
  }

  result = 1;
done:
  RAVE_OBJECT_RELEASE(sourcepj);
  RAVE_OBJECT_RELEASE(targetpj);
  RAVE_OBJECT_RELEASE(pipeline);
  return result;
}

int TransformInternal_verifySameParameterNames(RaveList_t* expected, RaveList_t* actual)
{
  int nexpected = 0, nactual = 0, ie = 0, ia = 0;
  nexpected = RaveList_size(expected);
  nactual = RaveList_size(actual);
  if (nexpected != nactual) {
    return 0;
  }
  for (ie = 0; ie < nexpected; ie++) {
    char* se = RaveList_get(expected, ie);
    int contains = 0;
    for (ia = 0; !contains && ia < nactual; ia++) {
      char* sa = RaveList_get(actual, ia);
      if ((se == NULL && sa == NULL) ||
          (se != NULL && sa != NULL && strcmp(se,sa) == 0)) {
        contains = 1;
      }
    }
    if (!contains) {
      return 0;
    }
  }
  return 1;
}

int TransformInternal_verifyCombineTilesObjects(RaveObjectList_t* tiles)
{
  int ntiles, i;
  RaveCoreObject* o = NULL;
  int result = 0;
  RaveList_t* parameterNames = NULL;

  ntiles = RaveObjectList_size(tiles);
  for (i = 0; i < ntiles; i++) {
    o = RaveObjectList_get(tiles, i);
    if (!RAVE_OBJECT_CHECK_TYPE(o, &Cartesian_TYPE)) {
      goto done;
    }
    if (parameterNames == NULL) {
      parameterNames = Cartesian_getParameterNames((Cartesian_t*)o);
    } else {
      RaveList_t* oNames = Cartesian_getParameterNames((Cartesian_t*)o);
      if (!TransformInternal_verifySameParameterNames(parameterNames, oNames)) {
        RAVE_ERROR0("Cartesian objects does not contain same parameters");
        RaveList_freeAndDestroy(&oNames);
        goto done;
      }
      RaveList_freeAndDestroy(&oNames);
    }
    RAVE_OBJECT_RELEASE(o);
  }

  result = 1; /* If we get here, all checks has passed */
done:
  RaveList_freeAndDestroy(&parameterNames);
  RAVE_OBJECT_RELEASE(o);
  return result;
}

/*@} End of Private functions */

/*@{ Interface functions */
int Transform_setMethod(Transform_t* transform, RaveTransformationMethod method)
{
  int result = 0;
  RAVE_ASSERT((transform != NULL), "transform was NULL");
  if (method >= NEAREST && method <= INVERSE) {
    transform->method = method;
    result = 1;
  }
  return result;
}

RaveTransformationMethod Transform_getMethod(Transform_t* transform)
{
  RAVE_ASSERT((transform != NULL), "transform was NULL");
  return transform->method;
}

int Transform_ppi(Transform_t* transform, PolarScan_t* scan, Cartesian_t* cartesian)
{
  int result = 0;
  long xsize = 0, ysize = 0, x = 0, y = 0;
  double cnodata = 0.0L, cundetect = 0.0L;
  Projection_t* sourcepj = NULL;
  Projection_t* targetpj = NULL;
  ProjectionPipeline_t* pipeline = NULL;

  RAVE_ASSERT((transform != NULL), "transform was NULL");
  RAVE_ASSERT((scan != NULL), "scan was NULL");
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");

  if (!Cartesian_isTransformable(cartesian) ||
      !PolarScan_isTransformable(scan)) {
    RAVE_ERROR0("Cartesian product or scan is not possible to transform");
    goto done;
  }

  sourcepj = Cartesian_getProjection(cartesian);
  targetpj = PolarScan_getProjection(scan);
  cnodata = Cartesian_getNodata(cartesian);
  cundetect = Cartesian_getUndetect(cartesian);
  xsize = Cartesian_getXSize(cartesian);
  ysize = Cartesian_getYSize(cartesian);

  pipeline = ProjectionPipeline_createPipeline(sourcepj, targetpj);
  if (pipeline == NULL) {
    RAVE_ERROR0("Failed to create pipeline");
    goto done;
  }

  for (y = 0; y < ysize; y++) {
    double herey = Cartesian_getLocationY(cartesian, y);
    double tmpy = herey;
    for (x = 0; x < xsize; x++) {
      double herex = Cartesian_getLocationX(cartesian, x);
      herey = tmpy; // So that we can use herey over and over again
      RaveValueType valid = RaveValueType_NODATA;
      double v = 0.0L;
      if (!ProjectionPipeline_fwd(pipeline, herex, herey, &herex, &herey)) {
        RAVE_ERROR0("Transform failed");
        goto done;
      }
      valid = PolarScan_getNearest(scan, herex, herey, 0, &v);

      if (valid == RaveValueType_NODATA) {
        v = cnodata;
      } else if (valid == RaveValueType_UNDETECT) {
        v = cundetect;
      }
      Cartesian_setValue(cartesian, x, y, v);
    }
  }

  result = 1;
done:
  RAVE_OBJECT_RELEASE(sourcepj);
  RAVE_OBJECT_RELEASE(targetpj);
  RAVE_OBJECT_RELEASE(pipeline);
  return result;
}

int Transform_cappi(Transform_t* transform, PolarVolume_t* pvol, Cartesian_t* cartesian, double height)
{
  return Transform_cappis_internal(transform, pvol, cartesian, height, 1);
}

int Transform_pcappi(Transform_t* transform, PolarVolume_t* pvol, Cartesian_t* cartesian, double height)
{
  return Transform_cappis_internal(transform, pvol, cartesian, height, 0);
}

PolarScan_t* Transform_ctoscan(Transform_t* transform, Cartesian_t* cartesian, RadarDefinition_t* def, double angle, const char* quantity)
{
  Projection_t* sourcepj = NULL;
  Projection_t* targetpj = NULL;
  ProjectionPipeline_t* pipeline = NULL;
  PolarScan_t* result = NULL;
  PolarScan_t* scan = NULL;
  PolarScanParam_t* parameter = NULL;
  CartesianParam_t* cparam = NULL;
  RaveDataType datatype = RaveDataType_UCHAR;

  double nodata = 0.0;
  double undetect = 0.0;
  long ray = 0, bin = 0;
  long nrays = 0, nbins = 0;

  RAVE_ASSERT((transform != NULL), "transform == NULL");
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  RAVE_ASSERT((quantity != NULL), "quantity == NULL");
  RAVE_ASSERT((def != NULL), "def == NULL");

  if (!Cartesian_isTransformable(cartesian)) {
    RAVE_ERROR0("Cartesian product is not possible transform");
    goto error;
  }
  scan = RAVE_OBJECT_NEW(&PolarScan_TYPE);
  if (scan == NULL) {
    goto error;
  }
  parameter = RAVE_OBJECT_NEW(&PolarScanParam_TYPE);
  if (parameter == NULL) {
    goto error;
  }
  cparam = Cartesian_getParameter(cartesian, quantity);
  if (cparam != NULL) {
    datatype = CartesianParam_getDataType(cparam);
  }

  if (!PolarScanParam_setQuantity(parameter, quantity)) {
    goto error;
  }

  nodata = Cartesian_getNodata(cartesian);
  undetect = Cartesian_getUndetect(cartesian);

  PolarScan_setBeamwH(scan, RadarDefinition_getBeamwH(def));
  PolarScan_setBeamwV(scan, RadarDefinition_getBeamwV(def));
  PolarScan_setElangle(scan, angle);
  PolarScan_setHeight(scan, RadarDefinition_getHeight(def));
  PolarScan_setLatitude(scan, RadarDefinition_getLatitude(def));
  PolarScan_setLongitude(scan, RadarDefinition_getLongitude(def));
  PolarScan_setRscale(scan, RadarDefinition_getScale(def));
  PolarScan_setRstart(scan, 0.0);
  PolarScan_setSource(scan, RadarDefinition_getID(def));
  PolarScanParam_setNodata(parameter, nodata);
  PolarScanParam_setUndetect(parameter, undetect);

  sourcepj = PolarScan_getProjection(scan);
  targetpj = Cartesian_getProjection(cartesian);
  pipeline = ProjectionPipeline_createPipeline(sourcepj, targetpj);
  if (pipeline == NULL) {
    RAVE_ERROR0("Failed to create pipeline");
    goto error;
  }

  if (!PolarScanParam_createData(parameter,
                                 RadarDefinition_getNbins(def),
                                 RadarDefinition_getNrays(def),
                                 datatype)) {
    goto error;
  }

  if (!PolarScan_addParameter(scan, parameter) ||
      !PolarScan_setDefaultParameter(scan, quantity)) {
    goto error;
  }

  nbins = RadarDefinition_getNbins(def);
  nrays = RadarDefinition_getNrays(def);

  for (ray = 0; ray < nrays; ray++) {
    for (bin = 0; bin < nbins; bin++) {
      double lon = 0.0, lat = 0.0;
      double v = 0.0L;
      if (PolarScan_getLonLatFromIndex(scan, bin, ray, &lon, &lat)) {
        double x = 0.0, y = 0.0;
        long xi = 0, yi = 0;
        if (!ProjectionPipeline_fwd(pipeline, lon, lat, &x, &y)) {
          goto error;
        }
        xi = Cartesian_getIndexX(cartesian, x);
        yi = Cartesian_getIndexY(cartesian, y);
        Cartesian_getValue(cartesian, xi, yi, &v);
        PolarScan_setValue(scan, bin, ray, v);
      }
    }
  }

  result = RAVE_OBJECT_COPY(scan);
error:
  RAVE_OBJECT_RELEASE(sourcepj);
  RAVE_OBJECT_RELEASE(targetpj);
  RAVE_OBJECT_RELEASE(pipeline);
  RAVE_OBJECT_RELEASE(parameter);
  RAVE_OBJECT_RELEASE(cparam);
  RAVE_OBJECT_RELEASE(scan);
  return result;
}

PolarVolume_t* Transform_ctop(Transform_t* transform, Cartesian_t* cartesian, RadarDefinition_t* def, const char* quantity)
{
  unsigned int nangles = 0;
  unsigned int i = 0;
  double* angles = NULL;
  PolarVolume_t* pvol = NULL;
  PolarVolume_t* result = NULL;
  PolarScan_t* scan = NULL;

  RAVE_ASSERT((transform != NULL), "transform == NULL");
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  RAVE_ASSERT((def != NULL), "def == NULL");
  RAVE_ASSERT((quantity != NULL), "quantity == NULL");

  if (!Cartesian_isTransformable(cartesian)) {
    RAVE_ERROR0("Cartesian product is not possible to transform");
    goto error;
  }

  pvol = RAVE_OBJECT_NEW(&PolarVolume_TYPE);
  if (pvol == NULL) {
    goto error;
  }
  PolarVolume_setHeight(pvol, RadarDefinition_getHeight(def));
  PolarVolume_setLatitude(pvol, RadarDefinition_getLatitude(def));
  PolarVolume_setLongitude(pvol, RadarDefinition_getLongitude(def));
  if (!PolarVolume_setSource(pvol, RadarDefinition_getID(def)) ||
      !PolarVolume_setDate(pvol, Cartesian_getDate(cartesian)) ||
      !PolarVolume_setTime(pvol, Cartesian_getTime(cartesian))) {
    goto error;
  }

  if (!RadarDefinition_getElangles(def, &nangles, &angles)) {
    goto error;
  }

  for (i = 0; i < nangles; i++) {
    scan = Transform_ctoscan(transform, cartesian, def, angles[i], quantity);
    if (scan != NULL) {
      if (!PolarVolume_addScan(pvol, scan)) {
        goto error;
      }
    } else {
      goto error;
    }
    RAVE_OBJECT_RELEASE(scan);
  }

  result = RAVE_OBJECT_COPY(pvol);
error:
  RAVE_OBJECT_RELEASE(pvol);
  RAVE_OBJECT_RELEASE(scan);
  RAVE_FREE(angles);
  return result;
}

Cartesian_t* Transform_fillGap(Transform_t* transform, Cartesian_t* cartesian)
{
  Cartesian_t* result = NULL;
  Cartesian_t* filled = NULL;
  RaveList_t* names = NULL;
  int i = 0, nrnames = 0;
  CartesianParam_t* parameter = NULL;
  CartesianParam_t* paramclone = NULL;

  RAVE_ASSERT((transform != NULL), "transform == NULL");
  if (cartesian == NULL) {
    RAVE_ERROR0("Filling gap on NULL product!?");
    goto done;
  }

  filled = RAVE_OBJECT_CLONE(cartesian);
  if (filled == NULL) {
    RAVE_ERROR0("Failed to clone product");
    goto done;
  }

  names = Cartesian_getParameterNames(filled);
  if (names == NULL) {
    RAVE_ERROR0("Failed to get parameter names");
    goto done;
  }

  nrnames = RaveList_size(names);
  for (i = 0; i < nrnames; i++) {
    parameter = Cartesian_getParameter(filled, (const char*)RaveList_get(names, i));
    if (parameter != NULL) {
      paramclone = Transform_fillGapOnParameter(transform, parameter);
      if (paramclone == NULL ||
          !Cartesian_addParameter(filled, paramclone)) {
        RAVE_ERROR0("Failed to clone of add parameter clone to result");
        goto done;
      }
    } else {
      RAVE_ERROR0("Null parameter in cartesian product");
      goto done;
    }
    RAVE_OBJECT_RELEASE(parameter);
    RAVE_OBJECT_RELEASE(paramclone);
  }

  result = RAVE_OBJECT_COPY(filled);
done:
  RaveList_freeAndDestroy(&names);
  RAVE_OBJECT_RELEASE(parameter);
  RAVE_OBJECT_RELEASE(paramclone);
  RAVE_OBJECT_RELEASE(filled);
  return result;
}

CartesianParam_t* Transform_fillGapOnParameter(Transform_t* transform, CartesianParam_t* param)
{
  CartesianParam_t* result = NULL;
  CartesianParam_t* filled = NULL;
  long nxsize = 0, nysize = 0, xsize = 0, ysize = 0, x = 0, y = 0;

  RAVE_ASSERT((transform != NULL), "transform == NULL");

  if (param == NULL) {
    RAVE_ERROR0("Filling gap on NULL param!?");
    goto done;
  }

  filled = RAVE_OBJECT_CLONE(param);
  if (filled == NULL) {
    RAVE_ERROR0("Failed to clone parameter");
    goto done;
  }
  xsize = CartesianParam_getXSize(filled);
  ysize = CartesianParam_getYSize(filled);
  nxsize = xsize - 1;
  nysize = ysize - 1;
  for (y = 1; y < nysize; y++) {
    for (x = 1; x < nxsize; x++) {
      double v1 = 0.0, v2 = 0.0, v3 = 0.0, v4 = 0.0, v5 = 0.0;
      RaveValueType t1, t2, t3, t4, t5;
      t1 = t2 = t3 = t4 = t5 = RaveValueType_NODATA;

      t1 = CartesianParam_getValue(param, x, y, &v1);
      if (t1 == RaveValueType_UNDETECT) {
        t2 = CartesianParam_getValue(param, x-1, y, &v2);
        t3 = CartesianParam_getValue(param, x+1, y, &v3);
        t4 = CartesianParam_getValue(param, x, y-1, &v4);
        t5 = CartesianParam_getValue(param, x, y+1, &v5);
        if (t2 == RaveValueType_DATA && t3 == RaveValueType_DATA && t4 == RaveValueType_DATA && t5 == RaveValueType_DATA) {
          v1 = (v2 + v3 + v4 + v5) / 4.0;
          CartesianParam_setValue(filled, x, y, v1);
        } else {
          CartesianParam_setValue(filled, x, y, v1);
        }
      } else {
        CartesianParam_setValue(filled, x, y, v1);
      }
    }
  }
  result = RAVE_OBJECT_COPY(filled);
done:
  RAVE_OBJECT_RELEASE(filled);
  return result;
}

CartesianParam_t* Transform_accumulate(Transform_t* self, CartesianParam_t* param, double zr_a, double zr_b)
{
  CartesianParam_t* result = NULL;
  RaveField_t *nd = NULL, *dd = NULL, *cd = NULL, *sd = NULL;
  long xsize = 0, ysize = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (param == NULL) {
    RAVE_ERROR0("CartesianParam == NULL");
    return NULL;
  }

  xsize = CartesianParam_getXSize(param);
  ysize = CartesianParam_getXSize(param);

  nd = RAVE_OBJECT_NEW(&RaveField_TYPE);
  dd = RAVE_OBJECT_NEW(&RaveField_TYPE);
  cd = RAVE_OBJECT_NEW(&RaveField_TYPE);
  sd = RAVE_OBJECT_NEW(&RaveField_TYPE);
  if (nd == NULL || dd == NULL || cd == NULL || sd == NULL ||
      !RaveField_createData(nd, xsize, ysize, RaveDataType_SHORT) ||
      !RaveField_createData(dd, xsize, ysize, RaveDataType_DOUBLE) ||
      !RaveField_createData(cd, xsize, ysize, RaveDataType_SHORT) ||
      !RaveField_createData(sd, xsize, ysize, RaveDataType_DOUBLE)) {
    RAVE_ERROR0("Memory allocation problems");
    goto done;
  }

done:
  RAVE_OBJECT_RELEASE(nd);
  RAVE_OBJECT_RELEASE(dd);
  RAVE_OBJECT_RELEASE(cd);
  RAVE_OBJECT_RELEASE(sd);
  return result;
}

static RaveAttribute_t* TransformInternal_mergeRadarIndexTaskArgs(const char* tgtHowTaskArgValue, const char* srcHowTaskArgValue)
{
  RaveAttribute_t* result = NULL;
  RaveList_t* srcTokens = NULL;
  int buffLength = 0;
  char* buff = NULL;

  if (tgtHowTaskArgValue == NULL && srcHowTaskArgValue == NULL) {
    return NULL;
  }
  if (tgtHowTaskArgValue == NULL && srcHowTaskArgValue != NULL) {
    return RaveAttributeHelp_createString("how/task_args", srcHowTaskArgValue);
  }
  if (tgtHowTaskArgValue != NULL && srcHowTaskArgValue == NULL) {
    return RaveAttributeHelp_createString("how/task_args", tgtHowTaskArgValue);
  }
  buffLength = strlen(tgtHowTaskArgValue) + strlen(srcHowTaskArgValue) + 2; /* One for ending new line and one for ',' between. */
  buff = RAVE_MALLOC(sizeof(char)*buffLength);
  if (buff != NULL) {
    strcpy(buff, tgtHowTaskArgValue);

    srcTokens = RaveUtilities_getTrimmedTokens(srcHowTaskArgValue, (int)',');
    if (srcTokens != NULL) {
      int i = 0;
      int nTokens = RaveList_size(srcTokens);
      for (i = 0; i < nTokens; i++) {
        char* tok = RaveList_get(srcTokens, i);
        if (tok != NULL && !strstr(buff, tok)) {
          strcat(buff, ",");
          strcat(buff, tok);
        }
      }
    }

    result = RaveAttributeHelp_createString("how/task_args", buff);
  }

  RAVE_FREE(buff);
  if (srcTokens != NULL) {
    RaveList_freeAndDestroy(&srcTokens);
  }
  return result;
}

static int TransformInternal_addTileToParameter(Transform_t* self, Cartesian_t* target, Cartesian_t* source, const char* quantity)
{
  CartesianParam_t* targetParameter = NULL;
  CartesianParam_t* sourceParameter = NULL;
  double tllX, tllY, turX, turY, sllX, sllY, surX, surY, xscale, yscale;
  long txsize = 0, tysize = 0, sxsize = 0, sysize = 0;
  int result = 0;
  long x = 0, y = 0, xoffset = 0, yoffset = 0;
  long nqfields = 0, j = 0;
  targetParameter = Cartesian_getParameter(target, quantity);
  sourceParameter = Cartesian_getParameter(source, quantity);
  if (targetParameter == NULL || sourceParameter == NULL) {
    RAVE_ERROR1("Could not find target or source parameter for %s", quantity);
    goto done;
  }
  Cartesian_getAreaExtent(target, &tllX, &tllY, &turX, &turY);
  Cartesian_getAreaExtent(source, &sllX, &sllY, &surX, &surY);
  CartesianParam_setNodata(targetParameter, CartesianParam_getNodata(sourceParameter));
  CartesianParam_setUndetect(targetParameter, CartesianParam_getUndetect(sourceParameter));
  CartesianParam_setGain(targetParameter, CartesianParam_getGain(sourceParameter));
  CartesianParam_setOffset(targetParameter, CartesianParam_getOffset(sourceParameter));

  xscale = Cartesian_getXScale(target);
  yscale = Cartesian_getYScale(target);
  txsize = Cartesian_getXSize(target);
  tysize = Cartesian_getYSize(target);
  sxsize = Cartesian_getXSize(source);
  sysize = Cartesian_getYSize(source);

  /* A tile should have some sort of offset in relation to the target */
  xoffset = (long)rint((sllX - tllX) / xscale);
  yoffset = (long)rint((turY - surY) / yscale);

  for (x = 0; x < sxsize; x++) {
    for (y = 0; y < sysize; y++) {
      double v = 0.0;
      CartesianParam_getValue(sourceParameter, x, y, &v);
      if (x+xoffset < 0 || x+xoffset >= txsize || y+yoffset < 0 || y + yoffset >= tysize) {
        RAVE_WARNING0("Offset error when moving tile source into the target parameter");
      } else {
        CartesianParam_setValue(targetParameter, x+xoffset, y+yoffset, v);
      }
    }
  }

  // And copy quality fields
  nqfields = CartesianParam_getNumberOfQualityFields(targetParameter);
  for (j = 0; j < nqfields; j++) {
    char* howTaskValue = NULL;
    RaveField_t* targetField = CartesianParam_getQualityField(targetParameter, j);
    RaveAttribute_t* attr = RaveField_getAttribute(targetField, "how/task");
    if (attr != NULL && RaveAttribute_getString(attr, &howTaskValue)) {
      RaveField_t* sourceField = CartesianParam_getQualityFieldByHowTask(sourceParameter, howTaskValue);
      if (sourceField != NULL) {
        for (x = 0; x < sxsize; x++) {
          for (y = 0; y < sysize; y++) {
            double v = 0.0;
            RaveField_getValue(sourceField, x, y, &v);
            if (x+xoffset < 0 || x+xoffset >= txsize || y+yoffset < 0 || y + yoffset >= tysize) {
              RAVE_WARNING0("Offset error when moving tile source into the target quality field");
            } else {
              RaveField_setValue(targetField, x+xoffset, y+yoffset, v);
            }
          }
        }

        if (strcmp("se.smhi.composite.index.radar", howTaskValue) == 0) { /* We want to concatenate all parts into one long string and ensure that there only is one / each */
          RaveAttribute_t* tgtHowTaskArgAttr = RaveField_getAttribute(targetField, "how/task_args");
          RaveAttribute_t* srcHowTaskArgAttr = RaveField_getAttribute(sourceField, "how/task_args");
          RaveAttribute_t* mergedHowTaskArgAttr = NULL;
          char *tgtHowTaskArgValue = NULL, *srcHowTaskArgValue = NULL;
          if (tgtHowTaskArgAttr != NULL) {
            RaveAttribute_getString(tgtHowTaskArgAttr, &tgtHowTaskArgValue);
          }
          if (srcHowTaskArgAttr != NULL) {
            RaveAttribute_getString(srcHowTaskArgAttr, &srcHowTaskArgValue);
          }
          mergedHowTaskArgAttr = TransformInternal_mergeRadarIndexTaskArgs(tgtHowTaskArgValue, srcHowTaskArgValue);
          if (mergedHowTaskArgAttr) {
            RaveField_addAttribute(targetField, mergedHowTaskArgAttr);
          }
          RAVE_OBJECT_RELEASE(tgtHowTaskArgAttr);
          RAVE_OBJECT_RELEASE(srcHowTaskArgAttr);
          RAVE_OBJECT_RELEASE(mergedHowTaskArgAttr);
        }
      }
      RAVE_OBJECT_RELEASE(sourceField);
    }
    RAVE_OBJECT_RELEASE(targetField);
    RAVE_OBJECT_RELEASE(attr);
  }

  result = 1;
done:
  RAVE_OBJECT_RELEASE(targetParameter);
  RAVE_OBJECT_RELEASE(sourceParameter);
  return result;
}

static int TransformInternal_copyAttributes(RaveField_t* target, RaveField_t* source)
{
  RaveList_t* attrnames = NULL;
  int sz = 0, i = 0;
  int result = 0;
  RaveAttribute_t *attr = NULL, *cattr = NULL;

  attrnames = RaveField_getAttributeNames(source);
  if (attrnames == NULL) {
    RAVE_ERROR0("Failed to get attribute names");
    goto done;
  }

  sz = RaveList_size(attrnames);
  for (i = 0; i < sz; i++) {
    attr = RaveField_getAttribute(source, (const char*)RaveList_get(attrnames, i));
    if (attr != NULL) {
      cattr = RAVE_OBJECT_CLONE(attr);
      if (cattr == NULL ||
          !RaveField_addAttribute(target, cattr)) {
        RAVE_ERROR0("Failed to add cloned attribute to target field");
        goto done;
      }
      RAVE_OBJECT_RELEASE(cattr);
    }
    RAVE_OBJECT_RELEASE(attr);
  }
  result = 1;
done:
  RAVE_OBJECT_RELEASE(cattr);
  RAVE_OBJECT_RELEASE(attr);
  RaveList_freeAndDestroy(&attrnames);
  return result;
}

static int TransformInternal_createQualityFieldsFromTile(Transform_t* self, Cartesian_t* target, Cartesian_t* tile)
{
  int i = 0;
  int result = 0;
  RaveField_t *field = NULL, *targetfield = NULL;
  int nrFields = Cartesian_getNumberOfQualityFields(tile);
  for (i = 0; i < nrFields; i++) {
    field = Cartesian_getQualityField(tile, i);
    targetfield = RAVE_OBJECT_NEW(&RaveField_TYPE);
    if (targetfield == NULL ||
        !RaveField_createData(targetfield, Cartesian_getXSize(target), Cartesian_getYSize(target), RaveField_getDataType(field)) ||
        !TransformInternal_copyAttributes(targetfield, field)) {
      RAVE_ERROR0("Could not create quality field for cartesian object");
      goto done;
    }
    if (!Cartesian_addQualityField(target, targetfield)) {
      RAVE_ERROR0("Failed to add quality field to cartesian product");
      goto done;
    }
    RAVE_OBJECT_RELEASE(targetfield);
    RAVE_OBJECT_RELEASE(field);
  }
  result = 1;
done:
  RAVE_OBJECT_RELEASE(field);
  RAVE_OBJECT_RELEASE(targetfield);
  return result;
}

static int TransformInternal_createParameterQualtiyFieldFromTile(Transform_t* self, CartesianParam_t* target, CartesianParam_t* tile)
{
  int i = 0;
  int result = 0;
  RaveField_t *field = NULL, *targetfield = NULL;
  int nrFields = CartesianParam_getNumberOfQualityFields(tile);
  for (i = 0; i < nrFields; i++) {
    field = CartesianParam_getQualityField(tile, i);
    targetfield = RAVE_OBJECT_NEW(&RaveField_TYPE);
    if (targetfield == NULL ||
        !RaveField_createData(targetfield, CartesianParam_getXSize(target), CartesianParam_getYSize(target), RaveField_getDataType(field)) ||
        !TransformInternal_copyAttributes(targetfield, field)) {
      RAVE_ERROR0("Could not create quality field for cartesian param object");
      goto done;
    }
    if (!CartesianParam_addQualityField(target, targetfield)) {
      RAVE_ERROR0("Failed to add quality field to cartesian parameter");
      goto done;
    }
    RAVE_OBJECT_RELEASE(targetfield);
    RAVE_OBJECT_RELEASE(field);
  }
  result = 1;
done:
  RAVE_OBJECT_RELEASE(field);
  RAVE_OBJECT_RELEASE(targetfield);
  return result;
}

static int TransformInternal_addQualityFieldDataFromTileToCartesian(Transform_t* self, Cartesian_t* target, Cartesian_t* source)
{
  RaveField_t *targetField = NULL, *sourceField = NULL;
  RaveAttribute_t* attr = NULL;
  double tllX, tllY, turX, turY, sllX, sllY, surX, surY, xscale, yscale;
  long txsize = 0, tysize = 0, sxsize = 0, sysize = 0;
  int result = 0;
  long x = 0, y = 0, xoffset = 0, yoffset = 0;
  int nqfields = Cartesian_getNumberOfQualityFields(target);
  int i = 0;

  Cartesian_getAreaExtent(target, &tllX, &tllY, &turX, &turY);
  Cartesian_getAreaExtent(source, &sllX, &sllY, &surX, &surY);
  xscale = Cartesian_getXScale(target);
  yscale = Cartesian_getYScale(target);
  txsize = Cartesian_getXSize(target);
  tysize = Cartesian_getYSize(target);
  sxsize = Cartesian_getXSize(source);
  sysize = Cartesian_getYSize(source);

  /* A tile should have some sort of offset in relation to the target */
  xoffset = (long)rint((sllX - tllX) / xscale);
  yoffset = (long)rint((turY - surY) / yscale);

  for (i = 0; i < nqfields; i++) {
    char* howTaskValue = NULL;
    targetField = Cartesian_getQualityField(target, i);
    attr = RaveField_getAttribute(targetField, "how/task");
    if (attr != NULL && RaveAttribute_getString(attr, &howTaskValue)) {
      sourceField = Cartesian_getQualityFieldByHowTask(source, howTaskValue);
      if (sourceField != NULL) {
        for (x = 0; x < sxsize; x++) {
          for (y = 0; y < sysize; y++) {
            double v = 0.0;
            RaveField_getValue(sourceField, x, y, &v);
            if (x+xoffset < 0 || x+xoffset >= txsize || y+yoffset < 0 || y + yoffset >= tysize) {
              RAVE_WARNING0("Offset error when moving tile source into the target quality field");
            } else {
              RaveField_setValue(targetField, x+xoffset, y+yoffset, v);
            }
          }
        }
      }
      RAVE_OBJECT_RELEASE(sourceField);
    }
    RAVE_OBJECT_RELEASE(targetField);
    RAVE_OBJECT_RELEASE(attr);
  }

  result = 1;
/*done:*/
  RAVE_OBJECT_RELEASE(targetField);
  RAVE_OBJECT_RELEASE(sourceField);
  return result;
}

Cartesian_t* Transform_combine_tiles(Transform_t* self, Area_t* area, RaveObjectList_t* tiles)
{
  Cartesian_t* result = NULL;
  Cartesian_t* combined = NULL;
  int ntiles = 0;
  RaveList_t* pNames = NULL;

  RAVE_ASSERT((self != NULL), "self == NULL");
  if (area == NULL || tiles == NULL) {
    RAVE_ERROR0("No area definition or tiles");
    return NULL;
  }
  if (!TransformInternal_verifyCombineTilesObjects(tiles)) {
    RAVE_ERROR0("Not properly defined tiles. Do they contain same number of parameters?");
    return NULL;
  }
  combined = RAVE_OBJECT_NEW(&Cartesian_TYPE);
  if (combined == NULL) {
    goto done;
  }
  Cartesian_init(combined, area);

  ntiles = RaveObjectList_size(tiles);
  if (ntiles > 0) {
    Cartesian_t* ci = (Cartesian_t*)RaveObjectList_get(tiles, 0);
    double llX, llY, urX, urY;
    Cartesian_getAreaExtent(ci, &llX, &llY, &urX, &urY);
    int nnames = 0, j = 0;
    pNames = Cartesian_getParameterNames(ci);
    Cartesian_setDate(combined, Cartesian_getDate(ci));
    Cartesian_setTime(combined, Cartesian_getTime(ci));
    Cartesian_setStartDate(combined, Cartesian_getStartDate(ci));
    Cartesian_setStartTime(combined, Cartesian_getStartTime(ci));
    Cartesian_setEndDate(combined, Cartesian_getEndDate(ci));
    Cartesian_setEndTime(combined, Cartesian_getEndTime(ci));
    Cartesian_setProduct(combined, Cartesian_getProduct(ci));
    Cartesian_setObjectType(combined, Cartesian_getObjectType(ci));

    nnames = RaveList_size(pNames);
    for (j = 0; j < nnames; j++) {
      const char* pname = (const char*)RaveList_get(pNames,j);
      CartesianParam_t* p = Cartesian_getParameter(ci, pname);
      if (p != NULL) {
        CartesianParam_t* cp = Cartesian_createParameter(combined,  pname, CartesianParam_getDataType(p), CartesianParam_getNodata(p));
        if (cp == NULL ||
            !TransformInternal_createParameterQualtiyFieldFromTile(self, cp, p)) {
          RAVE_ERROR1("Failed to create parameter %s in the combined area", pname);
        } else {
          int k = 0;
          for (k = 0; k < ntiles; k++) {
            Cartesian_t* tile = (Cartesian_t*)RaveObjectList_get(tiles, k);
            if (!TransformInternal_addTileToParameter(self, combined, tile, pname)) {
              RAVE_ERROR1("Failed to add tile information for parameter %s", pname);
            }
            RAVE_OBJECT_RELEASE(tile);
          }
        }
        RAVE_OBJECT_RELEASE(cp);
      } else {
        RAVE_ERROR1("Failed to extract parameter %s from first tile.", pname);
      }
      RAVE_OBJECT_RELEASE(p);
    }

    if (!TransformInternal_createQualityFieldsFromTile(self, combined, ci)) {
      RAVE_OBJECT_RELEASE(ci);
      goto done;
    }

    for (j = 0; j < ntiles; j++) {
      Cartesian_t* tile = (Cartesian_t*)RaveObjectList_get(tiles, j);
      if (!TransformInternal_addQualityFieldDataFromTileToCartesian(self, combined, tile)) {
        RAVE_ERROR1("Failed to add quality field for %d tile", j);
      }
      RAVE_OBJECT_RELEASE(tile);
    }

    RAVE_OBJECT_RELEASE(ci);
  }

  result = RAVE_OBJECT_COPY(combined);
done:
  RaveList_freeAndDestroy(&pNames);
  RAVE_OBJECT_RELEASE(combined);
  return result;
}

/*@} End of Interface functions */

RaveCoreObjectType Transform_TYPE = {
    "Transform",
    sizeof(Transform_t),
    Transform_constructor,
    Transform_destructor
};
