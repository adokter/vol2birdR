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
 * Defines the functions available when working with cartesian products
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-10-16
 */
#include "cartesianvolume.h"
#include "area.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "rave_datetime.h"
#include "rave_data2d.h"
#include "raveobject_hashtable.h"
#include "rave_utilities.h"
#include "rave_attribute_table.h"
#include <string.h>

/**
 * Represents the cartesian volume
 */
struct _CartesianVolume_t {
  RAVE_OBJECT_HEAD /** Always on top */
  Rave_ObjectType type; /**< what type of cartesian volume this is, COMP, CVOL or anything else */
  RaveDateTime_t* datetime;  /**< the date and time */
  char* source;              /**< where does this data come from */
  RaveObjectList_t* images;  /**< the list of images */
  RaveAttributeTable_t* attrs; /**< attributes */
  Projection_t* projection;     /**< this volumes projection definition */
  double xscale;                /**< x scale */
  double yscale;                /**< y scale */
  double zscale;                /**< z scale, introduced with ODIM 2.3, marked for DEPRECATION in 2.4 */
  double zstart;                /**< Height in meters above mean sea level of the lowest pixel in the Z dimension, introduced with ODIM 2.3. Marked for DEPRECATION in 2.4 */
  double llX;                   /**< lower left x-coordinate */
  double llY;                   /**< lower left y-coordinate */
  double urX;                   /**< upper right x-coordinate */
  double urY;                   /**< upper right x-coordinate */
  long xsize;                   /**< xsize */
  long ysize;                   /**< ysize */
};

/*@{ Private functions */
/**
 * Constructor.
 */
static int CartesianVolume_constructor(RaveCoreObject* obj)
{
  CartesianVolume_t* this = (CartesianVolume_t*)obj;
  this->type = Rave_ObjectType_CVOL;
  this->source = NULL;
  this->datetime = RAVE_OBJECT_NEW(&RaveDateTime_TYPE);
  this->images = RAVE_OBJECT_NEW(&RaveObjectList_TYPE);
  this->attrs = RAVE_OBJECT_NEW(&RaveAttributeTable_TYPE);
  this->projection = NULL;
  this->xscale = 0.0;
  this->yscale = 0.0;
  this->zscale = 0.0;
  this->zstart = 0.0;
  this->xsize = 0;
  this->ysize = 0;
  this->llX = 0.0;
  this->llY = 0.0;
  this->urX = 0.0;
  this->urY = 0.0;
  if (this->datetime == NULL || this->images == NULL || this->attrs == NULL) {
    goto error;
  }
  return 1;
error:
  RAVE_OBJECT_RELEASE(this->datetime);
  RAVE_OBJECT_RELEASE(this->images);
  RAVE_OBJECT_RELEASE(this->attrs);
  return 0;
}

/**
 * Copy constructor.
 */
static int CartesianVolume_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  CartesianVolume_t* this = (CartesianVolume_t*)obj;
  CartesianVolume_t* src = (CartesianVolume_t*)srcobj;
  this->type = src->type;
  this->source = NULL;
  this->datetime = RAVE_OBJECT_CLONE(src->datetime);
  this->images = RAVE_OBJECT_CLONE(src->images);
  this->attrs = RAVE_OBJECT_CLONE(src->attrs);
  this->xscale = src->xscale;
  this->yscale = src->yscale;
  this->zscale = src->zscale;
  this->xsize = src->xsize;
  this->ysize = src->ysize;
  this->zstart = src->zstart;
  this->llX = src->llX;
  this->llY = src->llY;
  this->urX = src->urX;
  this->urY = src->urY;
  if (this->datetime == NULL || this->images == NULL || this->attrs == NULL) {
    goto error;
  }
  if (src->projection != NULL) {
    this->projection = RAVE_OBJECT_CLONE(src->projection);
    if (this->projection == NULL) {
      goto error;
    }
  }
  if (!CartesianVolume_setSource(this, CartesianVolume_getSource(src))) {
    goto error;
  }
  return 1;
error:
  RAVE_OBJECT_RELEASE(this->datetime);
  RAVE_OBJECT_RELEASE(this->images);
  RAVE_FREE(this->source);
  RAVE_OBJECT_RELEASE(this->attrs);
  return 0;
}

/**
 * Destroys the cartesian product
 * @param[in] scan - the cartesian product to destroy
 */
static void CartesianVolume_destructor(RaveCoreObject* obj)
{
  CartesianVolume_t* this = (CartesianVolume_t*)obj;
  RAVE_OBJECT_RELEASE(this->datetime);
  RAVE_OBJECT_RELEASE(this->images);
  RAVE_OBJECT_RELEASE(this->attrs);
  RAVE_FREE(this->source);
  RAVE_OBJECT_RELEASE(this->projection);
}
/*@} End of Private functions */

/*@{ Interface functions */
int CartesianVolume_setTime(CartesianVolume_t* cvol, const char* value)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return RaveDateTime_setTime(cvol->datetime, value);
}

const char* CartesianVolume_getTime(CartesianVolume_t* cvol)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return RaveDateTime_getTime(cvol->datetime);
}

int CartesianVolume_setDate(CartesianVolume_t* cvol, const char* value)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return RaveDateTime_setDate(cvol->datetime, value);
}

const char* CartesianVolume_getDate(CartesianVolume_t* cvol)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return RaveDateTime_getDate(cvol->datetime);
}

int CartesianVolume_setSource(CartesianVolume_t* cvol, const char* value)
{
  char* tmp = NULL;
  int result = 0;
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  if (value != NULL) {
    tmp = RAVE_STRDUP(value);
    if (tmp != NULL) {
      RAVE_FREE(cvol->source);
      cvol->source = tmp;
      tmp = NULL;
      result = 1;
    }
  } else {
    RAVE_FREE(cvol->source);
    result = 1;
  }
  return result;
}

const char* CartesianVolume_getSource(CartesianVolume_t* cvol)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return (const char*)cvol->source;
}

int CartesianVolume_setObjectType(CartesianVolume_t* cvol, Rave_ObjectType type)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  if (type == Rave_ObjectType_CVOL || type == Rave_ObjectType_COMP) {
    cvol->type = type;
    return 1;
  }
  return 0;
}

Rave_ObjectType CartesianVolume_getObjectType(CartesianVolume_t* cvol)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return cvol->type;
}

void CartesianVolume_setProjection(CartesianVolume_t* cvol, Projection_t* projection)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  RAVE_OBJECT_RELEASE(cvol->projection);
  if (projection != NULL) {
    cvol->projection = RAVE_OBJECT_COPY(projection);
  }
}

Projection_t* CartesianVolume_getProjection(CartesianVolume_t* cvol)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return RAVE_OBJECT_COPY(cvol->projection);
}

const char* CartesianVolume_getProjectionString(CartesianVolume_t* cvol)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  if (cvol->projection != NULL) {
    return Projection_getDefinition(cvol->projection);
  }
  return NULL;
}

void CartesianVolume_setXScale(CartesianVolume_t* cvol, double xscale)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  cvol->xscale = xscale;
}

double CartesianVolume_getXScale(CartesianVolume_t* cvol)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return cvol->xscale;
}

void CartesianVolume_setYScale(CartesianVolume_t* cvol, double yscale)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  cvol->yscale = yscale;
}

double CartesianVolume_getYScale(CartesianVolume_t* cvol)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return cvol->yscale;
}

void CartesianVolume_setZScale(CartesianVolume_t* cvol, double zscale)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  cvol->zscale = zscale;
}

double CartesianVolume_getZScale(CartesianVolume_t* cvol)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return cvol->zscale;
}

long CartesianVolume_getXSize(CartesianVolume_t* cvol)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return cvol->xsize;
}

long CartesianVolume_getYSize(CartesianVolume_t* cvol)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return cvol->ysize;
}

long CartesianVolume_getZSize(CartesianVolume_t* cvol)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return CartesianVolume_getNumberOfImages(cvol);;
}

void CartesianVolume_setZStart(CartesianVolume_t* cvol, double zstart)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  cvol->zstart = zstart;
}

double CartesianVolume_getZStart(CartesianVolume_t* cvol)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return cvol->zstart;
}

void CartesianVolume_setAreaExtent(CartesianVolume_t* cvol, double llX, double llY, double urX, double urY)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  cvol->llX = llX;
  cvol->llY = llY;
  cvol->urX = urX;
  cvol->urY = urY;
}

void CartesianVolume_getAreaExtent(CartesianVolume_t* cvol, double* llX, double* llY, double* urX, double* urY)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  if (llX != NULL) {
    *llX = cvol->llX;
  }
  if (llY != NULL) {
    *llY = cvol->llY;
  }
  if (urX != NULL) {
    *urX = cvol->urX;
  }
  if (urY != NULL) {
    *urY = cvol->urY;
  }
}

int CartesianVolume_addImage(CartesianVolume_t* cvol, Cartesian_t* image)
{
  int result = 0;
  Projection_t* projection = NULL;

  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  RAVE_ASSERT((image != NULL), "image == NULL");

  if ((cvol->xsize != 0 && Cartesian_getXSize(image) != cvol->xsize) ||
      (cvol->ysize != 0 && Cartesian_getYSize(image) != cvol->ysize)) {
    RAVE_ERROR0("Conflicting sizes between cartesian images in volume\n");
    goto done;
  }

  if (RaveObjectList_add(cvol->images, (RaveCoreObject*)image)) {
    cvol->xsize = Cartesian_getXSize(image);
    cvol->ysize = Cartesian_getYSize(image);
    result = 1;
  }

  if (result == 1) {
    double llX=0.0, llY=0.0, urX=0.0, urY=0.0;
    // Adjust the image to get properties from this volume if missing..
    projection = Cartesian_getProjection(image);
    if (projection == NULL && cvol->projection != NULL) {
      Cartesian_setProjection(image, cvol->projection);
    }
    RAVE_OBJECT_RELEASE(projection);

    if (Cartesian_getXScale(image)==0.0) {
      Cartesian_setXScale(image, cvol->xscale);
    }

    if (Cartesian_getYScale(image)==0.0) {
      Cartesian_setYScale(image, cvol->yscale);
    }

    if (Cartesian_getDate(image) == NULL || Cartesian_getTime(image) == NULL) {
      RaveAttribute_t* stime = Cartesian_getAttribute(image, "what/starttime");
      RaveAttribute_t* sdate = Cartesian_getAttribute(image, "what/startdate");
      if (stime != NULL && sdate != NULL) {
        if (Cartesian_getTime(image) == NULL) {
          char* value = NULL;
          if (RaveAttribute_getString(stime, &value)) {
            Cartesian_setTime(image, value);
          }
        }
        if (Cartesian_getDate(image) == NULL) {
          char* value = NULL;
          if (RaveAttribute_getString(sdate, &value)) {
            Cartesian_setDate(image, value);
          }
        }
      } else {
        Cartesian_setTime(image, CartesianVolume_getTime(cvol));
        Cartesian_setDate(image, CartesianVolume_getDate(cvol));
      }
      RAVE_OBJECT_RELEASE(stime);
      RAVE_OBJECT_RELEASE(sdate);
    }

    if (Cartesian_getSource(image) == NULL) {
      Cartesian_setSource(image, CartesianVolume_getSource(cvol));
    }

    Cartesian_getAreaExtent(image, &llX, &llY, &urX, &urY);
    if (llX == 0.0 && llY == 0.0 && urX == 0.0 && urY == 0.0) {
      Cartesian_setAreaExtent(image, cvol->llX, cvol->llY, cvol->urX, cvol->urY);
    }
  }

done:
  return result;
}

Cartesian_t* CartesianVolume_getImage(CartesianVolume_t* cvol, int index)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return (Cartesian_t*)RaveObjectList_get(cvol->images, index);
}

int CartesianVolume_getNumberOfImages(CartesianVolume_t* cvol)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return RaveObjectList_size(cvol->images);
}

int CartesianVolume_addAttribute(CartesianVolume_t* cvol, RaveAttribute_t* attribute)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  RAVE_ASSERT((attribute != NULL), "attribute == NULL");
  return CartesianVolume_addAttributeVersion(cvol, attribute, RAVEIO_API_ODIM_VERSION);
}

int CartesianVolume_addAttributeVersion(CartesianVolume_t* cvol, RaveAttribute_t* attribute, RaveIO_ODIM_Version version)
{
  const char* name = NULL;
  char* aname = NULL;
  char* gname = NULL;
  int result = 0;
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  RAVE_ASSERT((attribute != NULL), "attribute == NULL");

  name = RaveAttribute_getName(attribute);
  if (name != NULL) {
    if (!RaveAttributeHelp_extractGroupAndName(name, &gname, &aname)) {
      RAVE_ERROR1("Failed to extract group and name from %s", name);
      goto done;
    }
    if ((strcasecmp("how", gname)==0) &&RaveAttributeHelp_validateHowGroupAttributeName(gname, aname)) {
      result = RaveAttributeTable_addAttributeVersion(cvol->attrs, attribute, version, NULL);
    } else if (strcasecmp("what/prodpar", name)==0) {
      result = RaveAttributeTable_addAttributeVersion(cvol->attrs, attribute, version, NULL);
    } else {
      RAVE_WARNING1("You are not allowed to add dynamic attributes in other groups than 'how': '%s'", name);
      goto done;
    }
  }

done:
  RAVE_FREE(aname);
  RAVE_FREE(gname);
  return result;
}

RaveAttribute_t* CartesianVolume_getAttribute(CartesianVolume_t* cvol,  const char* name)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return CartesianVolume_getAttributeVersion(cvol, name, RAVEIO_API_ODIM_VERSION);
}

RaveAttribute_t* CartesianVolume_getAttributeVersion(CartesianVolume_t* cvol,  const char* name, RaveIO_ODIM_Version version)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  if (name == NULL) {
    RAVE_ERROR0("Trying to get an attribute with NULL name");
    return NULL;
  }
  return RaveAttributeTable_getAttributeVersion(cvol->attrs, name, version);
}

int CartesianVolume_hasAttribute(CartesianVolume_t* cvol,  const char* name)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return RaveAttributeTable_hasAttribute(cvol->attrs, name);
}

RaveList_t* CartesianVolume_getAttributeNames(CartesianVolume_t* cvol)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return CartesianVolume_getAttributeNamesVersion(cvol, RAVEIO_API_ODIM_VERSION);
}

RaveList_t* CartesianVolume_getAttributeNamesVersion(CartesianVolume_t* cvol, RaveIO_ODIM_Version version)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return RaveAttributeTable_getAttributeNamesVersion(cvol->attrs, version);
}

RaveObjectList_t* CartesianVolume_getAttributeValues(CartesianVolume_t* cvol)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return CartesianVolume_getAttributeValuesVersion(cvol, RAVEIO_API_ODIM_VERSION);
}

RaveObjectList_t* CartesianVolume_getAttributeValuesVersion(CartesianVolume_t* cvol, RaveIO_ODIM_Version version)
{
  RAVE_ASSERT((cvol != NULL), "cvol == NULL");
  return RaveAttributeTable_getValuesVersion(cvol->attrs, version);
}

/*@} End of Interface functions */

RaveCoreObjectType CartesianVolume_TYPE = {
    "CartesianVolume",
    sizeof(CartesianVolume_t),
    CartesianVolume_constructor,
    CartesianVolume_destructor,
    CartesianVolume_copyconstructor
};
