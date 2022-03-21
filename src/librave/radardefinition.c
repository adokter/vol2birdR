/* --------------------------------------------------------------------
Copyright (C) 2009-10 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Defines a radar definition
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-08-31
 */
#include "radardefinition.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include <string.h>

/**
 * Represents the area
 */
struct _RadarDefinition_t {
  RAVE_OBJECT_HEAD /** Always on top */

  char* id;        /**< the id */
  char* description; /**< the description */
  double lon;        /**< the longitude (in radians)*/
  double lat;        /**< the latitude (in radians)*/
  double height;     /**< the height above sea level in meters */
  unsigned int nelangles;     /**< the size of the elangels array */
  double* elangles;  /**< the elevation angles (in radians) */
  long nrays;        /**< number of rays */
  long nbins;        /**< number of bins */
  double scale;      /**< the size of a bin */
  double beamwH;     /**< the horizontal beam width */
  double beamwV;     /**< the vertical beam width */
  double wavelength; /**< the wave length */
  Projection_t* projection; /**< the projection that is used for this radar, usually just a plain lonlat projection */
};

/*@{ Private functions */
/**
 * Constructor.
 */
static int RadarDefinition_constructor(RaveCoreObject* obj)
{
  RadarDefinition_t* this = (RadarDefinition_t*)obj;
  this->id = NULL;
  this->description = NULL;
  this->lon = 0.0;
  this->lat = 0.0;
  this->height = 0.0;
  this->nelangles = 0;
  this->elangles = NULL;
  this->nbins = 0;
  this->nrays = 0;
  this->scale = 0.0;
  this->beamwH = 0.0;
  this->beamwV = 0.0;
  this->wavelength = 0.0;
  this->projection = NULL;
  return 1;
}

/**
 * Copy constructor
 */
static int RadarDefinition_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  RadarDefinition_t* this = (RadarDefinition_t*)obj;
  RadarDefinition_t* src = (RadarDefinition_t*)srcobj;

  RadarDefinition_constructor(obj); // First just initialize everything like the constructor

  if (!RadarDefinition_setID(this, src->id)) {
    goto error;
  }
  if (!RadarDefinition_setDescription(this, src->description)) {
    goto error;
  }
  if (!RadarDefinition_setElangles(this, src->nelangles, src->elangles)) {
    goto error;
  }

  this->lon = src->lon;
  this->lat = src->lat;
  this->height = src->height;
  this->nbins = src->nbins;
  this->nrays = src->nrays;
  this->scale = src->scale;
  this->beamwH = src->beamwH;
  this->beamwV = src->beamwV;
  this->wavelength = src->wavelength;
  this->projection = RAVE_OBJECT_CLONE(src->projection);
  if (this->projection == NULL) {
    goto error;
  }
  return 1;
error:
  RAVE_OBJECT_RELEASE(this->projection);
  RAVE_FREE(this->id);
  RAVE_FREE(this->description);
  RAVE_FREE(this->elangles);
  return 0;
}

/**
 * Destroys the radar definition
 * @param[in] obj - the the RadarDefinition_t instance
 */
static void RadarDefinition_destructor(RaveCoreObject* obj)
{
  RadarDefinition_t* radar = (RadarDefinition_t*)obj;
  if (radar != NULL) {
    RAVE_FREE(radar->id);
    RAVE_FREE(radar->description);
    RAVE_FREE(radar->elangles);
    RAVE_OBJECT_RELEASE(radar->projection);
  }
}
/*@} End of Private functions */

/*@{ Interface functions */
int RadarDefinition_setID(RadarDefinition_t* radar, const char* id)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  RAVE_FREE(radar->id);
  if (id != NULL) {
    radar->id = RAVE_STRDUP(id);
    if (radar->id == NULL) {
      RAVE_CRITICAL0("Failure when copying id");
      return 0;
    }
  }
  return 1;
}

const char* RadarDefinition_getID(RadarDefinition_t* radar)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  return (const char*)radar->id;
}

int RadarDefinition_setDescription(RadarDefinition_t* radar, const char* descr)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  RAVE_FREE(radar->description);
  if (descr != NULL) {
    radar->description = RAVE_STRDUP(descr);
    if (radar->description == NULL) {
      RAVE_CRITICAL0("Failure when copying description");
      return 0;
    }
  }
  return 1;
}

const char* RadarDefinition_getDescription(RadarDefinition_t* radar)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  return (const char*)radar->description;
}

void RadarDefinition_setLongitude(RadarDefinition_t* radar, double lon)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  radar->lon = lon;
}

double RadarDefinition_getLongitude(RadarDefinition_t* radar)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  return radar->lon;
}

void RadarDefinition_setLatitude(RadarDefinition_t* radar, double lat)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  radar->lat = lat;
}

double RadarDefinition_getLatitude(RadarDefinition_t* radar)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  return radar->lat;
}

void RadarDefinition_setHeight(RadarDefinition_t* radar, double h)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  radar->height = h;
}

double RadarDefinition_getHeight(RadarDefinition_t* radar)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  return radar->height;
}

int RadarDefinition_setElangles(RadarDefinition_t* radar, unsigned int nangles, double* angles)
{
  int i = 0;
  int result = 0;
  double* tmparr = NULL;
  RAVE_ASSERT((radar != NULL), "radar == NULL");

  if (nangles > 0 && angles == NULL) {
    RAVE_ERROR0("Setting elevation angles with nangles > 0 and angles == NULL...");
    goto done;
  }
  tmparr = RAVE_MALLOC(sizeof(double)*nangles);
  if (tmparr == NULL) {
    RAVE_ERROR0("Failed to allocate memory for elevation angles");
    goto done;
  }
  for (i = 0; i < nangles; i++) {
    tmparr[i] = angles[i];
  }
  RAVE_FREE(radar->elangles);
  radar->elangles = tmparr;
  radar->nelangles = nangles;
  tmparr = NULL;
  result = 1;
done:
  RAVE_FREE(tmparr);
  return result;
}

int RadarDefinition_getElangles(RadarDefinition_t* radar, unsigned int* nangles, double** angles)
{
  int result = 0;
  int i = 0;
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  RAVE_ASSERT((nangles != NULL), "nangles == NULL");
  RAVE_ASSERT((angles != NULL), "angles == NULL");

  *angles = RAVE_MALLOC(radar->nelangles * sizeof(double));
  if (*angles == NULL) {
    RAVE_ERROR0("Failed to allocate memory for elevation angles");
    goto done;
  }
  for (i = 0; i < radar->nelangles; i++) {
    (*angles)[i] = radar->elangles[i];
  }
  *nangles = radar->nelangles;

  result = 1;
done:
  return result;
}

void RadarDefinition_setNrays(RadarDefinition_t* radar, long nrays)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  radar->nrays = nrays;
}

long RadarDefinition_getNrays(RadarDefinition_t* radar)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  return radar->nrays;
}

void RadarDefinition_setNbins(RadarDefinition_t* radar, long nbins)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  radar->nbins = nbins;
}

long RadarDefinition_getNbins(RadarDefinition_t* radar)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  return radar->nbins;
}

void RadarDefinition_setScale(RadarDefinition_t* radar, double scale)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  radar->scale = scale;
}

double RadarDefinition_getScale(RadarDefinition_t* radar)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  return radar->scale;
}

void RadarDefinition_setBeamwidth(RadarDefinition_t* radar, double bw)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  RadarDefinition_setBeamwH(radar, bw);
}

double RadarDefinition_getBeamwidth(RadarDefinition_t* radar)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  return RadarDefinition_getBeamwH(radar);
}

void RadarDefinition_setBeamwH(RadarDefinition_t* radar, double bw)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  radar->beamwH = bw;

}

double RadarDefinition_getBeamwH(RadarDefinition_t* radar)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  return radar->beamwH;
}

void RadarDefinition_setBeamwV(RadarDefinition_t* radar, double bw)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  radar->beamwV = bw;
}

double RadarDefinition_getBeamwV(RadarDefinition_t* radar)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  return radar->beamwV;
}

void RadarDefinition_setWavelength(RadarDefinition_t* radar, double l)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  radar->wavelength = l;
}

double RadarDefinition_getWavelength(RadarDefinition_t* radar)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  return radar->wavelength;
}

void RadarDefinition_setProjection(RadarDefinition_t* radar, Projection_t* projection)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  RAVE_OBJECT_RELEASE(radar->projection);
  radar->projection = RAVE_OBJECT_COPY(projection);
}

Projection_t* RadarDefinition_getProjection(RadarDefinition_t* radar)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  return RAVE_OBJECT_COPY(radar->projection);
}

/*@} End of Interface functions */

RaveCoreObjectType RadarDefinition_TYPE = {
    "RadarDefinition",
    sizeof(RadarDefinition_t),
    RadarDefinition_constructor,
    RadarDefinition_destructor,
    RadarDefinition_copyconstructor
};
