/* --------------------------------------------------------------------
Copyright (C) 2013 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Defines the functions available when creating composites from cartesian products.
 *
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2013-10-01
 */
#include "cartesiancomposite.h"
#include "cartesianparam.h"
#include "cartesian.h"
#include "projection_pipeline.h"
#include "raveobject_list.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "rave_datetime.h"
#include <string.h>
#include "raveobject_hashtable.h"

/**
 * Represents the cartesian composite generator.
 */
struct _CartesianComposite_t {
  RAVE_OBJECT_HEAD /** Always on top */
  CartesianCompositeSelectionMethod_t method; /**< selection method, default CartesianCompositeSelectionMethod_FIRST */
  RaveObjectList_t* list; /**< the list of cartesian objects */
  RaveDateTime_t* datetime;  /**< the date and time */
  char* distance_field; /**< the name (how/task) of the distance field */
  char* quantity; /**< the quantity to make a composite of */
  double offset; /**< the offset for the data */
  double gain; /**< the gain for the data */
  double nodata; /**< the nodata value for the composite */
  double undetect; /**< the undetect value for the composite */
};

/** The name of the task for specifying distance to radar */
#define DISTANCE_TO_RADAR_HOW_TASK "se.smhi.composite.distance.radar"

/*@{ Private functions */
/**
 * Constructor.
 * @param[in] obj - the created object
 */
static int CartesianComposite_constructor(RaveCoreObject* obj)
{
  CartesianComposite_t* this = (CartesianComposite_t*)obj;
  this->method = CartesianCompositeSelectionMethod_FIRST;
  this->datetime = RAVE_OBJECT_NEW(&RaveDateTime_TYPE);
  this->list = RAVE_OBJECT_NEW(&RaveObjectList_TYPE);
  this->quantity = RAVE_STRDUP("DBZH");
  this->distance_field = RAVE_STRDUP(DISTANCE_TO_RADAR_HOW_TASK);
  this->offset = 0.0;
  this->gain = 1.0;
  this->nodata = 0.0;
  this->undetect = 0.0;
  if (this->list == NULL || this->datetime == NULL || this->quantity == NULL || this->distance_field == NULL) {
    goto error;
  }
  return 1;
error:
  RAVE_OBJECT_RELEASE(this->list);
  RAVE_OBJECT_RELEASE(this->datetime);
  RAVE_FREE(this->quantity);
  RAVE_FREE(this->distance_field);
  return 0;
}

/**
 * Copy constructor.
 * @param[in] obj - the created object
 * @param[in] srcobj - the source (that is copied)
 */
static int CartesianComposite_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  CartesianComposite_t* this = (CartesianComposite_t*)obj;
  CartesianComposite_t* src = (CartesianComposite_t*)srcobj;
  this->method = src->method;
  this->list = RAVE_OBJECT_CLONE(src->list);
  this->datetime = RAVE_OBJECT_CLONE(src->datetime);
  this->quantity = RAVE_STRDUP(src->quantity); /* Assuming that we never let a quantity be set to NULL */
  this->distance_field = RAVE_STRDUP(src->distance_field);
  this->offset = src->offset;
  this->gain = src->gain;
  this->nodata = src->nodata;
  this->undetect = src->undetect;
  if (this->datetime == NULL || this->list == NULL || this->quantity == NULL || this->distance_field == NULL) {
    goto error;
  }

  return 1;
error:
  RAVE_OBJECT_RELEASE(this->list);
  RAVE_OBJECT_RELEASE(this->datetime);
  RAVE_FREE(this->quantity);
  RAVE_FREE(this->distance_field);
  return 0;
}

/**
 * Destructor
 * @param[in] obj - the object to destroy
 */
static void CartesianComposite_destructor(RaveCoreObject* obj)
{
  CartesianComposite_t* this = (CartesianComposite_t*)obj;
  RAVE_OBJECT_RELEASE(this->datetime);
  RAVE_OBJECT_RELEASE(this->list);
  RAVE_FREE(this->quantity);
  RAVE_FREE(this->distance_field);
}

/**
 * Creates the resulting composite image.
 * @param[in] self - self
 * @param[in] area - the area the composite image(s) should have
 * @returns the cartesian on success otherwise NULL
 */
static Cartesian_t* CartesianComposite_createCompositeImage(CartesianComposite_t* self, Area_t* area)
{
  Cartesian_t *result = NULL, *cartesian = NULL;
  RaveAttribute_t* prodpar = NULL;
  CartesianParam_t* cp = NULL;

  RAVE_ASSERT((self != NULL), "self == NULL");

  cartesian = RAVE_OBJECT_NEW(&Cartesian_TYPE);
  if (cartesian == NULL) {
    goto done;
  }
  Cartesian_init(cartesian, area);

  prodpar = RaveAttributeHelp_createString("what/prodpar", "some relevant information");
  if (prodpar == NULL) {
    goto done;
  }

  Cartesian_setObjectType(cartesian, Rave_ObjectType_COMP);
  Cartesian_setProduct(cartesian, Rave_ProductType_COMP);
  if (!Cartesian_addAttribute(cartesian, prodpar)) {
    goto done;
  }
  if (CartesianComposite_getTime(self) != NULL) {
    if (!Cartesian_setTime(cartesian, CartesianComposite_getTime(self))) {
      goto done;
    }
  }
  if (CartesianComposite_getDate(self) != NULL) {
    if (!Cartesian_setDate(cartesian, CartesianComposite_getDate(self))) {
      goto done;
    }
  }
  if (!Cartesian_setSource(cartesian, Area_getID(area))) {
    goto done;
  }

  cp = Cartesian_createParameter(cartesian, self->quantity, RaveDataType_UCHAR, 0);
  if (cp == NULL) {
    goto done;
  }
  CartesianParam_setNodata(cp, self->nodata);
  CartesianParam_setUndetect(cp, self->undetect);
  CartesianParam_setGain(cp, self->gain);
  CartesianParam_setOffset(cp, self->offset);
  RAVE_OBJECT_RELEASE(cp);

  result = RAVE_OBJECT_COPY(cartesian);
done:
  RAVE_OBJECT_RELEASE(cartesian);
  RAVE_OBJECT_RELEASE(prodpar);
  return result;
}

static int HasAllCartesianDistanceField(CartesianComposite_t* self)
{
  Cartesian_t* cartesian = NULL;
  RaveField_t* howtaskfield = NULL;
  int i = 0;
  int nc = CartesianComposite_getNumberOfObjects(self);
  int result = 0;
  for (i = 0; i < nc; i++) {
    cartesian = CartesianComposite_get(self, i);
    if (cartesian == NULL) {
      RAVE_WARNING0("Something is wrong with the in-objects to the composite generator");
      goto done;
    }
    howtaskfield = Cartesian_findQualityFieldByHowTask(cartesian, self->distance_field);
    if (howtaskfield == NULL) {
      RAVE_WARNING0("All in-objects must contain the wanted distance field when generating a composite according to DISTANCE");
      goto done;
    }
    RAVE_OBJECT_RELEASE(howtaskfield);
    RAVE_OBJECT_RELEASE(cartesian);
  }
  result = 1;
done:
  RAVE_OBJECT_RELEASE(cartesian);
  RAVE_OBJECT_RELEASE(howtaskfield);
  return result;
}

/*@} End of Private functions */

/*@{ Interface functions */
int CartesianComposite_add(CartesianComposite_t* self, Cartesian_t* o)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  RAVE_ASSERT((o != NULL), "o == NULL");
  return RaveObjectList_add(self->list, (RaveCoreObject*)o);
}

int CartesianComposite_getNumberOfObjects(CartesianComposite_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveObjectList_size(self->list);
}

Cartesian_t* CartesianComposite_get(CartesianComposite_t* self, int index)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return (Cartesian_t*)RaveObjectList_get(self->list, index);
}

int CartesianComposite_setMethod(CartesianComposite_t* self, CartesianCompositeSelectionMethod_t method)
{
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (method >= CartesianCompositeSelectionMethod_FIRST && method <= CartesianCompositeSelectionMethod_DISTANCE) {
    self->method = method;
    result = 1;
  }
  return result;
}

CartesianCompositeSelectionMethod_t CartesianComposite_getMethod(CartesianComposite_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->method;
}

int CartesianComposite_setDistanceField(CartesianComposite_t* self, const char* fieldname)
{
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (fieldname != NULL) {
    char* tmp = RAVE_STRDUP(fieldname);
    if (tmp != NULL) {
      RAVE_FREE(self->distance_field);
      self->distance_field = tmp;
      result = 1;
    }
  } else {
    RAVE_INFO0("distance field can not be NULL");
  }
  return result;
}

const char* CartesianComposite_getDistanceField(CartesianComposite_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return (const char*)self->distance_field;
}

int CartesianComposite_setTime(CartesianComposite_t* self, const char* value)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveDateTime_setTime(self->datetime, value);
}

const char* CartesianComposite_getTime(CartesianComposite_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveDateTime_getTime(self->datetime);
}

int CartesianComposite_setDate(CartesianComposite_t* self, const char* value)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveDateTime_setDate(self->datetime, value);
}

const char* CartesianComposite_getDate(CartesianComposite_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveDateTime_getDate(self->datetime);
}

int CartesianComposite_setQuantity(CartesianComposite_t* self, const char* quantity)
{
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (quantity != NULL) {
    char* tmp = RAVE_STRDUP(quantity);
    if (tmp != NULL) {
      RAVE_FREE(self->quantity);
      self->quantity = tmp;
      result = 1;
    }
  } else {
    RAVE_INFO0("Quantity can not be NULL");
  }
  return result;
}

const char* CartesianComposite_getQuantity(CartesianComposite_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return (const char*)self->quantity;
}

void CartesianComposite_setGain(CartesianComposite_t* self, double gain)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (gain != 0.0) {
    self->gain = gain;
  }
}

double CartesianComposite_getGain(CartesianComposite_t* self)
{
  RAVE_ASSERT((self != NULL), "cartesian == NULL");
  return self->gain;
}

void CartesianComposite_setOffset(CartesianComposite_t* self, double offset)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->offset = offset;
}

double CartesianComposite_getOffset(CartesianComposite_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->offset;
}

void CartesianComposite_setNodata(CartesianComposite_t* self, double nodata)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->nodata = nodata;
}

double CartesianComposite_getNodata(CartesianComposite_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->nodata;
}

void CartesianComposite_setUndetect(CartesianComposite_t* self, double undetect)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->undetect = undetect;
}

double CartesianComposite_getUndetect(CartesianComposite_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->undetect;
}


Cartesian_t* CartesianComposite_nearest(CartesianComposite_t* self, Area_t* area)
{
  Cartesian_t* ct = NULL, *inobj = NULL;
  Cartesian_t* result = NULL;
  int x = 0, y = 0, i = 0, xsize = 0, ysize = 0, nimages = 0;
  double ctnodata = 255.0, ctundetect = 0.0;
  Projection_t *tgtpj = NULL, *srcpj = NULL;
  ProjectionPipeline_t* pipeline = NULL;
  RaveObjectList_t* pipelines = NULL;

  if (self->method == CartesianCompositeSelectionMethod_DISTANCE) {
    if (!HasAllCartesianDistanceField(self)) {
      RAVE_WARNING0("All in-objects does not have the required distance field");
      goto done;
    }
  }

  pipelines = RAVE_OBJECT_NEW(&RaveObjectList_TYPE);
  if (pipelines == NULL) {
    goto done;
  }

  ct = CartesianComposite_createCompositeImage(self, area);
  ctnodata = Cartesian_getNodata(ct);
  ctundetect = Cartesian_getUndetect(ct);
  xsize = Cartesian_getXSize(ct);
  ysize = Cartesian_getYSize(ct);
  srcpj = Cartesian_getProjection(ct);
  nimages = CartesianComposite_getNumberOfObjects(self);

  for (i = 0; i < nimages; i++) {
    inobj = CartesianComposite_get(self, i);
    if (inobj != NULL) {
      Projection_t* p =  Cartesian_getProjection(inobj);
      if (p != NULL) {
        pipeline = ProjectionPipeline_createPipeline(srcpj, p);
        if (pipeline == NULL || !RaveObjectList_add(pipelines, (RaveCoreObject*)pipeline)) {
          RAVE_ERROR0("Could not create pipeline");
          RAVE_OBJECT_RELEASE(p);
          RAVE_OBJECT_RELEASE(inobj);
          RAVE_OBJECT_RELEASE(pipeline);
          goto done;
        }
        RAVE_OBJECT_RELEASE(pipeline);
      } else {
        RAVE_ERROR0("Cartesian product does not have a projection?");
        RAVE_OBJECT_RELEASE(p);
        RAVE_OBJECT_RELEASE(inobj);
        goto done;
      }
      RAVE_OBJECT_RELEASE(p);
    } else {
      RAVE_ERROR1("No cartesian at %d in composite", i);
      goto done;
    }
    RAVE_OBJECT_RELEASE(inobj);
  }

  for (y = 0; y < ysize; y++) {
    double herey = Cartesian_getLocationY(ct, y);
    double tmpy = herey;
    for (x = 0; x < xsize; x++) {
      int foundradar = 0; /* Used if we want to break early or similar */
      double sum = 0.0;
      int nvals = 0;
      double minval = ctnodata, maxval = ctnodata;
      double mindistance = 1e10;

      Cartesian_setValue(ct, x, y, self->nodata);

      for (i = 0; !foundradar && i < nimages; i++) {
        double herex = Cartesian_getLocationX(ct, x);
        RaveValueType valid = RaveValueType_NODATA;
        double v = 0.0L;
        herey = tmpy; // So that we can use herey over and over again
        inobj = CartesianComposite_get(self, i);
        pipeline = (ProjectionPipeline_t*)RaveObjectList_get(pipelines, i);

        if (pipeline == NULL || !ProjectionPipeline_fwd(pipeline, herex, herey, &herex, &herey)) {
          RAVE_ERROR0("Composite generation failed");
          goto done;
        }
        RAVE_OBJECT_RELEASE(pipeline);

        valid = Cartesian_getValueAtLocation(inobj, herex, herey, &v);

        if (valid == RaveValueType_NODATA) {
          v = ctnodata;
        } else if (valid == RaveValueType_UNDETECT) {
          v = ctundetect;
        }

        if (self->method == CartesianCompositeSelectionMethod_FIRST &&
            valid == RaveValueType_DATA) {
          Cartesian_setValue(ct, x, y, v);
          foundradar = 1;
        } else if (self->method == CartesianCompositeSelectionMethod_AVGVALUE &&
            valid == RaveValueType_DATA) {
          sum += v;
          nvals++;
          Cartesian_setValue(ct, x, y, (sum / (double)nvals));
        } else if (self->method == CartesianCompositeSelectionMethod_MINVALUE &&
            valid == RaveValueType_DATA) {
          if (minval == ctnodata || minval == ctundetect) {
            minval = v;
          } else if (minval > v) {
            minval = v;
          }
          Cartesian_setValue(ct, x, y, minval);
        } else if (self->method == CartesianCompositeSelectionMethod_MAXVALUE &&
            valid == RaveValueType_DATA) {
          if (maxval == ctnodata || maxval == ctundetect) {
            maxval = v;
          } else if (maxval < v) {
            maxval = v;
          }
          Cartesian_setValue(ct, x, y, maxval);
        } else if (self->method == CartesianCompositeSelectionMethod_DISTANCE &&
            valid == RaveValueType_DATA) {
          double cdistance = 0.0;
          if (Cartesian_getQualityValueAtLocation(inobj, herex, herey, self->distance_field, &cdistance)) {
            if (cdistance < mindistance) {
              mindistance = cdistance;
              Cartesian_setValue(ct, x, y, v);
            }
          }
        } else if (valid == RaveValueType_UNDETECT) {
          double xx = 0.0;
          RaveValueType xxvalid = Cartesian_getValue(ct, x, y, &xx);
          if (xxvalid != RaveValueType_DATA) {
            Cartesian_setValue(ct, x, y, ctundetect);
          }
        }

        RAVE_OBJECT_RELEASE(inobj);
        RAVE_OBJECT_RELEASE(tgtpj);
      }
    }
  }

  result = RAVE_OBJECT_COPY(ct);
done:
  RAVE_OBJECT_RELEASE(tgtpj);
  RAVE_OBJECT_RELEASE(srcpj);
  RAVE_OBJECT_RELEASE(inobj);
  RAVE_OBJECT_RELEASE(ct);
  RAVE_OBJECT_RELEASE(pipeline);
  RAVE_OBJECT_RELEASE(pipelines);
  return result;
}

/*@} End of Interface functions */

RaveCoreObjectType CartesianComposite_TYPE = {
    "CartesianComposite",
    sizeof(CartesianComposite_t),
    CartesianComposite_constructor,
    CartesianComposite_destructor,
    CartesianComposite_copyconstructor
};
