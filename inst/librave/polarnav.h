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
 * Utilities for performing polar navigation.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 *
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-10-21
 */
#ifndef POLARNAV_H
#define POLARNAV_H
#include "rave_object.h"

#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#endif

/**
 * Defines a polar navigator
 */
typedef struct _PolarNavigator_t PolarNavigator_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType PolarNavigator_TYPE;

/**
 * Sets the radius to the poles.
 * @param[in] polnav - the polar navigator
 * @param[in] radius - the radius to the poles (in meters)
 */
void PolarNavigator_setPoleRadius(PolarNavigator_t* polnav, double radius);

/**
 * Returns the radius to the poles
 * @param[in] polnav - the polar navigator
 * @return the radius to the poles (in meters)
 */
double PolarNavigator_getPoleRadius(PolarNavigator_t* polnav);

/**
 * Sets the radius at the equator
 * @param[in] polnav - the polar navigator
 * @param[in] radius - the radius at the equator (in meters)
 */
void PolarNavigator_setEquatorRadius(PolarNavigator_t* polnav, double radius);

/**
 * Returns the radius at the equator
 * @param[in] polnav - the polar navigator
 * @return the radius at the equator (in meters)
 */
double PolarNavigator_getEquatorRadius(PolarNavigator_t* polnav);

/**
 * Sets the origin longitude
 * @param[in] polnav - the polar navigator
 * @param[in] lon0 - the origin longitude (in radians)
 */
void PolarNavigator_setLon0(PolarNavigator_t* polnav, double lon0);

/**
 * Returns the origin longitude
 * @param[in] polnav - the polar navigator
 * @return the origin longitude (in radians)
 */
double PolarNavigator_getLon0(PolarNavigator_t* polnav);

/**
 * Sets the origin latitude
 * @param[in] polnav - the polar navigator
 * @param[in] lat0 - the origin latitude (in radians)
 */
void PolarNavigator_setLat0(PolarNavigator_t* polnav, double lat0);

/**
 * Returns the origin latitude
 * @param[in] polnav - the polar navigator
 * @return the origin latitude (in radians)
 */
double PolarNavigator_getLat0(PolarNavigator_t* polnav);

/**
 * Sets the origin altitude
 * @param[in] polnav - the polar navigator
 * @param[in] alt0 - the origin altitude (in meters)
 */
void PolarNavigator_setAlt0(PolarNavigator_t* polnav, double alt0);

/**
 * Returns the origin altitude
 * @param[in] polnav - the polar navigator
 * @return the origin altitude (in meters)
 */
double PolarNavigator_getAlt0(PolarNavigator_t* polnav);

/**
 * Sets the dndh (deflection).
 * @param[in] polnav - the polar navigator
 * @param[in] dndh - the dndh value
 */
void PolarNavigator_setDndh(PolarNavigator_t* polnav, double dndh);

/**
 * Returns the dndh
 * @param[in] polnav - the polar navigator
 * @return the dndh
 */
double PolarNavigator_getDndh(PolarNavigator_t* polnav);

/**
 * Returns the earth radius (in meters) at the specified latitude.
 * @param[in] polnav - the polar navigator
 * @param[in] lat - the latitude (in radians)
 * @return the radius (in meters) at the specified latitude
 */
double PolarNavigator_getEarthRadius(PolarNavigator_t* polnav, double lat);

/**
 * Returns the earth radius at the origin. Same as calling
 * PolarNavigator_getEarthRadius(polnav, PolarNavigator_getLat0(polnav)).
 * @param[in] polnav - the polar navigator
 * @return the earth radius (in meters)
 */
double PolarNavigator_getEarthRadiusOrigin(PolarNavigator_t* polnav);

/**
 * Returns the distance between lon0/lat0 and the provided lon/lat coordinates.
 * @param[in] polnav - self
 * @param[in] lat - the latitude
 * @param[in] lon - the longitude
 * @returns the distance in meters.
 */
double PolarNavigator_getDistance(PolarNavigator_t* polnav, double lat, double lon);

/**
 * Calculates the distance/azimuth from origin to the specified lon/lat.
 * @param[in] polnav - the polar navigator
 * @param[in] lat - the destination latitude (in radians)
 * @param[in] lon - the destination longitude (in radians)
 * @param[out] d - the distance (in meters)
 * @param[out] a - the azimuth (in radians)
 */
void PolarNavigator_llToDa(PolarNavigator_t* polnav, double lat, double lon, double* d, double* a);

/**
 * Calculates the lon/lat from origin to the specified azimuth/distance.
 * @param[in] polnav - the polar navigator
 * @param[in] d - the distance (in meters)
 * @param[in] a - the azimuth (in radians)
 * @param[out] lat - the latitude (in radians)
 * @param[out] lon - the longitude (in radians)
 */
void PolarNavigator_daToLl(PolarNavigator_t* polnav, double d, double a, double* lat, double* lon);

/**
 * Calculates the range/elevation from the specified distance and height.
 * @param[in] polnav - the polar navigator
 * @param[in] d - the distance (in meters)
 * @param[in] h - the height (in meters)
 * @param[out] r - the range (in meters)
 * @param[out] e - the elevation (in radians)
 */
void PolarNavigator_dhToRe(PolarNavigator_t* polnav, double d, double h, double* r, double* e);

/**
 * Calculates the range/height from the specified distance and elevation.
 * @param[in] polnav - the polar navigator
 * @param[in] d - the distance (in meters)
 * @param[in] e - the elevation (in radians)
 * @param[out] r - the range (in meters)
 * @param[out] h - the height (in meters)
 */
void PolarNavigator_deToRh(PolarNavigator_t* polnav, double d, double e, double* r, double* h);

/**
 * Calculates the distance/height from the specified range and elevation.
 * @param[in] polnav - the polar navigator
 * @param[in] r - the range (in meters)
 * @param[in] e - the elevation (in radians)
 * @param[out] d - the distance (in meters)
 * @param[out] h - the height (in meters)
 */
void PolarNavigator_reToDh(PolarNavigator_t* polnav, double r, double e, double* d, double* h);

/**
 * Calculates the range/distance from the specified elevation and height.
 * @param[in] polnav - the polar navigator
 * @param[in] e - the elevation (in radians)
 * @param[in] h - the height (in meters)
 * @param[out] r - the range (in meters)
 * @param[out] d - the distance (in meters)
 */
void PolarNavigator_ehToRd(PolarNavigator_t* polnav, double e, double h, double* r, double* d);

#endif
