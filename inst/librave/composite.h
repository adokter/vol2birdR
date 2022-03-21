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
 * Provides functionality for creating composites.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-01-19
 */
#ifndef COMPOSITE_H
#define COMPOSITE_H

#include "rave_object.h"
#include "rave_types.h"
#include "cartesian.h"
#include "area.h"
#include "composite_algorithm.h"
#include "raveobject_hashtable.h"
#include "limits.h"

#define COMPOSITE_QUALITY_FIELDS_GAIN   (1.0/UCHAR_MAX)
#define COMPOSITE_QUALITY_FIELDS_OFFSET 0.0

/**
 * What type of selection variant to use
 */
typedef enum CompositeSelectionMethod_t {
  CompositeSelectionMethod_NEAREST = 0, /**< Nearest radar defines pixel to use (default) */
  CompositeSelectionMethod_HEIGHT
} CompositeSelectionMethod_t;

/**
 * What type of interpolation method that is used to set values in the composite between the
 * discrete value positions of the input data
 */
typedef enum CompositeInterpolationMethod_t {
  /**< Nearest value is used */
  CompositeInterpolationMethod_NEAREST = 0,
  /**< Value calculated by performing a linear interpolation between the closest positions
   * above and below  */
  CompositeInterpolationMethod_LINEAR_HEIGHT,
  /**< Value calculated by performing a linear interpolation between the closest positions
   * before and beyond in the range dimension of the ray  */
  CompositeInterpolationMethod_LINEAR_RANGE,
  /**< Value calculated by performing a linear interpolation between the closest positions
   * on each side of the position, i.e., interpolation between consecutive rays  */
  CompositeInterpolationMethod_LINEAR_AZIMUTH,
  /**< Value calculated by performing a linear interpolation in azimuth and range
   * directions  */
  CompositeInterpolationMethod_LINEAR_RANGE_AND_AZIMUTH,
  /**< Value calculated by performing a linear interpolation in height, azimuth and range
   * directions  */
  CompositeInterpolationMethod_LINEAR_3D,
  /**< Value calculated by performing a quadratic interpolation between the closest positions
   * before and beyond in the range dimension of the ray. Quadratic interpolation means that
   * inverse distance weights raised to the power of 2 are used in value interpolation. */
  CompositeInterpolationMethod_QUADRATIC_HEIGHT,
  /**< Value calculated by performing a quadratic interpolation in height, azimuth and range
   * directions. Quadratic interpolation means that inverse distance weights raised to the
   * power of 2 are used in value interpolation. */
  CompositeInterpolationMethod_QUADRATIC_3D
} CompositeInterpolationMethod_t;

/**
 * Defines a Composite generator
 */
typedef struct _Composite_t Composite_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType Composite_TYPE;

/**
 * Adds one RaveCoreObject, currently, the only supported type is volumes but
 * this might be enhanced in the future to also allow for cartesian products
 * to be added.
 * @param[in] composite - self
 * @param[in] object - the item to be added to the composite
 * @returns 1 on success, otherwise 0
 */
int Composite_add(Composite_t* composite, RaveCoreObject* object);

/**
 * Returns the number of objects this composite will process
 * @param[in] composite - self
 * @return the number of objects
 */
int Composite_getNumberOfObjects(Composite_t* composite);

/**
 * Return the object at position index.
 * @param[in] composite - self
 * @param[in] index - the index, should be >= 0 and < getNumberOfObjects
 * @return the object or NULL if outside range
 */
RaveCoreObject* Composite_get(Composite_t* composite, int index);

/**
 * Return the radar index value that has been assigned to the object as position index.
 * @param[in] composite - self
 * @param[in] index - the index, should be >= 0 and < getNumberOfObjects
 * @return the radar index or 0 if no index has been assigned yet.
 */
int Composite_getRadarIndexValue(Composite_t* composite, int index);

/**
 * Sets the product type that should be generated when generating the
 * composite.
 * Height/Elevation angle and range are used in combination with the products.
 * PPI requires elevation angle
 * CAPPI, PCAPPI and PMAX requires height above sea level
 * PMAX also requires range in meters
 *
 * @param[in] composite - self
 * @param[in] type - the product type, PPI, CAPPI, PCAPPI and PMAX are currently supported.
 */
void Composite_setProduct(Composite_t* composite, Rave_ProductType type);

/**
 * Returns the product type
 * @returns the product type
 */
Rave_ProductType Composite_getProduct(Composite_t* composite);

/**
 * Sets the selection method to use. @see \ref #CompositeSelectionMethod_t.
 * @param[in] self - self
 * @param[in] method - the method to use
 * @return 1 on success otherwise 0
 */
int Composite_setSelectionMethod(Composite_t* self, CompositeSelectionMethod_t method);

/**
 * Returns the selection method. @see \ref #CompositeSelectionMethod_t
 * @param[in] self - self
 * @return the selection method
 */
CompositeSelectionMethod_t Composite_getSelectionMethod(Composite_t* self);

/**
 * Sets the interpolation method to use. @see \ref #CompositeInterpolationMethod_t.
 * @param[in] self - self
 * @param[in] interpolationMethod - the interpolation method to use
 * @return 1 on success otherwise 0
 */
int Composite_setInterpolationMethod(Composite_t* self, CompositeInterpolationMethod_t interpolationMethod);

/**
 * Returns the interpolation method. @see \ref #CompositeInterpolationMethod_t
 * @param[in] self - self
 * @return the interpolation method
 */
CompositeInterpolationMethod_t Composite_getInterpolationMethod(Composite_t* self);

/**
 * Sets the height that should be used when generating a
 * composite as CAPPI, PCAPPI or PMAX.
 * @param[in] composite - self
 * @param[in] height - the height
 */
void Composite_setHeight(Composite_t* composite, double height);

/**
 * Returns the height that is used for composite generation.
 * @param[in] composite - self
 * @returns the height
 */
double Composite_getHeight(Composite_t* composite);

/**
 * Sets the elevation angle that should be used when generating a
 * composite as PPI.
 * @param[in] composite - self
 * @param[in] angle - the angle in radians
 */
void Composite_setElevationAngle(Composite_t* composite, double angle);

/**
 * Returns the elevation angle that is used for composite generation.
 * @param[in] composite - self
 * @returns the elevation angle in radians
 */
double Composite_getElevationAngle(Composite_t* composite);

/**
 * Sets the range that should be used when generating the Pseudo MAX. This range
 * is the limit in meters for when the vertical max should be used. When outside
 * this range, the PCAPPI value is used instead.
 *
 * @param[in] composite - self
 * @param[in] angle - the range in meters
 */
void Composite_setRange(Composite_t* composite, double range);

/**
 * Returns the range in meters
 * @param[in] composite - self
 * @returns the range in meters
 */
double Composite_getRange(Composite_t* composite);

/**
 * If this field name is set, then the composite will be generated by first using the
 * quality indicator field for determining radar usage. If the field name is NULL, then
 * the selection method will be used instead.
 * @param[in] self - self
 * @param[in] qiFieldName - the quality indicator field name
 * @return 1 on success, 0 on failure (for example on memory allocation error)
 */
int Composite_setQualityIndicatorFieldName(Composite_t* self, const char* qiFieldName);

/**
 * @param[in] self - self
 * @return the quality indicator field name, can be NULL
 */
const char* Composite_getQualityIndicatorFieldName(Composite_t* self);

/**
 * Adds a parameter to be processed.
 * @param[in] composite - self
 * @param[in] quantity - the parameter quantity
 * @param[in] gain - the gain to be used for the parameter
 * @param[in] offset - the offset to be used for the parameter
 * @param[in] minvalue - the minimum value that can be represented for this
 *                       quantity in the composite
 * @return 1 on success
 */
int Composite_addParameter(Composite_t* composite, const char* quantity, double gain, double offset, double minvalue);

/**
 * Returns if this composite generator is going to process specified parameter
 * @param[in] composite - self
 * @param[in] quantity - the parameter quantity
 * @return 1 if yes otherwise 0
 */
int Composite_hasParameter(Composite_t* composite, const char* quantity);

/**
 * Returns the number of parameters to be processed
 * @param[in] composite - self
 * @return the number of parameters
 */
int Composite_getParameterCount(Composite_t* composite);

/**
 * Returns the parameter at specified index
 * @param[in] composite - self
 * @param[in] index - the index
 * @param[out] gain - the gain to be used for the parameter (MAY BE NULL)
 * @param[out] offset - the offset to be used for the parameter (MAY BE NULL)
 * @return the parameter name
 */
const char* Composite_getParameter(Composite_t* composite, int index, double* gain, double* offset);

/**
 * Sets the nominal time.
 * @param[in] composite - self
 * @param[in] value - the time in the format HHmmss
 * @returns 1 on success, otherwise 0
 */
int Composite_setTime(Composite_t* composite, const char* value);

/**
 * Returns the nominal time.
 * @param[in] composite - self
 * @returns the nominal time (or NULL if there is none)
 */
const char* Composite_getTime(Composite_t* composite);

/**
 * Sets the nominal date.
 * @param[in] composite - self
 * @param[in] value - the date in the format YYYYMMDD
 * @returns 1 on success, otherwise 0
 */
int Composite_setDate(Composite_t* composite, const char* value);

/**
 * Returns the nominal date.
 * @param[in] composite - self
 * @returns the nominal time (or NULL if there is none)
 */
const char* Composite_getDate(Composite_t* composite);

/**
 * If you want the objects included in the composite to have a specific index value when generating the
 * quality field se.smhi.composite.index.radar, then you can provide a hash table that maps source with
 * a RaveAttribute_t containing a long value. The source should be the full source as defined in the
 * added objects. The indexes must be unique values, preferrably starting from 1. If there is a mapping
 * missing, the default behaviour is to take first available integer closest to 1.
 *
 * Note, that in order to the mapping to take, this call must be performed after all the objects has
 * been added to the generator and before calling \ref Composite_generate.
 *
 * @param[in] composite - self
 * @param[in] mapping - the source - index mapping
 * @return 1 on success, otherwise 0.
 */
int Composite_applyRadarIndexMapping(Composite_t* composite, RaveObjectHashTable_t* mapping);

/**
 * Generates a composite according to the configured parameters in the composite structure.
 * @param[in] composite - self
 * @param[in] area - the area that should be used for defining the composite.
 * @param[in] qualityflags - A list of char pointers identifying how/task values in the quality fields of the polar data.
 *            Each entry in this list will result in the atempt to generate a corresponding quality field
 *            in the resulting cartesian product. (MAY BE NULL)
 * @returns the generated composite.
 */
Cartesian_t* Composite_generate(Composite_t* composite, Area_t* area, RaveList_t* qualityflags);

/**
 * Sets the algorithm to use when generating the composite.
 * @param[in] composite - self
 * @param[in] algorithm - the actual algorithm to be run (MAY BE NULL, indicating nothing particular should be done)
 */
void Composite_setAlgorithm(Composite_t* composite, CompositeAlgorithm_t* algorithm);

/**
 * Returns the currently used algorithm.
 * @param[in] composite - self
 * @return the algorithm (or NULL)
 */
CompositeAlgorithm_t* Composite_getAlgorithm(Composite_t* composite);
#endif /* COMPOSITE_H */
