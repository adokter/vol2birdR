/* --------------------------------------------------------------------
Copyright (C) 2021 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Handles compatibility issues related to PROJ versions and the definitions.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2021-10-27
 */
#ifndef RAVE_PROJ_H
#define RAVE_PROJ_H

#include <math.h>

#ifdef USE_PROJ4_API
#include <proj_api.h>
#ifdef PJ_VERSION
#define UV projUV
#define PJ projPJ
#endif
#else
#include <proj.h>
#define UV PJ_UV
#endif

#endif
