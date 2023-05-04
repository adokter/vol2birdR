/* --------------------------------------------------------------------
Copyright (C) 2011 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Provides functionality for creating composites.
 * @file
 * @author Harri Hohti, FMI
 * @author Daniel Michelson, SMHI (Intgration)
 * @author Anders Henja, SMHI (Adaption to rave framework)
 * @date 2010-01-19
 */
#include "detection_range.h"
#include "raveobject_list.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <math.h>

#define ER 8495000.0  /**< effective earth radius for bin height calculations */

#define ERKM 8495     /**< effective earth radius in km */

#define CONSTGRAD 50.0 /**< 10 dB/km = 1000 m / 20 dBN */

/**
 * Represents the detection range generator.
 */
struct _DetectionRange_t {
  RAVE_OBJECT_HEAD /** Always on top */
  char* lookupPath; /**< where lookup files are located, default is /tmp */
  double analysis_minrange; /**< the min range to be processed in meters, default is 10000.0 */
  double analysis_maxrange; /**< the min range to be processed in meters, default is 240000.0 */
};

/*@{ Private functions */
/**
 * Constructor.
 * @param[in] obj - the created object
 */
static int DetectionRange_constructor(RaveCoreObject* obj)
{
  DetectionRange_t* this = (DetectionRange_t*)obj;
  this->lookupPath = NULL;
  if (!DetectionRange_setLookupPath(this, "/tmp")) {
    return 0;
  }
  this->analysis_minrange = 10000.0;
  this->analysis_maxrange = 240000.0;
  return 1;
}

/**
 * Copy constructor.
 * @param[in] obj - the created object
 * @param[in] srcobj - the source (that is copied)
 */
static int DetectionRange_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  DetectionRange_t* this = (DetectionRange_t*)obj;
  DetectionRange_t* src = (DetectionRange_t*)srcobj;
  this->lookupPath = NULL;
  if (!DetectionRange_setLookupPath(this, DetectionRange_getLookupPath(src))) {
    return 0;
  }
  this->analysis_minrange = src->analysis_minrange;
  this->analysis_maxrange = src->analysis_maxrange;
  return 1;
}

/**
 * Destructor
 * @param[in] obj - the object to destroy
 */
static void DetectionRange_destructor(RaveCoreObject* obj)
{
  DetectionRange_t* this = (DetectionRange_t*)obj;
  RAVE_FREE(this->lookupPath);
}

/**
 * Calculates the height of the bin, assuming a ideal sphere.
 * @param[in] m - the range near surface (in meters)
 * @param[in] e - the elevation angle
 * @param[in] h0 - the height above ground for the radar
 * @return the height in meter
 */
static int DetectionRangeInternal_binheight(double m,double e,double h0)
{
   double rh;
   int h;

   rh=m/cos(e);
   h=(int)(h0+(rh*sin(e)+0.5*rh*rh/ER));
   return(h);
}

/**
 * Calculate the range of the bin, assuming an ideal sphere.
 * @param[in] h - altitude in meters
 * @param[in] e - the elevation in radians
 * @param[in] h0 - altitude0 the radar altitude
 * @return the range in meters
 */
static double DetectionRangeInternal_bindist(double h,double e,double h0)
{
   double r;
   r = cos(e)*ER*(sqrt(sin(e)*sin(e)+2*(h-h0)/ER) - sin(e));
   return(r);
}

/**
 * Sort function to be used by qsort to get sorting in decending order
 * @param[in] i1 - one value (unsigned char*)
 * @param[in] i2 - another value (unsigned char*)
 * @return the result of i1 < i2
 */
static int DetectionRangeInternal_sortUcharDesc(const void *i1, const void *i2)
{
  unsigned char *ci1 = (unsigned char *) i1;
  unsigned char *ci2 = (unsigned char *) i2;
  return (*ci1 < *ci2);
}

/**
 * Sort function to be used by qsort to get sorting in decending order
 * @param[in] i1 - one value (double*)
 * @param[in] i2 - another value (double*)
 * @return the result of i1 < i2
 */
static int DetectionRangeInternal_sortDoubleDesc(const void *i1, const void *i2)
{
  double *ci1 = (double *) i1;
  double *ci2 = (double *) i2;
  return (*ci1 < *ci2);
}

/**
 * Locates the lowest elevation angle in the volume and returns it.
 * @param[in] pvol - the polar volume
 * @returns the lowest elevation angle in radians
 */
static double DetectionRangeInternal_getLowestElevationAngle(PolarVolume_t* pvol)
{
  int nscans = 0, i = 0;
  double result = 9999999.0;
  nscans = PolarVolume_getNumberOfScans(pvol);
  for (i = 0; i < nscans; i++) {
    PolarScan_t* scan = PolarVolume_getScan(pvol, i);
    if (scan != NULL) {
      if (PolarScan_getElangle(scan) < result) {
        result = PolarScan_getElangle(scan);
      }
    }
    RAVE_OBJECT_RELEASE(scan);
  }
  return result;
}

/**
 * Sector weight factors generation. Linear weights with highest value
 * at center of the sector normalized to 1 and sector edges to 0.
 * @param[in] weightsector - width of weighting sector
 * @param[in] maxweight - maximum unnormalized sector weight
 * @param[in] inW - weight sector width input value
 * @param[out] Wsecsum - sum of sector weight factors
 * @return an array of sector weight factors with length = weightsector
 */
static double* DetectionRangeInternal_createSectorWeightFactors(
  int weightsector, double maxweight, int inW, double* Wsecsum)
{
  int i = 0;
  double wg = 0.0;
  double* weightarr = NULL;
  double sum = 0.0;

  RAVE_ASSERT((Wsecsum != NULL), "Wsecsum == NULL");

  weightarr = RAVE_MALLOC(sizeof(double)*weightsector);
  if (weightarr == NULL) {
    RAVE_CRITICAL0("Failed to allocate weight array");
    return NULL;
  }
  memset(weightarr, 0, sizeof(double)*weightsector);

  for(i = 0; i < weightsector; i++) {
    wg = (double)((inW+1)-abs(inW-i));
    weightarr[i] = wg/maxweight;
    sum += weightarr[i];
  }
  *Wsecsum = sum;
  return weightarr;
}

/**
 * Creates the previous top file name.
 * @param[in] self - self
 * @param[in] source - the source name (MAY NOT BE NULL))
 * @param[in,out] name - the generated file name
 * @param[in] len - the allocated length of name
 * @return 1 on success or 0 if something erroneous occured
 */
static int DetectionRangeInternal_createPreviousTopFilename(
  DetectionRange_t* self, const char* source, char* name, int len)
{
  int slen = 0;
  int result = 0;

  RAVE_ASSERT((self != NULL), "self == NULL");
  if (source == NULL) {
    RAVE_ERROR0("Providing a NULL value for source");
    goto done;
  }
  slen = strlen(source);

  if ((strlen(DetectionRange_getLookupPath(self)) + slen + 13) > len) {
    RAVE_WARNING0("Not enough memory allocated for top file name");
    goto done;
  }
  snprintf(name, len, "%s/%s_oldtop.txt", DetectionRange_getLookupPath(self), source);

  result = 1;
done:
  return result;
}

/**
 * Reads the previous background top value from the cached file. If no source
 * has been specified or if the lookup file does not exist, the climatological value
 * (5.5) will be returned.
 */
static double DetectionRangeInternal_readPreviousBackgroundTop(DetectionRange_t* self, const char* source)
{
  FILE* fp = NULL;
  char filename[1024];
  double TOPrev = 5.5;

  RAVE_ASSERT((self != NULL), "self == NULL");

  if (!DetectionRangeInternal_createPreviousTopFilename(self, source, filename, 1024)) {
    goto done;
  }
  fp = fopen(filename, "r");
  if (fp == NULL) {
    RAVE_INFO1("Could not locate lookup file %s, defaulting to TOPrev = 5.5", filename);
    goto done;
  } else {
    if (fscanf(fp,"%lf",&TOPrev)<=0) {
      RAVE_INFO0("Could not read previous top value");
      goto done;
    }
  }

done:
  if (fp != NULL) {
    fclose(fp);
  }
  return TOPrev;
}

/**
 * Writes the background top to the cache file.
 * @param[in] self - self
 * @param[in] source - the source for this value
 * @param[in] value - the value to be written
 * @return 1 on success otherwise 0
 */
static int DetectionRangeInternal_writeBackgroundTop(DetectionRange_t* self, const char* source, double value)
{
  FILE* fp = NULL;
  char filename[1024];
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");

  if (!DetectionRangeInternal_createPreviousTopFilename(self, source, filename, 1024)) {
    goto done;
  }
  fp = fopen(filename, "w");
  if (fp == NULL) {
    RAVE_ERROR1("Failed to open %s for writing", filename);
    goto done;
  } else {
    if (fprintf(fp,"%.1f\n",value)<0) {
      RAVE_ERROR1("Failed to write background top %.1f to file", value);
      goto done;
    }
  }
  result = 1;
done:
  if (fp != NULL) {
    fclose(fp);
  }
  return result;
}

/**
 * Returns the previous top files creation time.
 * @param[in] self - self
 * @param[in] source - the source of this radar
 * @return the time of the file (or current time if file not could be found)
 */
static time_t DetectionRangeInternal_getPreviousTopFiletime(DetectionRange_t* self, const char* source)
{
  time_t result;
  char filename[1024];
  struct stat filestat;

  time(&result); // Initialize time to now if any file operation fails.

  if (!DetectionRangeInternal_createPreviousTopFilename(self, source, filename, 1024)) {
    goto done;
  }
  if (stat(filename,&filestat) != 0) {
    goto done;
  }
  result = filestat.st_mtime;

done:
  return result;
}

/**
 * Backround TOP values generation based on previous TOP and its age.
 * TOP backround value converges from value of previous valid TOP to
 * climatological guess of 5.5 km during 48 hour ageing period beginning
 * if previous TOP is older than 2 hours
 * @param[in] self - self
 * @param[in] source - the source
 * @param[in] TOPprev
 * @returns the aged TOPprev
 */
static double DetectionRangeInternal_generateAgedTop(DetectionRange_t* self, const char* source, double TOPprev)
{
  time_t prevtoptime, curtime;
  double prevtop_age=0.0, max_prevtop_age=48.0, rel_age=0.0, TOPdiff=0.0, newTOPprev=0.0;

  time(&curtime);
  prevtoptime = DetectionRangeInternal_getPreviousTopFiletime(self, source);

  /* ageing of previous TOP begins if it's older than two hours */
  prevtop_age=(double)(curtime - prevtoptime)/3600.0-2.25;
  if(prevtop_age > 0.0) {
    rel_age = prevtop_age / max_prevtop_age;
    if(rel_age>1.0) {
      rel_age=1.0;
    }
    TOPdiff=TOPprev-5.5;
    newTOPprev=TOPprev-rel_age*TOPdiff;
    TOPprev=newTOPprev;
  }
  return TOPprev;
}

/**
 * Calculates the startbin index and the bin count from the provided rscale
 * and nbins when taking analysis_minrange and analysis_maxrange into account.
 * @param[in] self - self
 * @param[in] rscale - the scale of the bins
 * @param[in] nbins - the maximum number of bins
 * @param[out] startbin - the start position
 * @param[out] bincount - the number of bins we should work on from startbin
 */
static void DetectionRangeInternal_getStartBinAndCount(
  DetectionRange_t* self, double rscale, long nbins, int* startbin, int* bincount)
{
  double maxrange = rscale * (double)nbins;
  double minrange = 0.0;
  RAVE_ASSERT((startbin != NULL), "startbin == NULL");
  RAVE_ASSERT((bincount != NULL), "bincount == NULL");

  /* We don't want to go outside max and min range boundaries */
  if (self->analysis_maxrange < maxrange) {
    maxrange = self->analysis_maxrange;
  }
  if (self->analysis_minrange > minrange) {
    minrange = self->analysis_minrange;
  }

  if (rscale == 0.0) {
    *startbin = 0;
    *bincount = nbins;
  } else {
    int sbin = (int)(minrange/rscale);
    int binc = (int)((maxrange - minrange)/rscale);
    if (binc > nbins - sbin) {
      binc = nbins-sbin;
    }
    *startbin = sbin;
    *bincount = binc;
  }
}

/**
 * Sorts top values and selects representative TOPs per ray.
 * @param[in] self - self
 * @param[in] param - the top field we are working with
 * @param[in] startbin - the bin to start with
 * @param[in] bincount - the number of bins to work with
 * @param[in] sortage - the sortage in range 0 - 1.0 any other value will result in undefined behaviour
 * @param[in] samplepoint - define the position to pick a representative TOP value from highest
 *                          valid TOPs, typically near 0.5 (median) lower values (nearer to
 *                          highest TOP, 0.15) used in noisier radars like KOR.
 * @param[out] oray_pickhightop - TOP picked from highest TOPs defined by sortage and samplepoint
 * @param[out] osorttop -
 * @param[out] orayweight -
 */
static int DetectionRangeInternal_sortAndSelectRepresenativeTops(
  DetectionRange_t* self, PolarScanParam_t* param, int startbin, int bincount, double sortage, double samplepoint,
  double** oray_pickhightop, double** osorttop, double** orayweight) {

  long* topcount = NULL;
  double* ray_pickhightop = NULL;
  double* sorttop = NULL;
  unsigned char *ray_sortbuf = NULL;  /* buffer of ray TOPs to be sorted by qsort                  */
  double* rayweight = NULL;

  int itop = 0;                       /* integer top value ((int)(TOP[m]/100+1)), 0 if no TOP      */
  int sortpart_ray = 0;
  int result = 0;
  int nrays = 0;
  int rayi = 0;

  RAVE_ASSERT((self != NULL), "self == NULL");
  RAVE_ASSERT((param != NULL), "param == NULL");
  RAVE_ASSERT((oray_pickhightop != NULL), "oray_pickhightop == NULL");
  RAVE_ASSERT((osorttop != NULL), "osorttop == NULL");
  RAVE_ASSERT((orayweight != NULL), "orayweight == NULL");
  nrays = PolarScanParam_getNrays(param);

  ray_sortbuf = RAVE_MALLOC(sizeof(unsigned char) * bincount);
  topcount = RAVE_MALLOC(sizeof(long) * nrays);
  ray_pickhightop = RAVE_MALLOC(sizeof(double) * nrays);
  sorttop = RAVE_MALLOC(sizeof(double) * nrays);
  rayweight = RAVE_MALLOC(sizeof(double) * nrays);

  if (ray_sortbuf == NULL || topcount == NULL || ray_pickhightop == NULL || sorttop == NULL || rayweight == NULL) {
    RAVE_CRITICAL0("Failed to allocate memory");
    goto done;
  }

  for (rayi = 0; rayi < nrays; rayi++) {
    int i = 0, rsi = 0;
    double v = 0.0;
    topcount[rayi]=0;
    ray_pickhightop[rayi]=-1.0;
    sorttop[rayi]=0.0;

    for (i = startbin, rsi = 0; i < bincount; i++, rsi++) {
      PolarScanParam_getValue(param, i, rayi, &v);
      ray_sortbuf[rsi] = (unsigned char)v;
      if (ray_sortbuf[rsi] == 254) {
        ray_sortbuf[rsi] = 0;
      }
    }

    /* Sorting ray TOP values in _descending_ order */
    qsort(ray_sortbuf, bincount, sizeof(unsigned char), DetectionRangeInternal_sortUcharDesc);

    /* sortage [0-1] part of sorted ray taken to further analysis */
    sortpart_ray = (int)((double)bincount * sortage);

    for (i = 0; i < sortpart_ray; i++) {
      itop = ray_sortbuf[i];
      if (itop) {
        topcount[rayi]++;
      }
    }

    /*
     * TOP from samplepoint [0-1] relative position of sorted, valid TOPs
     * is taken as representative high TOP. 0 points to highest value, 0.5 median
     */
    if (topcount[rayi]) {
      int picbin = (int)((double)topcount[rayi] * samplepoint);
      ray_pickhightop[rayi] = sorttop[rayi] = (ray_sortbuf[picbin] - 1)/10.0;
    }

    /* weight of this ray depends on how many TOPs are existing in sort part of ray */
    rayweight[rayi] = (double)topcount[rayi]/(double)sortpart_ray;
  }

  *oray_pickhightop = ray_pickhightop;
  *osorttop = sorttop;
  *orayweight = rayweight;
  ray_pickhightop = NULL; // Not responsible for memory any longer
  sorttop = NULL;         // Not responsible for memory any longer
  rayweight = NULL;       // Not responsible for memory any longer
  result = 1;
done:
  RAVE_FREE(ray_sortbuf);
  RAVE_FREE(topcount)
  RAVE_FREE(ray_pickhightop)
  RAVE_FREE(sorttop);
  RAVE_FREE(rayweight);
  return result;
}

/**
 * Counts the number of valid raytop weights
 * @param[in] rayweights - the ray weight array (must be != NULL)
 * @param[in] nrays - the number of rays
 * @return the number of valid weights
 */
static long DetectionRangeInternal_getValidRaytopCount(double* rayweights, long nrays)
{
  long i = 0;
  long result = 0;
  RAVE_ASSERT((rayweights != NULL), "rayweights == NULL");
  for (i = 0; i < nrays; i++) {
    if (rayweights[i] > 0.99) {
      result++;
    }
  }
  return result;
}

/**
 * Creates a detection range scan with the height scan as a template.
 * @param[in] hghtScan - the height scan
 * @returns a detection range scan on success otherwise NULL
 */
static PolarScan_t* DetectionRangeInternal_createDetectionRangeScan(PolarScan_t* hghtScan)
{
  PolarScan_t* result = NULL;
  PolarScan_t* scanClone = NULL;
  PolarScanParam_t* hghtScanParam = NULL;
  PolarScanParam_t* paramClone = NULL;
  int rayi = 0, bini = 0;
  long nrays = 0, nbins = 0;

  RAVE_ASSERT((hghtScan != NULL), "hghtScan == NULL");

  hghtScanParam = PolarScan_getParameter(hghtScan, "HGHT");
  if (hghtScanParam == NULL) {
    RAVE_ERROR0("Provided scan does not contain a HGHT parameter");
    goto done;
  }
  paramClone = RAVE_OBJECT_CLONE(hghtScanParam);
  scanClone = RAVE_OBJECT_CLONE(hghtScan);
  if (scanClone == NULL || paramClone == NULL) {
    RAVE_ERROR0("Failed to clone scan or parameter");
    goto done;
  }
  PolarScan_removeAllParameters(scanClone);
  PolarScanParam_setQuantity(paramClone, "DR");
  PolarScanParam_setUndetect(paramClone, 255.0);
  PolarScanParam_setNodata(paramClone, 255.0);
  nrays = PolarScanParam_getNrays(paramClone);
  nbins = PolarScanParam_getNbins(paramClone);
  for (rayi = 0; rayi < nrays; rayi++) {
    for (bini = 0; bini < nbins; bini++) {
      PolarScanParam_setValue(paramClone, bini, rayi, 0.0);
    }
  }
  if (!PolarScan_addParameter(scanClone, paramClone)) {
    RAVE_ERROR0("Failed to add parameter to scan");
    goto done;
  }
  if (!PolarScan_setDefaultParameter(scanClone, "DR")) {
    RAVE_ERROR0("Failed to set default parameter to DR");
    goto done;
  }

  result = RAVE_OBJECT_COPY(scanClone);
done:
  RAVE_OBJECT_RELEASE(scanClone);
  RAVE_OBJECT_RELEASE(hghtScanParam);
  RAVE_OBJECT_RELEASE(paramClone);
  return result;
}
/*@} End of Private functions */

/*@{ Interface functions */
int DetectionRange_setLookupPath(DetectionRange_t* self, const char* path)
{
  int result = 0;
  char* tmp = NULL;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (path == NULL) {
    return 0;
  }
  tmp = RAVE_STRDUP(path);
  if (tmp != NULL) {
    RAVE_FREE(self->lookupPath);
    self->lookupPath = tmp;
    result = 1;
  }
  return result;
}

const char* DetectionRange_getLookupPath(DetectionRange_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return (const char*)self->lookupPath;
}

void DetectionRange_setAnalysisMinRange(DetectionRange_t* self, double minrange)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->analysis_minrange = minrange;
}

double DetectionRange_getAnalysisMinRange(DetectionRange_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->analysis_minrange;
}

void DetectionRange_setAnalysisMaxRange(DetectionRange_t* self, double maxrange)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->analysis_maxrange = maxrange;
}

double DetectionRange_getAnalysisMaxRange(DetectionRange_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->analysis_maxrange;
}

PolarScan_t* DetectionRange_top(DetectionRange_t* self, PolarVolume_t* pvol, double scale, double threshold_dBZN, char* paramname)
{
  PolarScan_t* maxdistancescan = NULL;
  PolarScan_t* result = NULL;
  PolarScan_t* retval = NULL;
  PolarScanParam_t* param = NULL;
  int nrscans = 0;
  double scaleFactor = 0.0;
  long nbins = 0, nrays = 0;
  int rayi = 0, bini = 0, elevi = 0;

  RAVE_ASSERT((self != NULL), "self == NULL");

  if (pvol == NULL) {
    RAVE_ERROR0("Can not determine top from a volume that is NULL");
    goto error;
  } else if (scale <= 0.0) {
    RAVE_ERROR0("Scale must be > 0.0");
    goto error;
  }

  PolarVolume_sortByElevations(pvol, 0); // Descending
  nrscans = PolarVolume_getNumberOfScans(pvol);
  maxdistancescan = PolarVolume_getScanWithMaxDistance(pvol);
  if (maxdistancescan == NULL) {
    goto error;
  }
  result = RAVE_OBJECT_CLONE(maxdistancescan);
  if (result == NULL) {
    RAVE_ERROR0("Failed to clone max distance scan");
    goto error;
  }
  PolarScan_removeAllParameters(result);

  // Calculate bins from scale
  scaleFactor = PolarScan_getRscale(maxdistancescan) / scale;
  nbins = (long)(scaleFactor * (double)PolarScan_getNbins(maxdistancescan));

  param = RAVE_OBJECT_NEW(&PolarScanParam_TYPE);
  if (param == NULL || !PolarScanParam_createData(param, nbins, PolarScan_getNrays(maxdistancescan), RaveDataType_UCHAR)) {
    RAVE_ERROR0("Failed to create polar scan parameter for echo top");
    goto error;
  }

  PolarScanParam_setQuantity(param, "HGHT");
  PolarScanParam_setGain(param, (double)0.1);  /* Email from Harri 2010-09-30 */
  PolarScanParam_setOffset(param, (double)-0.1);
  PolarScanParam_setNodata(param, (double)255.0);
  PolarScanParam_setUndetect(param, (double)0.0);
  PolarScan_addParameter(result, param);
  PolarScan_setElangle(result, DetectionRangeInternal_getLowestElevationAngle(pvol));
  PolarScan_setBeamwidth(result, PolarScan_getBeamwidth(maxdistancescan));
  PolarScan_setDefaultParameter(result, "HGHT");
  PolarScan_setRscale(result, scale);
  nrays = PolarScan_getNrays(result);

  for(rayi = 0; rayi < nrays; rayi++) {
    for (bini = 0; bini < nbins; bini++) {
      int topfound = 0;
      int overMaxelev = 0;
      int highest_ei = 0;
      double range = PolarScan_getRange(result, bini, 0);
      int bi = 0, lower_bi = 0;
      double toph = 0.0;
      int found = 0; /* Used to break elevation loop when value has been found */

      for (elevi = 0; !found && elevi < (nrscans - 1); elevi++) {
        PolarScan_t* scan = PolarVolume_getScan(pvol, elevi);
        PolarScan_t* lowscan = PolarVolume_getScan(pvol, elevi+1);
        double elangle = 0.0, lower_elangle = 0.0, height = 0.0, lower_height = 0.0;
        double binh = 0.0, lower_binh = 0.0, Dh = 0.0, dBZN = 0.0, lower_dBZN = 0.0;
        RaveValueType dBZN_type = RaveValueType_UNDEFINED, lower_dBZN_type = RaveValueType_UNDEFINED;
        PolarScan_setDefaultParameter(scan, paramname);
        PolarScan_setDefaultParameter(lowscan, paramname);

        bi = PolarScan_getRangeIndex(scan, range, PolarScanSelectionMethod_FLOOR, 0);
        lower_bi = PolarScan_getRangeIndex(lowscan, range, PolarScanSelectionMethod_FLOOR, 0);

        elangle = PolarScan_getElangle(scan);
        lower_elangle = PolarScan_getElangle(lowscan);

        height = PolarScan_getHeight(scan);
        lower_height = PolarScan_getHeight(lowscan);

        if (bi < 0) {
          highest_ei = elevi + 1;
        } else {
          binh = DetectionRangeInternal_binheight(range, elangle, height);
          lower_binh = DetectionRangeInternal_binheight(range, lower_elangle, lower_height);
          Dh=(double)(binh-lower_binh);

          dBZN_type = PolarScan_getConvertedValue(scan, bi, rayi, &dBZN);
          lower_dBZN_type = PolarScan_getConvertedValue(lowscan, lower_bi, rayi, &lower_dBZN);

          if (dBZN_type == RaveValueType_DATA || lower_dBZN_type == RaveValueType_DATA) {
            if (!found && dBZN > threshold_dBZN && elevi == highest_ei)
            {
              if (lower_dBZN)
              {
                overMaxelev = 1;
              }
              found = 1;
            }
            if (!found && dBZN == threshold_dBZN)
            {
              if(lower_dBZN)
              {
                topfound = 1;
                toph=(double)binh;
              }
              found = 1;
            }
            if (!found && lower_dBZN == threshold_dBZN)
            {
              topfound = 1;
              toph = (double)lower_binh;
              found = 1;
            }
            if (!found && lower_dBZN > threshold_dBZN)
            {
              topfound=1;
              if(!dBZN) {
                toph=lower_binh+(double)(lower_dBZN - threshold_dBZN)*CONSTGRAD;
              } else {
                toph=lower_binh+(double)(lower_dBZN - threshold_dBZN) * Dh/(double)(lower_dBZN - dBZN);
              }
              found = 1;
            }
          }
        }
        RAVE_OBJECT_RELEASE(scan);
        RAVE_OBJECT_RELEASE(lowscan);
      }

      if(overMaxelev) {
        PolarScan_setValue(result, bini, rayi, 254.0);
      } else if (topfound) {
        PolarScan_setValue(result, bini, rayi, toph/100.0 + 1.0);
      }
    }
  }

  retval = RAVE_OBJECT_COPY(result);
error:
  RAVE_OBJECT_RELEASE(maxdistancescan);
  RAVE_OBJECT_RELEASE(param);
  RAVE_OBJECT_RELEASE(result);
  return retval;
}

PolarScan_t* DetectionRange_filter(DetectionRange_t* self, PolarScan_t* scan)
{
  PolarScan_t* result = NULL;
  PolarScan_t* clone = NULL;
  PolarScanParam_t* param = NULL;

  int bi = 0, ri = 0;
  int nbins = 0, nrays = 0;

  RAVE_ASSERT((self != NULL), "self == NULL");

  if (scan == NULL) {
    RAVE_ERROR0("No point to filter a NULL field\n");
    goto done;
  }

  clone = RAVE_OBJECT_CLONE(scan);
  if (clone == NULL) {
    RAVE_CRITICAL0("Failed to clone scan");
    goto done;
  }
  nbins = PolarScan_getNbins(clone);
  nrays = PolarScan_getNrays(clone);

  if (!PolarScan_setDefaultParameter(clone, "HGHT")) {
    RAVE_ERROR0("Could not set default parameter to HGHT");
    goto done;
  }

  param = PolarScan_getParameter(scan, "HGHT");
  if (param == NULL) {
    RAVE_ERROR0("No HGHT parameter in scan");
    goto done;
  }

  for (bi = 0; bi < nbins; bi++) {
    for (ri = 0; ri < nrays; ri++) {
      double dob = 0.0, dnb = 0.0, db = 0.0;
      unsigned char ob = 0, nb = 0, b = 0;

      if (ri == 0) {
        PolarScanParam_getValue(param, bi, nrays-1, &dob);
      } else {
        PolarScanParam_getValue(param, bi, ri-1, &dob);
      }
      if (ri == nrays-1) {
        PolarScanParam_getValue(param, bi, 0, &dnb);
      } else {
        PolarScanParam_getValue(param, bi, ri+1, &dnb);
      }
      PolarScanParam_getValue(param, bi, ri, &db);

      ob = (unsigned char)dob;
      nb = (unsigned char)dnb;
      b = (unsigned char)db;

      if((!(ob | nb) && b) || (b > (ob+nb))) {
        PolarScan_setValue(clone, bi, ri, 0.0);
      }
    }
  }

  result = RAVE_OBJECT_COPY(clone);

done:
  RAVE_OBJECT_RELEASE(clone);
  RAVE_OBJECT_RELEASE(param);
  return result;
}

/**
 * @param[in] avgsector - width of the floating average azimuthal sector
 * @param[in] sortage - defining the higher portion of sorted ray to be analysed, typically 0.05 - 0.2
 * @param[in] samplepoint - define the position to pick a representative TOP value from highest
 *                          valid TOPs, typically near 0.5 (median) lower values (nearer to
 *                          highest TOP, 0.15) used in noisier radars like KOR.
 */
RaveField_t* DetectionRange_analyze(DetectionRange_t* self,
  PolarScan_t* scan, int avgsector, double sortage, double samplepoint)
{
  int weightsector = 0;         /* width of weighting sector [deg]                           */
  int inW = 0;                  /* weight sector width input value,  weightsector=inW*2+1    */
  int StartBin=0, BinCount=0;   /* starting bin and bin count of top "ray" analysis,         */
  int valid_raytop_count = 0;   /* count of "valid TOP rays" e.g. rays having
                                   top_count==sortpart_ray (=ray weight is > 0.99)           */
  int A = 0, B = 0;             /* azimuth and bin (range gate) indices                      */
  int wI = 0;                   /* index of weight factors array                             */
  int limitNrays;               /* Limit on the number of nrays necessary before writing previous top, 10% */
  double half_bw = 0.0;         /* half beam width [rad]                                     */
  double maxweight = 0.0;       /* maximum unnormalized sector weight                        */
  double lowest_elev = 0.0;     /* lowest elevation in radians */
  double Wsecsum = 0.0;         /* sum of sector weight factors                              */
  double TOPprev = 0.0;         /* background TOP value (based on previous TOP)              */
  double* weightarr = NULL;     /* array of sector weight factors, dimension=weightsector    */
  double* ray_pickhightop=NULL; /* TOP picked from highest TOPs defined by sortage and samplepoint */
  double* sorttop = NULL;       /* */
  double* rayweight = NULL;     /* weight ray depends on how many TOPs are existing in sort part of ray */
  double* Final_WTOP = NULL;    /* Final ray-, sector- and background weighted TOP for rays  */
  long nrays = 0, nbins = 0;

  PolarScanParam_t* param = NULL;
  RaveField_t* result = NULL;   /* The resulting field, will only be set on success */
  PolarScan_t* outscan = NULL;  /* The working scan where data will be set and on success copied to result */

  RAVE_ASSERT((self != NULL), "self == NULL");
  if (scan == NULL) {
    RAVE_ERROR0("Trying to analyse a NULL field");
    return NULL;
  }
  if (sortage < 0.0 || sortage > 1.0) {
    RAVE_ERROR0("sortage should be in range 0 - 1");
    return NULL;
  }
  param = PolarScan_getParameter(scan, "HGHT");
  if (param == NULL) {
    RAVE_ERROR0("Providing a scan without a HGHT field?");
    goto done;
  }

  outscan = DetectionRangeInternal_createDetectionRangeScan(scan);
  if (outscan == NULL) {
    RAVE_ERROR0("Failed to create a detection range scan");
    goto done;
  }
  nrays = PolarScanParam_getNrays(param);
  nbins = PolarScanParam_getNbins(param);

  limitNrays = (int)((double)nrays * 0.1);

  half_bw = PolarScan_getBeamwidth(scan) / 2.0;
  inW = avgsector / 2;
  weightsector = inW*2 + 1;
  maxweight=(double)inW+1.0;

  lowest_elev = PolarScan_getElangle(scan); // Original code uses 0.3 here and then later on the real lowest elangle !?

  Final_WTOP = RAVE_MALLOC(sizeof(double) * nrays);

  /* Sector weight factors generation. Linear weights with highest value
   * at center of the sector normalized to 1 and sector edges to 0.
   */
  weightarr = DetectionRangeInternal_createSectorWeightFactors(weightsector, maxweight, inW, &Wsecsum);

  if (Final_WTOP == NULL || weightarr == NULL) {
    RAVE_ERROR0("Failed to allocate memory for Final_WTOP or weightarr");
    goto done;
  }
  /** Calculate previous top value */
  TOPprev = DetectionRangeInternal_readPreviousBackgroundTop(self, PolarScan_getSource(scan));
  TOPprev = DetectionRangeInternal_generateAgedTop(self, PolarScan_getSource(scan), TOPprev);

  /* Sorting of TOP values and selection of representative TOPs per ray */

  /* determine StartBin and BinCount within the min/max range */
  DetectionRangeInternal_getStartBinAndCount(self, PolarScan_getRscale(scan), PolarScan_getNbins(scan), &StartBin, &BinCount);

  if (!DetectionRangeInternal_sortAndSelectRepresenativeTops(self, param, StartBin, BinCount, sortage, samplepoint,
                                                             &ray_pickhightop, &sorttop, &rayweight)) {
    RAVE_ERROR0("Failed to determine representative tops");
    goto done;
  }

  valid_raytop_count = DetectionRangeInternal_getValidRaytopCount(rayweight, PolarScanParam_getNrays(param));

  /* Finally it is time to determine the DR-field */

  /* -------------------------------------------------------------------------------*/
  /* Azimuthal sector weighting and statistical analysis of selected ray TOP values */
  for(A = 0; A < nrays; A++) {
    int bA, eA,                /* beginning and end values of azimuthal weighting sector
                                   [deg] are +- weightsector/2                               */
        cA, rA;                /* center-relative and real azimuth inside weighting sector  */

    double  k;            /* coefficient used to convert uncertainty of detection to byte 0-255 */
    double Secsum=0.0;    /* sum of weighted TOPs over sector         */
    double maxR_analyzed_highbeam = 250.0; /* final analysed maximum range of upper edge of ray  */
    double maxR_analyzed_lowbeam = 250.0;  /* final analysed maximum range of lower edge of ray  */

    bA = A - weightsector / 2;
    eA = A + weightsector / 2;

    wI=0;
    for(cA = bA; cA < eA; cA++) {
      double Wray,      /* ray weight from sorted ray TOPs    */
             Wsec,      /* sector weight of this ray          */
             highTOP,   /* picked high TOP value of this ray  */
             Rayval;    /* ray- and background weighted TOP per ray */

      rA=cA;
      if (cA < 0) {
        rA = cA + nrays;
      }
      if (cA >= nrays) {
        rA = cA - nrays;
      }

      Wray = rayweight[rA];
      highTOP = ray_pickhightop[rA];
      Wsec = weightarr[wI];

      Rayval = Wray*highTOP + (1.0-Wray)*TOPprev;
      Secsum += (Wsec * Rayval);
      wI++;
    }
    Final_WTOP[A] = Secsum / Wsecsum;

    maxR_analyzed_lowbeam = DetectionRangeInternal_bindist(Final_WTOP[A]*1000.0, lowest_elev - half_bw, 0.0) / 1000.0;
    maxR_analyzed_highbeam = DetectionRangeInternal_bindist(Final_WTOP[A]*1000.0, lowest_elev + half_bw, 0.0) / 1000.0;


    /* detection range values scaled to 0-255 and range to 500 m resolution for PGM output */
    k=255.0 / (maxR_analyzed_lowbeam - maxR_analyzed_highbeam);

    for(B = 0; B < nbins; B++) {
      int outbyte = 0;
      double rkm = (double)B * PolarScan_getRscale(scan) / 1000.0;
      if(rkm > maxR_analyzed_highbeam) {
        outbyte = (int)((rkm - maxR_analyzed_highbeam)*k);
      }
      if(outbyte > 255) {
        outbyte=255;
      }
      PolarScan_setValue(outscan, B, A, (double)outbyte);
    }
  }

  /*---------------------------------------------------------------------------------------*/
  /* Median of highest 10% of selected rayTOPS is proposed as new backround TOP.           */
  /* This new previous TOP is written if more than 10% of the rays are selected as valid   */
  if (valid_raytop_count > limitNrays) {
    int raysortcount = 0;
    qsort(sorttop, nrays, sizeof(double), DetectionRangeInternal_sortDoubleDesc);
    for (A = 0; A < nrays; A++) {
      if (sorttop[A] > 0) {
        raysortcount++;
      }
      if (raysortcount >= limitNrays) {
        break;
      }
    }
    if (raysortcount > 0) {
      TOPprev = sorttop[raysortcount/2];
    }

    // We don't need to fail here, just write an error and continue.
    if (!DetectionRangeInternal_writeBackgroundTop(self, PolarScan_getSource(scan), TOPprev)) {
      RAVE_ERROR0("Failed to write background top");
    }
  }

  RAVE_OBJECT_RELEASE(param);
  // Finally, convert the scan parameter into a quality field for detection range.
  param = PolarScan_getParameter(outscan, "DR");
  if (param != NULL) {
    RaveAttribute_t* attr = NULL;

    /* Set gain and offset before converting it into a quality field */
    PolarScanParam_setOffset(param, 1.0);
    PolarScanParam_setGain(param, -1.0/255.0);

    result = PolarScanParam_toField(param);
    if (result == NULL) {
      RAVE_ERROR0("Failed to convert parameter to field");
    }

    attr = RaveAttributeHelp_createString("how/task", "se.smhi.detector.poo");
    if (attr == NULL || !RaveField_addAttribute(result, attr)) {
      RAVE_ERROR0("Failed to add how/task to detection field");
      RAVE_OBJECT_RELEASE(result);
    }
    RAVE_OBJECT_RELEASE(attr);
  }

done:
  RAVE_OBJECT_RELEASE(outscan);
  RAVE_OBJECT_RELEASE(param);
  RAVE_FREE(weightarr);
  RAVE_FREE(ray_pickhightop);
  RAVE_FREE(sorttop);
  RAVE_FREE(rayweight);
  RAVE_FREE(Final_WTOP);

  return result;
}

/*@} End of Interface functions */

RaveCoreObjectType DetectionRange_TYPE = {
    "DetectionRange",
    sizeof(DetectionRange_t),
    DetectionRange_constructor,
    DetectionRange_destructor,
    DetectionRange_copyconstructor
};

