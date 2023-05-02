/* --------------------------------------------------------------------
Copyright (C) 2014 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Gauge radar adjustment (GRA). This class performs the actual gauge adjustment
 * using the derived coefficients. This can be done in two ways: one for ACRR products
 * (most common) and another for reflectivity (any of DBZH, DBZV, TH, TV).
 * In the case of ACRR, the parameter is already mm which is good.
 * In the case of reflectivity, the parameter needs to be converted to R (mm/hr), the correction applied, and
 * then the result converted back to reflectivity. This should be done in C. Functionality exists already in
 * raveutil.c/h: dBZ2R or raw2R and back.
 * Default Z-R coefficients are given in rave_defined.ZR_A and ZR_b. The C could look something like this (from N2):
 * F = A + B*DIST + C*pow(DIST, 2.0);
 * F = RAVEMIN(F, 2.0);    upper threshold 20 dBR
 * F = RAVEMAX(F, -0.25);  lower threshold -2.5 dBR
 * out = R*pow(10.0, F);
 *
 * final lower threhold on gauge-adjusted result
 *
 * if (out < lt) { out=0.0; }
 *
 * This object does support \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2014-03-28
 */
#ifndef RAVE_GRA_H
#define RAVE_GRA_H
#include "rave_object.h"
#include "rave_field.h"
#include "cartesianparam.h"

/**
 * Defines GRA
 */
typedef struct _RaveGra_t RaveGra_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType RaveGra_TYPE;

/**
 * Sets the A coefficient in the formula A + B*DIST + C*pow(DIST, 2.0);
 * @param[in] self - self
 * @param[in] A - the A coefficient
 */
void RaveGra_setA(RaveGra_t* self, double A);

/**
 * Returns the A coefficient.
 * @param[in] self - self
 * @return the A coefficient
 */
double RaveGra_getA(RaveGra_t* self);

/**
 * Sets the B coefficient in the formula A + B*DIST + C*pow(DIST, 2.0);
 * @param[in] self - self
 * @param[in] B - the B coefficient
 */
void RaveGra_setB(RaveGra_t* self, double B);

/**
 * Returns the B coefficient.
 * @param[in] self - self
 * @return the B coefficient
 */
double RaveGra_getB(RaveGra_t* self);

/**
 * Sets the C coefficient in the formula A + B*DIST + C*pow(DIST, 2.0);
 * @param[in] self - self
 * @param[in] C - the C coefficient
 */
void RaveGra_setC(RaveGra_t* self, double C);

/**
 * Returns the C coefficient.
 * @param[in] self - self
 * @return the C coefficient
 */
double RaveGra_getC(RaveGra_t* self);

/**
 * Sets the upper threshold in 10ths of dBR. Default is 2.0 (20 dBR)
 * @param[in] self - self
 * @param[in] threshold - the upper threshold
 */
void RaveGra_setUpperThreshold(RaveGra_t* self, double threshold);

/**
 * Returns the upper threshold
 * @param[in] self - self
 * @return the upper threshold
 */
double RaveGra_getUpperThreshold(RaveGra_t* self);

/**
 * Sets the lower threshold in 10ths of dBR. Default is -0.25 (-2.5 dBR)
 * @param[in] self - self
 * @param[in] threshold - the lower threshold
 */
void RaveGra_setLowerThreshold(RaveGra_t* self, double threshold);

/**
 * Returns the lower threshold
 * @param[in] self - self
 * @return the lower threshold
 */
double RaveGra_getLowerThreshold(RaveGra_t* self);

/**
 * Sets the ZR A coefficient when converting from reflectivity to MM/H;
 * @param[in] self - self
 * @param[in] zrA - the zrA coefficient
 */
void RaveGra_setZRA(RaveGra_t* self, double zrA);

/**
 * Returns the ZR A coefficient.
 * @param[in] self - self
 * @return the ZR A coefficient
 */
double RaveGra_getZRA(RaveGra_t* self);

/**
 * Sets the ZR B coefficient when converting from reflectivity to MM/H
 * @param[in] self - self
 * @param[in] zrb - the ZR b coefficient
 */
void RaveGra_setZRB(RaveGra_t* self, double zrb);

/**
 * Returns the ZR b coefficient.
 * @param[in] self - self
 * @return the ZR b coefficient
 */
double RaveGra_getZRB(RaveGra_t* self);

/**
 * Applies the coefficients on the parameter field. The distance field dimensions must match the parameter dimensions.
 * If the quantity is ACRR, then no conversion of the value is required. If the quantity is any of DBZH, DBZV, TH or TV, then
 * the values are converted to MM/H and then back again to reflectivity.
 * @param[in] self - self
 * @param[in] distance - the distance field
 * @param[in] parameter - the parameter to convert
 * @returns the parameter with the gra applied
 */
CartesianParam_t* RaveGra_apply(RaveGra_t* self, RaveField_t* distance, CartesianParam_t* parameter);

#endif /* RAVE_GRA_H */
