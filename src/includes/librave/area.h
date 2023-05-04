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
 * Defines an area, the extent, projection, etc.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-12-10
 */
#ifndef AREA_H
#define AREA_H
#include "rave_proj.h"
#include "projection.h"
#include "rave_object.h"

/**
 * Defines a Geographical Area
 */
typedef struct _Area_t Area_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType Area_TYPE;

/**
 * Sets the ID for this area.
 * @param[in] area - self
 * @param[in] id - the id
 * @returns 1 on success otherwise 0
 */
int Area_setID(Area_t* area, const char* id);

/**
 * Returns the ID for this area.
 * @param[in] area - self
 * @returns the id
 */
const char* Area_getID(Area_t* area);

/**
 * Sets the description for this area.
 * @param[in] area - self
 * @param[in] description - the description
 * @returns 1 on success otherwise 0
 */
int Area_setDescription(Area_t* area, const char* description);

/**
 * Returns the description for this area.
 * @param[in] area - self
 * @returns the description
 */
const char* Area_getDescription(Area_t* area);

/**
 * Sets the xsize
 * @param[in] area - the area
 * @param[in] xsize - the xsize
 */
void Area_setXSize(Area_t* area, long xsize);

/**
 * Returns the xsize
 * @param[in] area - self
 * @return the xsize
 */
long Area_getXSize(Area_t* area);

/**
 * Sets the ysize
 * @param[in] area - the area
 * @param[in] ysize - the ysize
 */
void Area_setYSize(Area_t* area, long ysize);

/**
 * Returns the ysize
 * @param[in] area - the area
 * @return the ysize
 */
long Area_getYSize(Area_t* area);

/**
 * Sets the xscale
 * @param[in] area - the area
 * @param[in] xscale - the xscale
 */
void Area_setXScale(Area_t* area, double xscale);

/**
 * Returns the xscale
 * @param[in] area - the area
 * @return the xscale
 */
double Area_getXScale(Area_t* area);

/**
 * Sets the yscale
 * @param[in] area - the area
 * @param[in] yscale - the yscale
 */
void Area_setYScale(Area_t* area, double yscale);

/**
 * Returns the yscale
 * @param[in] area - the area
 * @return the yscale
 */
double Area_getYScale(Area_t* area);

/**
 * Sets the area extent (lower-left, upper-right)
 * @param[in] area - self
 * @param[in] llX - lower left X position
 * @param[in] llY - lower left Y position
 * @param[in] urX - upper right X position
 * @param[in] urY - upper right Y position
 */
void Area_setExtent(Area_t* area, double llX, double llY, double urX, double urY);

/**
 * Returns the area extent (lower-left, upper-right)
 * @param[in] area - self
 * @param[out] llX - lower left X position (may be NULL)
 * @param[out] llY - lower left Y position (may be NULL)
 * @param[out] urX - upper right X position (may be NULL)
 * @param[out] urY - upper right Y position (may be NULL)
 */
void Area_getExtent(Area_t* area, double* llX, double* llY, double* urX, double* urY);

/**
 * Sets the projection that defines this area.
 * @param[in] area - self
 * @param[in] projection - the projection
 */
void Area_setProjection(Area_t* area, Projection_t* projection);

/**
 * Returns the projection that defines this area.
 * @param[in] area - self
 * @returns the projection
 */
Projection_t* Area_getProjection(Area_t* area);

/**
 * The pcsid (projection id) for this area. When setting this
 * id, the projection (if any) will be released unless the
 * pcsid == Projection_getID.
 * @param[in] area - self
 * @param[in] pcsid - the projection id
 * @return 1 on success
 */
int Area_setPcsid(Area_t* area, const char* pcsid);

/**
 * Returns the projection id for this area.
 * @param[in] area - self
 * @returns the projection id
 */
const char* Area_getPcsid(Area_t* area);

#endif /* AREA_H */
