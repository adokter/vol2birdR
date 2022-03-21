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
 * Implementation of a rave object list that ensures that the objects
 * contained within the list are released upon destruction etc.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-11-26
 */
#include "rave_list.h"
#include "raveobject_list.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include <string.h>
#include <stdio.h>
/**
 * Represents a list
 */
struct _RaveObjectList_t {
  RAVE_OBJECT_HEAD /** Always on top */
  RaveList_t* list;   /**< the list */
};


/*@{ Private functions */

/**
 * Constructor.
 */
static int RaveObjectList_constructor(RaveCoreObject* obj)
{
  RaveObjectList_t* oblist = (RaveObjectList_t*)obj;
  int result = 0;
  oblist->list = RAVE_OBJECT_NEW(&RaveList_TYPE);
  if (oblist->list != NULL) {
    result = 1;
  }
  return result;
}

/**
 * Copy constructor.
 */
static int RaveObjectList_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  RaveObjectList_t* this = (RaveObjectList_t*)obj;
  RaveObjectList_t* src = (RaveObjectList_t*)srcobj;
  int result = 0;
  this->list = RAVE_OBJECT_NEW(&RaveList_TYPE);
  if (this->list != NULL) {
    int i = 0;
    result = 1;
    int len = RaveObjectList_size(src);
    for (i = 0; result == 1 && i < len; i++) {
      RaveCoreObject* object = RaveObjectList_get(src, i);
      if (object != NULL && RAVE_OBJECT_ISCLONEABLE(object)) {
        RaveCoreObject* clone = RAVE_OBJECT_CLONE(object);
        if (clone != NULL) {
          result = RaveObjectList_add(this, clone);
        } else {
          result = 0;
        }
        RAVE_OBJECT_RELEASE(clone);
      }
      RAVE_OBJECT_RELEASE(object);
    }
  }
  return result;
}

/**
 * Destructor
 */
static void RaveObjectList_destructor(RaveCoreObject* obj)
{
  RaveObjectList_t* list = (RaveObjectList_t*)obj;
  if (list != NULL) {
    RaveCoreObject* coreobject = NULL;
    while ((coreobject = RaveObjectList_removeLast(list)) != NULL) {
      RAVE_OBJECT_RELEASE(coreobject);
    }
    RAVE_OBJECT_RELEASE(list->list);
  }
}
/*@} End of Private functions */
int RaveObjectList_add(RaveObjectList_t* list, RaveCoreObject* obj)
{
  int result = 0;
  RAVE_ASSERT((list != NULL), "list == NULL");
  RAVE_ASSERT((obj != NULL), "obj == NULL");
  RaveCoreObject* objCopy = RAVE_OBJECT_COPY(obj);
  result = RaveList_add(list->list, objCopy);
  if (!result) {
    RAVE_OBJECT_RELEASE(objCopy);
  }
  return result;
}

int RaveObjectList_insert(RaveObjectList_t* list, int index, RaveCoreObject* obj)
{
  int result = 0;
  RAVE_ASSERT((list != NULL), "list == NULL");
  RAVE_ASSERT((obj != NULL), "list == NULL");
  RaveCoreObject* objCopy = RAVE_OBJECT_COPY(obj);
  result = RaveList_insert(list->list, index, objCopy);
  if (!result) {
    RAVE_OBJECT_RELEASE(objCopy);
  }
  return result;
}

int RaveObjectList_size(RaveObjectList_t* list)
{
  RAVE_ASSERT((list != NULL), "list == NULL");
  return RaveList_size(list->list);
}

RaveCoreObject* RaveObjectList_get(RaveObjectList_t* list, int index)
{
  RAVE_ASSERT((list != NULL), "list == NULL");
  RaveCoreObject* obj = RaveList_get(list->list, index);
  return RAVE_OBJECT_COPY(obj);
}

RaveCoreObject* RaveObjectList_getLast(RaveObjectList_t* list)
{
  RAVE_ASSERT((list != NULL), "list == NULL");
  RaveCoreObject* obj = RaveList_getLast(list->list);
  return RAVE_OBJECT_COPY(obj);
}

RaveCoreObject* RaveObjectList_remove(RaveObjectList_t* list, int index)
{
  RAVE_ASSERT((list != NULL), "list == NULL");
  RaveCoreObject* obj = RaveList_remove(list->list, index);
  return obj;
}

void RaveObjectList_release(RaveObjectList_t* list, int index)
{
  RAVE_ASSERT((list != NULL), "list == NULL");
  RaveCoreObject* obj = RaveList_remove(list->list, index);
  RAVE_OBJECT_RELEASE(obj);
}

void RaveObjectList_clear(RaveObjectList_t* list)
{
  RaveCoreObject* coreobject = NULL;
  RAVE_ASSERT((list != NULL), "list == NULL");
  while ((coreobject = RaveObjectList_removeLast(list)) != NULL) {
    RAVE_OBJECT_RELEASE(coreobject);
  }
}

RaveCoreObject* RaveObjectList_removeLast(RaveObjectList_t* list)
{
  RAVE_ASSERT((list != NULL), "list == NULL");
  RaveCoreObject* obj = RaveList_removeLast(list->list);
  return obj;
}

void RaveObjectList_sort(RaveObjectList_t* list, int (*sortfun)(const void*, const void*))
{
  RAVE_ASSERT((list != NULL), "list == NULL");
  RaveList_sort(list->list, sortfun);
}

int RaveObjectList_indexOf(RaveObjectList_t* list, RaveCoreObject* obj)
{
  int nsize = 0, i = 0;
  RAVE_ASSERT((list != NULL), "list == NULL");
  nsize = RaveList_size(list->list);
  if (obj != NULL) {
    for (i = 0; i < nsize; i++) {
      if ((void*)obj == (void*)RaveList_get(list->list, i)) {
        return i;
      }
    }
  }
  return -1;
}
/*@} End of Interface functions */

RaveCoreObjectType RaveObjectList_TYPE = {
    "RaveObjectList",
    sizeof(RaveObjectList_t),
    RaveObjectList_constructor,
    RaveObjectList_destructor,
    RaveObjectList_copyconstructor
};
