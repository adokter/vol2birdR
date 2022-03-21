/**
 * Navigation routines for calculating distances and heights.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 1998-
 */
#include "polar.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "rave_alloc.h"

/**
 * Radius at the equator
 */
const double R_EQU = 6378160.0;

/**
 * Radius to the poles.
 */
const double R_POL = 6356780.0;

void resetPosStruct(Position* pos)
{
  if (!pos) {
    return;
  }
  pos->alt0 = 0L;
  pos->lat0 = 0L;
  pos->lon0 = 0L;
  pos->alt = 0L;
  pos->lat = 0L;
  pos->lon = 0L;
  pos->azimuth = 0L;
  pos->distance = 0L;
  pos->dndh = 0L;
  pos->range = 0L;
  pos->elevation = 0L;
  pos->momelev = 0L;
}

Position* copyPosStruct(Position* src)
{
  Position* ret = RAVE_MALLOC(sizeof(Position));
  ret->alt0 = src->alt0;
  ret->lat0 = src->lat0;
  ret->lon0 = src->lon0;
  ret->alt = src->alt;
  ret->lat = src->lat;
  ret->lon = src->lon;
  ret->azimuth = src->azimuth;
  ret->distance = src->distance;
  ret->dndh = src->dndh;
  ret->range = src->range;
  ret->elevation = src->elevation;
  ret->momelev = src->momelev;

  return ret;
}

double getEarthRadius(double lat0)
{
  double radius = 0L;
  double a = sin(lat0) * R_POL;
  double b = cos(lat0) * R_EQU;
  radius = sqrt(a * a + b * b);
  return radius;
}

void llToDa(Position* src, Position* tgt)
{
  double dLon;
  double dLat;

  dLon = (src->lon - src->lon0) * cos(src->lat0);
  dLat = src->lat - src->lat0;

  /* This equation is not exactly true, since the radius will change depending
   * on the latitude
   */

  tgt->distance = sqrt(dLon * dLon + dLat * dLat) * getEarthRadius(src->lat0);

  if (tgt->distance == 0.0) {
    tgt->azimuth = 0.0;
  } else if (dLat == 0.0) {
    if (dLon > 0.0) {
      tgt->azimuth = M_PI / 2.0;
    } else {
      tgt->azimuth = -(M_PI / 2.0);
    }
  } else if (dLat > 0.0) {
    tgt->azimuth = atan(dLon / dLat);
  } else {
    tgt->azimuth = M_PI + atan(dLon / dLat);
  }

  if (tgt->azimuth < 0.0)
    tgt->azimuth += 2*M_PI;
}

void daToLl(Position* src, Position* tgt)
{
  double evalDist;

  if (cos(src->lat0) == 0.0) {
    printf("When trying to translate length and azimuth\n");
    printf("to longitude and latitude\n");
    printf("cos(original latitude) would result in division by zero.\n");
    return;
  }

  evalDist = src->distance / getEarthRadius(src->lat0);

  tgt->lon = src->lon0 + evalDist * (sin(src->azimuth) / cos(src->lat0));
  tgt->lat = src->lat0 + evalDist * cos(src->azimuth);
}

void dhToRe(Position* src, Position* tgt)
{
  double A_prim, B_prim, Lambda_prim, C_prim, R_prim;
  double height;
  double R_earth = getEarthRadius(src->lat0);

  if (abs(src->dndh + 1.0 / R_earth) < 1.0e-9 * (src->dndh)) {
    /* The rays and the earth-surface are modelled as being straight lines.*/
    height = src->alt - src->alt0;
    tgt->range = sqrt(height * height + src->distance * src->distance);

    if (abs(src->distance) < 1.0) {
      tgt->elevation = atan(height / src->distance);
    } else {
      tgt->elevation = M_PI / 2.0;
    }
    tgt->momelev = src->elevation;
    return;
  }

  R_prim = 1.0 / ((1.0 / R_earth) + src->dndh);
  C_prim = R_prim + src->alt;
  Lambda_prim = src->distance / R_prim;
  A_prim = C_prim * cos(Lambda_prim);
  B_prim = C_prim * sin(Lambda_prim);
  height = A_prim - (R_prim + src->alt0);
  tgt->range = sqrt(height * height + B_prim * B_prim);

  if (((B_prim * height < 1.0e-9) && (B_prim * height > 0.0))
      || ((height > 0.0) && (B_prim == 0.0))) {
    tgt->elevation = M_PI / 2.0;
  } else if (((B_prim * height > -1.0e-9) && (B_prim * height < 0.0))
      || ((height < 0.0) && (B_prim == 0.0))) {
    tgt->elevation = M_PI / 2.0;
  } else {
    tgt->elevation = atan(height / B_prim);
  }
  tgt->momelev = tgt->elevation + Lambda_prim;
}

void deToRh(Position* src, Position* tgt)
{
  double R_prim, A_prim, B_prim, gamma, h, A;

  double R_earth = getEarthRadius(src->lat0);

  if (abs(1.0 / R_earth + src->dndh) < 1.0e-9 * (src->dndh)) {
    h = src->alt - src->alt0;
    tgt->range = sqrt(h * h + src->distance * src->distance);
    tgt->alt = src->alt0 + (tgt->range * sin(src->elevation));
    tgt->momelev = src->elevation;
    return;
  }

  R_prim = 1.0 / ((1.0 / R_earth) + src->dndh);
  A = R_prim + src->alt0;
  gamma = src->distance / R_prim;
  tgt->range = A * tan(gamma) * sin(M_PI / 2.0 - gamma) / (sin(M_PI / 2.0
      - src->elevation - gamma));
  A_prim = A + tgt->range * sin(src->elevation);
  B_prim = tgt->range * cos(src->elevation);
  tgt->alt = sqrt(A_prim * A_prim + B_prim * B_prim) - R_prim;
  tgt->momelev = src->elevation + gamma;
}

void reToDh(Position* src, Position* tgt)
{
  double R_prim, A_prim, B_prim, Lambda_prim, R_earth;

  R_earth = getEarthRadius(src->lat0);

  if (abs(src->dndh + 1.0 / R_earth) < 1.0e-9 * (src->dndh)) {
    /*Straight lines*/
    tgt->alt = src->alt0 + src->range * sin(src->elevation);
    tgt->distance = src->range * cos(src->elevation);
    tgt->momelev = src->alt0;
    return;
  }

  R_prim = 1.0 / ((1.0 / R_earth) + src->dndh);
  A_prim = R_prim + src->alt0 + src->range * sin(src->elevation);
  B_prim = src->range * cos(src->elevation);
  Lambda_prim = atan(B_prim / (A_prim));
  tgt->alt = sqrt(A_prim * A_prim + B_prim * B_prim) - R_prim;
  tgt->distance = R_prim * Lambda_prim;
  tgt->momelev = src->elevation + Lambda_prim;
}

void ehToRd(Position* src, Position* tgt)
{
  double R_prim, A, A_prim, B_prim, Lambda_prim, P, Q, C1;
  double tmpValue;
  double R_earth = getEarthRadius(src->lat0);

  tmpValue = src->dndh + 1.0 / R_earth;

  if (tmpValue < 0)
    tmpValue = -tmpValue;

  if (tmpValue < 1.0e-9 * (src->dndh)) {
    /*Straight lines*/
    if (sin(src->elevation) == 0.0) {
      printf("Trying to divide by zero");
      return;
    }
    tgt->range = (src->alt - src->alt0) / sin(src->elevation);
    tgt->distance = tgt->range * cos(src->elevation);
    return;
  }

  R_prim = 1.0 / ((1.0 / R_earth) + src->dndh);
  A = R_prim + src->alt0;

  C1 = R_prim + src->alt;

  P = 2.0 * A * sin(src->elevation);
  Q = A * A - C1 * C1;

  tgt->range = -P / 2.0 + sqrt((P / 2.0) * (P / 2.0) - Q);

  /* Will give the distance, as soon as the range is correct*/
  A_prim = R_prim + src->alt0 + tgt->range * sin(src->elevation);
  B_prim = tgt->range * cos(src->elevation);
  Lambda_prim = atan(B_prim / A_prim);
  tgt->distance = R_prim * Lambda_prim;
  tgt->momelev = src->elevation + Lambda_prim;
}
