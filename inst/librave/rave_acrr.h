/* --------------------------------------------------------------------
Copyright (C) 2012 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Implementation of the Precipitation accumulation - ACRR algorithm
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2012-05-31
 */
#ifndef RAVE_ACRR_H
#define RAVE_ACRR_H
#include "rave_object.h"
#include "cartesianparam.h"

/**
 * Defines ACRR
 */
typedef struct _RaveAcrr_t RaveAcrr_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType RaveAcrr_TYPE;

/**
 * Sums a parameter with the previously calculated values.
 * @param[in] self - self
 * @param[in] param - the cartesian parameter
 */
int RaveAcrr_sum(RaveAcrr_t* self, CartesianParam_t* param, double zr_a, double zr_b);

/**
 * Generates the result
 * @param[in] self - self
 * @param[in] acpt - the limit for accepting the accumulation (percent between 0 - 1)
 * @param[in] N - the number of expected calls to sum (will be used when calculating acceptable limit)
 * @param[in] hours - the number of hours this accumulation has been performed on.
 * @return the cartesian parameter with quantity ACRR on success otherwise NULL
 */
CartesianParam_t* RaveAcrr_accumulate(RaveAcrr_t* self, double acpt, long N, double hours);

/**
 * Returns if this instance has been initialized or not
 * @param[in] self - self
 * @return 1 if instance has been initialized otherwise 0
 */
int RaveAcrr_isInitialized(RaveAcrr_t* self);

/**
 * Sets the nodata to be used for the accumulation.
 * @param[in] self - self
 * @param[in] nodata - the nodata value to use
 */
void RaveAcrr_setNodata(RaveAcrr_t* self, double nodata);

/**
 * Returns the nodata that will be used in the accumulation.
 * @param[in] self - self
 * @return the nodata value to be used
 */
double RaveAcrr_getNodata(RaveAcrr_t* self);

/**
 * Sets the undetect to be used for the accumulation.
 * @param[in] self - self
 * @param[in] undetect - the undetect value to use
 */
void RaveAcrr_setUndetect(RaveAcrr_t* self, double undetect);

/**
 * Returns the undetect that will be used in the accumulation.
 * @param[in] self - self
 * @return the undetect value to be used
 */
double RaveAcrr_getUndetect(RaveAcrr_t* self);

/**
 * Returns the quantity for this generator instance.
 * @param[in] self - self
 * @return the quantity for this generator instance or NULL if not initialized.
 */
const char* RaveAcrr_getQuantity(RaveAcrr_t* self);

/**
 * Sets the name of the quality field (how/task value) to be used as
 * distance field when performing the Acrr algorithm.
 * (default is se.smhi.composite.distance.radar).
 * @param[in] self - self
 * @param[in] fieldname - the name of the quality field (MAY NOT BE NULL)
 * @return 1 on success or 0 on error
 */
int RaveAcrr_setQualityFieldName(RaveAcrr_t* self, const char* fieldname);

/**
 * Returns the name of the quality field (how/task value) to be used as
 * distance field when performing the Acrr algorithm.
 * @param[in] self - self
 * @return the quality field name
 */
const char* RaveAcrr_getQualityFieldName(RaveAcrr_t* self);

#endif /* RAVE_ACRR_H */
