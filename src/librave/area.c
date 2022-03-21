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
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-12-10
 */
#include "area.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include <string.h>


/**
 * Represents the area
 */
struct _Area_t {
  RAVE_OBJECT_HEAD /** Always on top */

  char* id;        /**< the id */
  char* description; /**< the description */
  char* pcsid;     /**< the pcs id */
  // Where
  long xsize;      /**< xsize */
  long ysize;      /**< ysize */
  double xscale;   /**< xscale */
  double yscale;   /**< yscale */

  double llX;      /**< lower left x-coordinate */
  double llY;      /**< lower left y-coordinate */
  double urX;      /**< upper right x-coordinate */
  double urY;      /**< upper right y-coordinate */

  Projection_t* projection; /**< the projection that is used for this area */
};

/*@{ Private functions */
/**
 * Constructor.
 */
static int Area_constructor(RaveCoreObject* obj)
{
  Area_t* this = (Area_t*)obj;
  this->id = NULL;
  this->description = NULL;
  this->pcsid = NULL;
  this->xsize = 0;
  this->ysize = 0;
  this->xscale = 0.0L;
  this->yscale = 0.0L;
  this->llX = 0.0L;
  this->llY = 0.0L;
  this->urX = 0.0L;
  this->urY = 0.0L;
  this->projection = NULL;
  return 1;
}

/**
 * Copy constructor
 */
static int Area_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  Area_t* this = (Area_t*)obj;
  Area_t* src = (Area_t*)srcobj;

  Area_constructor(obj); // First just initialize everything like the constructor

  this->xsize = src->xsize;
  this->ysize = src->ysize;
  this->xscale = src->xscale;
  this->yscale = src->yscale;
  this->llX = src->llX;
  this->llY = src->llY;
  this->urX = src->urX;
  this->urY = src->urY;

  if (!Area_setID(this, src->id)) {
    goto error;
  }
  if (!Area_setDescription(this, src->description)) {
    goto error;
  }
  if (!Area_setPcsid(this, src->pcsid)) {
    goto error;
  }

  this->projection = RAVE_OBJECT_CLONE(src->projection);
  if (this->projection == NULL) {
    goto error;
  }
  return 1;
error:
  RAVE_OBJECT_RELEASE(this->projection);
  RAVE_FREE(this->id);
  RAVE_FREE(this->description);
  RAVE_FREE(this->pcsid);
  return 0;
}

/**
 * Destroys the area
 * @param[in] obj - the the Area_t instance
 */
static void Area_destructor(RaveCoreObject* obj)
{
  Area_t* area = (Area_t*)obj;
  if (area != NULL) {
    RAVE_FREE(area->id);
    RAVE_FREE(area->description);
    RAVE_FREE(area->pcsid);
    RAVE_OBJECT_RELEASE(area->projection);
  }
}
/*@} End of Private functions */

/*@{ Interface functions */
int Area_setID(Area_t* area, const char* id)
{
  RAVE_ASSERT((area != NULL), "area was NULL");
  RAVE_FREE(area->id);
  if (id != NULL) {
    area->id = RAVE_STRDUP(id);
    if (area->id == NULL) {
      RAVE_CRITICAL0("Failure when copying id");
      return 0;
    }
  }
  return 1;
}

const char* Area_getID(Area_t* area)
{
  RAVE_ASSERT((area != NULL), "area was NULL");
  return (const char*)area->id;
}

int Area_setDescription(Area_t* area, const char* description)
{
  RAVE_ASSERT((area != NULL), "area was NULL");
  RAVE_FREE(area->description);
  if (description != NULL) {
    area->description = RAVE_STRDUP(description);
    if (area->description == NULL) {
      RAVE_CRITICAL0("Failure when copying id");
      return 0;
    }
  }
  return 1;
}

const char* Area_getDescription(Area_t* area)
{
  RAVE_ASSERT((area != NULL), "area was NULL");
  return (const char*)area->description;
}

void Area_setXSize(Area_t* area, long xsize)
{
  RAVE_ASSERT((area != NULL), "area was NULL");
  area->xsize = xsize;
}

long Area_getXSize(Area_t* area)
{
  RAVE_ASSERT((area != NULL), "area was NULL");
  return area->xsize;
}

void Area_setYSize(Area_t* area, long ysize)
{
  RAVE_ASSERT((area != NULL), "area was NULL");
  area->ysize = ysize;
}

long Area_getYSize(Area_t* area)
{
  RAVE_ASSERT((area != NULL), "area was NULL");
  return area->ysize;
}

void Area_setXScale(Area_t* area, double xscale)
{
  RAVE_ASSERT((area != NULL), "area was NULL");
  area->xscale = xscale;
}

double Area_getXScale(Area_t* area)
{
  RAVE_ASSERT((area != NULL), "area was NULL");
  return area->xscale;
}

void Area_setYScale(Area_t* area, double yscale)
{
  RAVE_ASSERT((area != NULL), "area was NULL");
  area->yscale = yscale;
}

double Area_getYScale(Area_t* area)
{
  RAVE_ASSERT((area != NULL), "area was NULL");
  return area->yscale;
}

void Area_setExtent(Area_t* area, double llX, double llY, double urX, double urY)
{
  RAVE_ASSERT((area != NULL), "area was NULL");
  area->llX = llX;
  area->llY = llY;
  area->urX = urX;
  area->urY = urY;
}

void Area_getExtent(Area_t* area, double* llX, double* llY, double* urX, double* urY)
{
  RAVE_ASSERT((area != NULL), "area was NULL");
  if (llX != NULL) {
    *llX = area->llX;
  }
  if (llY != NULL) {
    *llY = area->llY;
  }
  if (urX != NULL) {
    *urX = area->urX;
  }
  if (urY != NULL) {
    *urY = area->urY;
  }
}

void Area_setProjection(Area_t* area, Projection_t* projection)
{
  RAVE_ASSERT((area != NULL), "area was NULL");
  RAVE_OBJECT_RELEASE(area->projection);
  RAVE_FREE(area->pcsid);
  area->projection = RAVE_OBJECT_COPY(projection);
}

Projection_t* Area_getProjection(Area_t* area)
{
  RAVE_ASSERT((area != NULL), "area was NULL");
  return RAVE_OBJECT_COPY(area->projection);
}

int Area_setPcsid(Area_t* area, const char* pcsid)
{
  int result = 0;

  RAVE_ASSERT((area != NULL), "area was NULL");
  if (pcsid == NULL) {
    RAVE_OBJECT_RELEASE(area->projection);
    RAVE_FREE(area->pcsid);
  } else {
    if (area->projection != NULL) {
      if (Projection_getID(area->projection) != NULL &&
          strcmp(pcsid, Projection_getID(area->projection)) == 0) {
        result = 1;
        goto done;
      }
    }
    RAVE_OBJECT_RELEASE(area->projection);
    RAVE_FREE(area->pcsid);
    area->pcsid = RAVE_STRDUP(pcsid);
    if (area->pcsid == NULL) {
      RAVE_CRITICAL0("Failure when copying id");
      goto done;
    }
  }
  result = 1;
done:
  return result;
}

const char* Area_getPcsid(Area_t* area)
{
  RAVE_ASSERT((area != NULL), "area was NULL");
  if (area->projection != NULL) {
    return Projection_getID(area->projection);
  } else {
    return area->pcsid;
  }
}

/*@} End of Interface functions */

RaveCoreObjectType Area_TYPE = {
    "Area",
    sizeof(Area_t),
    Area_constructor,
    Area_destructor,
    Area_copyconstructor
};
