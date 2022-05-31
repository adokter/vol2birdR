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
 * Generic implementation of an object that is used within rave. All
 * objects should use this as template for their structure.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-11-25
 */
#include "rave_object.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include <string.h>
#include <stdio.h>

static long objectsCreated = 0;
static long objectsDestroyed = 0;

/**
 * Heap structure when allocating rave objects
 */
typedef struct _heapobject {
  RaveCoreObject* obj;         /**< stored object */
  char filename[1024];         /**< file the object was allocated in */
  int lineno;                  /**< line number */
  struct _heapobject* next;    /**< next */
  struct _heapobject* prev;    /**< previous */
} heapobject;

static heapobject* OBJECT_HEAP = NULL;
static heapobject* LAST_OBJECT_HEAP = NULL;

static heapobject* RaveCoreObjectInternal_createHeapEntry(RaveCoreObject* obj, const char* filename, int lineno)
{
  heapobject* result = RAVE_MALLOC(sizeof(heapobject));
  if (result != NULL) {
    result->obj = obj;
    strcpy(result->filename,filename);
    result->lineno = lineno;
    result->next = NULL;
    result->prev = NULL;
  }
  return result;
}

static void RaveCoreObjectInternal_objCreated(RaveCoreObject* obj, const char* filename, int lineno)
{
  heapobject* entry = NULL;

  entry = RaveCoreObjectInternal_createHeapEntry(obj, filename, lineno);

  if (entry == NULL) {
    RAVE_CRITICAL0("Could not create heap entry");
    return;
  }

  if (OBJECT_HEAP == NULL) {
    OBJECT_HEAP = entry;
    LAST_OBJECT_HEAP = entry;
  } else {
    LAST_OBJECT_HEAP->next = entry;
    entry->prev = LAST_OBJECT_HEAP;
    LAST_OBJECT_HEAP = entry;
  }
}

static void RaveCoreObjectInternal_objDestroyed(RaveCoreObject* obj)
{
  heapobject* ho = OBJECT_HEAP;
  while (ho != NULL && ho->obj != obj) {
    ho = ho->next;
  }
  if (ho != NULL && ho->obj == obj) {
    if (ho == OBJECT_HEAP) {
      if (ho->next != NULL) {
        ho->next->prev = NULL;
        OBJECT_HEAP = ho->next;
      } else {
        OBJECT_HEAP = NULL;
        LAST_OBJECT_HEAP = NULL;
      }
      RAVE_FREE(ho);
    } else if (ho == LAST_OBJECT_HEAP) {
      if (ho->prev != NULL) {
        ho->prev->next = NULL;
        LAST_OBJECT_HEAP = ho->prev;
      }
      RAVE_FREE(ho);
    } else {
      if (ho->next != NULL) {
        ho->next->prev = ho->prev;
      }
      if (ho->prev != NULL) {
        ho->prev->next = ho->next;
      }
      RAVE_FREE(ho)
    }
  }
}

RaveCoreObject* RaveCoreObject_new(RaveCoreObjectType* type, const char* filename, int lineno)
{
  RaveCoreObject* result = NULL;
  RAVE_ASSERT((type != NULL), "type == NULL");
  result = RAVE_MALLOC(type->type_size);
  if (result != NULL) {
    result->roh_refCnt = 1;
    result->roh_type = type;
    result->roh_bindingData = NULL;
    if (result->roh_type->constructor != NULL) {
      if (!result->roh_type->constructor(result)) {
        RAVE_FREE(result);
      }
    }
  }
  if (result != NULL) {
    RaveCoreObjectInternal_objCreated(result, filename, lineno);
    objectsCreated++;
  }
  return result;
}

void RaveCoreObject_release(RaveCoreObject* obj, const char* filename, int lineno)
{
  if (obj != NULL) {
    obj->roh_refCnt--;
    if (obj->roh_refCnt == 0) {
      if (obj->roh_type->destructor != NULL) {
        obj->roh_type->destructor(obj);
      }
      obj->roh_bindingData = NULL;
      RaveCoreObjectInternal_objDestroyed(obj);
      RAVE_FREE(obj);
      objectsDestroyed++;
    } else if (obj->roh_refCnt < 0) {
      Rave_printf("Got negative reference count, aborting");
      RAVE_ABORT();
    }
  }
}

RaveCoreObject* RaveCoreObject_copy(RaveCoreObject* src, const char* filename, int lineno)
{
  if (src != NULL) {
    src->roh_refCnt++;
  }
  return src;
}

RaveCoreObject* RaveCoreObject_clone(RaveCoreObject* src, const char* filename, int lineno)
{
  RaveCoreObject* result = NULL;
  if (src != NULL) {
    result = RAVE_MALLOC(src->roh_type->type_size);
    if (result != NULL) {
      result->roh_refCnt = 1;
      result->roh_type = src->roh_type;
      result->roh_bindingData = NULL;
      if (result->roh_type->copyconstructor != NULL) {
        if (!result->roh_type->copyconstructor(result, src)) {
          RAVE_FREE(result);
        }
      }
    }
  }
  if (result != NULL) {
    RaveCoreObjectInternal_objCreated(result, filename, lineno);
    objectsCreated++;
  }
  return result;
}

int RaveCoreObject_getRefCount(RaveCoreObject* src)
{
  if (src != NULL) {
    return src->roh_refCnt;
  }
  return -1;
}

void RaveCoreObject_bind(RaveCoreObject* src, void* bindingData)
{
  RAVE_ASSERT((src != NULL), "object == NULL");
  RAVE_ASSERT((bindingData != NULL), "bindingData == NULL");

  if (RaveCoreObject_getBindingData(src) != NULL) {
    RAVE_ASSERT(0, "This object has already been bound");
  }
  src->roh_bindingData = bindingData;
}

void RaveCoreObject_unbind(RaveCoreObject* src, void* bindingData)
{
  RAVE_ASSERT((src != NULL), "object == NULL");
  if (bindingData == src->roh_bindingData) {
    src->roh_bindingData = NULL;
  }
}

void* RaveCoreObject_getBindingData(RaveCoreObject* src)
{
  RAVE_ASSERT((src != NULL), "object == NULL");
  return src->roh_bindingData;
}

int RaveCoreObject_isCloneable(RaveCoreObject* src)
{
  int result = 0;
  if (src != NULL) {
    if (src->roh_type->copyconstructor != NULL) {
      result = 1;
    }
  }
  return result;
}

void RaveCoreObject_printCurrentObjectStatus(void)
{
  Rave_printf("Created: %ld, Deleted: %ld, Pending: %ld\n", objectsCreated, objectsDestroyed, objectsCreated-objectsDestroyed);
}

void RaveCoreObject_printStatistics(void)
{
  Rave_printf("Objects created: %ld\n", objectsCreated);
  Rave_printf("Objects deleted: %ld\n", objectsDestroyed);
  Rave_printf("Objects pending: %ld\n", objectsCreated - objectsDestroyed);

  if (OBJECT_HEAP != NULL) {
    heapobject* ho = OBJECT_HEAP;
    while (ho != NULL) {
      Rave_printf("%s at %s:%d has not been released (refcnt = %d)\n",
              ho->obj->roh_type->name, ho->filename, ho->lineno, ho->obj->roh_refCnt);
      ho = ho->next;
    }
  }
}
