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
 * Interface for defining your own compositing algorithm. This object type is used
 * when you need to define your own way of determining if a composite value should
 * be set or not. It is also quite strict when it comes to how to handle the
 * method invocations.
 * When the \ref Composite_nearest or similar is started, the method
 * \ref CompositeAlgorithm_initialize will be called. Then, for each value calculated
 * \ref CompositeAlgorithm_process will be called.
 *
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2011-10-28
 */
#ifndef COMPOSITE_ALGORITHM_H
#define COMPOSITE_ALGORITHM_H

#include "rave_object.h"
#include "rave_types.h"
#include "raveobject_list.h"
#include "rave_field.h"

/**
 * Forward declaration of the composite struct
 */
struct _Composite_t;

/**
 * Forward declaration of struct
 */
struct _CompositeAlgorithm_t;

/**
 * @returns the unique name for this composite algorithm
 */
typedef const char*(*composite_algorithm_getName_fun)(struct _CompositeAlgorithm_t* self);

/**
 * The resetting mechanism that will be called for each new x/y position in the
 * composite. This function might be useful if you want to set some start values.
 */
typedef void(*composite_algorithm_reset_fun)(struct _CompositeAlgorithm_t* self, int x, int y);

/**
 * The supports process function that indicates if process is supported or not.
 */
typedef int(*composite_algorithm_supportsProcess_fun)(struct _CompositeAlgorithm_t* self);

/**
 * The processor function definition.
 *
 * @param[in] self - self
 * @param[in] obj - the polar object (currently only scan and volume)
 * @param[in] quantity - the quantity we currently are working with
 * @param[in] olon - the longitude in radians
 * @param[in] olat - the latitude in radians
 * @param[in] dist - the distance from the radar origin to the given lon/lat.
 * @param[in,out] otype - the type of the data found
 * @param[in,out] ovalue - the value of the data found
 * @param[in] navinfo - the navigation info for the provided obj/olon/olat
 * @returns 1 if this value should be used
 */
typedef int(*composite_algorithm_processor_fun)(struct _CompositeAlgorithm_t* self, \
    RaveCoreObject* obj, const char* quantity, double olon, double olat, double dist ,RaveValueType* otype, double* ovalue, \
    PolarNavigationInfo* navinfo);


/**
 * The initializing function so that we know what composite we are working with. Note, this
 * function will be called by the composite module and will override any previous calls.
 * @return 1 on success otherwise 0
 */
typedef int(*composite_algorithm_initialize_fun)(struct _CompositeAlgorithm_t* self, struct _Composite_t* composite);

/**
 * Function to be used when querying if this algorithm supports the quality field with how/task value
 */
typedef int(*composite_algorithm_supportsFillQualityInformation_fun)(struct _CompositeAlgorithm_t* self, const char* howtask);

/**
 * Function to be used when filling quality information into a rave field at pos x,y-
 * @param[in] self - self
 * @param[in] obj - the object that was selected for setting the composite value
 * @param[in] howtask - the how/task value  defining what quality attribute we are processing
 * @param[in] quantity - the quantity we are working with
 * @param[in] field - the quality field to be set
 * @param[in] x - the x position to be set in the field
 * @param[in] y - the y position to be set in the field
 * @param[in] navinfo - the navigation information that was used within the rave object
 * @return 1 on success otherwise 0
 */
typedef int(*composite_algorithm_fillQualityInformation_fun)(struct _CompositeAlgorithm_t* self, RaveCoreObject* obj, const char* howtask, const char* quantity, RaveField_t* field, long x, long y, PolarNavigationInfo* navinfo, double gain, double offset);

/**
 * The head part for a CompositeAlgorithm subclass. Should be placed directly under
 * RAVE_OBJECT_HEAD like in CompositeAlgorithm_t.
 */
#define COMPOSITE_ALGORITHM_HEAD \
  composite_algorithm_getName_fun getName; \
  composite_algorithm_supportsProcess_fun supportsProcess; \
  composite_algorithm_processor_fun process; \
  composite_algorithm_initialize_fun initialize; \
  composite_algorithm_reset_fun reset; \
  composite_algorithm_supportsFillQualityInformation_fun supportsFillQualityInformation; \
  composite_algorithm_fillQualityInformation_fun fillQualityInformation;

/**
 * The basic composite algorithm that can be cast into a subclassed processor.
 */
typedef struct _CompositeAlgorithm_t {
  RAVE_OBJECT_HEAD /**< Always on top */
  COMPOSITE_ALGORITHM_HEAD /**< composite specifics */
} CompositeAlgorithm_t;

/**
 * Macro expansion for calling the name function
 * @param[in] self - self
 * @returns the unique name for this algorithm
 */
#define CompositeAlgorithm_getName(self) \
  ((CompositeAlgorithm_t*)self)->getName((CompositeAlgorithm_t*)self)

/**
 * Macro expansion for calling the process function
 * @param[in] self - self
 * @returns 1 if this value should be considered
 */
#define CompositeAlgorithm_reset(self, x, y) \
    ((CompositeAlgorithm_t*)self)->reset((CompositeAlgorithm_t*)self, x, y)

/**
 * Macro expansion if this algorithm supports process or not
 */
#define CompositeAlgorithm_supportsProcess(self) \
    ((CompositeAlgorithm_t*)self)->supportsProcess((CompositeAlgorithm_t*)self)

/**
 * Macro expansion for calling the process function
 * @param[in] self - self
 * @param[in] obj - the polar object (currently only scan and volume)
 * @param[in] quantity - the quantity we currently are working with
 * @param[in] olon - the longitude in radians
 * @param[in] olat - the latitude in radians
 * @param[in] dist - the distance from the radar origin to the given lon/lat.
 * @param[in,out] otype - the type of the data found
 * @param[in,out] ovalue - the value of the data found
 * @param[in] navinfo - the navigation info for the provided obj/olon/olat
 * @returns 1 if this value should be considered
 */
#define CompositeAlgorithm_process(self,obj,quantity,olon,olat,dist,otype,ovalue,navinfo) \
    ((CompositeAlgorithm_t*)self)->process((CompositeAlgorithm_t*)self,obj,quantity,olon,olat,dist,otype,ovalue,navinfo)

/**
 * Macro expansion for initializing the algorithm
 * @param[in] self - self
 * @param[in] composite - the composite this algorithm will be working with
 * @returns 1 on success otherwise 0
 */
#define CompositeAlgorithm_initialize(self, objects) \
    ((CompositeAlgorithm_t*)self)->initialize((CompositeAlgorithm_t*)self, composite)

/**
 * Macro expansion if this algorithm supports process or not
 */
#define CompositeAlgorithm_supportsFillQualityInformation(self,howtask) \
    ((CompositeAlgorithm_t*)self)->supportsFillQualityInformation((CompositeAlgorithm_t*)self,howtask)

/**
 * Macro expansion if this algorithm supports process or not
 */
#define CompositeAlgorithm_fillQualityInformation(self,obj,howtask,quantity,field,x,y,navinfo,gain,offset) \
    ((CompositeAlgorithm_t*)self)->fillQualityInformation((CompositeAlgorithm_t*)self,obj,howtask,quantity,field,x,y,navinfo,gain,offset)

#endif /* COMPOSITE_ALGORITHM_H */
