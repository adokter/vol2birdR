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
 * Helper class to support both legacy PROJ.4 projection and > PROJ.4.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2021-10-14
 */
#include "projection_pipeline.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

#ifdef USE_PROJ4_API
#include <proj_api.h>
#ifdef PJ_VERSION
#define UV projUV
#define PJ projPJ
#endif
#else
#include <proj.h>
#define UV PJ_UV
#endif


/**
 * Represents one projection
 */
struct _ProjectionPipeline_t {
  RAVE_OBJECT_HEAD /** Always on top */
  int initialized;
  Projection_t *first; /**< First projection */
  Projection_t *second; /**< Second projection */
  int firstIsLatlong;
  int secondIsLatlong;

#ifndef USE_PROJ4_API
  PJ_CONTEXT* context; /**< context */
  PJ* pj; /**< projection */
#endif
};

/*@{ Private functions */
static int ProjectionPipeline_constructor(RaveCoreObject *obj)
{
  ProjectionPipeline_t *pipeline = (ProjectionPipeline_t*) obj;
  pipeline->initialized = 0;
  pipeline->first = NULL;
  pipeline->second = NULL;
  pipeline->firstIsLatlong = 0;
  pipeline->secondIsLatlong = 0;
#ifndef USE_PROJ4_API
  pipeline->context = NULL;
  pipeline->pj = NULL;
#endif
  return 1;
}

/**
 * Copy constructor.
 */
static int ProjectionPipeline_copyconstructor(RaveCoreObject *obj, RaveCoreObject *srcobj)
{
  int result = 0;
  ProjectionPipeline_t *this = (ProjectionPipeline_t*) obj;
  ProjectionPipeline_t *src = (ProjectionPipeline_t*) srcobj;

  Projection_t *firstP = RAVE_OBJECT_CLONE(src->first);
  Projection_t *secondP = RAVE_OBJECT_CLONE(src->second);

  if (firstP == NULL || secondP == NULL) {
    RAVE_ERROR0("Failed to clone projections");
    goto done;
  }
  this->initialized = 0;
  this->first = NULL;
  this->second = NULL;
  this->firstIsLatlong = src->firstIsLatlong;
  this->secondIsLatlong = src->secondIsLatlong;
#ifndef USE_PROJ4_API
  this->context = NULL;
  this->pj = NULL;
#endif

  if (!ProjectionPipeline_init(this, firstP, secondP)) {
    RAVE_ERROR0("Failed to initialize pipeline");
    goto done;
  }

  result = 1;
done:
  RAVE_OBJECT_RELEASE(firstP);
  RAVE_OBJECT_RELEASE(secondP);

  return result;
}

/**
 * Destroys the projection
 * @param[in] projection - the projection to destroy
 */
static void ProjectionPipeline_destructor(RaveCoreObject *obj)
{
  ProjectionPipeline_t *pipeline = (ProjectionPipeline_t*) obj;
  if (pipeline != NULL) {
    RAVE_OBJECT_RELEASE(pipeline->first);
    RAVE_OBJECT_RELEASE(pipeline->second);
#ifndef USE_PROJ4_API
    if (pipeline->pj != NULL) {
      proj_destroy(pipeline->pj);
    }
    if (pipeline->context != NULL) {
      proj_context_destroy(pipeline->context);
    }
#endif
  }
}
/*@} End of Private functions */

/*@{ Interface functions */
ProjectionPipeline_t* ProjectionPipeline_createPipeline(Projection_t* first, Projection_t* second)
{
  ProjectionPipeline_t* result = NULL;
  result = RAVE_OBJECT_NEW(&ProjectionPipeline_TYPE);
  if (result != NULL) {
    if (!ProjectionPipeline_init(result, first, second)) {
      RAVE_OBJECT_RELEASE(result);
    }
  }
  return result;
}

ProjectionPipeline_t* ProjectionPipeline_createPipelineFromDef(const char* first, const char* second)
{
  ProjectionPipeline_t* result = NULL;
  result = RAVE_OBJECT_NEW(&ProjectionPipeline_TYPE);
  if (result != NULL) {
    if (!ProjectionPipeline_initFromDef(result, first, second)) {
      RAVE_OBJECT_RELEASE(result);
    }
  }
  return result;
}

ProjectionPipeline_t* ProjectionPipeline_createDefaultLonLatPipeline(Projection_t* other)
{
  Projection_t* lonlatproj = Projection_createDefaultLonLatProjection();
  ProjectionPipeline_t* pipeline = NULL;
  if (lonlatproj != NULL) {
    pipeline = ProjectionPipeline_createPipeline(lonlatproj, other);
  }
  RAVE_OBJECT_RELEASE(lonlatproj);
  return pipeline;
}


ProjectionPipeline_t* ProjectionPipeline_createDefaultLonLatPipelineFromDef(const char* other)
{
  return ProjectionPipeline_createPipelineFromDef(Projection_getDefaultLonLatProjDef(), other);
}


int ProjectionPipeline_init(ProjectionPipeline_t *pipeline, Projection_t *first,
    Projection_t *second)
{
  RAVE_ASSERT((pipeline != NULL), "pipeline was NULL");
  RAVE_ASSERT((pipeline->initialized == 0), "pipeline was already initalized");

  if (first == NULL || second == NULL) {
    RAVE_ERROR0("One of first or second was NULL when initializing");
    return 0;
  }

  return ProjectionPipeline_initFromDef(pipeline, Projection_getDefinition(first), Projection_getDefinition(second));
}

int ProjectionPipeline_initFromDef(ProjectionPipeline_t *pipeline, const char* first,
    const char *second)
{
  int result = 0;
  Projection_t *firstPj = NULL, *secondPj = NULL;
  RAVE_ASSERT((pipeline != NULL), "pipeline was NULL");
  RAVE_ASSERT((pipeline->initialized == 0), "pipeline was already initalized");

  if (first == NULL || second == NULL) {
    RAVE_ERROR0("One of first or second was NULL when initializing");
    return 0;
  }

  firstPj = Projection_create("firstPj", "first projection", first);
  secondPj = Projection_create("secondPj", "second projection", second);
  if (firstPj == NULL) {
    RAVE_ERROR1("Failed to create first projection from %s", first);
    goto done;
  }
  if (secondPj == NULL) {
    RAVE_ERROR1("Failed to create second projection from %s", second);
    goto done;
  }

  /* If we are using proj4 api, then original projections will be used. If on other hand
   * we are using new proj api. We need to create the actual pipeline.
   */
#ifndef USE_PROJ4_API
  {
    PJ *p = NULL;
    PJ_CONTEXT* context = proj_context_create();
    if (context == NULL) {
      RAVE_ERROR0("Failed to create context for projection");
      goto done;
    }
    proj_log_level(context, Projection_getDebugLevel());
    p = proj_create_crs_to_crs(context, first, second, NULL);
    if (p == NULL) {
      RAVE_ERROR2("Failed to create crs_to_crs_projection: %d, %s", proj_errno(0), proj_errno_string(proj_errno(0)));
      proj_context_destroy(context);
      goto done;
    }

    pipeline->pj = p;
    pipeline->context = context;
  }
#endif
  pipeline->first = RAVE_OBJECT_COPY(firstPj);
  pipeline->second = RAVE_OBJECT_COPY(secondPj);
  pipeline->firstIsLatlong = Projection_isLatLong(pipeline->first);
  pipeline->secondIsLatlong = Projection_isLatLong(pipeline->second);
  pipeline->initialized = 1;
  result = 1;

done:
  RAVE_OBJECT_RELEASE(firstPj);
  RAVE_OBJECT_RELEASE(secondPj);
  return result;
}

Projection_t* ProjectionPipeline_getFirstProjection(ProjectionPipeline_t* pipeline)
{
  RAVE_ASSERT((pipeline != NULL), "pipeline was NULL");
  return RAVE_OBJECT_COPY(pipeline->first);
}

Projection_t* ProjectionPipeline_getSecondProjection(ProjectionPipeline_t* pipeline)
{
  RAVE_ASSERT((pipeline != NULL), "pipeline was NULL");
  return RAVE_OBJECT_COPY(pipeline->second);
}

int ProjectionPipeline_fwd(ProjectionPipeline_t* pipeline, double inu, double inv, double* outu, double* outv)
{
  int result = 1;

  RAVE_ASSERT((pipeline != NULL), "pipeline was NULL");
  RAVE_ASSERT((outu != NULL), "outu == NULL");
  RAVE_ASSERT((outv != NULL), "outv == NULL");

#ifdef USE_PROJ4_API
  result = Projection_transformx(pipeline->first, pipeline->second, inu, inv, 0.0, outu, outv, NULL);
#else
  {
    /* Proj > 4 calculates in degrees instead of radians.. */
    PJ_COORD in,out;
    in.uv.u = inu;
    in.uv.v = inv;
    if (pipeline->firstIsLatlong) {
      in.uv.u = inu * 180.0 / M_PI;
      in.uv.v = inv * 180.0 / M_PI;
    }
    out = proj_trans(pipeline->pj, PJ_FWD, in);
    *outu = out.uv.u;
    *outv = out.uv.v;
    if (pipeline->secondIsLatlong) {
      *outu = out.uv.u * M_PI / 180.0;
      *outv = out.uv.v * M_PI / 180.0;
    }
  }
#endif

  return result;
}

int ProjectionPipeline_inv(ProjectionPipeline_t* pipeline, double inu, double inv, double* outu, double* outv)
{
  int result = 1;

  RAVE_ASSERT((pipeline != NULL), "pipeline was NULL");
  RAVE_ASSERT((outu != NULL), "outu == NULL");
  RAVE_ASSERT((outv != NULL), "outv == NULL");

#ifdef USE_PROJ4_API
  result = Projection_transformx(pipeline->second, pipeline->first, inu, inv, 0.0, outu, outv, NULL);
#else
  {
    /* Proj > 4 calculates in degrees instead of radians.. */
    PJ_COORD in,out;
    in.uv.u = inu;
    in.uv.v = inv;
    if (pipeline->secondIsLatlong) {
      in.uv.u = inu * 180.0 / M_PI;
      in.uv.v = inv * 180.0 / M_PI;
    }
    out = proj_trans(pipeline->pj, PJ_INV, in);
    *outu = out.uv.u;
    *outv = out.uv.v;
    if (pipeline->firstIsLatlong) {
      *outu = out.uv.u * M_PI / 180.0;
      *outv = out.uv.v * M_PI / 180.0;
    }
  }
#endif

  return result;
}

/*@} End of Interface functions */

RaveCoreObjectType ProjectionPipeline_TYPE =
{
    "ProjectionPipeline",
    sizeof(ProjectionPipeline_t),
    ProjectionPipeline_constructor,
    ProjectionPipeline_destructor,
    ProjectionPipeline_copyconstructor
};
