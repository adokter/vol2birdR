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
 * Defines the functions available when working with cartesian volumes.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 *
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-06-23
 */
#ifndef CARTESIANVOLUME_H
#define CARTESIANVOLUME_H
#include "projection.h"
#include "rave_object.h"
#include "rave_types.h"
#include "raveobject_list.h"
#include "cartesian.h"

/**
 * Defines a Cartesian volume
 */
typedef struct _CartesianVolume_t CartesianVolume_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType CartesianVolume_TYPE;

/**
 * Sets the nominal time.
 * @param[in] cvol - self
 * @param[in] value - the time in the format HHmmss
 * @returns 1 on success, otherwise 0
 */
int CartesianVolume_setTime(CartesianVolume_t* cvol, const char* value);

/**
 * Returns the nominal time.
 * @param[in] cvol - self
 * @returns the nominal time (or NULL if there is none)
 */
const char* CartesianVolume_getTime(CartesianVolume_t* cvol);

/**
 * Sets the nominal date.
 * @param[in] cvol - self
 * @param[in] value - the date in the format YYYYMMDD
 * @returns 1 on success, otherwise 0
 */
int CartesianVolume_setDate(CartesianVolume_t* cvol, const char* value);

/**
 * Returns the nominal date.
 * @param[in] cvol - self
 * @returns the nominal time (or NULL if there is none)
 */
const char* CartesianVolume_getDate(CartesianVolume_t* cvol);

/**
 * Sets the source.
 * @param[in] cvol - self
 * @param[in] value - the source
 * @returns 1 on success, otherwise 0
 */
int CartesianVolume_setSource(CartesianVolume_t* cvol, const char* value);

/**
 * Returns the source.
 * @param[in] cvol - self
 * @returns the source or NULL if there is none
 */
const char* CartesianVolume_getSource(CartesianVolume_t* cvol);

/**
 * Sets the object type this cartesian volume should represent.
 * @param[in] cvol - self
 * @param[in] type - the object type
 * @returns 1 if the specified object type is supported, otherwise 0
 */
int CartesianVolume_setObjectType(CartesianVolume_t* cvol, Rave_ObjectType type);

/**
 * Returns the object type this cartesian volume represents.
 * @param[in] cvol - self
 * @returns the object type
 */
Rave_ObjectType CartesianVolume_getObjectType(CartesianVolume_t* cvol);

/**
 * Sets the projection that defines this cartesian volume.
 * @param[in] cvol - self
 * @param[in] projection - the projection
 */
void CartesianVolume_setProjection(CartesianVolume_t* cvol, Projection_t* projection);

/**
 * Returns a copy of the projection that is used for this cartesian volume.
 * I.e. remember to release it.
 * @param[in] cvol - self
 * @returns a projection (or NULL if none is set)
 */
Projection_t* CartesianVolume_getProjection(CartesianVolume_t* cvol);

/**
 * Returns the projection string defining this cartesian volume.
 * @param[in] cvol - self
 * @return the projection string or NULL if none defined
 */
const char* CartesianVolume_getProjectionString(CartesianVolume_t* cvol);

/**
 * Sets the xscale
 * @param[in] cvol - self
 * @param[in] xscale - the xscale
 */
void CartesianVolume_setXScale(CartesianVolume_t* cvol, double xscale);

/**
 * Returns the xscale
 * @param[in] cvol - self
 * @return the xscale
 */
double CartesianVolume_getXScale(CartesianVolume_t* cvol);

/**
 * Sets the yscale
 * @param[in] cvol - self
 * @param[in] yscale - the yscale
 */
void CartesianVolume_setYScale(CartesianVolume_t* cvol, double yscale);

/**
 * Returns the yscale
 * @param[in] cvol - self
 * @return the yscale
 */
double CartesianVolume_getYScale(CartesianVolume_t* cvol);

/**
 * Sets the zscale
 * @param[in] cvol - self
 * @param[in] zscale - the zscale
 */
void CartesianVolume_setZScale(CartesianVolume_t* cvol, double zscale);

/**
 * Returns the zscale
 * @param[in] cvol - self
 * @return the zscale
 */
double CartesianVolume_getZScale(CartesianVolume_t* cvol);

/**
 * Sets the zstart
 * @param[in] cvol - self
 * @param[in] zstart - the zstart
 */
void CartesianVolume_setZStart(CartesianVolume_t* cvol, double zstart);

/**
 * Returns the zstart
 * @param[in] cvol - self
 * @return the zstart
 */
double CartesianVolume_getZStart(CartesianVolume_t* cvol);

/**
 * Returns the xsize
 * @param[in] cvol - self
 * @return the xsize
 */
long CartesianVolume_getXSize(CartesianVolume_t* cvol);

/**
 * Returns the ysize
 * @param[in] cvol - self
 * @return the ysize
 */
long CartesianVolume_getYSize(CartesianVolume_t* cvol);

/**
 * Returns the zsize, should be same as number of images in volume
 * @param[in] cvol - self
 * @return the zsize
 */
long CartesianVolume_getZSize(CartesianVolume_t* cvol);

/**
 * Sets the area extent for this cartesian product.
 * @param[in] cvol - self
 * @param[in] llX - lower left X position
 * @param[in] llY - lower left Y position
 * @param[in] urX - upper right X position
 * @param[in] urY - upper right Y position
 */
void CartesianVolume_setAreaExtent(CartesianVolume_t* cvol, double llX, double llY, double urX, double urY);

/**
 * Gets the area extent for this cartesian product.
 * @param[in] cvol - self
 * @param[out] llX - lower left X position (may be NULL)
 * @param[out] llY - lower left Y position (may be NULL)
 * @param[out] urX - upper right X position (may be NULL)
 * @param[out] urY - upper right Y position (may be NULL)
 */
void CartesianVolume_getAreaExtent(CartesianVolume_t* cvol, double* llX, double* llY, double* urX, double* urY);

/**
 * Adds a cartesian image to the volume. The image will automatically be assigned the
 * volumes projection.
 * @param[in] cvol - self
 * @param[in] image - the cartesian image
 * ®return 0 on failure, otherwise success
 */
int CartesianVolume_addImage(CartesianVolume_t* cvol, Cartesian_t* image);

/**
 * Returns the image at given index.
 * @param[in] cvol - the volume
 * @param[in] index - the index
 * @returns the cartesian image at the specified index or NULL on failure.
 */
Cartesian_t* CartesianVolume_getImage(CartesianVolume_t* cvol, int index);

/**
 * Returns the number of images.
 * @param[in] cvol - the volume
 * @returns -1 on failure, otherwise a value >= 0
 */
int CartesianVolume_getNumberOfImages(CartesianVolume_t* cvol);

/**
 * Adds a rave attribute to the volume.
 * what/date, what/time, what/source, where/lon, where/lat and where/height
 * are handled specially and will be added to respective member instead
 * of stored as attributes.
 * what/date, what/time and what/source must be string.
 * where/lon and where/lat must be double in degrees.
 * where/height must be double in meters.
 * @param[in] cvol - self
 * @param[in] attribute - the attribute
 * @return 1 on success otherwise 0
 */
int CartesianVolume_addAttribute(CartesianVolume_t* cvol, RaveAttribute_t* attribute);

/**
 * Returns the rave attribute that is named accordingly.
 * @param[in] pvol - self
 * @param[in] name - the name of the attribute
 * @returns the attribute if found otherwise NULL
 */
RaveAttribute_t* CartesianVolume_getAttribute(CartesianVolume_t* cvol,  const char* name);

/**
 * Returns if the specified attribute exists in the how-attributes or not.
 * @param[in] cvol - self
 * @param[in] name - the name of the attribute
 * @returns 1 if it exists, otherwise 0
 */
int CartesianVolume_hasAttribute(CartesianVolume_t* cvol,  const char* name);

/**
 * Returns a list of attribute names. Release with \@ref #RaveList_freeAndDestroy.
 * @param[in] pvol - self
 * @returns a list of attribute names
 */
RaveList_t* CartesianVolume_getAttributeNames(CartesianVolume_t* cvol);

/**
 * Returns a list of attribute values that should be stored for this volume.
 * @param[in] pvol - self
 * @returns a list of RaveAttributes.
 */
RaveObjectList_t* CartesianVolume_getAttributeValues(CartesianVolume_t* cvol);

#endif /* CARTESIANVOLUME_H */
