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
 * Defines the functions available when working with polar scans
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-10-15
 */
#include "polarscan.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include <string.h>
#include <math.h>
#include "rave_object.h"
#include "rave_datetime.h"
#include "rave_transform.h"
#include "rave_data2d.h"
#include "raveobject_hashtable.h"
#include "rave_utilities.h"

/**
 * This is the default parameter value that should be used when working
 * with scans.
 */
#define DEFAULT_PARAMETER_NAME "DBZH"

/**
 * Represents one scan in a volume.
 */
struct _PolarScan_t {
  RAVE_OBJECT_HEAD /** Always on top */

  char* source;    /**< the source string */

  long nbins;      /**< number of bins */
  long nrays;      /**< number of rays */

  // Where
  double elangle;    /**< elevation of scan */
  double rscale;     /**< scale */
  double rstart;     /**< start of ray */
  long a1gate;       /**< something */

  // How
  double beamwH;  /**< horizontal beam width, default is 1.0 * M_PI/180.0 */
  double beamwV;  /**< vertical beam width, default is 1.0 * M_PI/180.0 */
  int bwpvolH; /**< indicates if the beamwH comes from a polar volume or not */
  int bwpvolV; /**< indicates if the beamwV comes from a polar volume or not */

  // Date/Time
  RaveDateTime_t* datetime;     /**< the date, time instance */
  RaveDateTime_t* startdatetime; /**< the start date, time instance */
  RaveDateTime_t* enddatetime;  /**< the stop date, time instance */

  // Navigator
  PolarNavigator_t* navigator; /**< a navigator for calculating polar navigation */

  // Projection wrapper
  Projection_t* projection; /**< projection for this scan */

  // Keeps all parameters
  RaveObjectHashTable_t* parameters;

  // Manages the default parameter
  char* paramname;
  PolarScanParam_t* param;

  RaveObjectHashTable_t* attrs; /**< attributes */

  RaveObjectList_t* qualityfields; /**< quality fields */

  // Keeps track of maximum distance. Should be reset each time
  // bins, scale or elangle changes.
  double maxdistance; /** maximum distance, cached value */

  int useAzimuthalNavInformation; /**< Indicate if astart, startazA and stopazA should be used when navigating */
  //
  // Keeps track of astart & startazA to handle when it can be assumed that rays are not aligned according to specification rules.
  // Should not be available outside this class and are set when addAttribute is called with how/astart and how/startazA respectively.
  //
  double astart;
  int hasAstart;
  double* azimuthArr;
  int azimuthArrLen;
  int hasAzimuthArr;
  double rayWidth;
};

/*@{ Private functions */
/**
 * Constructor.
 */
static int PolarScan_constructor(RaveCoreObject* obj)
{
  PolarScan_t* scan = (PolarScan_t*)obj;
  scan->nbins = 0;
  scan->nrays = 0;
  scan->datetime = NULL;
  scan->source = NULL;
  scan->elangle = 0.0;
  scan->rscale = 0.0;
  scan->rstart = 0.0;
  scan->a1gate = 0;
  scan->beamwH = 1.0 * M_PI/180.0;
  scan->beamwV = 1.0 * M_PI/180.0;
  scan->bwpvolH = -1;
  scan->bwpvolV = -1;
  scan->navigator = NULL;
  scan->projection = NULL;
  scan->parameters = NULL;
  scan->paramname = NULL;
  scan->param = NULL;
  scan->datetime = RAVE_OBJECT_NEW(&RaveDateTime_TYPE);
  scan->startdatetime = RAVE_OBJECT_NEW(&RaveDateTime_TYPE);
  scan->enddatetime = RAVE_OBJECT_NEW(&RaveDateTime_TYPE);
  scan->parameters = RAVE_OBJECT_NEW(&RaveObjectHashTable_TYPE);
  scan->projection = RAVE_OBJECT_NEW(&Projection_TYPE);
  scan->attrs = RAVE_OBJECT_NEW(&RaveObjectHashTable_TYPE);
  scan->qualityfields = RAVE_OBJECT_NEW(&RaveObjectList_TYPE);
  scan->maxdistance = -1.0;

  scan->useAzimuthalNavInformation = 1;
  scan->astart = 0.0;
  scan->hasAstart = 0;
  scan->azimuthArr = NULL;
  scan->azimuthArrLen = 0;
  scan->hasAzimuthArr = 0;
  scan->rayWidth = 0.0;

  if (scan->projection != NULL) {
    if(!Projection_init(scan->projection, "lonlat", "lonlat", Projection_getDefaultLonLatProjDef())) {
      goto error;
    }
  }
  scan->navigator = RAVE_OBJECT_NEW(&PolarNavigator_TYPE);
  if (scan->datetime == NULL || scan->projection == NULL ||
      scan->navigator == NULL || scan->parameters == NULL ||
      scan->attrs == NULL || scan->qualityfields == NULL ||
      scan->startdatetime == NULL || scan->enddatetime == NULL) {
    goto error;
  }
  if (!PolarScan_setDefaultParameter(scan, DEFAULT_PARAMETER_NAME)) {
    goto error;
  }
  return 1;
error:
  RAVE_OBJECT_RELEASE(scan->datetime);
  RAVE_OBJECT_RELEASE(scan->startdatetime);
  RAVE_OBJECT_RELEASE(scan->enddatetime);
  RAVE_OBJECT_RELEASE(scan->projection);
  RAVE_OBJECT_RELEASE(scan->navigator);
  RAVE_OBJECT_RELEASE(scan->parameters);
  RAVE_OBJECT_RELEASE(scan->attrs);
  RAVE_OBJECT_RELEASE(scan->qualityfields);
  RAVE_FREE(scan->paramname);
  RAVE_OBJECT_RELEASE(scan->param);
  return 0;
}

static int PolarScan_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  PolarScan_t* this = (PolarScan_t*)obj;
  PolarScan_t* src = (PolarScan_t*)srcobj;
  this->datetime = NULL;
  this->nbins = src->nbins;
  this->nrays = src->nrays;
  this->elangle = src->elangle;
  this->rscale = src->rscale;
  this->rstart = src->rstart;
  this->a1gate = src->a1gate;
  this->beamwH = src->beamwH;
  this->beamwV = src->beamwV;
  this->bwpvolH = src->bwpvolH;
  this->bwpvolV = src->bwpvolV;
  this->maxdistance = src->maxdistance;
  this->navigator = NULL;
  this->projection = NULL;
  this->paramname = NULL;
  this->param = NULL;

  this->useAzimuthalNavInformation = src->useAzimuthalNavInformation;
  this->astart = src->astart;
  this->hasAstart = src->hasAstart;
  this->azimuthArr = NULL;
  this->azimuthArrLen = 0;
  this->hasAzimuthArr = 0;
  this->rayWidth = src->rayWidth;

  this->source = NULL;
  this->datetime = RAVE_OBJECT_CLONE(src->datetime);
  this->startdatetime = RAVE_OBJECT_CLONE(src->startdatetime);
  this->enddatetime = RAVE_OBJECT_CLONE(src->enddatetime);
  this->projection = RAVE_OBJECT_CLONE(src->projection);
  this->navigator = RAVE_OBJECT_CLONE(src->navigator);
  this->parameters = RAVE_OBJECT_CLONE(src->parameters);
  this->attrs = RAVE_OBJECT_CLONE(src->attrs);
  this->qualityfields = RAVE_OBJECT_CLONE(src->qualityfields);
  if (this->datetime == NULL || this->projection == NULL ||
      this->navigator == NULL || this->parameters == NULL ||
      this->attrs == NULL || this->qualityfields == NULL ||
      this->startdatetime == NULL || this->enddatetime == NULL) {
    RAVE_ERROR0("Failed to clone base objects");
    goto error;
  }
  if (!PolarScan_setSource(this, PolarScan_getSource(src))) {
    goto error;
  }
  if (!PolarScan_setDefaultParameter(this, PolarScan_getDefaultParameter(src))) {
    goto error;
  }
  if (src->azimuthArr != NULL && src->azimuthArrLen > 0) {
    this->azimuthArr = RAVE_MALLOC(sizeof(double) * src->azimuthArrLen);
    if (this->azimuthArr == NULL) {
      RAVE_ERROR0("Failed to duplicate azimuth array");
      goto error;
    }
    memcpy(this->azimuthArr, src->azimuthArr, sizeof(double) * src->azimuthArrLen);
  }

  return 1;
error:
  RAVE_ERROR0("Failed to clone polar scan");
  RAVE_FREE(this->source);
  RAVE_OBJECT_RELEASE(this->datetime);
  RAVE_OBJECT_RELEASE(this->startdatetime);
  RAVE_OBJECT_RELEASE(this->enddatetime);
  RAVE_OBJECT_RELEASE(this->projection);
  RAVE_OBJECT_RELEASE(this->navigator);
  RAVE_OBJECT_RELEASE(this->parameters);
  RAVE_FREE(this->paramname);
  RAVE_FREE(this->azimuthArr);
  RAVE_OBJECT_RELEASE(this->param);
  RAVE_OBJECT_RELEASE(this->attrs);
  RAVE_OBJECT_RELEASE(this->qualityfields);
  return 0;
}

/**
 * Destructor.
 */
static void PolarScan_destructor(RaveCoreObject* obj)
{
  PolarScan_t* scan = (PolarScan_t*)obj;
  RAVE_FREE(scan->source);
  RAVE_OBJECT_RELEASE(scan->datetime);
  RAVE_OBJECT_RELEASE(scan->startdatetime);
  RAVE_OBJECT_RELEASE(scan->enddatetime);
  RAVE_OBJECT_RELEASE(scan->navigator);
  RAVE_OBJECT_RELEASE(scan->projection);
  RAVE_OBJECT_RELEASE(scan->parameters);
  RAVE_OBJECT_RELEASE(scan->param);
  RAVE_FREE(scan->paramname);
  RAVE_FREE(scan->azimuthArr)
  RAVE_OBJECT_RELEASE(scan->attrs);
  RAVE_OBJECT_RELEASE(scan->qualityfields);
}

void PolarScanInternal_createAzimuthNavigationInfo(PolarScan_t* self, const char* aname)
{
  RaveAttribute_t *startAz = NULL, *stopAz = NULL, *astart = NULL;
  double *tmpStartazArr = NULL, *tmpStopazArr = NULL, *mvAzArr = NULL;
  int tmpStartazLen = 0, tmpStopazLen = 0, i = 0;
  if (strcasecmp("startazA", aname) == 0 || strcasecmp("stopazA", aname) == 0) {
    startAz = (RaveAttribute_t*)RaveObjectHashTable_get(self->attrs, "how/startazA");
    stopAz = (RaveAttribute_t*)RaveObjectHashTable_get(self->attrs, "how/stopazA");
    if (startAz != NULL && !RaveAttribute_getDoubleArray(startAz, &tmpStartazArr, &tmpStartazLen)) {
      RAVE_ERROR0("Failed to extract startazA array");
      goto done;
    }

    if (stopAz != NULL && !RaveAttribute_getDoubleArray(stopAz, &tmpStopazArr, &tmpStopazLen)) {
      RAVE_ERROR0("Failed to extract stopazA array");
      goto done;
    }
    if (startAz != NULL && tmpStartazLen > 0 && tmpStartazLen == self->nrays) {
      mvAzArr = RAVE_MALLOC(sizeof(double) * tmpStartazLen);
      if (mvAzArr == NULL) {
        goto done;
      }
      memset(mvAzArr, 0, sizeof(double)*tmpStartazLen);
      if (tmpStopazArr != NULL && tmpStopazLen == tmpStartazLen) {
        for (i = 0; i < tmpStartazLen; i++) {
          if (tmpStopazArr[i] - tmpStartazArr[i] < 0.0) {
            mvAzArr[i] = tmpStartazArr[i] + (tmpStopazArr[i] + 360.0 - tmpStartazArr[i])/2.0;
          } else {
            mvAzArr[i] = tmpStartazArr[i] + (tmpStopazArr[i] - tmpStartazArr[i])/2.0;
          }
          mvAzArr[i] *= M_PI / 180.0;
        }
      } else {
        for (i = 0; i < tmpStartazLen; i++) {
          int starti = i, stopi = i+1;
          if (stopi == tmpStartazLen)
            stopi = 0;
          if (tmpStartazArr[stopi] - tmpStartazArr[starti] < 0.0) {
            mvAzArr[i] = tmpStartazArr[starti] + (tmpStartazArr[stopi] + 360.0 - tmpStartazArr[starti])/2.0;
          } else {
            mvAzArr[i] = tmpStartazArr[starti] + (tmpStartazArr[stopi] - tmpStartazArr[starti])/2.0;
          }
          mvAzArr[i] *= M_PI / 180.0;
        }
      }
      RAVE_FREE(self->azimuthArr);
      self->azimuthArr = mvAzArr;
      self->azimuthArrLen = tmpStartazLen;
      self->hasAzimuthArr = 1;
      mvAzArr = NULL;
    }
  } else if (strcasecmp("astart", aname) == 0) {
    astart = (RaveAttribute_t*)RaveObjectHashTable_get(self->attrs, "how/astart");
    if (astart != NULL) {
      double tmpv = 0.0;
      self->astart=0.0;
      if (!RaveAttribute_getDouble(astart, &tmpv)) {
        RAVE_ERROR0("Failed to extract astart");
        goto done;
      }
      self->astart=tmpv * M_PI / 180.0;
      self->hasAstart = 1;
    }
  }

done:
  RAVE_OBJECT_RELEASE(startAz);
  RAVE_OBJECT_RELEASE(stopAz);
  RAVE_OBJECT_RELEASE(astart);
  RAVE_FREE(mvAzArr);
}

static int PolarScanInternal_roundBySelectionMethod(
    double value, 
    PolarScanSelectionMethod_t selectionMethod, 
    int* result)
{
  RAVE_ASSERT((result != NULL), "result == NULL");

  if (selectionMethod == PolarScanSelectionMethod_ROUND) {
    *result = (int)rint(value);
  } else if (selectionMethod == PolarScanSelectionMethod_FLOOR) {
    *result = (int)floor(value);
  } else if (selectionMethod == PolarScanSelectionMethod_CEIL) {
    *result = (int)ceil(value);
  } else {
    RAVE_WARNING1("Invalid selection method: %i", selectionMethod);
    return 0;
  }

  return 1;
}

static PolarNavigationInfo* PolarScan_getNavigationInfo(
    PolarScan_t* scan,
    PolarNavigationInfo* sourceNavinfo,
    PolarScanSelectionMethod_t rangeSelectionMethod,
    PolarScanSelectionMethod_t azimuthSelectionMethod,
    int rangeMidpoint)
{
  RAVE_ASSERT((sourceNavinfo != NULL), "sourceNavinfo == NULL");
  
  if (scan == NULL) {
    return NULL;
  }

  PolarNavigationInfo* navinfo = RAVE_MALLOC(sizeof(PolarNavigationInfo));

  memcpy(navinfo, sourceNavinfo, sizeof(PolarNavigationInfo));

  navinfo->elevation = PolarScan_getElangle(scan); // So that we get exact scan elevation angle instead

  double dummydistance = 0.0;
  // To get the actual height
  PolarNavigator_reToDh(scan->navigator, navinfo->range, navinfo->elevation, &dummydistance, &navinfo->actual_height);
  if (!PolarScan_fillNavigationIndexFromAzimuthAndRange(scan, azimuthSelectionMethod, rangeSelectionMethod, rangeMidpoint, navinfo)) {
    RAVE_FREE(navinfo);
    return NULL;
  }

  if (navinfo->ai < 0 && navinfo->ri < 0 && navinfo->ei < 0) {
    RAVE_FREE(navinfo);
    return NULL;
  }

  return navinfo;
}

static int PolarScanInternal_shiftArrayIfExists(PolarScan_t* scan, const char* attrname, int nx)
{
  RaveAttribute_t* attr = NULL;
  int result = 1;

  attr = PolarScan_getAttribute(scan, attrname);
  if (attr != NULL) {
    if (RaveAttribute_getFormat(attr) == RaveAttribute_Format_LongArray ||
        RaveAttribute_getFormat(attr) == RaveAttribute_Format_DoubleArray) {
      result = RaveAttribute_shiftArray(attr, nx);
    }
  }
  RAVE_OBJECT_RELEASE(attr);

  return result;
}

/*@} End of Private functions */

/*@{ Interface functions */
void PolarScan_setNavigator(PolarScan_t* scan, PolarNavigator_t* navigator)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RAVE_ASSERT((navigator != NULL), "navigator was NULL");
  RAVE_OBJECT_RELEASE(scan->navigator);
  scan->navigator = RAVE_OBJECT_COPY(navigator);
  scan->maxdistance = -1.0;
}

PolarNavigator_t* PolarScan_getNavigator(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return RAVE_OBJECT_COPY(scan->navigator);
}

void PolarScan_setProjection(PolarScan_t* scan, Projection_t* projection)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RAVE_OBJECT_RELEASE(scan->projection);
  scan->projection = RAVE_OBJECT_COPY(projection);
}

Projection_t* PolarScan_getProjection(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return RAVE_OBJECT_COPY(scan->projection);
}

int PolarScan_setTime(PolarScan_t* scan, const char* value)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return RaveDateTime_setTime(scan->datetime, value);
}

const char* PolarScan_getTime(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return RaveDateTime_getTime(scan->datetime);
}

int PolarScan_setStartTime(PolarScan_t* scan, const char* value)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return RaveDateTime_setTime(scan->startdatetime, value);
}

const char* PolarScan_getStartTime(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  if (RaveDateTime_getTime(scan->startdatetime) == NULL) {
    return PolarScan_getTime(scan);
  } else {
    return RaveDateTime_getTime(scan->startdatetime);
  }
}

int PolarScan_setEndTime(PolarScan_t* scan, const char* value)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return RaveDateTime_setTime(scan->enddatetime, value);
}

const char* PolarScan_getEndTime(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  if (RaveDateTime_getTime(scan->enddatetime) == NULL) {
    return PolarScan_getTime(scan);
  } else {
    return RaveDateTime_getTime(scan->enddatetime);
  }
}

int PolarScan_setDate(PolarScan_t* scan, const char* value)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return RaveDateTime_setDate(scan->datetime, value);
}

const char* PolarScan_getDate(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return RaveDateTime_getDate(scan->datetime);
}

int PolarScan_setStartDate(PolarScan_t* scan, const char* value)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return RaveDateTime_setDate(scan->startdatetime, value);
}

const char* PolarScan_getStartDate(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  if (RaveDateTime_getDate(scan->startdatetime) == NULL) {
    return PolarScan_getDate(scan);
  } else {
    return RaveDateTime_getDate(scan->startdatetime);
  }
}

int PolarScan_setEndDate(PolarScan_t* scan, const char* value)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return RaveDateTime_setDate(scan->enddatetime, value);
}

const char* PolarScan_getEndDate(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  if (RaveDateTime_getDate(scan->enddatetime) == NULL) {
    return PolarScan_getDate(scan);
  } else {
    return RaveDateTime_getDate(scan->enddatetime);
  }
}

int PolarScan_setSource(PolarScan_t* scan, const char* value)
{
  char* tmp = NULL;
  int result = 0;
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  if (value != NULL) {
    tmp = RAVE_STRDUP(value);
    if (tmp != NULL) {
      RAVE_FREE(scan->source);
      scan->source = tmp;
      tmp = NULL;
      result = 1;
    }
  } else {
    RAVE_FREE(scan->source);
    result = 1;
  }
  return result;
}

const char* PolarScan_getSource(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return (const char*)scan->source;
}

void PolarScan_setLongitude(PolarScan_t* scan, double lon)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  scan->maxdistance = -1.0;
  PolarNavigator_setLon0(scan->navigator, lon);
}

double PolarScan_getLongitude(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return PolarNavigator_getLon0(scan->navigator);
}

void PolarScan_setLatitude(PolarScan_t* scan, double lat)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  scan->maxdistance = -1.0;
  PolarNavigator_setLat0(scan->navigator, lat);
}

double PolarScan_getLatitude(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return PolarNavigator_getLat0(scan->navigator);
}

void PolarScan_setHeight(PolarScan_t* scan, double height)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  PolarNavigator_setAlt0(scan->navigator, height);
}

double PolarScan_getHeight(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return PolarNavigator_getAlt0(scan->navigator);
}

double PolarScan_getDistance(PolarScan_t* scan, double lon, double lat)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return PolarNavigator_getDistance(scan->navigator, lat, lon);
}

double PolarScan_getMaxDistance(PolarScan_t* scan)
{
  double h = 0.0;

  RAVE_ASSERT((scan != NULL), "scan == NULL");

  if (scan->maxdistance < 0.0) {
    scan->maxdistance = 0.0;
    PolarNavigator_reToDh(scan->navigator, (scan->nbins+1) * scan->rscale, scan->elangle, &scan->maxdistance, &h);
  }

  return scan->maxdistance;
}

void PolarScan_setElangle(PolarScan_t* scan, double elangle)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  scan->elangle = elangle;
  scan->maxdistance = -1.0;
}

double PolarScan_getElangle(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return scan->elangle;
}

long PolarScan_getNbins(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return scan->nbins;
}

void PolarScan_setRscale(PolarScan_t* scan, double rscale)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  scan->rscale = rscale;
  scan->maxdistance = -1.0;
}

double PolarScan_getRscale(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return scan->rscale;
}

long PolarScan_getNrays(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return scan->nrays;
}

void PolarScan_setRstart(PolarScan_t* scan, double rstart)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  scan->rstart = rstart;
}

double PolarScan_getRstart(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return scan->rstart;
}

RaveDataType PolarScan_getDataType(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  if (scan->param == NULL) {
    return RaveDataType_UNDEFINED;
  }
  return PolarScanParam_getDataType(scan->param);
}

void PolarScan_setA1gate(PolarScan_t* scan, long a1gate)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  scan->a1gate = a1gate;
}

long PolarScan_getA1gate(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return scan->a1gate;
}

void PolarScan_setBeamwidth(PolarScan_t* scan, double beamwidth)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  PolarScan_setBeamwH(scan, beamwidth);
}

double PolarScan_getBeamwidth(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return PolarScan_getBeamwH(scan);
}

void PolarScan_setBeamwH(PolarScan_t* scan, double beamwidth)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  scan->beamwH = beamwidth;
  scan->bwpvolH = 0;
}

double PolarScan_getBeamwH(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return scan->beamwH;
}

void PolarScan_setBeamwV(PolarScan_t* scan, double beamwidth)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  scan->beamwV = beamwidth;
  scan->bwpvolV = 0;
}

double PolarScan_getBeamwV(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return scan->beamwV;
}

int PolarScan_setDefaultParameter(PolarScan_t* scan, const char* quantity)
{
  int result = 0;
  char* tmp = NULL;
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  if (quantity == NULL) {
    return 0;
  }
  tmp = RAVE_STRDUP(quantity);
  if (tmp != NULL) {
    RAVE_FREE(scan->paramname);
    scan->paramname = tmp;
    RAVE_OBJECT_RELEASE(scan->param);
    scan->param = (PolarScanParam_t*)RaveObjectHashTable_get(scan->parameters, quantity);
    result = 1;
  }
  return result;
}

const char* PolarScan_getDefaultParameter(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return (const char*)scan->paramname;
}

int PolarScan_addParameter(PolarScan_t* scan, PolarScanParam_t* parameter)
{
  const char* quantity;
  int result = 0;

  RAVE_ASSERT((scan != NULL), "scan == NULL");
  if (parameter == NULL) {
    RAVE_WARNING0("Passing in NULL as parameter");
    return 0;
  }

  quantity = PolarScanParam_getQuantity(parameter);
  if (quantity == NULL) {
    RAVE_WARNING0("No quantity in parameter, can not handle");
    return 0;
  }

  if (RaveObjectHashTable_size(scan->parameters)<=0) {
    scan->nrays = PolarScanParam_getNrays(parameter);
    scan->nbins = PolarScanParam_getNbins(parameter);
    scan->maxdistance = -1.0;
    if (RaveObjectHashTable_exists(scan->attrs, "how/startazA")) {
      PolarScanInternal_createAzimuthNavigationInfo(scan, "startazA");
    }
    if (RaveObjectHashTable_exists(scan->attrs, "how/astart")) {
      PolarScanInternal_createAzimuthNavigationInfo(scan, "astart");
    }
  } else {
    if (scan->nrays != PolarScanParam_getNrays(parameter) ||
        scan->nbins != PolarScanParam_getNbins(parameter)) {
      RAVE_WARNING0("Different number of rays/bins for various parameters are not allowed");
      return 0;
    }
  }

  if (scan->nrays > 0) {
    scan->rayWidth = (360.0 / scan->nrays)*M_PI/180.0;
  }

  result = RaveObjectHashTable_put(scan->parameters, quantity, (RaveCoreObject*)parameter);
  if (result == 1 && strcmp(quantity, scan->paramname)==0) {
    RAVE_OBJECT_RELEASE(scan->param);
    scan->param = RAVE_OBJECT_COPY(parameter);
  }

  return result;
}

PolarScanParam_t* PolarScan_removeParameter(PolarScan_t* scan, const char* quantity)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return (PolarScanParam_t*)RaveObjectHashTable_remove(scan->parameters, quantity);
}

int PolarScan_removeAllParameters(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RaveObjectHashTable_clear(scan->parameters);
  return PolarScan_setDefaultParameter(scan, DEFAULT_PARAMETER_NAME);
}

PolarScanParam_t* PolarScan_getParameter(PolarScan_t* scan, const char* quantity)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return (PolarScanParam_t*)RaveObjectHashTable_get(scan->parameters, quantity);
}

RaveObjectList_t* PolarScan_getParameters(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return RaveObjectHashTable_values(scan->parameters);
}

int PolarScan_hasParameter(PolarScan_t* scan, const char* quantity)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return RaveObjectHashTable_exists(scan->parameters, quantity);
}

RaveList_t* PolarScan_getParameterNames(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return RaveObjectHashTable_keys(scan->parameters);
}

int PolarScan_addQualityField(PolarScan_t* scan, RaveField_t* field)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return RaveObjectList_add(scan->qualityfields, (RaveCoreObject*)field);
}

int PolarScan_addOrReplaceQualityField(PolarScan_t* scan, RaveField_t* field)
{
  RaveAttribute_t* attr = NULL;
  char *fieldstr = NULL;
  int result = 0;
  int found = 0;
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  attr = RaveField_getAttribute(field, "how/task");
  if (attr != NULL && RaveAttribute_getString(attr, &fieldstr) && fieldstr != NULL) {
    int i;
    int nQualityFields = PolarScan_getNumberOfQualityFields(scan);
    for (i = 0; found == 0 && i < nQualityFields; i++) {
      RaveField_t* qfield = PolarScan_getQualityField(scan, i);
      if (qfield != NULL) {
        char *qfieldstr = NULL;
        RaveAttribute_t* qfieldattr = RaveField_getAttribute(qfield, "how/task");
        if (qfieldattr != NULL && RaveAttribute_getString(qfieldattr, &qfieldstr) && qfieldstr != NULL) {
          if (strcmp(fieldstr, qfieldstr) == 0) {
            PolarScan_removeQualityField(scan, i);
            found = 1;
          }
        }
        RAVE_OBJECT_RELEASE(qfieldattr);
      }
      RAVE_OBJECT_RELEASE(qfield);
    }
  }

  result = PolarScan_addQualityField(scan, field);
  RAVE_OBJECT_RELEASE(attr);
  return result;
}


RaveField_t* PolarScan_getQualityField(PolarScan_t* scan, int index)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return (RaveField_t*)RaveObjectList_get(scan->qualityfields, index);
}

int PolarScan_getNumberOfQualityFields(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return RaveObjectList_size(scan->qualityfields);
}

void PolarScan_removeQualityField(PolarScan_t* scan, int index)
{
  RaveField_t* field = NULL;
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  field = (RaveField_t*)RaveObjectList_remove(scan->qualityfields, index);
  RAVE_OBJECT_RELEASE(field);
}

RaveObjectList_t* PolarScan_getQualityFields(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return (RaveObjectList_t*)RAVE_OBJECT_COPY(scan->qualityfields);
}

RaveField_t* PolarScan_getQualityFieldByHowTask(PolarScan_t* scan, const char* value)
{
  int nfields = 0, i = 0;
  RaveField_t* result = NULL;

  RAVE_ASSERT((scan != NULL), "scan == NULL");
  if (value == NULL) {
    RAVE_WARNING0("Trying to use PolarScan-getQualityFieldByHowTask without a how/task value");
    return NULL;
  }
  nfields = RaveObjectList_size(scan->qualityfields);
  for (i = 0; result == NULL && i < nfields; i++) {
    RaveField_t* field = (RaveField_t*)RaveObjectList_get(scan->qualityfields, i);
    if (field != NULL && RaveField_hasAttributeStringValue(field, "how/task", value)) {
      result = RAVE_OBJECT_COPY(field);
    }
    RAVE_OBJECT_RELEASE(field);
  }
  return result;
}

RaveField_t* PolarScan_findQualityFieldByHowTask(PolarScan_t* scan, const char* value, const char* quantity)
{
  RaveField_t* result = NULL;
  PolarScanParam_t* param = NULL;

  RAVE_ASSERT((scan != NULL), "scan == NULL");
  if (quantity != NULL) {
    param = PolarScan_getParameter(scan, quantity);
  } else {
    param = RAVE_OBJECT_COPY(scan->param);
  }

  if (param != NULL) {
    result = PolarScanParam_getQualityFieldByHowTask(param, value);
  }

  if (result == NULL) {
    result = PolarScan_getQualityFieldByHowTask(scan, value);
  }

  RAVE_OBJECT_RELEASE(param);
  return result;
}

RaveField_t* PolarScan_findAnyQualityFieldByHowTask(PolarScan_t* scan, const char* value)
{
  RaveField_t *result = NULL;
  PolarScanParam_t* param = NULL;
  RaveObjectList_t* params = NULL;

  RAVE_ASSERT((scan != NULL), "scan == NULL");

  params = RaveObjectHashTable_values(scan->parameters);
  if (params != NULL) {
    int nparams = 0, i = 0;
    nparams = RaveObjectList_size(params);
    for (i = 0; result == NULL && i < nparams; i++) {
      param = (PolarScanParam_t*)RaveObjectList_get(params, i);
      result = PolarScanParam_getQualityFieldByHowTask(param, value);
      RAVE_OBJECT_RELEASE(param);
    }
  }

  if (result == NULL) {
    result = PolarScan_getQualityFieldByHowTask(scan, value);
  }

  RAVE_OBJECT_RELEASE(params);
  return result;
}

int PolarScan_getRangeIndex(
    PolarScan_t* scan,
    double r, 
    PolarScanSelectionMethod_t selectionMethod, 
    int rangeMidpoint)
{
  int result = -1;
  double range = 0.0L;

  RAVE_ASSERT((scan != NULL), "scan was NULL");

  if (scan->nbins <= 0 || scan->rscale <= 0.0) {
    RAVE_WARNING0("Can not calculate range index");
    return -1;
  }

  range = r - scan->rstart*1000.0;

  if (rangeMidpoint) {
    range -= scan->rscale / 2.0;
  }

  if (range >= 0.0) {
    if (!PolarScanInternal_roundBySelectionMethod(range/scan->rscale, selectionMethod, &result)) {
      return -1;
    }
  }

  if (result >= scan->nbins || result < 0) {
    result = -1;
  }

  return result;
}

double PolarScan_getRange(PolarScan_t* scan, int ri, int rangeMidpoint)
{
  double result = -1.0L;
  RAVE_ASSERT((scan != NULL), "scan == NULL");

  if (scan->nbins <= 0 || scan->rscale <= 0.0) {
    RAVE_WARNING0("Can not calculate range");
    goto done;
  }
  if (ri < 0 || ri >= scan->nbins) {
    RAVE_INFO0("Providing range index outside boundaries");
    goto done;
  }
  result = ((double)ri) * scan->rscale;

  result += scan->rstart*1000.0;

  if (rangeMidpoint) {
    result += scan->rscale / 2.0;
  }
done:
  return result;
}

void PolarScan_setUseAzimuthalNavInformation(PolarScan_t* self, int v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (v == 0) {
    self->useAzimuthalNavInformation = 0;
  } else {
    self->useAzimuthalNavInformation = 1;
  }
}

int PolarScan_useAzimuthalNavInformation(PolarScan_t* self)
{
  RAVE_ASSERT((self != NULL), "scan == NULL");
  return self->useAzimuthalNavInformation;
}

int PolarScan_getNorthmostIndex(PolarScan_t* self)
{
  int i=0, result=0, foundidx = 0;
  RaveAttribute_t *startAz = NULL, *stopAz = NULL;
  double* startazArr = NULL;
  int startazLen = 0;
  double minv = 360.0;

  RAVE_ASSERT((self != NULL), "scan == NULL");

  startAz = (RaveAttribute_t*)RaveObjectHashTable_get(self->attrs, "how/startazA");
  stopAz = (RaveAttribute_t*)RaveObjectHashTable_get(self->attrs, "how/stopazA");
  if (startAz != NULL && !RaveAttribute_getDoubleArray(startAz, &startazArr, &startazLen)) {
    RAVE_ERROR0("Failed to extract startazA array");
    goto done;
  }

  if (startazArr != NULL) { /* We have got startazA, use that one to determine northmost index. */
    for (i = 0; i < startazLen; i++) {
      double tmpv = 180.0 - fabs(fabs(startazArr[i] - 360.0)-180.0);
      if (tmpv < minv) {
        minv = tmpv;
        foundidx = i;
      }
    }
  } else if (self->hasAstart) { /* This one is set when adding the how/astart attribute. Note that self->astart is stored in radians. */
    double azlimit = 0.0;
    double azOffset = 0.0;
    if (self->nrays == 0) {
      RAVE_ERROR0("Trying to determine north most index without having and data");
      result = -1;
      goto done;
    }
    azlimit = M_PI/self->nrays; /* half teoretical ray width ought to be enough. Actual formula would be 2*M_PI/2*self->nrays  or 360/(2*nrays)*/
    azOffset = 2*azlimit;
    PolarScanInternal_roundBySelectionMethod((azlimit - self->astart)/azOffset, PolarScanSelectionMethod_FLOOR, &foundidx);
    if (foundidx < 0) {
      foundidx += self->nrays;
    }
  }

  result = foundidx;
done:
  RAVE_OBJECT_RELEASE(startAz);
  RAVE_OBJECT_RELEASE(stopAz);
  return result;
}

int PolarScan_getRotationRequiredToNorthmost(PolarScan_t* self)
{
  int result = 0;
  RAVE_ASSERT((self != NULL), "scan == NULL");
  result = PolarScan_getNorthmostIndex(self);
  if (result != 0) {
    if (result > self->nrays/2) {
      result = self->nrays - result;
    } else {
      result = 0 - result;
    }
  }
  return result;
}


static int PolarScanInternal_handleAzimuthIndexWrap(int nrays, int index) {
  int result = index;
  if (result < 0 || result >= nrays) {
    while (result >= nrays) result -= nrays;
    while (result < 0) result += nrays;
  }
  return result;
}

int PolarScan_getAzimuthIndex(PolarScan_t* scan, double a, PolarScanSelectionMethod_t selectionMethod)
{
  int result = -1;
  double azOffset = 0.0L;
  RAVE_ASSERT((scan != NULL), "scan was NULL");

  if (scan->nrays <= 0) {
    RAVE_WARNING0("Can not calculate azimuth index");
    return -1;
  }
  azOffset = 2*M_PI/scan->nrays;
  if (scan->useAzimuthalNavInformation) {
    if (scan->hasAzimuthArr && scan->azimuthArrLen == scan->nrays) {
      int i = 0;
      /** Best first guess where we try to find the index closest directly. Since the angles always will
       * increasing / decreasing we can test index before and after to see if we have found best fit immediately.
       * If not we either search backward or forward and break when we are starting to get increased angles again.
       */
      // Best first guess
      int bestGuessIndex = PolarScanInternal_handleAzimuthIndexWrap(scan->nrays, (int)rint(a/azOffset));
      int prevIndex = PolarScanInternal_handleAzimuthIndexWrap(scan->nrays, bestGuessIndex-1);
      int nextIndex = PolarScanInternal_handleAzimuthIndexWrap(scan->nrays, bestGuessIndex+1);
      double bestGuessDist = fabs(M_PI - fabs(fabs(scan->azimuthArr[bestGuessIndex] - a) - M_PI));
      double nextGuessDist = fabs(M_PI - fabs(fabs(scan->azimuthArr[nextIndex] - a) - M_PI));
      double prevGuessDist = fabs(M_PI - fabs(fabs(scan->azimuthArr[prevIndex] - a) - M_PI));

      if (nextGuessDist < bestGuessDist) {
        result = nextIndex;
        for (i = 1; i < scan->azimuthArrLen; i++) {
          int nxt = PolarScanInternal_handleAzimuthIndexWrap(scan->nrays, nextIndex+i);
          double tmp = fabs(M_PI - fabs(fabs(scan->azimuthArr[nxt] - a) - M_PI));
          if (tmp > nextGuessDist) { // If we start to get longer distances to original azimuth we have already reached close hit
            break;
          }
          result = nxt;
        }
      } else if (prevGuessDist < bestGuessDist) {
        result = prevIndex;
        for (i = 1; i < scan->azimuthArrLen; i++) {
          int prev = PolarScanInternal_handleAzimuthIndexWrap(scan->nrays, prevIndex-i);
          double tmp = fabs(M_PI - fabs(fabs(scan->azimuthArr[prev] - a) - M_PI));
          if (tmp > prevGuessDist) { // If we start to get longer distances to original azimuth we have already reached close hit
            break;
          }
          result = prev;
          prevGuessDist=tmp;
        }
      } else {
        result = bestGuessIndex;
      }
    } else {
      if (scan->hasAstart) {
        a = a - scan->astart;
        if (!PolarScanInternal_roundBySelectionMethod(a/azOffset, PolarScanSelectionMethod_FLOOR, &result)) {
          return -1;
        }
      } else {
        if (!PolarScanInternal_roundBySelectionMethod(a/azOffset, selectionMethod, &result)) {
          return -1;
        }
      }
    }
  } else  {
    if (!PolarScanInternal_roundBySelectionMethod(a/azOffset, selectionMethod, &result)) {
      return -1;
    }
  }

  if (result >= scan->nrays) {
    result -= scan->nrays;
  } else if (result < 0) {
    result += scan->nrays;
  }
  return result;
}

double PolarScan_getAzimuth(PolarScan_t* scan, int ai)
{
  double result = -1.0L;
  double azOffset = 0.0L;
  RAVE_ASSERT((scan != NULL), "scan was NULL");
  if (scan->nrays <= 0) {
    RAVE_WARNING0("Can not calculate azimuth index");
    return -1;
  }
  if (ai < 0 || ai >= scan->nrays) {
    return -1.0;
  }
  azOffset = 2*M_PI/scan->nrays;
  if (scan->useAzimuthalNavInformation) {
    if (scan->hasAzimuthArr && scan->azimuthArrLen == scan->nrays) {
      result = scan->azimuthArr[ai];
    } else {
      result = (double)ai * azOffset;
      if (scan->hasAstart) {
        result += scan->astart;
        while (result < 0) {
          result += 2*M_PI;
        }
      }
    }
  } else {
    result = (double)ai * azOffset;
  }

  return result;
}

int PolarScan_setValue(PolarScan_t* scan, int bin, int ray, double v)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  if (scan->param == NULL) {
    return 0;
  }
  return PolarScanParam_setValue(scan->param, bin, ray, v);
}

int PolarScan_setParameterValue(PolarScan_t* scan, const char* quantity, int bin, int ray, double v)
{
  PolarScanParam_t* param = NULL;
  int result = 0;
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RAVE_ASSERT((quantity != NULL), "quantity == NULL");
  param = (PolarScanParam_t*)RaveObjectHashTable_get(scan->parameters, quantity);
  if (param != NULL) {
    result = PolarScanParam_setValue(param, bin, ray, v);
  }
  RAVE_OBJECT_RELEASE(param);
  return result;
}

RaveValueType PolarScan_getValue(PolarScan_t* scan, int bin, int ray, double* v)
{
  RaveValueType result = RaveValueType_NODATA;
  double value = 0.0;
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  if (scan->param == NULL) {
    return RaveValueType_UNDEFINED;
  }
  value = PolarScanParam_getNodata(scan->param);
  result = PolarScanParam_getValue(scan->param, bin, ray, &value);
  if (v != NULL) {
    *v = value;
  }

  return result;
}

RaveValueType PolarScan_getParameterValue(PolarScan_t* scan, const char* quantity, int bin, int ray, double* v)
{
  PolarScanParam_t* param = NULL;
  RaveValueType result = 0;
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RAVE_ASSERT((quantity != NULL), "quantity == NULL");
  param = (PolarScanParam_t*)RaveObjectHashTable_get(scan->parameters, quantity);
  if (param != NULL) {
    result = PolarScanParam_getValue(param, bin, ray, v);
  }
  RAVE_OBJECT_RELEASE(param);
  return result;
}

RaveValueType PolarScan_getConvertedValue(PolarScan_t* scan, int bin, int ray, double* v)
{
  RaveValueType result = RaveValueType_NODATA;
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  if (scan->param == NULL) {
    return RaveValueType_UNDEFINED;
  }

  if (v != NULL) {
    result =  PolarScan_getValue(scan, bin, ray, v);
    if (result == RaveValueType_DATA) {
      *v = PolarScanParam_getOffset(scan->param) + (*v) * PolarScanParam_getGain(scan->param);
    }
  }

  return result;
}

RaveValueType PolarScan_getConvertedParameterValue(PolarScan_t* scan, const char* quantity, int bin, int ray, double* v)
{
  PolarScanParam_t* param = NULL;
  RaveValueType result = RaveValueType_UNDEFINED;
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RAVE_ASSERT((quantity != NULL), "quantity == NULL");

  param = (PolarScanParam_t*)RaveObjectHashTable_get(scan->parameters, quantity);
  if (param != NULL) {
    result = PolarScanParam_getConvertedValue(param, bin, ray, v);
  }
  RAVE_OBJECT_RELEASE(param);
  return result;
}

int PolarScan_getIndexFromAzimuthAndRange(
    PolarScan_t* scan,
    double a,
    double r,
    PolarScanSelectionMethod_t azimuthSelectionMethod,
    PolarScanSelectionMethod_t rangeSelectionMethod,
    int rangeMidpoint,
    int* ray,
    int* bin)
{
  int result = 0;
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RAVE_ASSERT((bin != NULL), "bin == NULL");
  RAVE_ASSERT((ray != NULL), "ray == NULL");
  *ray = PolarScan_getAzimuthIndex(scan, a, azimuthSelectionMethod);
  if (*ray < 0) {
    goto done;
  }
  *bin = PolarScan_getRangeIndex(scan, r, rangeSelectionMethod, rangeMidpoint);
  if (*bin < 0) {
    goto done;
  }
  result = 1;
done:
  return result;
}

int PolarScan_getAzimuthAndRangeFromIndex(PolarScan_t* scan, int bin, int ray, double* a, double* r)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RAVE_ASSERT((a != NULL), "a == NULL");
  RAVE_ASSERT((r != NULL), "r == NULL");
  *r = bin * scan->rscale;
  *a = (2*M_PI/scan->nrays)*ray;
  return 1;
}

RaveValueType PolarScan_getValueAtAzimuthAndRange(PolarScan_t* scan, double a, double r, int convert, double* v)
{
  RaveValueType result = RaveValueType_NODATA;
  int ai = 0, ri = 0;
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RAVE_ASSERT((v != NULL), "v == NULL");
  if (scan->param == NULL) {
    return RaveValueType_UNDEFINED;
  }
  if (v != NULL) {
    *v = PolarScanParam_getNodata(scan->param);
  }
  if (!PolarScan_getIndexFromAzimuthAndRange(scan, a, r, PolarScanSelectionMethod_ROUND, PolarScanSelectionMethod_FLOOR, 0, &ai, &ri)) {
    goto done;
  }

  if (convert) {
    result = PolarScan_getConvertedValue(scan, ri, ai, v);
  } else {
    result = PolarScan_getValue(scan, ri, ai, v);
  }
done:
  return result;
}

RaveValueType PolarScan_getParameterValueAtAzimuthAndRange(PolarScan_t* scan, const char* quantity, double a, double r, double* v)
{
  RaveValueType result = RaveValueType_UNDEFINED;
  int ai = 0, ri = 0;
  PolarScanParam_t* param = NULL;

  RAVE_ASSERT((scan != NULL), "scan was NULL");
  RAVE_ASSERT((v != NULL), "v was NULL");
  param = PolarScan_getParameter(scan, quantity);
  if (param != NULL) {
    result = RaveValueType_NODATA;
    *v = PolarScanParam_getNodata(param);
    if (!PolarScan_getIndexFromAzimuthAndRange(scan, a, r, PolarScanSelectionMethod_ROUND, PolarScanSelectionMethod_FLOOR, 0, &ai, &ri)) {
      goto done;
    }
    result = PolarScanParam_getValue(param, ri, ai, v);
  }
done:
  RAVE_OBJECT_RELEASE(param);
  return result;
}

RaveValueType PolarScan_getConvertedParameterValueAtAzimuthAndRange(PolarScan_t* scan, const char* quantity, double a, double r, double* v)
{
  RaveValueType result = RaveValueType_UNDEFINED;
  int ai = 0, ri = 0;
  PolarScanParam_t* param = NULL;

  RAVE_ASSERT((scan != NULL), "scan was NULL");
  RAVE_ASSERT((v != NULL), "v was NULL");
  param = PolarScan_getParameter(scan, quantity);
  if (param != NULL) {
    result = RaveValueType_NODATA;
    *v = PolarScanParam_getNodata(param);
    if (!PolarScan_getIndexFromAzimuthAndRange(scan, a, r, PolarScanSelectionMethod_ROUND, PolarScanSelectionMethod_FLOOR, 0, &ai, &ri)) {
      goto done;
    }
    result = PolarScanParam_getConvertedValue(param, ri, ai, v);
  }
done:
  RAVE_OBJECT_RELEASE(param);
  return result;
}

void PolarScan_getLonLatNavigationInfo(PolarScan_t* scan, double lon, double lat, PolarNavigationInfo* info)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RAVE_ASSERT((info != NULL), "info == NULL");
  info->lon = lon;
  info->lat = lat;
  info->distance = 0.0L;
  info->azimuth = 0.0L;
  info->range = 0.0L;
  info->height = 0.0L;
  info->actual_height = 0.0L;
  info->elevation = scan->elangle;

  info->otype = Rave_ObjectType_SCAN;
  info->ei = -1;
  info->ri = -1;
  info->ai = -1;

  PolarNavigator_llToDa(scan->navigator, lat, lon, &info->distance, &info->azimuth);
  PolarNavigator_deToRh(scan->navigator, info->distance, info->elevation, &info->range, &info->height);
  info->actual_height = info->height;
}

int PolarScan_fillNavigationIndexFromAzimuthAndRange(
  PolarScan_t* scan,
  PolarScanSelectionMethod_t azimuthSelectionMethod,
  PolarScanSelectionMethod_t rangeSelectionMethod,
  int rangeMidpoint,
  PolarNavigationInfo* info)
{
  int ai = -1, ri = -1, result = 0;
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RAVE_ASSERT((info != NULL), "info == NULL");
  info->ai = -1;
  info->ri = -1;
  result = PolarScan_getIndexFromAzimuthAndRange(scan, info->azimuth, info->range, azimuthSelectionMethod, rangeSelectionMethod, rangeMidpoint, &ai, &ri);
  if (result) {
    info->ai = ai;
    info->ri = ri;
    info->actual_range = PolarScan_getRange(scan, ri, rangeMidpoint);
    info->actual_azimuth = PolarScan_getAzimuth(scan, ai);
  }
  return result;
}

RaveValueType PolarScan_getNearest(PolarScan_t* scan, double lon, double lat, int convert, double* v)
{
  RaveValueType result = RaveValueType_NODATA;
  PolarNavigationInfo info;
  RAVE_ASSERT((scan != NULL), "scan was NULL");
  RAVE_ASSERT((v != NULL), "v was NULL");
  if (scan->param == NULL) {
    return RaveValueType_UNDEFINED;
  }
  PolarScan_getLonLatNavigationInfo(scan, lon, lat, &info);

  result = PolarScan_getValueAtAzimuthAndRange(scan, info.azimuth, info.range, convert, v);

  return result;
}

RaveValueType PolarScan_getNearestParameterValue(PolarScan_t* scan, const char* quantity, double lon, double lat, double* v)
{
  RaveValueType result = RaveValueType_NODATA;
  PolarNavigationInfo info;
  RAVE_ASSERT((scan != NULL), "scan was NULL");
  RAVE_ASSERT((v != NULL), "v was NULL");

  PolarScan_getLonLatNavigationInfo(scan, lon, lat, &info);

  result = PolarScan_getParameterValueAtAzimuthAndRange(scan, quantity, info.azimuth, info.range, v);

  return result;
}

int PolarScan_addSurroundingNavigationInfosForTarget(
    PolarScan_t* scan,
    PolarNavigationInfo* targetNavInfo,
    int surroundingRangeBins,
    int surroundingRays,
    int noofNavinfos,
    PolarNavigationInfo navinfos[])
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RAVE_ASSERT((targetNavInfo != NULL), "targetNavInfo == NULL");
  RAVE_ASSERT((navinfos != NULL), "navinfos == NULL");

  int ceilRange = 0;
  int ceilAzimuth = 0;
  for (ceilRange = 0; ceilRange <= surroundingRangeBins; ceilRange++) {
    for (ceilAzimuth = 0; ceilAzimuth <= surroundingRays; ceilAzimuth++) {
      PolarScanSelectionMethod_t azimuthSelectionMethod = PolarScanSelectionMethod_ROUND;
      if (surroundingRays) {
        if (ceilAzimuth) {
          azimuthSelectionMethod = PolarScanSelectionMethod_CEIL;
        } else {
          azimuthSelectionMethod = PolarScanSelectionMethod_FLOOR;
        }
      }

      PolarScanSelectionMethod_t rangeSelectionMethod = PolarScanSelectionMethod_FLOOR;
      if (ceilRange) {
        rangeSelectionMethod = PolarScanSelectionMethod_CEIL;
      }

      PolarNavigationInfo* navinfo = PolarScan_getNavigationInfo(scan, targetNavInfo, rangeSelectionMethod, azimuthSelectionMethod, surroundingRangeBins);

      if (navinfo != NULL) {
        navinfos[noofNavinfos] = *navinfo;
        noofNavinfos++;
        RAVE_FREE(navinfo);
      }
    }
  }

  return noofNavinfos;
}

int PolarScan_getSurroundingNavigationInfos(
    PolarScan_t* scan,
    double lon,
    double lat,
    int surroundingRangeBins,
    int surroundingRays,
    PolarNavigationInfo navinfos[])
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RAVE_ASSERT((navinfos != NULL), "navinfos == NULL");

  PolarNavigationInfo targetNavInfo;
  PolarScan_getLonLatNavigationInfo(scan, lon, lat, &targetNavInfo);

  return PolarScan_addSurroundingNavigationInfosForTarget(scan, &targetNavInfo, surroundingRangeBins, surroundingRays, 0, navinfos);
}

int PolarScan_getNearestNavigationInfo(PolarScan_t* scan, double lon, double lat, PolarNavigationInfo* navinfo)
{
  int result = 0;
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RAVE_ASSERT((navinfo != NULL), "navinfo == NULL");
  PolarScan_getLonLatNavigationInfo(scan, lon, lat, navinfo);
  PolarScan_fillNavigationIndexFromAzimuthAndRange(scan, PolarScanSelectionMethod_ROUND, PolarScanSelectionMethod_FLOOR, 0, navinfo);
  if (navinfo->ai >= 0 && navinfo->ri >= 0) {
    result = 1;
  }
  return result;
}

RaveValueType PolarScan_getNearestConvertedParameterValue(PolarScan_t* scan, const char* quantity, double lon, double lat, double* v, PolarNavigationInfo* navinfo)
{
  RaveValueType result = RaveValueType_NODATA;
  PolarNavigationInfo info;

  RAVE_ASSERT((scan != NULL), "scan was NULL");
  RAVE_ASSERT((v != NULL), "v was NULL");

  PolarScan_getNearestNavigationInfo(scan, lon, lat, &info);

  result = PolarScan_getConvertedParameterValue(scan, quantity, info.ri, info.ai, v);

  if (navinfo != NULL) {
    *navinfo = info;
  }
  return result;
}

int PolarScan_getNearestIndex(PolarScan_t* scan, double lon, double lat, int* bin, int* ray)
{
  PolarNavigationInfo info;
  int result = 0;
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RAVE_ASSERT((bin != NULL), "bin == NULL");
  RAVE_ASSERT((ray != NULL), "ray == NULL");

  PolarScan_getLonLatNavigationInfo(scan, lon, lat, &info);

  result = PolarScan_getIndexFromAzimuthAndRange(scan, info.azimuth, info.range, PolarScanSelectionMethod_ROUND, PolarScanSelectionMethod_FLOOR, 0, ray, bin);

  return result;
}

int PolarScan_getLonLatFromIndex(PolarScan_t* scan, int bin, int ray, double* lon, double* lat)
{
  int result = 0;
  double d = 0.0, h = 0.0, a = 0.0, r = 0.0;

  if (!PolarScan_getAzimuthAndRangeFromIndex(scan, bin, ray, &a, &r)) {
    goto done;
  }
  PolarNavigator_reToDh(scan->navigator, r, scan->elangle, &d, &h);
  PolarNavigator_daToLl(scan->navigator, d, a, lat, lon);

  result = 1;
done:
  return result;
}

int PolarScan_getQualityValueAt(PolarScan_t* scan, const char* quantity, int ri, int ai, const char* name, int convert, double* v)
{
  PolarScanParam_t* param = NULL;
  RaveField_t* quality = NULL;
  int result = 0;

  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RAVE_ASSERT((v != NULL), "v == NULL");

  if (quantity != NULL) {
    param = PolarScan_getParameter(scan, quantity);
    if (param == NULL) {
      goto done;
    }
    quality = PolarScanParam_getQualityFieldByHowTask(param, name);
  }

  if (quality == NULL) {
    quality = PolarScan_getQualityFieldByHowTask(scan, name);
  }

  if (quality == NULL) {
    /*RAVE_WARNING1("Failed to locate a quality field with how/task = %s", name);*/
    goto done;
  }
  if (convert) {
    result = RaveField_getConvertedValue(quality, ri, ai, v);
  } else {
    result = RaveField_getValue(quality, ri, ai, v);
  }

done:
  RAVE_OBJECT_RELEASE(param);
  RAVE_OBJECT_RELEASE(quality);
  return result;
}

int PolarScan_isTransformable(PolarScan_t* scan)
{
  int result = 0;
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  if (scan->projection != NULL &&
      scan->navigator != NULL &&
      scan->rscale > 0.0) {
    result = 1;
  }
  return result;
}

int PolarScan_addAttribute(PolarScan_t* scan, RaveAttribute_t* attribute)
{
  const char* name = NULL;
  char* aname = NULL;
  char* gname = NULL;
  int result = 0;
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  RAVE_ASSERT((attribute != NULL), "attribute == NULL");

  name = RaveAttribute_getName(attribute);
  if (name != NULL) {
    if (!RaveAttributeHelp_extractGroupAndName(name, &gname, &aname)) {
      RAVE_ERROR1("Failed to extract group and name from %s", name);
      goto done;
    }
    if ((strcasecmp("how", gname)==0) &&
        RaveAttributeHelp_validateHowGroupAttributeName(gname, aname)) {
      result = RaveObjectHashTable_put(scan->attrs, name, (RaveCoreObject*)attribute);
      if (result) {
        if (strcasecmp("astart", aname) == 0 ||
            strcasecmp("startazA", aname) == 0 ||
            strcasecmp("stopazA", aname) == 0) {
          PolarScanInternal_createAzimuthNavigationInfo(scan, (const char*)aname);
        }
      }
    } else {
      RAVE_DEBUG1("Trying to add attribute: %s but only valid attributes are how/...", name);
    }
  }

done:
  RAVE_FREE(aname);
  RAVE_FREE(gname);
  return result;
}

void PolarScan_removeAttribute(PolarScan_t* scan, const char* attrname)
{
  char* aname = NULL;
  char* gname = NULL;
  RAVE_ASSERT((scan != NULL), "scan == NULL");
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
      RaveCoreObject* attr = RaveObjectHashTable_remove(scan->attrs, attrname);
      RAVE_OBJECT_RELEASE(attr);
    }
  }

done:
  RAVE_FREE(aname);
  RAVE_FREE(gname);
}


RaveAttribute_t* PolarScan_getAttribute(PolarScan_t* scan, const char* name)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  if (name == NULL) {
    RAVE_ERROR0("Trying to get an attribute with NULL name");
    return NULL;
  }
  return (RaveAttribute_t*)RaveObjectHashTable_get(scan->attrs, name);
}

int PolarScan_hasAttribute(PolarScan_t* scan, const char* name)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return RaveObjectHashTable_exists(scan->attrs, name);
}

RaveList_t* PolarScan_getAttributeNames(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return RaveObjectHashTable_keys(scan->attrs);
}

RaveObjectList_t* PolarScan_getAttributeValues(PolarScan_t* scan)
{
  RaveObjectList_t* result = NULL;
  RaveObjectList_t* tableattrs = NULL;

  RAVE_ASSERT((scan != NULL), "scan == NULL");
  tableattrs = RaveObjectHashTable_values(scan->attrs);
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

int PolarScan_shiftAttribute(PolarScan_t* scan, const char* name, int nx)
{
  RaveAttribute_t* attr = NULL;
  int result = 0;

  attr = PolarScan_getAttribute(scan, name);
  if (attr != NULL) {
    if (RaveAttribute_getFormat(attr) == RaveAttribute_Format_LongArray ||
        RaveAttribute_getFormat(attr) == RaveAttribute_Format_DoubleArray) {
      result = RaveAttribute_shiftArray(attr, nx);
    }
  }
  RAVE_OBJECT_RELEASE(attr);

  return result;
}

int PolarScan_isValid(PolarScan_t* scan, Rave_ObjectType otype)
{
  int result = 1;
  RAVE_ASSERT((scan != NULL), "scan == NULL");

  if (otype == Rave_ObjectType_PVOL) {
    if (PolarScan_getTime(scan) == NULL ||
        PolarScan_getDate(scan) == NULL ||
        !RaveObjectHashTable_exists(scan->attrs, "what/enddate") ||
        !RaveObjectHashTable_exists(scan->attrs, "what/endtime")) {
      RAVE_INFO0("Missing start/end date/time information");
      goto done;
    }
    if (PolarScan_getNbins(scan) <= 0 ||
        PolarScan_getNrays(scan) <= 0) {
      RAVE_INFO0("Missing size information");
      goto done;
    }
    if (RaveObjectHashTable_size(scan->parameters) <= 0) {
      RAVE_INFO0("Must at least contain one parameter");
      goto done;
    }
  } else if (otype == Rave_ObjectType_SCAN) {
    if (PolarScan_getTime(scan) == NULL ||
        PolarScan_getDate(scan) == NULL ||
        PolarScan_getSource(scan) == NULL) {
      RAVE_INFO0("date, time and source must be specified");
      goto done;
    }
    if (PolarScan_getNbins(scan) <= 0 ||
        PolarScan_getNrays(scan) <= 0) {
      RAVE_INFO0("Missing size information");
      goto done;
    }
    if (RaveObjectHashTable_size(scan->parameters) <= 0) {
      RAVE_INFO0("Must at least contain one parameter");
      goto done;
    }
  } else {
    RAVE_ERROR0("Only valid types for isValid are PVOL and SCAN");
    goto done;
  }

  result = 1;
done:
  return result;
}

PolarScan_t* PolarScan_createFromScanAndField(PolarScan_t* self, RaveField_t* field)
{
  PolarScan_t* result = NULL;
  PolarScan_t* scan = NULL;
  PolarScanParam_t* param = NULL;

  RAVE_ASSERT((self != NULL), "scan == NULL");

  if (field == NULL) {
    RAVE_ERROR0("Trying to create scan from NULL field");
    return NULL;
  }

  scan = RAVE_OBJECT_CLONE(self);
  if (scan == NULL) {
    goto done;
  }
  RaveObjectHashTable_clear(scan->parameters);
  RaveObjectList_clear(scan->qualityfields);
  RAVE_OBJECT_RELEASE(scan->param);

  param = PolarScanParam_fromField(field);
  if (param == NULL) {
    goto done;
  }

  if (PolarScanParam_getQuantity(param) == NULL) {
    if (!PolarScanParam_setQuantity(param, "UNKNOWN")) {
      goto done;
    }
  }

  if (!PolarScan_addParameter(scan, param)) {
    goto done;
  }

  if (!PolarScan_setDefaultParameter(scan, PolarScanParam_getQuantity(param))) {
    goto done;
  }

  result = RAVE_OBJECT_COPY(scan);
done:
  RAVE_OBJECT_RELEASE(scan);
  RAVE_OBJECT_RELEASE(param);
  return result;
}

/**
 * Returns the height or distance field for this scan. The height is the altitude at the
 * location represented by each bin and the distance is the distance on ground level.
 * @param[in] self - self
 * @param[in] ftype - if 0 then distance field will be generated otherwise the height field will be generated.
 * @returns the rave field
 */
static RaveField_t* PolarScanInternal_getHeightOrDistanceField(PolarScan_t* self, int ftype)
{
  RaveField_t *f = NULL, *result = NULL;
  int i = 0;

  RAVE_ASSERT((self != NULL), "self == NULL");

  f = RAVE_OBJECT_NEW(&RaveField_TYPE);
  if (f == NULL) {
    RAVE_ERROR0("Failed to allocate memory for rave field");
    goto done;
  }

  if (!RaveField_createData(f, self->nbins, 1, RaveDataType_DOUBLE)) {
    RAVE_ERROR0("Failed to create data for distance field");
    goto done;
  }

  for (i = 0; i < self->nbins; i++) {
    double d = 0.0, h = 0.0;
    PolarNavigator_reToDh(self->navigator, i*self->rscale, self->elangle, &d, &h);
    if (ftype == 0) {
      RaveField_setValue(f, i, 0, d);
    } else {
      RaveField_setValue(f, i, 0, h);
    }
  }

  result = RAVE_OBJECT_COPY(f);
done:
  RAVE_OBJECT_RELEASE(f);
  return result;
}

RaveField_t* PolarScan_getDistanceField(PolarScan_t* self)
{
  return PolarScanInternal_getHeightOrDistanceField(self, 0);
}

RaveField_t* PolarScan_getHeightField(PolarScan_t* self)
{
  return PolarScanInternal_getHeightOrDistanceField(self, 1);
}

int PolarScan_shiftData(PolarScan_t* self, int nrays)
{
  int result = 0, nrparams = 0, nrqualityfields = 0, i = 0;
  RaveObjectList_t* parameters = NULL;
  RAVE_ASSERT((self != NULL), "self == NULL");
  parameters = PolarScan_getParameters(self);
  if (parameters == NULL) {
    goto fail;
  }

  nrparams = RaveObjectList_size(parameters);
  for (i = 0; i < nrparams; i++) {
    PolarScanParam_t* parameter = (PolarScanParam_t*)RaveObjectList_get(parameters, i);
    if (parameter != NULL) {
      if (!PolarScanParam_shiftData(parameter, nrays)) {
        RAVE_ERROR1("Failed to shift rays for %s", PolarScanParam_getQuantity(parameter));
        RAVE_OBJECT_RELEASE(parameter);
        goto fail;
      }
    } else {
      RAVE_ERROR0("Failed to shift rays on parameter");
      RAVE_OBJECT_RELEASE(parameter);
      goto fail;
    }
    RAVE_OBJECT_RELEASE(parameter);
  }

  nrqualityfields = PolarScan_getNumberOfQualityFields(self);
  for (i = 0; i < nrqualityfields; i++) {
    RaveField_t* field = PolarScan_getQualityField(self, i);
    if (field != NULL) {
      if(!RaveField_circshiftData(field, 0, nrays)) {
        RAVE_ERROR1("Failed to shift rays for quality field %d", i);
        RAVE_OBJECT_RELEASE(field);
        goto fail;
      }
    } else {
      RAVE_ERROR0("Programming error, should not be possible to get here");
      RAVE_OBJECT_RELEASE(field);
      goto fail;
    }
    RAVE_OBJECT_RELEASE(field);
  }

  result = 1;
fail:
  RAVE_OBJECT_RELEASE(parameters);
  return result;
}


int PolarScan_shiftDataAndAttributes(PolarScan_t* self, int nrays)
{
  long newa1gate = 0;
  if (!PolarScan_shiftData(self, nrays)) {
    return 0;
  }

  if (!PolarScanInternal_shiftArrayIfExists(self, "how/elangles", nrays) ||
      !PolarScanInternal_shiftArrayIfExists(self, "how/startazA", nrays) ||
      !PolarScanInternal_shiftArrayIfExists(self, "how/stopazA", nrays) ||
      !PolarScanInternal_shiftArrayIfExists(self, "how/startazT", nrays) ||
      !PolarScanInternal_shiftArrayIfExists(self, "how/stopazT", nrays) ||
      !PolarScanInternal_shiftArrayIfExists(self, "how/startelA", nrays) ||
      !PolarScanInternal_shiftArrayIfExists(self, "how/stopelA", nrays) ||
      !PolarScanInternal_shiftArrayIfExists(self, "how/startelT", nrays) ||
      !PolarScanInternal_shiftArrayIfExists(self, "how/stopelT", nrays) ||
      !PolarScanInternal_shiftArrayIfExists(self, "how/TXpower", nrays)) {
    return 0;
  }

  if (self->hasAstart && PolarScan_hasAttribute(self, "how/startazA")) {
    double astart = 0.0;
    double* startazA = NULL;
    int nstartazA = 0;
    int tmpresult = 1;
    RaveAttribute_t* astartAttr = PolarScan_getAttribute(self, "how/astart");
    RaveAttribute_t* startazAttr = PolarScan_getAttribute(self, "how/startazA");
    if (astartAttr == NULL) {
      RAVE_ERROR0("Could not extract how/astart");
      tmpresult = 0;
    }
    if (startazAttr == NULL || !RaveAttribute_getDoubleArray(startazAttr, &startazA, &nstartazA)) {
      RAVE_ERROR0("Could not extract how/startazA");
      tmpresult = 0;
    }
    if (tmpresult) {
      double sa = startazA[0];
      if (sa > 180.0) {
        sa = sa - 360.0; /* Adjust if value is on other side of circle so that we get same sign as astart. */
      }
      astart = self->astart * 180.0/M_PI;
      if (fabs(astart - sa) > 0.0001) {
        self->astart = M_PI/180.0;
        RaveAttribute_setDouble(astartAttr, sa); /* But we want to use values between - raywidth / 2 */
      }
    }
    RAVE_OBJECT_RELEASE(astartAttr);
    RAVE_OBJECT_RELEASE(startazAttr);
    if (!tmpresult) {
      return 0;
    }
  } else if (self->hasAstart && self->nrays > 0) {
    double astart = self->astart * 180.0/M_PI;
    double azoffset = 360.0 / self->nrays;
    RaveAttribute_t* astartAttr = PolarScan_getAttribute(self, "how/astart");
    if (astartAttr == NULL) {
      RAVE_ERROR0("Could not extract how/astart");
    } else {
      astart = astart - azoffset * nrays;
      self->astart = astart * M_PI/180.0;
      RaveAttribute_setDouble(astartAttr, astart);
    }
    RAVE_OBJECT_RELEASE(astartAttr);
  }

  newa1gate=self->a1gate + nrays;
  if (newa1gate >= self->nrays) {
    newa1gate %= self->nrays;
  }
  while (newa1gate < 0 && self->nrays > 0) {
    newa1gate += self->nrays;
  }
  self->a1gate = newa1gate;

  return 1;
}

static int PolarScanInternal_containsString(const char* name, RaveList_t* strlist)
{
  int nstrs = 0, i = 0;
  nstrs = RaveList_size(strlist);
  for (i = 0; i < nstrs; i++) {
    char* str = (char*)RaveList_get(strlist, i);
    if (strcmp(name, str) == 0) {
      return 1;
    }
  }
  return 0;
}

int PolarScan_removeParametersExcept(PolarScan_t* scan, RaveList_t* parameters)
{
  int nCurrentParameters = 0, i = 0;
  int result = 0;

  RaveList_t* currentParameters = NULL;

  RAVE_ASSERT((scan != NULL), "scan == NULL");
  if (parameters == NULL) {
    goto done;
  }

  currentParameters = PolarScan_getParameterNames(scan);
  if (currentParameters == NULL) {
    goto done;
  }

  nCurrentParameters = RaveList_size(currentParameters);

  for (i = 0; i < nCurrentParameters; i++) {
    char* str = (char*)RaveList_get(currentParameters, i);
    if (!PolarScanInternal_containsString(str, parameters)) {
      PolarScanParam_t* param = PolarScan_removeParameter(scan, str);
      RAVE_OBJECT_RELEASE(param);
    }
  }
  result = 1;

done:
  if (currentParameters != NULL) {
    RaveList_freeAndDestroy(&currentParameters);
  }
  return result;
}

void PolarScanInternal_setPolarVolumeBeamwH(PolarScan_t* scan, double bwH)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  scan->beamwH = bwH;
  scan->bwpvolH = 1;
}

void PolarScanInternal_setPolarVolumeBeamwV(PolarScan_t* scan, double bwV)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  scan->beamwV = bwV;
  scan->bwpvolV = 1;
}

int PolarScanInternal_isPolarVolumeBeamwH(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return scan->bwpvolH;
}

int PolarScanInternal_isPolarVolumeBeamwV(PolarScan_t* scan)
{
  RAVE_ASSERT((scan != NULL), "scan == NULL");
  return scan->bwpvolV;
}

/*@} End of Interface functions */

RaveCoreObjectType PolarScan_TYPE = {
    "PolarScan",
    sizeof(PolarScan_t),
    PolarScan_constructor,
    PolarScan_destructor,
    PolarScan_copyconstructor
};
