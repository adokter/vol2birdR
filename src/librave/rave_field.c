/* --------------------------------------------------------------------
Copyright (C) 2009-2010 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Generic field that only provides a 2-dim data field and a number of dynamic
 * attributes.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-07-05
 */
#include "rave_field.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "raveobject_hashtable.h"
#include "rave_utilities.h"
#include <string.h>
/**
 * Represents the cartesian volume
 */
struct _RaveField_t {
  RAVE_OBJECT_HEAD /** Always on top */
  RaveData2D_t* data; /**< the data */
  LazyDataset_t* lazyDataset; /**< the lazy dataset */
  RaveObjectHashTable_t* attrs; /**< attributes */
};

/*@{ Private functions */
/**
 * Constructor.
 */
static int RaveField_constructor(RaveCoreObject* obj)
{
  RaveField_t* this = (RaveField_t*)obj;
  this->attrs = RAVE_OBJECT_NEW(&RaveObjectHashTable_TYPE);
  this->data = RAVE_OBJECT_NEW(&RaveData2D_TYPE);
  this->lazyDataset = NULL;
  if (this->attrs == NULL || this->data == NULL) {
    goto error;
  }
  return 1;
error:
  RAVE_OBJECT_RELEASE(this->attrs);
  RAVE_OBJECT_RELEASE(this->data);
  return 0;
}

/**
 * Ensures that we have got a data 2d field to work with regardless
 * if it already has been set or if it supposed to be lazy loaded.
 * @param[in] field - the rave field
 * @returns a loaded rave data 2D field
 */
static RaveData2D_t* RaveFieldInternal_ensureData2D(RaveField_t* field)
{
  if (field->lazyDataset != NULL) {
    RaveData2D_t* loaded = LazyDataset_get(field->lazyDataset);
    if (loaded != NULL) {
      /*fprintf(stderr, "RaveFieldInternal_ensureData2D: LazyDataset fetched\n");*/
      RAVE_OBJECT_RELEASE(field->data);
      field->data = RAVE_OBJECT_COPY(loaded);
      RAVE_OBJECT_RELEASE(field->lazyDataset);
    }
    RAVE_OBJECT_RELEASE(loaded);
  }
  return field->data;
}

/**
 * Copy constructor.
 */
static int RaveField_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  RaveField_t* this = (RaveField_t*)obj;
  RaveField_t* src = (RaveField_t*)srcobj;
  this->attrs = RAVE_OBJECT_CLONE(src->attrs);
  this->data = RAVE_OBJECT_CLONE(RaveFieldInternal_ensureData2D(src));
  this->lazyDataset = NULL;
  if (this->data == NULL || this->attrs == NULL) {
    RAVE_ERROR0("Failed to duplicate data or attributes");
    goto error;
  }
  return 1;
error:
  RAVE_OBJECT_RELEASE(this->attrs);
  RAVE_OBJECT_RELEASE(this->data);
  return 0;
}

/**
 * Destroys the cartesian product
 * @param[in] scan - the cartesian product to destroy
 */
static void RaveField_destructor(RaveCoreObject* obj)
{
  RaveField_t* this = (RaveField_t*)obj;
  RAVE_OBJECT_RELEASE(this->attrs);
  RAVE_OBJECT_RELEASE(this->data);
  RAVE_OBJECT_RELEASE(this->lazyDataset);
}


/*@} End of Private functions */

/*@{ Interface functions */
int RaveField_setData(RaveField_t* field, long xsize, long ysize, void* data, RaveDataType type)
{
  int result = 0;
  RAVE_ASSERT((field != NULL), "field == NULL");
  result = RaveData2D_setData(field->data, xsize, ysize, data, type);
  if (result) {
    RAVE_OBJECT_RELEASE(field->lazyDataset);
  }
  return result;
}

int RaveField_setLazyDataset(RaveField_t* field, LazyDataset_t* lazyDataset)
{
  RAVE_ASSERT((field != NULL), "field == NULL");
  if (RaveData2D_getData(field->data) == NULL) {
    field->lazyDataset = RAVE_OBJECT_COPY(lazyDataset);
    return 1;
  } else {
    RAVE_ERROR0("Trying to set lazy dataset loader when data exists");
    return 0;
  }
}

int RaveField_createData(RaveField_t* field, long xsize, long ysize, RaveDataType type)
{
  int result = 0;
  RAVE_ASSERT((field != NULL), "field == NULL");
  result = RaveData2D_createData(field->data, xsize, ysize, type, 0);
  if (result) {
    RAVE_OBJECT_RELEASE(field->lazyDataset);
  }
  return result;
}

int RaveField_setDatafield(RaveField_t* field, RaveData2D_t* datafield)
{
  int result = 0;
  RAVE_ASSERT((field != NULL), "field == NULL");
  if (datafield != NULL) {
    RaveData2D_t* d = RAVE_OBJECT_CLONE(datafield);
    if (d != NULL) {
      RAVE_OBJECT_RELEASE(field->data);
      RAVE_OBJECT_RELEASE(field->lazyDataset);
      field->data = d;
      result = 1;
    } else {
      RAVE_ERROR0("Failed to clone 2d field");
    }
  }
  return result;
}

void* RaveField_getData(RaveField_t* field)
{
  RAVE_ASSERT((field != NULL), "field == NULL");
  return RaveData2D_getData(RaveFieldInternal_ensureData2D(field));
}

RaveData2D_t* RaveField_getDatafield(RaveField_t* field)
{
  RaveData2D_t* result = NULL;

  RAVE_ASSERT((field != NULL), "field == NULL");

  result = RAVE_OBJECT_CLONE(RaveFieldInternal_ensureData2D(field));
  if (result == NULL) {
    RAVE_ERROR0("Failed to clone data field");
  }

  return result;
}

int RaveField_getValue(RaveField_t* field, long x, long y, double* v)
{
  RAVE_ASSERT((field != NULL), "field == NULL");
  return RaveData2D_getValue(RaveFieldInternal_ensureData2D(field), x, y, v);
}

int RaveField_setValue(RaveField_t* field, long x, long y, double value)
{
  RAVE_ASSERT((field != NULL), "field == NULL");
  return RaveData2D_setValue(RaveFieldInternal_ensureData2D(field), x, y, value);
}

int RaveField_getConvertedValue(RaveField_t* field, long x, long y, double* v)
{
  double offset = 0.0, gain = 1.0;
  RaveAttribute_t* attr = NULL;
  int result = 0;

  RAVE_ASSERT((field != NULL), "field == NULL");

  attr = RaveField_getAttribute(field, "what/gain");
  if (attr != NULL) {
    RaveAttribute_getDouble(attr, &gain);
  }
  RAVE_OBJECT_RELEASE(attr);
  attr = RaveField_getAttribute(field, "what/offset");
  if (attr != NULL) {
    RaveAttribute_getDouble(attr, &offset);
  }
  RAVE_OBJECT_RELEASE(attr);

  result = RaveData2D_getValue(RaveFieldInternal_ensureData2D(field), x, y, v);
  if (result) {
    *v = offset + (*v) * gain;
  }
  return result;
}

long RaveField_getXsize(RaveField_t* field)
{
  RAVE_ASSERT((field != NULL), "field == NULL");
  if (field->lazyDataset != NULL) {
    return LazyDataset_getXsize(field->lazyDataset);
  }
  return RaveData2D_getXsize(field->data);
}

long RaveField_getYsize(RaveField_t* field)
{
  RAVE_ASSERT((field != NULL), "field == NULL");
  if (field->lazyDataset != NULL) {
    return LazyDataset_getYsize(field->lazyDataset);
  }
  return RaveData2D_getYsize(field->data);
}

RaveDataType RaveField_getDataType(RaveField_t* field)
{
  RAVE_ASSERT((field != NULL), "field == NULL");
  if (field->lazyDataset != NULL) {
    return LazyDataset_getDataType(field->lazyDataset);
  }
  return RaveData2D_getType(field->data);
}

int RaveField_addAttribute(RaveField_t* field,  RaveAttribute_t* attribute)
{
  char* aname = NULL;
  char* gname = NULL;
  const char* name = NULL;
  int result = 0;

  RAVE_ASSERT((field != NULL), "field == NULL");

  name = RaveAttribute_getName(attribute);
  if (!RaveAttributeHelp_extractGroupAndName(name, &gname, &aname)) {
    RAVE_ERROR1("Failed to extract group and name from %s", name);
    goto done;
  }

  if ((strcasecmp("how", gname)==0 && RaveAttributeHelp_validateHowGroupAttributeName(gname, aname)) ||
      ((strcasecmp("what", gname)==0 || strcasecmp("where", gname)==0) && strchr(aname, '/') == NULL)) {
    result = RaveObjectHashTable_put(field->attrs, name, (RaveCoreObject*)attribute);
  }

done:
  RAVE_FREE(gname);
  RAVE_FREE(aname);
  return result;
}

RaveAttribute_t* RaveField_getAttribute(RaveField_t* field, const char* name)
{
  RAVE_ASSERT((field != NULL), "field == NULL");
  if (name == NULL) {
    RAVE_ERROR0("Trying to get an attribute with NULL name");
    return NULL;
  }
  return (RaveAttribute_t*)RaveObjectHashTable_get(field->attrs, name);
}

int RaveField_hasAttribute(RaveField_t* field, const char* name)
{
  RAVE_ASSERT((field != NULL), "field == NULL");
  return RaveObjectHashTable_exists(field->attrs, name);
}

RaveList_t* RaveField_getAttributeNames(RaveField_t* field)
{
  RAVE_ASSERT((field != NULL), "field == NULL");
  return RaveObjectHashTable_keys(field->attrs);
}

RaveObjectList_t* RaveField_getAttributeValues(RaveField_t* field)
{
  RaveObjectList_t* result = NULL;
  RaveObjectList_t* tableattrs = NULL;

  RAVE_ASSERT((field != NULL), "field == NULL");

  tableattrs = RaveObjectHashTable_values(field->attrs);
  if (tableattrs == NULL) {
    goto error;
  }
  result = RAVE_OBJECT_CLONE(tableattrs);
  if (result == NULL) {
    goto error;
  }

  RAVE_OBJECT_RELEASE(tableattrs);
  return result;
error:
  RAVE_OBJECT_RELEASE(result);
  RAVE_OBJECT_RELEASE(tableattrs);
  return NULL;
}

void RaveField_removeAttributes(RaveField_t* field)
{
  RAVE_ASSERT((field != NULL), "field == NULL");
  RaveObjectHashTable_clear(field->attrs);
}

int RaveField_hasAttributeStringValue(RaveField_t* field, const char* name, const char* value)
{
  RaveAttribute_t* attr = NULL;
  int result = 0;

  RAVE_ASSERT((field != NULL), "field == NULL");
  if (name != NULL && value != NULL) {
    attr = (RaveAttribute_t*)RaveObjectHashTable_get(field->attrs, name);
    if (attr != NULL && RaveAttribute_getFormat(attr) == RaveAttribute_Format_String) {
      char* aval = NULL;
      RaveAttribute_getString(attr, &aval);
      if (aval != NULL && strcmp(aval, value) == 0) {
        result = 1;
      }
    }
  }
  RAVE_OBJECT_RELEASE(attr);
  return result;
}

RaveField_t* RaveField_concatX(RaveField_t* field, RaveField_t* other)
{
  RaveField_t *result = NULL;
  RaveData2D_t* dfield = NULL;

  RAVE_ASSERT((field != NULL), "field == NULL");
  if (other == NULL) {
    return NULL;
  }

  dfield = RaveData2D_concatX(RaveFieldInternal_ensureData2D(field), RaveFieldInternal_ensureData2D(other));
  if (dfield != NULL) {
    result = RAVE_OBJECT_NEW(&RaveField_TYPE);
    if (result == NULL) {
      RAVE_ERROR0("Failed to create rave field");
    } else {
      RAVE_OBJECT_RELEASE(result->data);
      result->data = RAVE_OBJECT_COPY(dfield);
    }
  }

  RAVE_OBJECT_RELEASE(dfield);
  return result;
}

/**
 * Circular shift of the internal field in x & y dimension.
 * @param[in] field - the field to be shifted
 * @param[in] nx - the number of steps to be shifted in x-direction. Can be both positive and negative
 * @param[in] ny - the number of steps to be shifted in y-direction. Can be both positive and negative
 * @returns 1 if shift was successful otherwise 0
 */
int RaveField_circshiftData(RaveField_t* field, int nx, int ny)
{
  RAVE_ASSERT((field != NULL), "field == NULL");
  return RaveData2D_circshiftData(RaveFieldInternal_ensureData2D(field), nx, ny);
}

/*@} End of Interface functions */


RaveCoreObjectType RaveField_TYPE = {
    "RaveField",
    sizeof(RaveField_t),
    RaveField_constructor,
    RaveField_destructor,
    RaveField_copyconstructor
};
