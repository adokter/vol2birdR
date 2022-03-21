/* --------------------------------------------------------------------
Copyright (C) 2014 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Implementation of the QI-total algorithm
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2014-02-27
 */
#ifndef RAVE_QITOTAL_H
#define RAVE_QITOTAL_H
#include "rave_object.h"
#include "raveobject_list.h"
#include "rave_field.h"

/**
 * Defines QI total
 */
typedef struct _RaveQITotal_t RaveQITotal_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType RaveQITotal_TYPE;

/**
 * The data type to use for the resulting field
 * @param[in] self - self
 * @param[in] dtype - the data type
 */
void RaveQITotal_setDatatype(RaveQITotal_t* self, RaveDataType dtype);

/**
 * @param[in] self - self
 * @return the data type
 */
RaveDataType RaveQITotal_getDatatype(RaveQITotal_t* self);

/**
 * The gain to use for the resulting field
 * @param[in] self - self
 * @param[in] gain - the gain
 */
int RaveQITotal_setGain(RaveQITotal_t* self, double gain);

/**
 * @param[in] self - self
 * @return the gain
 */
double RaveQITotal_getGain(RaveQITotal_t* self);

/**
 * The offset to use for the resulting field
 * @param[in] self - self
 * @param[in] offset - the offset
 */
void RaveQITotal_setOffset(RaveQITotal_t* self, double offset);

/**
 * @param[in] self - self
 * @return the offset
 */
double RaveQITotal_getOffset(RaveQITotal_t* self);

/**
 * Sets the weight for the specified how/task. The weights are used differently for the different algorithms, please
 * look at the corresponding function on how the weights are used.
 * @param[in] howtask the how/task name for the field
 * @param[in] w the weight, must be >= 0 and preferably below 1.0.
 * @return 1 if the how/task - weight could be defined, otherwise 0
 */
int RaveQITotal_setWeight(RaveQITotal_t* self, const char* howtask, double w);

/**
 * Returns the weight for the specified how/task.
 * @param[in] self - self
 * @param[in] howtask the how/task name for the field
 * @param[out] w the weight for specified how/task.
 * @return 1 if the how/task could be found, otherwise 0
 */
int RaveQITotal_getWeight(RaveQITotal_t* self, const char* howtask, double* w);

/**
 * Removes the weight for the specified how/task
 * @param[in] self - self
 * @param[in] howtas - the how/task name for the field
 */
void RaveQITotal_removeWeight(RaveQITotal_t* self, const char* howtask);

/**
 * Performs the multiplicative QI total algorithm.
 * @param[in] self - self
 * @param[in] fields - the quality fields to multiply
 */
RaveField_t* RaveQITotal_multiplicative(RaveQITotal_t* self, RaveObjectList_t* fields);

/**
 * Performs the additive QI total algorithm.
 * The weights that are specified for the additive algorithm is the old traditional weighting. I.e., the
 * total sum of the weights are summed. Then each weight is dividied by this sum so that we get a total weight of 1.0.
 * These weights are in turn used on respective field. If a field does not have a weight set, then a default weight will be
 * used for this field which is 1/number of fields.
 * @param[in] self - self
 * @param[in] fields - the quality fields to add
 */
RaveField_t* RaveQITotal_additive(RaveQITotal_t* self, RaveObjectList_t* fields);

/**
 * Performs the minimum QI total algorithm.
 * @param[in] self - self
 * @param[in] fields - the quality fields to test for min
 */
RaveField_t* RaveQITotal_minimum(RaveQITotal_t* self, RaveObjectList_t* fields);

#endif /* RAVE_QITOTAL_H */
