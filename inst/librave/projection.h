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
 * Wrapper around PROJ.4.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-10-20
 */
#ifndef PROJECTION_H
#define PROJECTION_H

#include "rave_object.h"
#include "rave_proj.h"

/**
 * Defines a transformer
 */
typedef struct _Projection_t Projection_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType Projection_TYPE;

/**
 * Sets the debug level when using proj API
 * @param[in] debugPj - Value between 0 (NONE) to 3 (FULL) and 4 (TELL?)
 */
void Projection_setDebugLevel(int debugPj);

/**
 * Returns the debug level when using proj API
 * @return Value between 0 (NONE) to 3 (FULL) and 4 (TELL?)
 */
int Projection_getDebugLevel(void);

/**
 * Returns the currently used Proj version.
 * @returns the proj version
 */
const char* Projection_getProjVersion(void);

/**
 * Sets the default lon/lat proj definition to use. Default is '+proj=longlat +ellps=WGS84 +datum=WGS84'
  * @param[in] projdef - A proj string, not longer than 1023 chars
 */
void Projection_setDefaultLonLatProjDef(const char* projdef);

/**
 * Returns the default lonlat proj definition
 * @return the default lonlat proj definition
 */
const char* Projection_getDefaultLonLatProjDef(void);

/**
 * Initializes a projection with the projection
 */
int Projection_init(Projection_t* projection, const char* id, const char* description, const char* definition);

/**
 * Creates a projection directly. Like writing:
 * Projection_t* p = RAVE_OBJECT_NEW(&Projection_TYPE);
 * if (p != NULL) {
 *   if (!Projection_init(p, id, description, definition)) {
 *     RAVE_OBJECT_RELEASE(p);
 *   }
 * }
 * @param[in] id - the id
 * @param[in] description - the description
 * @param[in] definition - the definition
 * @return the created projection
 */
Projection_t* Projection_create(const char* id, const char* description, const char* definition);

/**
 * Creates a default lon lat projection by using the default lonlat pcs definition
 * @returns the projection or NULL on failure
 */
Projection_t* Projection_createDefaultLonLatProjection(void);

/**
 * Returns the ID for this projection.
 * @param[in] projection - the projection
 * @return the ID for this projection
 */
const char* Projection_getID(Projection_t* projection);

/**
 * Returns the description for this projection.
 * @param[in] projection - the projection
 * @return the description for this projection
 */
const char* Projection_getDescription(Projection_t* projection);

/**
 * Returns the definition for this projection.
 * @param[in] projection - the projection
 * @return the definition for this projection
 */
const char* Projection_getDefinition(Projection_t* projection);

/**
 * Returns if this projection is a latlong or not
 * @param[in] projection - self
 * @return if this is a latlong projection or not
 */
int Projection_isLatLong(Projection_t* projection);

/**
 * Transforms the coordinates in this projection into the target projection.
 * @param[in] projection - this projection
 * @param[in] tgt - target projection
 * @param[in,out] x - coordinate
 * @param[in,out] y - coordinate
 * @param[in,out] z - coordinate (MAY BE NULL in some cases), see PROJ.4
 * @param[in] projection - the projection
 * @return 0 on failure, otherwise success
 */
int Projection_transform(Projection_t* projection, Projection_t* tgt, double* x, double* y, double* z);

/**
 * This is an alternate version of \ref #Projection_transform. This function
 * will set the output valuesin ox, oy and oz respectively. x/y and ox/oy are
 * always required. Some projections requires the z values as well but
 * that is not enforced by this function and is up to the user to manage properly.
 * If oz == NULL, then the transform will atempt to project without the z value.
 * @param[in] projection - self
 * @param[in] tgt - the target projection
 * @param[in] x - coordinate
 * @param[in] y - coordinate
 * @param[in] z - coordinate
 * @param[out] ox - coordinate
 * @param[out] oy - coordinate
 * @param[out] oz - coordinate
 * @returns 1 on success, otherwise 0
 */
int Projection_transformx(Projection_t* projection, Projection_t* tgt,
  double x, double y, double z, double* ox, double* oy, double* oz);

/**
 * Translates surface coordinate into lon/lat.
 * @param[in] projection - the projection
 * @param[in]    x - the x coordinate
 * @param[in]    y - the y coordinate
 * @param[out] lon - the longitude (in radians)
 * @param[out] lat - the latitude  (in radians)
 * @return 0 on failure otherwise success
 */
int Projection_inv(Projection_t* projection, double x, double y, double* lon, double* lat);

/**
 * Translates lon/lat coordinate into a surface coordinate.
 * @param[in] projection - the projection
 * @param[in] lon - the longitude (in radians)
 * @param[in] lat - the latitude (in radians)
 * @param[out]  x - the x coordinate
 * @param[out]  y - the y coordinate
 * @return 0 on failure otherwise success
 */
int Projection_fwd(Projection_t* projection, double lon, double lat, double* x, double* y);

#endif
