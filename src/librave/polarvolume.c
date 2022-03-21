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
 * Defines the functions available when working with polar volumes
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-10-15
 */
#include "polarvolume.h"
#include "polarnav.h"
#include "raveobject_list.h"
#include "rave_datetime.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "raveobject_hashtable.h"
#include "rave_utilities.h"
#include <string.h>
#include <float.h>
#include <math.h>
/**
 * This is the default parameter value that should be used when working
 * with scans.
 */
#define DEFAULT_PARAMETER_NAME "DBZH"

/**
 * Represents a volume
 */
struct _PolarVolume_t {
  RAVE_OBJECT_HEAD /** Always on top */
  Projection_t* projection; /**< projection for this volume */
  PolarNavigator_t* navigator; /**< a polar navigator */
  RaveObjectList_t* scans;  /**< the list of scans */
  RaveDateTime_t* datetime; /**< the date / time */
  RaveObjectHashTable_t* attrs; /**< the attributes */
  char* source;             /**< the source string */
  char* paramname;          /**< the default parameter */
  double beamwH;            /**< the horizontal beamwidth, default bw is 1.0 * M_PI/180.0 */
  double beamwV;            /**< the vertical beamwidth, default bw is 1.0 * M_PI/180.0 */
};

/*@{ Private functions */
/**
 * Constructor
 */
static int PolarVolume_constructor(RaveCoreObject* obj)
{
  PolarVolume_t* this = (PolarVolume_t*)obj;
  this->projection = NULL;
  this->navigator = NULL;
  this->scans = NULL;
  this->datetime = NULL;
  this->source = NULL;
  this->paramname = NULL;
  this->beamwH = 1.0 * M_PI/180.0;
  this->beamwV = 1.0 * M_PI/180.0;
  this->attrs = RAVE_OBJECT_NEW(&RaveObjectHashTable_TYPE);
  this->datetime = RAVE_OBJECT_NEW(&RaveDateTime_TYPE);

  // Always initialize to default projection for lon/lat calculations
  this->projection = RAVE_OBJECT_NEW(&Projection_TYPE);
  if (this->projection != NULL) {
    if(!Projection_init(this->projection, "lonlat", "lonlat", Projection_getDefaultLonLatProjDef())) {
      goto error;
    }
  }
  this->navigator = RAVE_OBJECT_NEW(&PolarNavigator_TYPE);
  this->scans = RAVE_OBJECT_NEW(&RaveObjectList_TYPE);

  if (this->datetime == NULL || this->projection == NULL ||
      this->scans == NULL || this->navigator == NULL || this->attrs == NULL) {
    goto error;
  }

  if (!PolarVolume_setDefaultParameter(this, DEFAULT_PARAMETER_NAME)) {
    goto error;
  }

  return 1;
error:
  RAVE_OBJECT_RELEASE(this->datetime);
  RAVE_OBJECT_RELEASE(this->projection);
  RAVE_OBJECT_RELEASE(this->navigator);
  RAVE_OBJECT_RELEASE(this->scans);
  RAVE_OBJECT_RELEASE(this->attrs);
  RAVE_FREE(this->source);
  RAVE_FREE(this->paramname);
  return 0;
}

/**
 * Copy constructor
 */
static int PolarVolume_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  PolarVolume_t* this = (PolarVolume_t*)obj;
  PolarVolume_t* src = (PolarVolume_t*)srcobj;

  this->projection = RAVE_OBJECT_CLONE(src->projection);
  this->navigator = RAVE_OBJECT_CLONE(src->navigator);
  this->scans = RAVE_OBJECT_CLONE(src->scans); // the list only contains scans and they are cloneable
  this->datetime = RAVE_OBJECT_CLONE(src->datetime);
  this->attrs = RAVE_OBJECT_CLONE(src->attrs);
  this->source = NULL;
  this->paramname = NULL;
  this->beamwH = src->beamwH;
  this->beamwV = src->beamwV;

  if (this->datetime == NULL || this->projection == NULL ||
      this->scans == NULL || this->navigator == NULL || this->attrs == NULL) {
    goto error;
  }

  if (!PolarVolume_setSource(this, src->source)) {
    goto error;
  }
  if (!PolarVolume_setDefaultParameter(this, src->paramname)) {
    goto error;
  }

  return 1;
error:
  RAVE_FREE(this->source);
  RAVE_OBJECT_RELEASE(this->datetime);
  RAVE_OBJECT_RELEASE(this->projection);
  RAVE_OBJECT_RELEASE(this->navigator);
  RAVE_OBJECT_RELEASE(this->scans);
  RAVE_OBJECT_RELEASE(this->attrs);
  RAVE_FREE(this->source);
  RAVE_FREE(this->paramname);
  return 0;
}

/**
 * Destructor
 */
static void PolarVolume_destructor(RaveCoreObject* obj)
{
  PolarVolume_t* volume = (PolarVolume_t*)obj;
  RAVE_OBJECT_RELEASE(volume->datetime);
  RAVE_OBJECT_RELEASE(volume->projection);
  RAVE_OBJECT_RELEASE(volume->navigator);
  RAVE_OBJECT_RELEASE(volume->scans);
  RAVE_OBJECT_RELEASE(volume->attrs);
  RAVE_FREE(volume->source);
  RAVE_FREE(volume->paramname);
}

/**
 * Returns the elevation angle for the specified scan index.
 * @param
 */
static double PolarVolumeInternal_getElangle(PolarVolume_t* pvol, int index)
{
  PolarScan_t* scan = NULL;
  double elangle = 0.0L;
  scan = (PolarScan_t*)RaveObjectList_get(pvol->scans, index);
  if (scan != NULL) {
    elangle = PolarScan_getElangle(scan);
  } else {
    RAVE_CRITICAL1("Could not fetch scan for index = %d\n", index);
  }

  RAVE_OBJECT_RELEASE(scan);
  return elangle;
}

/**
 * Returns the scans surrounding the target elevation, or just the one closest, if parameter
 * onlyClosest set to 1.
 * @param[in] pvol - self
 * @param[in] targetElevation - the target elevation
 * @param[in] insidee - if the estimated elevation must be within the min-max elevation or not to be valid
 * @param[in] onlyClosest - if set to 1, only the closest scan will be returned. Otherwise, both scan above
 *                          and below will be returned.
 * @param[out] scanAbove - found scan above. Will be NULL if onlyClosest is set and the closest scan is located below.
 * @param[out] scanBelow - found scan below. Will be NULL if onlyClosest is set and the closest scan is located above.
 * @return 1 if successful and 0 otherwise
 */
static int PolarVolumeInternal_getSurroundingScans(PolarVolume_t* pvol, double targetElevation, int insidee, int onlyClosest, PolarScan_t** scanAbove, PolarScan_t** scanBelow)
{
  double closestElevation = 0.0L, closestElevationDiff = 0.0L;
  int scanIndexBelow = 0;
  int scanIndexAbove = 1;
  int closestScanIndex = 0;
  int i = 0;
  int nrScans = 0;
  int scanFound = 0;
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");

  *scanAbove = NULL;
  *scanBelow = NULL;

  nrScans = RaveObjectList_size(pvol->scans);

  if (insidee) {
    double max = PolarVolumeInternal_getElangle(pvol, nrScans-1);
    double min = PolarVolumeInternal_getElangle(pvol, 0);
    if (targetElevation < min || targetElevation > max) {
      return 0;
    }
  }

  closestElevation = PolarVolumeInternal_getElangle(pvol, 0);
  scanIndexBelow = 0;
  closestElevationDiff = fabs(targetElevation - closestElevation);
  for (i = 0; i < nrScans; i++) {
    double elev = PolarVolumeInternal_getElangle(pvol, i);

    double elevDiff = fabs(targetElevation - elev);
    if (scanFound == 0 || elevDiff < closestElevationDiff) {
      closestElevation = elev;
      closestElevationDiff = elevDiff;
      closestScanIndex = i;
      scanFound = 1;
    } else {
      break;
    }
  }

  if (scanFound == 0) {
    return 0;
  }

  if ((closestElevation - targetElevation) < 0) {
    scanIndexBelow = closestScanIndex;
    scanIndexAbove = closestScanIndex + 1;
  } else {
    scanIndexBelow = closestScanIndex - 1;
    scanIndexAbove = closestScanIndex;
  }

  if (onlyClosest) {
    if (closestScanIndex == scanIndexBelow) {
      *scanBelow = (PolarScan_t*)RaveObjectList_get(pvol->scans, scanIndexBelow);
    } else {
      *scanAbove = (PolarScan_t*)RaveObjectList_get(pvol->scans, scanIndexAbove);
    }
  } else {
    if (scanIndexBelow >= 0 && scanIndexBelow < nrScans) {
      *scanBelow = (PolarScan_t*)RaveObjectList_get(pvol->scans, scanIndexBelow);
    }
    if (scanIndexAbove >= 0 && scanIndexAbove < nrScans) {
      *scanAbove = (PolarScan_t*)RaveObjectList_get(pvol->scans, scanIndexAbove);
    }
  }

  return 1;
}

/**
 * Used to sort the scans by elevation in ascending order
 * @param[in] a - scan a (will be casted to *(PolarScan_t**))
 * @param[in] b - scan b (will be casted to *(PolarScan_t**))
 * @return -1 if a.elangle < b.elangle, 1 if a.elangle > b.elangle and 0 otherwise
 */
static int PolarVolumeInternal_ascendingElevationSort(const void* a, const void* b)
{
  PolarScan_t* scanA = *(PolarScan_t**)a;
  PolarScan_t* scanB = *(PolarScan_t**)b;
  double angleA = PolarScan_getElangle(scanA);
  double angleB = PolarScan_getElangle(scanB);
  if (angleA < angleB) {
    return -1;
  } else if (angleA > angleB) {
    return 1;
  }
  return 0;
}

/**
 * Used to sort the scans by elevation in descending order
 * @param[in] a - scan a (will be casted to *(PolarScan_t**))
 * @param[in] b - scan b (will be casted to *(PolarScan_t**))
 * @return -1 if a.elangle > b.elangle, 1 if a.elangle < b.elangle and 0 otherwise
 */
static int PolarVolumeInternal_descendingElevationSort(const void* a, const void* b)
{
  PolarScan_t* scanA = *(PolarScan_t**)a;
  PolarScan_t* scanB = *(PolarScan_t**)b;
  double angleA = PolarScan_getElangle(scanA);
  double angleB = PolarScan_getElangle(scanB);
  if (angleA > angleB) {
    return -1;
  } else if (angleA < angleB) {
    return 1;
  }
  return 0;
}

/*@} End of Private functions */

/*@{ Interface functions */
int PolarVolume_setTime(PolarVolume_t* pvol, const char* value)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  return RaveDateTime_setTime(pvol->datetime, value);
}

const char* PolarVolume_getTime(PolarVolume_t* pvol)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  return RaveDateTime_getTime(pvol->datetime);
}

int PolarVolume_setDate(PolarVolume_t* pvol, const char* value)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  return RaveDateTime_setDate(pvol->datetime, value);
}

const char* PolarVolume_getDate(PolarVolume_t* pvol)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  return RaveDateTime_getDate(pvol->datetime);
}

int PolarVolume_setSource(PolarVolume_t* pvol, const char* value)
{
  char* tmp = NULL;
  int result = 0;
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  if (value != NULL) {
    tmp = RAVE_STRDUP(value);
    if (tmp != NULL) {
      RAVE_FREE(pvol->source);
      pvol->source = tmp;
      tmp = NULL;
      result = 1;
    }
  } else {
    RAVE_FREE(pvol->source);
    result = 1;
  }
  return result;
}

const char* PolarVolume_getSource(PolarVolume_t* pvol)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  return (const char*)pvol->source;
}

void PolarVolume_setLongitude(PolarVolume_t* pvol, double lon)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  PolarNavigator_setLon0(pvol->navigator, lon);
}

double PolarVolume_getLongitude(PolarVolume_t* pvol)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  return PolarNavigator_getLon0(pvol->navigator);
}

void PolarVolume_setLatitude(PolarVolume_t* pvol, double lat)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  PolarNavigator_setLat0(pvol->navigator, lat);
}

double PolarVolume_getLatitude(PolarVolume_t* pvol)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  return PolarNavigator_getLat0(pvol->navigator);
}

void PolarVolume_setHeight(PolarVolume_t* pvol, double height)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  PolarNavigator_setAlt0(pvol->navigator, height);
}

double PolarVolume_getHeight(PolarVolume_t* pvol)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  return PolarNavigator_getAlt0(pvol->navigator);
}

void PolarVolume_setBeamwidth(PolarVolume_t* pvol, double bw)
{
  int i = 0, nlen = 0;
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  pvol->beamwH = bw;

  nlen = RaveObjectList_size(pvol->scans);
  for (i = 0; i < nlen; i++) {
    PolarScan_t* scan = (PolarScan_t*)RaveObjectList_get(pvol->scans, i);
    PolarScanInternal_setPolarVolumeBeamwH(scan, bw);
    RAVE_OBJECT_RELEASE(scan);
  }
}

double PolarVolume_getBeamwidth(PolarVolume_t* pvol)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  return pvol->beamwH;
}

void PolarVolume_setBeamwH(PolarVolume_t* self, double beamwidth)
{
  int i = 0, nlen = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->beamwH = beamwidth;
  nlen = RaveObjectList_size(self->scans);
  for (i = 0; i < nlen; i++) {
    PolarScan_t* scan = (PolarScan_t*)RaveObjectList_get(self->scans, i);
    PolarScanInternal_setPolarVolumeBeamwH(scan, beamwidth);
    RAVE_OBJECT_RELEASE(scan);
  }
}

double PolarVolume_getBeamwH(PolarVolume_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->beamwH;
}

void PolarVolume_setBeamwV(PolarVolume_t* self, double beamwidth)
{
  int i = 0, nlen = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->beamwV = beamwidth;
  nlen = RaveObjectList_size(self->scans);
  for (i = 0; i < nlen; i++) {
    PolarScan_t* scan = (PolarScan_t*)RaveObjectList_get(self->scans, i);
    PolarScanInternal_setPolarVolumeBeamwV(scan, beamwidth);
    RAVE_OBJECT_RELEASE(scan);
  }
}

double PolarVolume_getBeamwV(PolarVolume_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->beamwV;
}

double PolarVolume_getDistance(PolarVolume_t* pvol, double lon, double lat)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  return PolarNavigator_getDistance(pvol->navigator, lat, lon);
}

double PolarVolume_getMaxDistance(PolarVolume_t* pvol)
{
  int nrscans = 0;
  int i = 0;
  double maxdistance = 0.0;
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  nrscans = PolarVolume_getNumberOfScans(pvol);
  for (i = 0; i < nrscans; i++) {
    PolarScan_t* scan = PolarVolume_getScan(pvol, i);
    double dist = PolarScan_getMaxDistance(scan);
    if (dist > maxdistance) {
      maxdistance = dist;
    }
    RAVE_OBJECT_RELEASE(scan);
  }
  return maxdistance;
}

PolarScan_t* PolarVolume_getScanWithMaxDistance(PolarVolume_t* pvol)
{
  int nrscans = 0;
  int i = 0;
  double maxdistance = 0.0;
  PolarScan_t* result = NULL;

  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  nrscans = PolarVolume_getNumberOfScans(pvol);
  for (i = 0; i < nrscans; i++) {
    PolarScan_t* scan = PolarVolume_getScan(pvol, i);
    double dist = PolarScan_getMaxDistance(scan);
    if (dist > maxdistance) {
      maxdistance = dist;
      RAVE_OBJECT_RELEASE(result);
      result = RAVE_OBJECT_COPY(scan);
    }
    RAVE_OBJECT_RELEASE(scan);
  }
  return result;

}

void PolarVolume_setProjection(PolarVolume_t* pvol, Projection_t* projection)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  RAVE_OBJECT_RELEASE(pvol->projection);
  pvol->projection = NULL;
  if (projection != NULL) {
    int index = 0;
    int nrScans = RaveObjectList_size(pvol->scans);
    pvol->projection = RAVE_OBJECT_COPY(projection);
    for (index = 0; index < nrScans; index++) {
      PolarScan_t* scan = (PolarScan_t*)RaveObjectList_get(pvol->scans, index);
      PolarScan_setProjection(scan, projection);
      RAVE_OBJECT_RELEASE(scan);
    }
  }
}

Projection_t* PolarVolume_getProjection(PolarVolume_t* pvol)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  if (pvol->projection != NULL) {
    return RAVE_OBJECT_COPY(pvol->projection);
  }
  return NULL;
}

int PolarVolume_addScan(PolarVolume_t* pvol, PolarScan_t* scan)
{
  int result = 0;
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  if (RaveObjectList_add(pvol->scans, (RaveCoreObject*)scan)) {
    PolarScan_setNavigator(scan, pvol->navigator);
    PolarScan_setProjection(scan, pvol->projection);
    PolarScan_setDefaultParameter(scan, pvol->paramname);
    if (PolarScan_getSource(scan) == NULL) {
      if (!PolarScan_setSource(scan, PolarVolume_getSource(pvol))) {
        goto done;
      }
    }
    if (PolarScanInternal_isPolarVolumeBeamwH(scan) == -1) { /* if default beamwidth */
      PolarScanInternal_setPolarVolumeBeamwH(scan, pvol->beamwH);
    }
    if (PolarScanInternal_isPolarVolumeBeamwV(scan) == -1) { /* if default beamwidth */
      PolarScanInternal_setPolarVolumeBeamwV(scan, pvol->beamwV);
    }

    if (PolarScan_getTime(scan) == NULL || PolarScan_getDate(scan) == NULL) {
      if (!PolarScan_setTime(scan, PolarVolume_getTime(pvol)) ||
          !PolarScan_setDate(scan, PolarVolume_getDate(pvol))) {
        goto done;
      }
    }

    result = 1;
  }
done:
  return result;
}

PolarScan_t* PolarVolume_getScan(PolarVolume_t* pvol, int index)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  return (PolarScan_t*)RaveObjectList_get(pvol->scans, index);
}

int PolarVolume_getNumberOfScans(PolarVolume_t* pvol)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  return RaveObjectList_size(pvol->scans);
}

int PolarVolume_removeScan(PolarVolume_t* pvol, int index)
{
  PolarScan_t* scan = NULL;
  int result = 0;
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  scan = (PolarScan_t*)RaveObjectList_remove(pvol->scans, index);
  if (scan != NULL) {
    result = 1;
  }
  RAVE_OBJECT_RELEASE(scan);
  return result;
}

PolarScan_t* PolarVolume_getScanClosestToElevation(PolarVolume_t* pvol, double e, int inside)
{
  double se = 0.0L, eld = 0.0L;
  int ei = 0;
  int i = 0;
  int nrScans = 0;
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");

  nrScans = RaveObjectList_size(pvol->scans);

  if (inside) {
    if ((e < PolarVolumeInternal_getElangle(pvol, 0)) ||
        (e > PolarVolumeInternal_getElangle(pvol, nrScans-1))) {
      return NULL;
    }
  }

  se = PolarVolumeInternal_getElangle(pvol, 0);
  ei = 0;
  eld = fabs(e - se);
  for (i = 1; i < nrScans; i++) {
    double elev = PolarVolumeInternal_getElangle(pvol, i);
    double elevd = fabs(e - elev);
    if (elevd < eld) {
      se = elev;
      eld = elevd;
      ei = i;
    } else {
      break;
    }
  }
  return (PolarScan_t*)RaveObjectList_get(pvol->scans, ei);
}

int PolarVolume_indexOf(PolarVolume_t* pvol, PolarScan_t* scan)
{
  int result = -1;
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  if (scan != NULL) {
    result = RaveObjectList_indexOf(pvol->scans, (RaveCoreObject*)scan);
  }
  return result;
}

void PolarVolume_getLonLatNavigationInfo(PolarVolume_t* pvol, double lon, double lat, double height, PolarNavigationInfo* info)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  RAVE_ASSERT((info != NULL), "info == NULL");
  info->lon = lon;
  info->lat = lat;
  info->distance = 0.0L;
  info->azimuth = 0.0L;
  info->range = 0.0L;
  info->height = height;
  info->actual_height = height;
  info->elevation = 0.0L;

  info->otype = Rave_ObjectType_PVOL;
  info->ei = -1;
  info->ri = -1;
  info->ai = -1;

  PolarNavigator_llToDa(pvol->navigator, lat, lon, &info->distance, &info->azimuth);
  PolarNavigator_dhToRe(pvol->navigator, info->distance, info->height, &info->range, &info->elevation);
}

RaveValueType PolarVolume_getNearest(PolarVolume_t* pvol, double lon, double lat, double height, int insidee, double* v)
{
  double d = 0.0L, a = 0.0L, r = 0.0L, e = 0.0L;
  RaveValueType result = RaveValueType_NODATA;
  PolarScan_t* scan = NULL;
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  RAVE_ASSERT((v != NULL), "v == NULL");
  *v = 0.0;

  PolarNavigator_llToDa(pvol->navigator, lat, lon, &d, &a);
  PolarNavigator_dhToRe(pvol->navigator, d, height, &r, &e);

  scan = PolarVolume_getScanClosestToElevation(pvol, e, insidee);
  if (scan != NULL) {
    //@todo: Eventually use the actual elevation and calculate proper range instead.
    // Now we have the elevation angle, fetch value by providing azimuth and range.
    result = PolarScan_getValueAtAzimuthAndRange(scan, a, r, 0, v);
  }

  RAVE_OBJECT_RELEASE(scan);

  return result;
}

RaveValueType PolarVolume_getNearestParameterValue(PolarVolume_t* pvol, const char* quantity, double lon, double lat, double height, int insidee, double* v)
{
  double d = 0.0L, a = 0.0L, r = 0.0L, e = 0.0L;
  RaveValueType result = RaveValueType_NODATA;
  PolarScan_t* scan = NULL;

  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  RAVE_ASSERT((quantity != NULL), "quantity == NULL");
  RAVE_ASSERT((v != NULL), "v == NULL");
  *v = 0.0;

  PolarNavigator_llToDa(pvol->navigator, lat, lon, &d, &a);
  PolarNavigator_dhToRe(pvol->navigator, d, height, &r, &e);

  // Find relevant elevation
  scan = PolarVolume_getScanClosestToElevation(pvol, e, insidee);
  if (scan != NULL) {
    result = PolarScan_getParameterValueAtAzimuthAndRange(scan, quantity, a, r, v);
  }

  RAVE_OBJECT_RELEASE(scan);

  return result;
}

RaveValueType PolarVolume_getConvertedVerticalMaxValue(PolarVolume_t* self, const char* quantity, double lon, double lat, double* v, PolarNavigationInfo* navinfo)
{
  RaveValueType result = RaveValueType_NODATA;
  int nrscans = 0, i = 0;
  PolarNavigationInfo info;

  RAVE_ASSERT((self != NULL), "pvol == NULL");
  RAVE_ASSERT((quantity != NULL), "quantity == NULL");
  RAVE_ASSERT((v != NULL), "v == NULL");

  memset(&info, 0, sizeof(PolarNavigationInfo));

  nrscans = RaveObjectList_size(self->scans);

  for (i = 0; i < nrscans; i++) {
    PolarScan_t* scan = (PolarScan_t*)RaveObjectList_get(self->scans, i);
    double value = 0.0;
    RaveValueType type = PolarScan_getNearestConvertedParameterValue(scan, quantity, lon, lat, &value, &info);
    if (type == RaveValueType_UNDETECT || type == RaveValueType_DATA) {
      if (result == RaveValueType_DATA && type == RaveValueType_DATA) {
        if (value > *v) {
          double dummydistance = 0.0;
          *v = value;
          info.ei = i;
          info.elevation = PolarScan_getElangle(scan); // So that we get exact scan elevation angle instead
          PolarNavigator_reToDh(self->navigator, info.range, info.elevation, &dummydistance, &info.actual_height);
          if (navinfo != NULL) {
            *navinfo = info;
          }
        }
      } else if (result == RaveValueType_UNDETECT && type == RaveValueType_UNDETECT) {
        RAVE_OBJECT_RELEASE(scan);
        continue; /* We always want to use the first UNDETECT if it only exists UNDETECT */
      } else if (result != RaveValueType_DATA) {
        double dummydistance = 0.0;
        *v = value;
        result = type;
        info.ei = i;
        info.elevation = PolarScan_getElangle(scan); // So that we get exact scan elevation angle instead
        PolarNavigator_reToDh(self->navigator, info.range, info.elevation, &dummydistance, &info.actual_height);
        if (navinfo != NULL) {
          *navinfo = info;
        }
      }
    }
    RAVE_OBJECT_RELEASE(scan);
  }

  return result;
}

RaveValueType PolarVolume_getConvertedParameterValueAt(PolarVolume_t* pvol, const char* quantity, int ei, int ri, int ai, double* v)
{
  RaveValueType result = RaveValueType_NODATA;
  PolarScan_t* scan = NULL;

  RAVE_ASSERT((pvol != NULL), "scan == NULL");

  scan = PolarVolume_getScan(pvol, ei);
  if (scan != NULL) {
    result = PolarScan_getConvertedParameterValue(scan, quantity, ri, ai, v);
  }

  RAVE_OBJECT_RELEASE(scan);
  return result;
}

int PolarVolume_getNearestNavigationInfo(PolarVolume_t* pvol, double lon, double lat, double height, int insidee, PolarNavigationInfo* navinfo)
{
  int result = 0;
  PolarScan_t* scan = NULL;

  RAVE_ASSERT((pvol != NULL), "scan == NULL");
  RAVE_ASSERT((navinfo != NULL), "navinfo == NULL");

  PolarVolume_getLonLatNavigationInfo(pvol, lon, lat, height, navinfo);

  scan = PolarVolume_getScanClosestToElevation(pvol, navinfo->elevation, insidee);
  if (scan != NULL) {
    double dummydistance = 0.0;
    navinfo->elevation = PolarScan_getElangle(scan); // So that we get exact scan elevation angle instead
    navinfo->ei = RaveObjectList_indexOf(pvol->scans, (RaveCoreObject*)scan);

    // To get the actual height
    PolarNavigator_reToDh(pvol->navigator, navinfo->range, navinfo->elevation, &dummydistance, &navinfo->actual_height);

    if (!PolarScan_fillNavigationIndexFromAzimuthAndRange(scan, PolarScanSelectionMethod_ROUND, PolarScanSelectionMethod_FLOOR, 0, navinfo)) {
      goto done;
    }
  }

  if (navinfo->ai >= 0 && navinfo->ri >= 0 && navinfo->ei >= 0) {
    result = 1;
  }

done:
  RAVE_OBJECT_RELEASE(scan);
  return result;
}

int PolarVolume_getSurroundingNavigationInfos(
    PolarVolume_t* pvol,
    double lon,
    double lat,
    double height,
    int insidee,
    int surroundingScans,
    int surroundingRangeBins,
    int surroundingRays,
    PolarNavigationInfo navinfos[])
{
  int noofNavinfos = 0;
  int noofNavinfosAbove = 0;
  int noofNavinfosBelow = 0;

  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  RAVE_ASSERT((navinfos != NULL), "navinfos == NULL");

  PolarNavigationInfo targetNavInfo;

  PolarVolume_getLonLatNavigationInfo(pvol, lon, lat, height, &targetNavInfo);

  PolarScan_t* scanAbove = NULL;
  PolarScan_t* scanBelow = NULL;
  if (PolarVolumeInternal_getSurroundingScans(pvol, targetNavInfo.elevation, insidee, !surroundingScans, &scanAbove, &scanBelow)) {
    if (scanAbove != NULL) {
      noofNavinfos = PolarScan_addSurroundingNavigationInfosForTarget(scanAbove, &targetNavInfo, surroundingRangeBins, surroundingRays, noofNavinfos, navinfos);
      noofNavinfosAbove = noofNavinfos;

      PolarVolume_addEiForNavInfos(pvol, scanAbove, navinfos, noofNavinfos, 0);

      if (noofNavinfosAbove == 0) {
        noofNavinfos = 0;
        goto done;
      }
    }

    if (scanBelow != NULL) {
      noofNavinfos = PolarScan_addSurroundingNavigationInfosForTarget(scanBelow, &targetNavInfo, surroundingRangeBins, surroundingRays, noofNavinfos, navinfos);
      noofNavinfosBelow = noofNavinfos - noofNavinfosAbove;

      PolarVolume_addEiForNavInfos(pvol, scanBelow, navinfos, noofNavinfos, noofNavinfosAbove);

      if (noofNavinfosBelow == 0) {
        noofNavinfos = 0;
        goto done;
      }
    }
  }


done:
  RAVE_OBJECT_RELEASE(scanAbove);
  RAVE_OBJECT_RELEASE(scanBelow);

  return noofNavinfos;
}

void PolarVolume_addEiForNavInfos(
    PolarVolume_t* pvol,
    PolarScan_t* scan,
    PolarNavigationInfo navinfos[],
    int noofNavinfos,
    int startNavInfoIndex)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RAVE_ASSERT((navinfos != NULL), "navinfos == NULL");

  int ei = RaveObjectList_indexOf(pvol->scans, (RaveCoreObject*)scan);

  int i = 0;
  for (i = startNavInfoIndex; i < noofNavinfos; i++) {
    navinfos[i].ei = ei;
  }
}

RaveValueType PolarVolume_getNearestConvertedParameterValue(PolarVolume_t* pvol, const char* quantity, double lon, double lat, double height, int insidee, double* v, PolarNavigationInfo* navinfo)
{
  PolarNavigationInfo info;
  RaveValueType result = RaveValueType_NODATA;

  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  RAVE_ASSERT((quantity != NULL), "quantity == NULL");
  RAVE_ASSERT((v != NULL), "v == NULL");

  info.ei = -1;
  info.ri = -1;
  info.ai = -1;
  *v = 0.0;

  if (PolarVolume_getNearestNavigationInfo(pvol, lon, lat, height, insidee, &info)) {
    result = PolarVolume_getConvertedParameterValueAt(pvol, quantity, info.ei, info.ri, info.ai, v);
    if (navinfo != NULL) {
      *navinfo = info;
    }
  }

  return result;
}

int PolarVolume_getQualityValueAt(PolarVolume_t* pvol, const char* quantity, int ei, int ri, int ai, const char* name, int convert, double* v)
{
  int result = 0;
  PolarScan_t* scan = NULL;
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  scan = PolarVolume_getScan(pvol, ei);
  if (scan != NULL) {
    result = PolarScan_getQualityValueAt(scan, quantity, ri, ai, name, convert, v);
  }
  RAVE_OBJECT_RELEASE(scan);
  return result;
}

int PolarVolume_setDefaultParameter(PolarVolume_t* pvol, const char* quantity)
{
  int result = 0;
  char* tmp = NULL;
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  RAVE_ASSERT((quantity != NULL), "quantity == NULL");
  tmp = RAVE_STRDUP(quantity);
  if (tmp != NULL) {
    int i = 0;
    int nlen = RaveObjectList_size(pvol->scans);
    result = 1; /* Asume everything is ok and let the scans default parameter decide the result */
    RAVE_FREE(pvol->paramname);
    pvol->paramname = tmp;
    for (i = 0; result == 1 && i < nlen; i++) {
      PolarScan_t* scan = (PolarScan_t*)RaveObjectList_get(pvol->scans, i);
      if (scan != NULL) {
        result = PolarScan_setDefaultParameter(scan, quantity);
      }
      RAVE_OBJECT_RELEASE(scan);
    }
  }
  return result;
}

const char* PolarVolume_getDefaultParameter(PolarVolume_t* pvol)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  return (const char*)pvol->paramname;
}

void PolarVolume_sortByElevations(PolarVolume_t* pvol, int ascending)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");

  if (ascending == 1) {
    RaveObjectList_sort(pvol->scans, PolarVolumeInternal_ascendingElevationSort);
  } else {
    RaveObjectList_sort(pvol->scans, PolarVolumeInternal_descendingElevationSort);
  }
}

int PolarVolume_isAscendingScans(PolarVolume_t* pvol)
{
  int result = 1;
  int index = 0;
  double lastelev = 0.0L;
  int nrScans = 0;
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  nrScans = RaveObjectList_size(pvol->scans);
  if (nrScans > 0) {
    lastelev = PolarVolumeInternal_getElangle(pvol, 0);
    for (index = 1; result == 1 && index < nrScans; index++) {
      double nextelev = PolarVolumeInternal_getElangle(pvol, index);
      if (nextelev < lastelev) {
        result = 0;
      }
      lastelev = nextelev;
    }
  }
  return result;
}

int PolarVolume_isTransformable(PolarVolume_t* pvol)
{
  int result = 0;
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  // Verify that the volume at least got one scan and that the scans
  // are sorted in ascending order.
  if (RaveObjectList_size(pvol->scans) > 0 && PolarVolume_isAscendingScans(pvol)) {
    result = 1;
  }
  return result;
}

int PolarVolume_addAttribute(PolarVolume_t* pvol,
  RaveAttribute_t* attribute)
{
  const char* name = NULL;
  char* aname = NULL;
  char* gname = NULL;
  int result = 0;
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  RAVE_ASSERT((attribute != NULL), "attribute == NULL");

  name = RaveAttribute_getName(attribute);
  if (name != NULL) {
    if (!RaveAttributeHelp_extractGroupAndName(name, &gname, &aname)) {
      RAVE_ERROR1("Failed to extract group and name from %s", name);
      goto done;
    }
    if ((strcasecmp("how", gname)==0 && RaveAttributeHelp_validateHowGroupAttributeName(gname, aname)) ||
        ((strcasecmp("what", gname)==0 || strcasecmp("where", gname)==0) && strchr(aname, '/') == NULL)) {
      result = RaveObjectHashTable_put(pvol->attrs, name, (RaveCoreObject*)attribute);
    }
  }

done:
  RAVE_FREE(aname);
  RAVE_FREE(gname);
  return result;
}

RaveAttribute_t* PolarVolume_getAttribute(PolarVolume_t* pvol,
  const char* name)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  if (name == NULL) {
    RAVE_ERROR0("Trying to get an attribute with NULL name");
    return NULL;
  }
  return (RaveAttribute_t*)RaveObjectHashTable_get(pvol->attrs, name);
}

void PolarVolume_removeAttribute(PolarVolume_t* pvol, const char* attrname)
{
  char* aname = NULL;
  char* gname = NULL;
  RAVE_ASSERT((pvol != NULL), "scan == NULL");
  if (attrname != NULL) {
    if (!RaveAttributeHelp_extractGroupAndName(attrname, &gname, &aname)) {
      RAVE_ERROR1("Failed to extract group and name from %s", attrname);
      goto done;
    }
    if (strcasecmp("how", gname)==0) {
      if (!RaveAttributeHelp_validateHowGroupAttributeName(gname, aname)) {
        RAVE_ERROR1("Not possible to validate how/group attribute name %s", attrname);
        goto done;
      }
      RaveCoreObject* attr = RaveObjectHashTable_remove(pvol->attrs, attrname);
      RAVE_OBJECT_RELEASE(attr);
    }
  }

done:
  RAVE_FREE(aname);
  RAVE_FREE(gname);
}

RaveList_t* PolarVolume_getAttributeNames(PolarVolume_t* pvol)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  return RaveObjectHashTable_keys(pvol->attrs);
}

int PolarVolume_hasAttribute(PolarVolume_t* pvol, const char* name)
{
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  return RaveObjectHashTable_exists(pvol->attrs, name);
}

RaveObjectList_t* PolarVolume_getAttributeValues(PolarVolume_t* pvol)
{
  RaveObjectList_t* result = NULL;
  RaveObjectList_t* tableattrs = NULL;

  RAVE_ASSERT((pvol != NULL), "pvol == NULL");
  tableattrs = RaveObjectHashTable_values(pvol->attrs);
  if (tableattrs == NULL) {
    goto error;
  }
  result = RAVE_OBJECT_CLONE(tableattrs);
  if (result == NULL) {
    goto error;
  }

  RAVE_OBJECT_RELEASE(tableattrs);
  return result;
error:
  RAVE_OBJECT_RELEASE(result);
  RAVE_OBJECT_RELEASE(tableattrs);
  return NULL;
}

int PolarVolume_isValid(PolarVolume_t* pvol)
{
  int result = 0;
  int nscans = 0;
  int i = 0;
  RAVE_ASSERT((pvol != NULL), "pvol == NULL");

  if (PolarVolume_getDate(pvol) == NULL ||
      PolarVolume_getTime(pvol) == NULL ||
      PolarVolume_getSource(pvol) == NULL) {
    RAVE_INFO0("date, time and source must be specified");
    goto done;
  }

  if ((nscans = RaveObjectList_size(pvol->scans)) <= 0) {
    RAVE_INFO0("Must have at least one scan");
    goto done;
  }

  result = 1;

  for (i = 0; result == 1 && i < nscans; i++) {
    PolarScan_t* scan = PolarVolume_getScan(pvol, i);
    result = PolarScan_isValid(scan, Rave_ObjectType_PVOL);
    RAVE_OBJECT_RELEASE(scan);
  }

done:
  return result;
}

PolarScan_t* PolarVolume_findScanWithQualityFieldByHowTask(PolarVolume_t* pvol, const char* howtaskvalue, const char* quantity)
{
  PolarScan_t* result = NULL;
  int nrscans = 0, i = 0;

  RAVE_ASSERT((pvol != NULL), "pvol == NULL");

  nrscans = RaveObjectList_size(pvol->scans);
  for (i = 0; result == NULL && i < nrscans; i++) {
    PolarScan_t* scan = (PolarScan_t*)RaveObjectList_get(pvol->scans, i);
    RaveField_t* field = PolarScan_findQualityFieldByHowTask(scan, howtaskvalue, quantity);
    if (field != NULL) {
      result = RAVE_OBJECT_COPY(scan);
    }
    RAVE_OBJECT_RELEASE(field);
    RAVE_OBJECT_RELEASE(scan);
  }
  return result;
}

PolarScan_t* PolarVolume_findAnyScanWithQualityFieldByHowTask(PolarVolume_t* pvol, const char* howtaskvalue)
{
  PolarScan_t* result = NULL;
  int nrscans = 0, i = 0;

  RAVE_ASSERT((pvol != NULL), "pvol == NULL");

  nrscans = RaveObjectList_size(pvol->scans);
  for (i = 0; result == NULL && i < nrscans; i++) {
    PolarScan_t* scan = (PolarScan_t*)RaveObjectList_get(pvol->scans, i);
    RaveField_t* field = PolarScan_findAnyQualityFieldByHowTask(scan, howtaskvalue);
    if (field != NULL) {
      result = RAVE_OBJECT_COPY(scan);
    }
    RAVE_OBJECT_RELEASE(field);
    RAVE_OBJECT_RELEASE(scan);
  }
  return result;
}

/**
 * Returns the height or distance field for this volume. The height is the altitude at the
 * location represented by each bin and the distance is the distance on ground level.
 * @param[in] self - self
 * @param[in] ftype - if 0 then distance field will be generated otherwise the height field will be generated.
 * @returns the rave field
 */
static RaveField_t* PolarVolumeInternal_getHeightOrDistanceField(PolarVolume_t* self, int ftype)
{
  RaveField_t *f = NULL, *result = NULL;
  int i = 0, j = 0, nscans = 0, maxnbins = 0;

  RAVE_ASSERT((self != NULL), "self == NULL");
  nscans = RaveObjectList_size(self->scans);

  for (i = 0; i < nscans; i++) {
    PolarScan_t* scan = PolarVolume_getScan(self, i);
    int nbins = PolarScan_getNbins(scan);
    if (nbins > maxnbins) {
      maxnbins = nbins;
    }
    RAVE_OBJECT_RELEASE(scan);
  }
  f = RAVE_OBJECT_NEW(&RaveField_TYPE);
  if (f == NULL || !RaveField_createData(f, maxnbins, nscans, RaveDataType_DOUBLE)) {
    RAVE_ERROR0("Failed to create field for distance field");
    goto done;
  }
  for (i = 0; i < nscans; i++) {
    PolarScan_t* scan = PolarVolume_getScan(self, i);
    RaveField_t* dfield = NULL;

    if (ftype == 0) {
      dfield = PolarScan_getDistanceField(scan);
    } else {
      dfield = PolarScan_getHeightField(scan);
    }
    int nbins = 0;
    if (dfield == NULL) {
      RAVE_OBJECT_RELEASE(scan);
      goto done;
    }
    nbins = RaveField_getXsize(dfield);

    for (j = 0; j < nbins; j++) {
      double v = -99999.0;
      RaveField_getValue(dfield,j,0,&v);
      RaveField_setValue(f, j, i, v);
    }
    // And if for some reason we have less than max number of bins, we just set a negative value
    for (j=nbins; j < maxnbins; j++) {
      RaveField_setValue(f, j, i, -99999.0);
    }
    RAVE_OBJECT_RELEASE(dfield);
    RAVE_OBJECT_RELEASE(scan);
  }

  result = RAVE_OBJECT_COPY(f);
done:
  RAVE_OBJECT_RELEASE(f);
  return result;

}

RaveField_t* PolarVolume_getDistanceField(PolarVolume_t* self)
{
  return PolarVolumeInternal_getHeightOrDistanceField(self, 0);
}

RaveField_t* PolarVolume_getHeightField(PolarVolume_t* self)
{
  return PolarVolumeInternal_getHeightOrDistanceField(self, 1);
}

PolarObservation* PolarVolume_getCorrectedValuesAtHeight(PolarVolume_t* self, double height, double gap, int* nobservations)
{
  int scanIndex = 0, nScans = 0;
  PolarObservation* result = NULL;
  PolarObservationLinkedList* polist = NULL;
  PolarObservationLinkedList* pbitptr = NULL;

  RAVE_ASSERT((self != NULL), "self == NULL");
  nScans = RaveObjectList_size(self->scans);
  for (scanIndex = 0; scanIndex < nScans; scanIndex++) {
    PolarScan_t* scan = (PolarScan_t*)RaveObjectList_get(self->scans, scanIndex);
    double rscale = PolarScan_getRscale(scan);
    double elangle = PolarScan_getElangle(scan);
    double rl = 0.0, ru = 0.0, dl = 0.0, du = 0.0;
    int bi = 0, bEnd = 0, ri = 0, rEnd = 0;
    PolarNavigator_ehToRd(self->navigator, elangle, height-gap/2.0, &rl, &dl);
    PolarNavigator_ehToRd(self->navigator, elangle, height+gap/2.0, &ru, &du);
    rEnd = PolarScan_getNrays(scan);
    bEnd = (int) ru / rscale;

    for (ri = 0; ri < rEnd; ri++) {
      for (bi = (int) rl / rscale; bi < bEnd; bi++) {
        PolarObservationLinkedList* pbit = RAVE_MALLOC(sizeof(PolarObservationLinkedList));
        if (pbit == NULL) {
          RAVE_CRITICAL0("Failed to allocate memory for polar observation information");
          RaveTypes_FreePolarObservationLinkedList(polist);
          polist = NULL;
          goto done;
        }
        pbit->next = NULL;
        pbit->obs.vt = PolarScan_getConvertedParameterValue(scan, self->paramname, bi, ri, &pbit->obs.v);
        pbit->obs.elangle = elangle;
        pbit->obs.range = bi * rscale;
        pbit->obs.range = bi * rscale;
        PolarNavigator_reToDh(self->navigator, pbit->obs.range, pbit->obs.elangle, &pbit->obs.distance, &pbit->obs.height);
        if (polist == NULL) {
          polist = pbit;
          pbitptr = polist;
        } else {
          pbitptr->next = pbit;
          pbitptr = pbit;
        }
      }
    }
    RAVE_OBJECT_RELEASE(scan);
  }

done:
  if (polist != NULL) {
    *nobservations = 0;
    result = RaveTypes_PolarObservationLinkedListToArray(polist, nobservations);
    RaveTypes_FreePolarObservationLinkedList(polist);
  }
  return result;
}

void PolarVolume_setUseAzimuthalNavInformation(PolarVolume_t* self, int v)
{
  int i = 0;
  int numberOfScans = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  numberOfScans = PolarVolume_getNumberOfScans(self);
  for (i = 0; i < numberOfScans; i++) {
    PolarScan_t* s = PolarVolume_getScan(self, i);
    if (s != NULL) {
      PolarScan_setUseAzimuthalNavInformation(s, v);
    }
    RAVE_OBJECT_RELEASE(s);
  }
}

int PolarVolume_useAzimuthalNavInformation(PolarVolume_t* self)
{
  int i = 0;
  int numberOfScans = 0;
  int result = 0;
  RAVE_ASSERT((self != NULL), "scan == NULL");
  numberOfScans = PolarVolume_getNumberOfScans(self);
  for (i = 0; i < numberOfScans && result == 0; i++) {
    PolarScan_t* s = PolarVolume_getScan(self, i);
    if (s != NULL) {
      result = PolarScan_useAzimuthalNavInformation(s);
    }
    RAVE_OBJECT_RELEASE(s);
  }
  if (numberOfScans == 0) {
    result = 0;
  }
  return result;
}

int PolarVolume_removeParametersExcept(PolarVolume_t* self, RaveList_t* parameters)
{
  int i = 0;
  int numberOfScans = 0;
  int result = 1;

  RAVE_ASSERT((self != NULL), "scan == NULL");

  numberOfScans = PolarVolume_getNumberOfScans(self);
  for (i = 0; i < numberOfScans && result == 1; i++) {
    PolarScan_t* s = PolarVolume_getScan(self, i);
    if (s != NULL) {
      result = PolarScan_removeParametersExcept(s, parameters);
    }
    RAVE_OBJECT_RELEASE(s);
  }

  return result;
}


/*@} End of Interface functions */
RaveCoreObjectType PolarVolume_TYPE = {
    "PolarVolume",
    sizeof(PolarVolume_t),
    PolarVolume_constructor,
    PolarVolume_destructor,
    PolarVolume_copyconstructor
};
