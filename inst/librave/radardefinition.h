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
 * Defines a radar definition
 * This object supports \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-08-31
 */
#ifndef RADARDEFINITION_H
#define RADARDEFINITION_H
#include "rave_proj.h"
#include "projection.h"
#include "rave_object.h"

/**
 * Defines a Radar definition
 */
typedef struct _RadarDefinition_t RadarDefinition_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType RadarDefinition_TYPE;

/**
 * Sets the ID for this definition.
 * @param[in] radar - self
 * @param[in] id - the id
 * @returns 1 on success otherwise 0
 */
int RadarDefinition_setID(RadarDefinition_t* radar, const char* id);

/**
 * Returns the ID for this definition.
 * @param[in] radar - self
 */
const char* RadarDefinition_getID(RadarDefinition_t* radar);

/**
 * Sets the description for this definition.
 * @param[in] radar - self
 * @param[in] descr - the description to set
 * @returns 1 on success otherwise 0
 */
int RadarDefinition_setDescription(RadarDefinition_t* radar, const char* descr);

/**
 * Returns the description for this definition.
 * @param[in] radar - self
 */
const char* RadarDefinition_getDescription(RadarDefinition_t* radar);

/**
 * @param[in] radar - self
 * @param[in] lon - the longitude of the radar (in radians)
 */
void RadarDefinition_setLongitude(RadarDefinition_t* radar, double lon);

/**
 * @param[in] radar - self
 * @returns the longitude in radians
 */
double RadarDefinition_getLongitude(RadarDefinition_t* radar);

/**
 * @param[in] radar - self
 * @param[in] lat - the latitude of the radar (in radians)
 */
void RadarDefinition_setLatitude(RadarDefinition_t* radar, double lat);

/**
 * @param[in] radar - self
 * @returns the latitude in radians
 */
double RadarDefinition_getLatitude(RadarDefinition_t* radar);

/**
 * @param[in] radar - self
 * @param[in] h - the height above sea level (in meters)
 */
void RadarDefinition_setHeight(RadarDefinition_t* radar, double h);

/**
 * @param[in] radar - self
 * @returns the height
 */
double RadarDefinition_getHeight(RadarDefinition_t* radar);

/**
 * Sets the elevation angles
 * @param[in] radar - self
 * @param[in] nangles - the size of the angles array
 * @param[in] angles - the elevation angles in radians
 * @returns 1 on success otherwise 0
 */
int RadarDefinition_setElangles(RadarDefinition_t* radar, unsigned int nangles, double* angles);

/**
 * Returns the elevation angles
 * @param[in] radar - self
 * @param[out] nangles - the size of the angles array
 * @param[out] angles - the elevation angles in radians
 * @returns 1 on success otherwise 0
 */
int RadarDefinition_getElangles(RadarDefinition_t* radar, unsigned int* nangles, double** angles);


/**
 * Sets the projection that defines this radar.
 * @param[in] radar - self
 * @param[in] projection - the projection
 */
void RadarDefinition_setProjection(RadarDefinition_t* radar, Projection_t* projection);

/**
 * Returns the projection that defines this radar.
 * @param[in] radar - self
 * @returns the projection
 */
Projection_t* RadarDefinition_getProjection(RadarDefinition_t* radar);

/**
 * @param[in] radar - self
 * @param[in] nrays - the number of rays to set
 */
void RadarDefinition_setNrays(RadarDefinition_t* radar, long nrays);

/**
 * @param[in] radar - self
 * @returns the number of rays
 */
long RadarDefinition_getNrays(RadarDefinition_t* radar);

/**
 * @param[in] radar - self
 * @param[in] nbins - the number of bins to set
 */
void RadarDefinition_setNbins(RadarDefinition_t* radar, long nbins);

/**
 * @param[in] radar - self
 * @returns the number of bins
 */
long RadarDefinition_getNbins(RadarDefinition_t* radar);

/**
 * @param[in] radar - self
 * @param[in] scale - the scale to set
 */
void RadarDefinition_setScale(RadarDefinition_t* radar, double scale);

/**
 * @param[in] radar - self
 * @returns the scale
 */
double RadarDefinition_getScale(RadarDefinition_t* radar);

/**
 * @param[in] radar - self
 * @param[in] bw - the beam width to set
 */
void RadarDefinition_setBeamwidth(RadarDefinition_t* radar, double bw);

/**
 * @param[in] radar - self
 * @returns the beam width
 */
double RadarDefinition_getBeamwidth(RadarDefinition_t* radar);

/**
 * @param[in] radar - self
 * @param[in] bw - the horizontal beam width to set
 */
void RadarDefinition_setBeamwH(RadarDefinition_t* radar, double bw);

/**
 * @param[in] radar - self
 * @returns the horizontal beam width
 */
double RadarDefinition_getBeamwH(RadarDefinition_t* radar);

/**
 * @param[in] radar - self
 * @param[in] bw - the vertical beam width to set
 */
void RadarDefinition_setBeamwV(RadarDefinition_t* radar, double bw);

/**
 * @param[in] radar - self
 * @returns the vertical beam width
 */
double RadarDefinition_getBeamwV(RadarDefinition_t* radar);

/**
 * @param[in] radar - self
 * @param[in] l - the wave length to set
 */
void RadarDefinition_setWavelength(RadarDefinition_t* radar, double l);

/**
 * @param[in] radar - self
 * @returns the wave length
 */
double RadarDefinition_getWavelength(RadarDefinition_t* radar);

#endif /* RADARDEFINITION_H */
