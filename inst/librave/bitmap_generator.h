/* --------------------------------------------------------------------
Copyright (C) 2016 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Provides functionallity for creating a surrounding bitmap on a composite.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2016-12-20
 */
#ifndef BITMAP_GENERATOR_H
#define BITMAP_GENERATOR_H
#include "rave_object.h"
#include "cartesian.h"
#include "cartesianparam.h"
#include "rave_field.h"


/**
 * Defines an object for creating bitmaps
 */
typedef struct _BitmapGenerator_t BitmapGenerator_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType BitmapGenerator_TYPE;

/**
 * Creates a bitmask that surrounds all radars.
 * @param[in] self - self
 * @param[in] param - the cartesian parameter
 * @return a field with the surrounding bitmask
 */
RaveField_t* BitmapGenerator_create_surrounding(BitmapGenerator_t* self, CartesianParam_t* param);

/**
 * Creates a bitmask that shows the intersection between the radars.
 * @param[in] self - self
 * @param[in] param - the cartesian parameter
 * @param[in] qualityFieldName - the name of the quality field in the parameter that consists of the radar index
 * @return a field with the intersections defined.
 */
RaveField_t* BitmapGenerator_create_intersect(BitmapGenerator_t* self, CartesianParam_t* param, const char* qualityFieldName);

#endif
