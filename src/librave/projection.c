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
 * Wrapper around PROJ.4
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-10-20
 */
#include "projection.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

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
 * The debug level to use when using PROJ
 */
static int proj_debug_level = 0;

/**
 * The default proj def to be used when creating default lon lat projection
 */
static char lon_lat_projdef[1024];

/**
 * Represents one projection
 */
struct _Projection_t {
  RAVE_OBJECT_HEAD /** Always on top */
  int initialized;     /**< if this instance has been initialized */
  char* id;            /**< the id of this projection */
  char* description;   /**< the description */
  char* definition;    /**< the proj.4 definition string */
  PJ* pj;              /**< the proj.4 instance */
#ifndef USE_PROJ4_API
  PJ_CONTEXT* context; /**< the proj.4 context */
#endif
};

/*@{ Private functions */
static void ProjectionInternal_freePJ(PJ* pj)
{
#ifdef USE_PROJ4_API
  pj_free(pj);
#else
  proj_destroy(pj);
#endif
}

static int Projection_constructor(RaveCoreObject* obj)
{
  Projection_t* projection = (Projection_t*)obj;
  projection->initialized = 0;
  projection->id = NULL;
  projection->description = NULL;
  projection->definition = NULL;
  projection->pj = NULL;
#ifndef USE_PROJ4_API
  projection->context = NULL;
#endif
  return 1;
}

/**
 * Copy constructor.
 */
static int Projection_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  int result = 0;
  Projection_t* this = (Projection_t*)obj;
  Projection_t* src = (Projection_t*)srcobj;

  this->initialized = 0;
  this->id = NULL;
  this->description = NULL;
  this->definition = NULL;
  this->pj = NULL;
#ifndef USE_PROJ4_API
  this->context = NULL;
#endif

  result = Projection_init(this, src->id, src->description, src->definition);

  return result;
}

/**
 * Destroys the projection
 * @param[in] projection - the projection to destroy
 */
static void Projection_destructor(RaveCoreObject* obj)
{
  Projection_t* projection = (Projection_t*)obj;
  if (projection != NULL) {
    RAVE_FREE(projection->id);
    RAVE_FREE(projection->description);
    RAVE_FREE(projection->definition);
    if (projection->pj != NULL) {
      ProjectionInternal_freePJ(projection->pj);
    }
#ifndef USE_PROJ4_API
    if (projection->context != NULL) {
      proj_context_destroy(projection->context);
    }
#endif
  }
}
/*@} End of Private functions */

/*@{ Interface functions */
void Projection_setDebugLevel(int debugPj) {
  proj_debug_level = debugPj;
}

int Projection_getDebugLevel(void) {
  return proj_debug_level;
}

const char* Projection_getProjVersion(void)
{
  static char result[100];
  if (strcmp(result, "") != 0) {
    return (const char*)result;
  }
#ifdef USE_PROJ4_API
#ifdef PJ_VERSION
strcpy(result, pj_release);
#else
strcpy(result, "unknown");
#endif
#else
strcpy(result, pj_release);
#endif
  return (const char*)result;
}


void Projection_setDefaultLonLatProjDef(const char* projdef)
{
  if (projdef != NULL && strlen(projdef) < 1023) {
    strcpy(lon_lat_projdef, projdef);
  }
}

const char* Projection_getDefaultLonLatProjDef(void)
{
  if (strcmp(lon_lat_projdef, "")==0) {
    strcpy(lon_lat_projdef, "+proj=longlat +ellps=WGS84 +datum=WGS84");
  }
  return (const char*)lon_lat_projdef;
}


int Projection_init(Projection_t* projection, const char* id, const char* description, const char* definition)
{
  int result = 0;
  RAVE_ASSERT((projection != NULL), "projection was NULL");
  RAVE_ASSERT((projection->initialized == 0), "projection was already initalized");
  if (id == NULL || description == NULL || definition == NULL) {
    RAVE_ERROR0("One of id, description or definition was NULL when initializing");
    return 0;
  }
#ifdef USE_PROJ4_API
  projection->pj = pj_init_plus(definition);
#else
  projection->context = proj_context_create();
  if (projection->context == NULL) {
    RAVE_ERROR0("Could not create projection context");
    return 0;
  }
  proj_log_level(projection->context, Projection_getDebugLevel());
  projection->pj = proj_create(projection->context, definition);
#endif
  projection->id = RAVE_STRDUP(id);
  projection->description = RAVE_STRDUP(description);
  projection->definition = RAVE_STRDUP(definition);

  if (projection->id == NULL || projection->description == NULL ||
      projection->definition == NULL || projection->pj == NULL) {
    if (projection->id == NULL) {
      RAVE_ERROR0("Could not set id");
    }
    if (projection->description == NULL) {
      RAVE_ERROR0("Could not set description");
    }
    if (projection->definition == NULL) {
      RAVE_ERROR0("Could not set definition");
    }
    if (projection->pj == NULL) {
      RAVE_ERROR1("Failed to create projection for %s", id);
    }
    RAVE_FREE(projection->id);
    RAVE_FREE(projection->description);
    RAVE_FREE(projection->definition);
    if (projection->pj != NULL) {
      ProjectionInternal_freePJ(projection->pj);
    }
  } else {
    result = 1;
    projection->initialized = 1;
  }
  return result;
}

Projection_t* Projection_create(const char* id, const char* description, const char* definition)
{
  Projection_t* result = NULL;
  result = RAVE_OBJECT_NEW(&Projection_TYPE);
  if (result != NULL) {
    if (!Projection_init(result, id, description, definition)) {
      RAVE_OBJECT_RELEASE(result);
    }
  }
  return result;
}

Projection_t* Projection_createDefaultLonLatProjection(void)
{
  Projection_t* result = NULL;
  result = RAVE_OBJECT_NEW(&Projection_TYPE);
  if (result != NULL) {
    if (!Projection_init(result, "defaultLonLat", "default lon/lat projection", Projection_getDefaultLonLatProjDef())) {
      RAVE_OBJECT_RELEASE(result);
    }
  }
  return result;
}

const char* Projection_getID(Projection_t* projection)
{
  RAVE_ASSERT((projection != NULL), "projection was NULL");
  return (const char*)projection->id;
}

const char* Projection_getDescription(Projection_t* projection)
{
  RAVE_ASSERT((projection != NULL), "projection was NULL");
  return (const char*)projection->description;
}

const char* Projection_getDefinition(Projection_t* projection)
{
  RAVE_ASSERT((projection != NULL), "projection was NULL");
  return (const char*)projection->definition;
}

int Projection_isLatLong(Projection_t* projection)
{
  RAVE_ASSERT((projection != NULL), "projection was NULL");
#ifdef USE_PROJ4_API
  return pj_is_latlong(projection->pj);
#else
  PJ_PROJ_INFO info = proj_pj_info(projection->pj);
  if (info.id != NULL) {
    if (strcmp("lonlat", info.id) == 0 ||
        strcmp("latlon", info.id) == 0 ||
        strcmp("latlong", info.id) == 0 ||
        strcmp("longlat", info.id) == 0) {
      return 1;
    }
  }
  return 0;
#endif
}

#ifndef USE_PROJ4_API
static Projection_t* ProjectionInternal_createCrsPipeline(Projection_t* projection, Projection_t* tgt)
{
  Projection_t *proj = NULL, *result = NULL;
  PJ_CONTEXT* context = NULL;
  PJ* pj = NULL;
  proj = RAVE_OBJECT_NEW(&Projection_TYPE);
  if (proj == NULL) {
    goto done;
  }
  context = proj_context_create();
  if (context == NULL) {
    RAVE_ERROR0("Failed to create proj context");
    goto done;
  }
  proj_log_level(context, Projection_getDebugLevel());
  pj = proj_create_crs_to_crs(context, projection->definition, tgt->definition, NULL);
  if (pj == NULL) {
    RAVE_ERROR0("Failed to create crs to crs");
    goto done;
  }

  proj->context = context;
  proj->pj = pj;
  proj->initialized = 1;
  context = NULL;
  pj = NULL;
  result = RAVE_OBJECT_COPY(proj);
done:
  if (context != NULL) {
    proj_context_destroy(context);
  }
  if (pj != NULL)  {
    ProjectionInternal_freePJ(pj);
  }
  RAVE_OBJECT_RELEASE(proj);
  return result;
}
#endif

int Projection_transform(Projection_t* projection, Projection_t* tgt, double* x, double* y, double* z)
{
  int result = 1;

  RAVE_ASSERT((projection != NULL), "projection was NULL");
  RAVE_ASSERT((tgt != NULL), "target projection was NULL");
  RAVE_ASSERT((x != NULL), "x was NULL");
  RAVE_ASSERT((y != NULL), "y was NULL");
#ifdef USE_PROJ4_API
  {
    int pjv = 0;
    if ((pjv = pj_transform(projection->pj, tgt->pj, 1, 1, x, y, z)) != 0)
    {
      RAVE_ERROR1("Transform failed with pj_errno: %d\n", pjv);
      result = 0;
    }
  }
#else
  {
    Projection_t* crsPipe = ProjectionInternal_createCrsPipeline(projection, tgt);
    if (crsPipe == NULL) {
      RAVE_ERROR0("Failed to create crs to crs\n");
      result = 0;
    } else {
      PJ_COORD c, outc;
      c.xyz.x = *x;
      c.xyz.y = *y;
      c.xyz.z = 0.0;
      c.lpzt.t = HUGE_VAL;
      if (z != NULL) {
        c.xyz.z = *z;
      }
      if (Projection_isLatLong(projection)) {
        c.xyz.x = c.xyz.x * 180.0 / M_PI;
        c.xyz.y = c.xyz.y * 180.0 / M_PI;
        c.xyz.z = c.xyz.z * 180.0 / M_PI;
      }
      outc = proj_trans(crsPipe->pj, PJ_FWD, c);
      if (Projection_isLatLong(tgt)) {
        outc.xyz.x = outc.xyz.x * M_PI / 180.0;
        outc.xyz.y = outc.xyz.y * M_PI / 180.0;
        outc.xyz.z = outc.xyz.z * M_PI / 180.0;
      }
      *x = outc.xyz.x;
      *y = outc.xyz.y;
      if (z != NULL) {
        *z = outc.xyz.z;
      }
    }
    RAVE_OBJECT_RELEASE(crsPipe);
  }
#endif

  return result;
}

int Projection_transformx(Projection_t* projection, Projection_t* tgt,
  double x, double y, double z, double* ox, double* oy, double* oz)
{
  int result = 1;
  double vx = 0.0, vy = 0.0, vz = 0.0;
  RAVE_ASSERT((projection != NULL), "projection == NULL");
  RAVE_ASSERT((tgt != NULL), "tgt == NULL");
  RAVE_ASSERT((ox != NULL), "ox == NULL");
  RAVE_ASSERT((oy != NULL), "oy == NULL");

  vx = x;
  vy = y;
  vz = z;
#ifdef USE_PROJ4_API
  if (oz == NULL) {
    int pjv = 0;
    if ((pjv = pj_transform(projection->pj, tgt->pj, 1, 1, &vx, &vy, NULL)) != 0) {
      RAVE_ERROR1("Transform failed with pj_errno: %d\n", pjv);
      result = 0;
    }
  } else {
    int pjv = 0;
    if ((pjv = pj_transform(projection->pj, tgt->pj, 1, 1, &vx, &vy, &vz)) != 0) {
      RAVE_ERROR1("Transform failed with pj_errno: %d\n", pjv);
      result = 0;
    }
  }
#else
  Projection_t* crsPipe = ProjectionInternal_createCrsPipeline(projection, tgt);
  if (crsPipe == NULL) {
    RAVE_ERROR0("Failed to create crs to crs\n");
    result = 0;
  } else {
    PJ_COORD c, outc;
    c.xyz.x = vx;
    c.xyz.y = vy;
    c.xyz.z = vz;
    if (Projection_isLatLong(projection)) {
      c.xyz.x = c.xyz.x * 180.0 / M_PI;
      c.xyz.y = c.xyz.y * 180.0 / M_PI;
      c.xyz.z = c.xyz.z * 180.0 / M_PI;
    }
    outc = proj_trans(crsPipe->pj, PJ_FWD, c);
    if (Projection_isLatLong(tgt)) {
      outc.xyz.x = outc.xyz.x * M_PI / 180.0;
      outc.xyz.y = outc.xyz.y * M_PI / 180.0;
      outc.xyz.z = outc.xyz.z * M_PI / 180.0;
    }
    vx = outc.xyz.x;
    vy = outc.xyz.y;
    vz = outc.xyz.z;
  }
  RAVE_OBJECT_RELEASE(crsPipe);
#endif
  if (result == 1) {
    *ox = vx;
    *oy = vy;
    if (oz != NULL) {
      *oz = vz;
    }
  }
  return result;
}

int Projection_inv(Projection_t* projection, double x, double y, double* lon, double* lat)
{
  int result = 1;

  RAVE_ASSERT((projection != NULL), "projection was NULL");
  RAVE_ASSERT((lon != NULL), "lon was NULL");
  RAVE_ASSERT((lat != NULL), "lat was NULL");

#ifdef USE_PROJ4_API
  {
    UV in,out;
    in.u = x;
    in.v = y;
    out = pj_inv(in, projection->pj);
    *lon = out.u;
    *lat = out.v;
  }
#else
  {
    PJ_COORD in,out;
    in.uv.u = x;
    in.uv.v = y;
    in.v[2]=0.0;
    out = proj_trans(projection->pj, PJ_INV, in);
    *lon = out.uv.u;
    *lat = out.uv.v;
  }
#endif
  return result;
}

int Projection_fwd(Projection_t* projection, double lon, double lat, double* x, double* y)
{
  int result = 1;
  RAVE_ASSERT((projection != NULL), "projection was NULL");
  RAVE_ASSERT((x != NULL), "x was NULL");
  RAVE_ASSERT((y != NULL), "y was NULL");

#ifdef USE_PROJ4_API
  {
    UV in,out;
    in.u = lon;
    in.v = lat;
    out = pj_fwd(in, projection->pj);
    *x = out.u;
    *y = out.v;
  }
#else
  PJ_COORD in,out;
  in.uv.u = lon;
  in.uv.v = lat;
  out = proj_trans(projection->pj, PJ_FWD, in);
  *x = out.uv.u;
  *y = out.uv.v;
#endif
  return result;
}

/*@} End of Interface functions */

RaveCoreObjectType Projection_TYPE = {
    "Projection",
    sizeof(Projection_t),
    Projection_constructor,
    Projection_destructor,
    Projection_copyconstructor
};
