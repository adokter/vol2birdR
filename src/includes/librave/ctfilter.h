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
/** Functionality for identifying and removing residual non-precipitation
 * echoes in Cartesian radar products, using the standard MSG-based
 * SAF-NWC cloud-top product available through EUMETCAST in HDF5.
 * @file
 * @author Daniel Michelson, SMHI
 * @date 2014-03-25
 */
#ifndef CTFILTER_H
#define CTFILTER_H

#include "cartesian.h"
#include "cartesianparam.h"
#include "rave_field.h"
#include "rave_attribute.h"
#include "rave_object.h"
#include "rave_alloc.h"
#include "rave_types.h"
#include "raveutil.h"
#include "rave_debug.h"


/**
 * Filters product with cloud-top information. A quality field is created
 * and added to the input product, containing removed echoes.
 * Pixel values are from the CT product header. Probabilities of rain from:
 * Dybbroe et al. 2005: NWCSAF AVHRR Cloud Detection and Analysis Using
 * Dynamic Thresholds and Radiative Transfer Modelling. Part II. Tuning
 * and Validation. J. Appl. Meteor. 44. p. 55-71. Table 11, page 69.
 * Yes, we know the article addresses AVHRR and we are addressing MSG ...
 * @param[in] product - input Cartesian image product
 * @param[in] ct - input Cartesian cloud-type product
 * @returns int 1 if image is filtered, otherwise 0
 */
int ctFilter(Cartesian_t* product, Cartesian_t* ct);


#endif /* CTFILTER_H */
