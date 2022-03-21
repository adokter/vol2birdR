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
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-10-21
 */
#include "polarnav.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "math.h"
#include <string.h>

/**
 * Radius at the equator
 */
#define DEFAULT_EQUATOR_RADIUS 6378160.0;

/**
 * Radius to the poles.
 */
#define DEFAULT_POLE_RADIUS 6356780.0;

/**
 * Represents one polar navigator
 */
struct _PolarNavigator_t {
  RAVE_OBJECT_HEAD /** Always on top */

  double poleRadius; /**< the radius to the poles */
  double equatorRadius; /**< the radius at the equator */
  double lon0; /**< the origin longitude */
  double lat0; /**< the origin latitude */
  double alt0; /**< the origin altitude */
  double dndh; /**< the dndh */
};

/*@{ Private functions */
/**
 * Constructor.
 */
static int PolarNavigator_constructor(RaveCoreObject* obj)
{
  PolarNavigator_t* result = (PolarNavigator_t*)obj;
  result->poleRadius = DEFAULT_POLE_RADIUS;
  result->equatorRadius = DEFAULT_EQUATOR_RADIUS;
  result->lon0 = 0.0;
  result->lat0 = 0.0;
  result->alt0 = 0.0;
  result->dndh = (-3.9e-5)/1000;
  return 1;
}

/**
 * Constructor.
 */
static int PolarNavigator_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  PolarNavigator_t* this = (PolarNavigator_t*)obj;
  PolarNavigator_t* src = (PolarNavigator_t*)srcobj;

  this->poleRadius = src->poleRadius;
  this->equatorRadius = src->equatorRadius;
  this->lon0 = src->lon0;
  this->lat0 = src->lat0;
  this->alt0 = src->alt0;
  this->dndh = src->dndh;
  return 1;
}

/**
 * Destroys the polar navigator
 * @param[in] polnav - the polar navigator to destroy
 */
static void PolarNavigator_destructor(RaveCoreObject* obj)
{
  // NO OP
}
/*@} End of Private functions */

/*@{ Interface functions */
void PolarNavigator_setPoleRadius(PolarNavigator_t* polnav, double radius)
{
  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  polnav->poleRadius = radius;
}

double PolarNavigator_getPoleRadius(PolarNavigator_t* polnav)
{
  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  return polnav->poleRadius;
}

void PolarNavigator_setEquatorRadius(PolarNavigator_t* polnav, double radius)
{
  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  polnav->equatorRadius = radius;
}

double PolarNavigator_getEquatorRadius(PolarNavigator_t* polnav)
{
  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  return polnav->equatorRadius;
}

void PolarNavigator_setLon0(PolarNavigator_t* polnav, double lon0)
{
  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  polnav->lon0 = lon0;
}

double PolarNavigator_getLon0(PolarNavigator_t* polnav)
{
  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  return polnav->lon0;
}

void PolarNavigator_setLat0(PolarNavigator_t* polnav, double lat0)
{
  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  polnav->lat0 = lat0;
}

double PolarNavigator_getLat0(PolarNavigator_t* polnav)
{
  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  return polnav->lat0;
}

void PolarNavigator_setAlt0(PolarNavigator_t* polnav, double alt0)
{
  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  polnav->alt0 = alt0;
}

double PolarNavigator_getAlt0(PolarNavigator_t* polnav)
{
  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  return polnav->alt0;
}

void PolarNavigator_setDndh(PolarNavigator_t* polnav, double dndh)
{
  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  polnav->dndh = dndh;
}

double PolarNavigator_getDndh(PolarNavigator_t* polnav)
{
  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  return polnav->dndh;
}

double PolarNavigator_getEarthRadius(PolarNavigator_t* polnav, double lat)
{
  double radius = 0L;
  double a = 0L;
  double b = 0L;
  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  a = sin(lat) * polnav->poleRadius;
  b = cos(lat) * polnav->equatorRadius;
  radius = sqrt(a * a + b * b);
  return radius;
}

double PolarNavigator_getEarthRadiusOrigin(PolarNavigator_t* polnav)
{
  double radius = 0L;
  double a = 0L;
  double b = 0L;
  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  a = sin(polnav->lat0) * polnav->poleRadius;
  b = cos(polnav->lat0) * polnav->equatorRadius;
  radius = sqrt(a * a + b * b);
  return radius;
}

double PolarNavigator_getDistance(PolarNavigator_t* polnav, double lat, double lon)
{
  double dLon = 0.0L, dLat = 0.0L, distance = 0.0L;
  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  dLon = (lon - polnav->lon0) * cos(polnav->lat0);
  dLat = lat - polnav->lat0;

  distance = sqrt(dLon * dLon + dLat * dLat) * PolarNavigator_getEarthRadiusOrigin(polnav);

  return distance;
}

void PolarNavigator_llToDa(PolarNavigator_t* polnav, double lat, double lon, double* d, double* a)
{
  double dLon = 0.0L;
  double dLat = 0.0L;
  double distance = 0.0L;
  double azimuth = 0.0L;
  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  RAVE_ASSERT((d != NULL && a != NULL), "a and/or d missing");

  dLon = (lon - polnav->lon0) * cos(polnav->lat0);
  dLat = lat - polnav->lat0;

  distance = sqrt(dLon * dLon + dLat * dLat) * PolarNavigator_getEarthRadiusOrigin(polnav);

  if (distance == 0.0) {
    azimuth = 0.0;
  } else if (dLat == 0.0) {
    if (dLon > 0.0) {
      azimuth = M_PI / 2.0;
    } else {
      azimuth = -(M_PI / 2.0);
    }
  } else if (dLat > 0.0) {
    azimuth = atan(dLon / dLat);
  } else {
    azimuth = M_PI + atan(dLon / dLat);
  }

  if (azimuth < 0.0) {
    azimuth += 2*M_PI;
  }
  *d = distance;
  *a = azimuth;
}

void PolarNavigator_daToLl(PolarNavigator_t* polnav, double d, double a, double* lat, double* lon)
{
  double evalDist = 0.0L;

  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  RAVE_ASSERT((lat != NULL && lon != NULL), "lat and/or lon missing");

  if (cos(polnav->lat0) == 0.0) {
    RAVE_CRITICAL0("PolarNavigator_daToLl would result in division by zero.");
    return;
  }

  evalDist = d / PolarNavigator_getEarthRadiusOrigin(polnav);

  *lon = polnav->lon0 + evalDist * (sin(a) / cos(polnav->lat0));
  *lat = polnav->lat0 + evalDist * cos(a);
}

void PolarNavigator_dhToRe(PolarNavigator_t* polnav, double d, double h, double* r, double* e)
{
  double A_prim = 0.0L, B_prim = 0.0L, Lambda_prim = 0.0L, C_prim = 0.0L, R_prim = 0.0L;
  double height = 0.0L;
  double R_earth = 0.0L;

  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  RAVE_ASSERT((r != NULL && e != NULL), "r and/or e missing");

  R_earth = PolarNavigator_getEarthRadiusOrigin(polnav);

  if (abs(polnav->dndh + 1.0 / R_earth) < 1.0e-9 * (polnav->dndh)) {
    /* The rays and the earth-surface are modelled as being straight lines.*/
    height = h - polnav->alt0;
    *r = sqrt(height * height + d * d);

    if (abs(d) < 1.0) {
      *e = atan(height / d);
    } else {
      *e = M_PI / 2.0;
    }
    return;
  }

  R_prim = 1.0 / ((1.0 / R_earth) + polnav->dndh);
  C_prim = R_prim + h;
  Lambda_prim = d / R_prim;
  A_prim = C_prim * cos(Lambda_prim);
  B_prim = C_prim * sin(Lambda_prim);
  height = A_prim - (R_prim + polnav->alt0);
  *r = sqrt(height * height + B_prim * B_prim);

  if (((B_prim * height < 1.0e-9) && (B_prim * height > 0.0))
      || ((height > 0.0) && (B_prim == 0.0))) {
    *e = M_PI / 2.0;
  } else if (((B_prim * height > -1.0e-9) && (B_prim * height < 0.0))
      || ((height < 0.0) && (B_prim == 0.0))) {
    *e = M_PI / 2.0;
  } else {
    *e = atan(height / B_prim);
  }
}

void PolarNavigator_deToRh(PolarNavigator_t* polnav, double d, double e, double* r, double* h)
{
  double R_prim = 0.0L, A_prim = 0.0L, B_prim = 0.0L, gamma = 0.0L, height = 0.0L, A = 0.0L;
  double R_earth = 0.0L;

  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  RAVE_ASSERT((r != NULL && h != NULL), "r and/or h missing");

  R_earth = PolarNavigator_getEarthRadiusOrigin(polnav);

  if (abs(1.0 / R_earth + polnav->dndh) < 1.0e-9 * (polnav->dndh)) {
    height = polnav->alt0;
    *r = sqrt(height * height + d * d);
    *h = polnav->alt0 + ((*r) * sin(e));
    return;
  }

  R_prim = 1.0 / ((1.0 / R_earth) + polnav->dndh);
  A = R_prim + polnav->alt0;
  gamma = d / R_prim;
  *r = A * tan(gamma) * sin(M_PI / 2.0 - gamma) / (sin(M_PI / 2.0 - e - gamma));
  A_prim = A + (*r) * sin(e);
  B_prim = (*r) * cos(e);
  *h = sqrt(A_prim * A_prim + B_prim * B_prim) - R_prim;
}

void PolarNavigator_reToDh(PolarNavigator_t* polnav, double r, double e, double* d, double* h)
{
  double R_prim = 0.0L, A_prim = 0.0L, B_prim = 0.0L, Lambda_prim = 0.0L, R_earth = 0.0L;

  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  RAVE_ASSERT((d != NULL && h != NULL), "d and/or h missing");

  R_earth = PolarNavigator_getEarthRadiusOrigin(polnav);

  if (abs(polnav->dndh + 1.0 / R_earth) < 1.0e-9 * (polnav->dndh)) {
    /*Straight lines*/
    *h = polnav->alt0 + r * sin(e);
    *d = r * cos(e);
    return;
  }

  R_prim = 1.0 / ((1.0 / R_earth) + polnav->dndh);
  A_prim = R_prim + polnav->alt0 + r * sin(e);
  B_prim = r * cos(e);
  Lambda_prim = atan(B_prim / (A_prim));
  *h = sqrt(A_prim * A_prim + B_prim * B_prim) - R_prim;
  *d = R_prim * Lambda_prim;
}

void PolarNavigator_ehToRd(PolarNavigator_t* polnav, double e, double h, double* r, double* d)
{
  double R_prim = 0.0L, A = 0.0L, A_prim = 0.0L, B_prim = 0.0L, Lambda_prim = 0.0L, P = 0.0L, Q = 0.0L, C1 = 0.0L;
  double tmpValue = 0.0L, R_earth = 0.0L;

  RAVE_ASSERT((polnav != NULL), "polnav was NULL");
  RAVE_ASSERT((r != NULL && d != NULL), "r and/or d missing");

  R_earth = PolarNavigator_getEarthRadiusOrigin(polnav);

  tmpValue = polnav->dndh + 1.0 / R_earth;

  if (tmpValue < 0)
    tmpValue = -tmpValue;

  if (tmpValue < 1.0e-9 * (polnav->dndh)) {
    /*Straight lines*/
    if (sin(e) == 0.0) {
      RAVE_CRITICAL0("Trying to divide by zero");
      return;
    }
    *r = (h - polnav->alt0) / sin(e);
    *d = (*r) * cos(e);
    return;
  }

  R_prim = 1.0 / ((1.0 / R_earth) + polnav->dndh);
  A = R_prim + polnav->alt0;
  C1 = R_prim + h;
  P = 2.0 * A * sin(e);
  Q = A * A - C1 * C1;

  *r = -P / 2.0 + sqrt((P / 2.0) * (P / 2.0) - Q);

  /* Will give the distance, as soon as the range is correct*/
  A_prim = R_prim + polnav->alt0 + (*r) * sin(e);
  B_prim = (*r) * cos(e);
  Lambda_prim = atan(B_prim / A_prim);
  *d = R_prim * Lambda_prim;
}

/*@} End of Interface functions */

RaveCoreObjectType PolarNavigator_TYPE = {
    "PolarNavigator",
    sizeof(PolarNavigator_t),
    PolarNavigator_constructor,
    PolarNavigator_destructor,
    PolarNavigator_copyconstructor
};
