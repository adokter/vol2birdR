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
 * Helper class to support both legacy PROJ.4 projection and > PROJ.4
 * This object supports \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2021-10-14
 */
#ifndef PROJECTION_PIPELINE_H
#define PROJECTION_PIPELINE_H
#include "rave_object.h"
#include "projection.h"

/**
 * Defines a projection pipeline
 */
typedef struct _ProjectionPipeline_t ProjectionPipeline_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType ProjectionPipeline_TYPE;

/**
 * Creates a pipeline from one projection to another.
 * @param[in] first - first projection
 * @param[in] second - second projection
 * @return the pipeline on success otherwise NULL
 */
ProjectionPipeline_t* ProjectionPipeline_createPipeline(Projection_t* first, Projection_t* second);

/**
 * Creates a pipeline from one projection to another.
 * @param[in] first - first projection definition
 * @param[in] second - second projection definition
 * @return the pipeline on success otherwise NULL
 */
ProjectionPipeline_t* ProjectionPipeline_createPipelineFromDef(const char* first, const char* second);

/**
 * Creates a default pipeline used for translating between lon/lat and the other projection.
 * More or less same as writing:
 * ProjectionPipeline_createPipeline(Projection_createDefaultLonLatProjection(), other)
 *
 * @param[in] other - the other projection
 * @returns the pipeline on success otherwise NULL
 */
ProjectionPipeline_t* ProjectionPipeline_createDefaultLonLatPipeline(Projection_t* other);

/**
 * Creates a default pipeline used for translating between lon/lat and the other projection
 * where other is defined as a string.
 * More or less same as writing:
 * ProjectionPipeline_createPipelineFromDef(Projection_getDefaultLonLatProjDef(), other)
 *
 * @param[in] other - the other proj definition
 * @returns the pipeline on success otherwise NULL
 */
ProjectionPipeline_t* ProjectionPipeline_createDefaultLonLatPipelineFromDef(const char* other);

/**
 * Initializes a pipeline with the projections
 * @param[in] first - first projection
 * @param[in] second - second projection
 * @return 1 on success otherwise NULL
 */
int ProjectionPipeline_init(ProjectionPipeline_t* pipeline, Projection_t* first, Projection_t* second);

/**
 * Initializes a pipeline with the projection definitions
 * @param[in] first - first projection definition
 * @param[in] second - second projection definition
 * @return 1 on success otherwise NULL
 */
int ProjectionPipeline_initFromDef(ProjectionPipeline_t *pipeline, const char* first, const char *second);

/**
 * Returns the first projection.
 * @param[in] pipeline - self
 * @return the first pipeline
 */
Projection_t* ProjectionPipeline_getFirstProjection(ProjectionPipeline_t* pipeline);

/**
 * Returns the second projection.
 * @param[in] pipeline - self
 * @return the second pipeline
 */
Projection_t* ProjectionPipeline_getSecondProjection(ProjectionPipeline_t* pipeline);

/**
 * Transforms the coordinates from first projection to second projection using this pipeline.
 * @param[in] pipeline - this pipeline
 * @param[in] inu - coordinate
 * @param[in] inv - coordinate
 * @param[out] outu - coordinate
 * @param[out] outv - coordinate
 * @return 0 on failure, otherwise success
 */
int ProjectionPipeline_fwd(ProjectionPipeline_t* pipeline, double inu, double inv, double* outu, double* outv);

/**
 * Transforms the coordinates from second projection to first projection using this pipeline
 * @param[in] pipeline - this pipeline
 * @param[in] inu - coordinate
 * @param[in] inv - coordinate
 * @param[out] outu - coordinate
 * @param[out] outv - coordinate
 * @return 0 on failure, otherwise success
 */
int ProjectionPipeline_inv(ProjectionPipeline_t* pipeline, double inu, double inv, double* outu, double* outv);

#endif
