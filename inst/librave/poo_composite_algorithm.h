/* --------------------------------------------------------------------
Copyright (C) 2011 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * POO compositing algorithm.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2011-10-28
 */
#ifndef POO_COMPOSITE_ALGORITHM_H
#define POO_COMPOSITE_ALGORITHM_H
#include "composite_algorithm.h"

/**
 * Defines a Composite generator
 */
typedef struct _PooCompositeAlgorithm_t PooCompositeAlgorithm_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType PooCompositeAlgorithm_TYPE;

/**
 * Implements the name part of the poo composite algorithm
 * @param[in] self - self
 * @returns the unique name of this algorithm
 */
const char* PooCompositeAlgorithm_getName(CompositeAlgorithm_t* self);

/**
 * Resets the internals to be able to handle one pixel in the composite.
 * @param[in] self - self
 * @param[in] x - the x coordinate
 * @param[in] y - the y coordinate
 */
void PooCompositeAlgorithm_reset(CompositeAlgorithm_t* self, int x, int y);

/**
 * Indicates if this algorithm supports process.
 * @returns always 0 (no support)
 */
int PooCompositeAlgorithm_supportsProcess(CompositeAlgorithm_t* self);

/**
 * Implements the processing part of the poo composite algorithm. Prior to this function call,
 * the value/type and distance has already been calculated so instead of letting this function
 * calculate these values again they are provided to this function.
 *
 * @param[in] self - self
 * @param[in] obj - the polar object (currently only scan and volume)
 * @param[in] quantity - the quantity
 * @param[in] olon - the longitude in radians
 * @param[in] olat - the latitude in radians
 * @param[in] dist - the distance from the radar origin to the given lon/lat. Can be calculated by using the obj as well
 * @param[in,out] otype - the type of the data found
 * @param[in,out] ovalue - the value of the data found
 * @param[in] navinfo - the navigation info for the provided obj/olon/olat
 * @return 1 if the catype/cavalue should be used, otherwise 0
 */
int PooCompositeAlgorithm_process(CompositeAlgorithm_t* self, \
  RaveCoreObject* obj, const char* quantity, double olon, double olat, double dist, RaveValueType* otype, double* ovalue, \
  PolarNavigationInfo* navinfo);

/**
 * Initializes self.
 * @param[in] self - self
 * @param[in] composite - the composite we are working with
 * @return 1 on success otherwise 0
 */
int PooCompositeAlgorithm_initialize(CompositeAlgorithm_t* self, struct _Composite_t* composite);

/**
 * Returns that this object supports the fillQualityInformation method for howtask = se.smhi.detector.poo
 * @returns 1 if the provided howtask value is se.smhi.detector.poo, otherwise 0
 */
int PooCompositeAlgorithm_supportsFillQualityInformation(CompositeAlgorithm_t* self, const char* howtask);

/**
 * Fills the quality information in field for howtask values = se.smhi.detector.poo.
 * @param[in] self - self
 * @param[in] obj - the rave core object, most likely a volume or scan
 * @param[in] howtask - the how/task value
 * @param[in] quantity - the quantity
 * @param[in] field - the rave quality field that should get it's value set
 * @param[in] x - the x coordinate in the field
 * @param[in] y - the y coordinate in the field
 * @param[in] navinfo - the navigation information that was used for the provided obj
 * @return 1 on success otherwise 0
 */
int PooCompositeAlgorithm_fillQualityInformation(CompositeAlgorithm_t* self, RaveCoreObject* obj,const char* howtask,const char* quantity,RaveField_t* field,long x, long y, PolarNavigationInfo* navinfo, double gain, double offset);

#endif /* POO_COMPOSITE_ALGORITHM_H */
