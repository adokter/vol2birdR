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
 * Defines the functions available when transforming between different
 * types of products.
 * This object does NOT support \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-10-20
 */
#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "rave_proj.h"
#include "polarvolume.h"
#include "cartesian.h"
#include "radardefinition.h"
#include "area.h"

/**
 * Defines a transformer
 */
typedef struct _Transform_t Transform_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType Transform_TYPE;

/**
 * Sets the transformation method to be used, like NEAREST, BILINEAR, ...
 * @param[in] transform - the transformer
 * @param[in] method - the transformation method
 * @return 0 if method is not known, otherwise the method was set
 */
int Transform_setMethod(Transform_t* transform, RaveTransformationMethod method);

/**
 * Returns the transformation method.
 * @param[in] transform - the transformer
 * @return the transformation method
 */
RaveTransformationMethod Transform_getMethod(Transform_t* transform);

/**
 * Creates a ppi from a polar scan.
 * @param[in] transform - the transformer
 * @param[in] scan - the polar scan
 * @param[in] cartesian - the cartesian product
 * @returns 0 on failure, otherwise success
 */
int Transform_ppi(Transform_t* transform, PolarScan_t* scan, Cartesian_t* cartesian);

/**
 * Creates a cappi from a polar volume
 * @param[in] transform - the transformer
 * @param[in] pvol - the polar volume
 * @param[in] cartesian - the cartesian product
 * @param[in] height - the height of the cappi
 * @returns 0 on failure, otherwise success
 */
int Transform_cappi(Transform_t* transform, PolarVolume_t* pvol, Cartesian_t* cartesian, double height);

/**
 * Creates a pseudo-cappi from a polar volume
 * @param[in] transform - the transformer
 * @param[in] pvol - the polar volume
 * @param[in] cartesian - the cartesian product
 * @param[in] height - the height of the cappi
 * @returns 0 on failure, otherwise success
 */
int Transform_pcappi(Transform_t* transform, PolarVolume_t* pvol, Cartesian_t* cartesian, double height);

/**
 * Mirrors a cartesian product into a polar scan.
 * @param[in] transform - self
 * @param[in] cartesian - the source
 * @param[in] def - the scan definition
 * @param[in] angle - the elevation angle that should be used
 * @param[in] quantity - what parameter that should be transformed
 * @return the scan on success otherwise NULL
 */
PolarScan_t* Transform_ctoscan(Transform_t* transform, Cartesian_t* cartesian, RadarDefinition_t* def, double angle, const char* quantity);

/**
 * Mirrors a cartesian product into a polar volume.
 * @param[in] transform - self
 * @param[in] cartesian - the source
 * @param[in] def - the volume definition
 * @param[in] quantity - what parameter that should be transformed
 * @return the volume on success otherwise NULL
 */
PolarVolume_t* Transform_ctop(Transform_t* transform, Cartesian_t* cartesian, RadarDefinition_t* def, const char* quantity);

/**
 * Fills the gaps in a cartesian product.
 * @param[in] transform - self
 * @param[in] cartesian - the product that should be gap filled
 * @returns the filled cartesian or NULL on failure
 */
Cartesian_t* Transform_fillGap(Transform_t* transform, Cartesian_t* cartesian);

/**
 * Fills the gaps in a cartesian parameter.
 * @param[in] transform - self
 * @param[in] param - the parameter that should be gap filled
 * @returns the filled cartesian or NULL on failure
 */
CartesianParam_t* Transform_fillGapOnParameter(Transform_t* transform, CartesianParam_t* param);

/**
 * Accumulated precipitation from reflectivity composites.
 * @param[in] self - self
 * @param[in] param - the cartesian parameter. Should contain
 */
CartesianParam_t* Transform_accumulate(Transform_t* self, CartesianParam_t* param, double zr_a, double zr_b);

/**
 * Combines a number of tiles into the area defined by area. This function should not be mixed up with
 * functionality in cartesian composite. Instead this function will in a way concatenate the individual
 * tiles into a big one. There is no overlapping functionallity or other clever ways to combine the different
 * tiles.
 * @param[in] self - self
 * @param[in] area - the resulting area
 * @param[in] tiles - the list of tiles
 * @return the complete cartesian product
 */
Cartesian_t* Transform_combine_tiles(Transform_t* self, Area_t* area, RaveObjectList_t* tiles);

#endif
