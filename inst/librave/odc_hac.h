/* --------------------------------------------------------------------
Copyright (C) 2013 Swedish Meteorological and Hydrological Institute, SMHI,

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
/** Function for performing hit-accumulation clutter filtering.
 * @file
 * @author Daniel Michelson, SMHI
 * @date 2013-01-23
 */
#ifndef ODCHAC_H
#define ODCHAC_H

#include "polarscan.h"
#include "polarscanparam.h"
#include "rave_field.h"
#include "rave_attribute.h"
#include "rave_object.h"
#include "rave_alloc.h"
#include "rave_types.h"
#include "raveutil.h"
#include "rave_debug.h"

/**
 * Performs HAC filtering.
 * @param[in] scan - input scan
 * @param[in] hac - input field containing hits
 * @returns int 1 if successful, otherwise 0
 */
int hacFilter(PolarScan_t* scan, RaveField_t* hac, char* quant);

/**
 * Increments the HAC for that radar and elevation angle.
 * @param[in] scan - input scan
 * @param[in] hac - input field containing hits
 * @param[in] quant - string containing quantity, e.g. "DBZH"
 * @returns int 1 if successful, otherwise 0
 */
int hacIncrement(PolarScan_t* scan, RaveField_t* hac, char* quant);

/**
 * Derives Z-diff quality indicator.
 * @param[in] scan - input scan
 * @returns int 1 if successful, otherwise 0
 */
int zdiff(PolarScan_t* scan, double thresh);

#endif /* ODCHAC_H */
