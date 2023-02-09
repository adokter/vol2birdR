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
 * Object for managing date and time.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-12-16
 */
#ifndef RAVE_DATETIME_H
#define RAVE_DATETIME_H

#include "rave_object.h"

/**
 * Defines a Rave date/time
 */
typedef struct _RaveDateTime_t RaveDateTime_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType RaveDateTime_TYPE;

/**
 * Sets the nominal time.
 * @param[in] dt - self
 * @param[in] value - the time in the format HHmmss
 * @returns 1 on success, otherwise 0
 */
int RaveDateTime_setTime(RaveDateTime_t* dt, const char* value);

/**
 * Returns the nominal time.
 * @param[in] dt - self
 * @returns the nominal time (or NULL if there is none)
 */
const char* RaveDateTime_getTime(RaveDateTime_t* dt);

/**
 * Sets the nominal date.
 * @param[in] dt - self
 * @param[in] value - the date in the format YYYYMMDD
 * @returns 1 on success, otherwise 0
 */
int RaveDateTime_setDate(RaveDateTime_t* dt, const char* value);

/**
 * Returns the nominal date.
 * @param[in] dt - self
 * @returns the nominal time (or NULL if there is none)
 */
const char* RaveDateTime_getDate(RaveDateTime_t* dt);

/**
 * Compares self with another datetime object. If the datetime is equal, 0 is returned, if self is before other a negative
 * value is returned and if self is after other a positive number is returned.
 */
int RaveDateTime_compare(RaveDateTime_t* self, RaveDateTime_t* other);

#endif /* RAVE_DATETIME_H */
