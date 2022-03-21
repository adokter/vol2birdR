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
 * Implementation of a simple list.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-11-20
 */
#include "rave_list.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include <string.h>

#define LIST_EXPAND_NR_ENTRIES 20  /**< Expand the list array with this number of entries / reallocation */
#define DEFAULT_NR_RAVE_LIST_ENTRIES 20 /**< Default number of list entries */

/**
 * Represents a list
 */
struct _RaveList_t {
  RAVE_OBJECT_HEAD /** Always on top */
  void** list;     /**< the list */
  int nrEntries;   /**< the number of entries */
  int nrAlloc;     /**< the number of available positions to store entries in */
};

/*@{ Private functions */

/**
 * Should be called before adding an entry to a list so that the size of the list
 * never is to small when inserting a new entry.
 * @param[in] list - the list
 * @returns 1 on success or 0 on any kind of failure.
 */
static int RaveListInternal_ensureCapacity(RaveList_t* list)
{
  int result = 0;
  RAVE_ASSERT((list != NULL), "list == NULL");
  if (list->nrAlloc == 0 && list->list == NULL) {
    list->list = RAVE_MALLOC(sizeof(void*)*DEFAULT_NR_RAVE_LIST_ENTRIES);
    if (list->list == NULL) {
      RAVE_CRITICAL0("Failed to create list storage");
      goto done;
    }
    list->nrAlloc = DEFAULT_NR_RAVE_LIST_ENTRIES;
  }
  else if (list->nrEntries >= list->nrAlloc - 1) {
    int nsz = list->nrAlloc + LIST_EXPAND_NR_ENTRIES;
    void** narr = RAVE_REALLOC(list->list, nsz * sizeof(void*));
    int i;
    if (narr == NULL) {
      RAVE_CRITICAL0("Failed to reallocate memory for list");
      goto done;
    }
    list->list = narr;
    for (i = list->nrEntries; i < nsz; i++) {
      list->list[i] = NULL;
    }
    list->nrAlloc = nsz;
  }
  result = 1;
done:
  return result;
}

static int RaveList_constructor(RaveCoreObject* obj)
{
  RaveList_t* rlist = (RaveList_t*)obj;
  rlist->list = NULL;
  rlist->nrAlloc = 0;
  rlist->nrEntries = 0;
  return 1;
}

/**
 * Destroys the list
 * @param[in] list - the list to destroy
 */
static void RaveList_destructor(RaveCoreObject* obj)
{
  RaveList_t* list = (RaveList_t*)obj;
  if (list != NULL) {
    RAVE_FREE(list->list);
  }
}
/*@} End of Private functions */

/*@{ Interface functions */
int RaveList_add(RaveList_t* list, void* ob)
{
  RAVE_ASSERT((list != NULL), "list == NULL");
  return RaveList_insert(list, -1, ob);
}

int RaveList_insert(RaveList_t* list, int index, void* ob)
{
  int result = 0;
  RAVE_ASSERT((list != NULL), "list == NULL");

  if (!RaveListInternal_ensureCapacity(list)) {
    RAVE_CRITICAL0("Can not add entry to list since size does not allow it");
    goto done;
  }

  if (index < 0 || index >= list->nrEntries) {
    list->list[list->nrEntries++] = ob;
  } else {
    int i = 0;
    for (i = list->nrEntries; i > index; i--) {
      list->list[i] = list->list[i-1];
    }
    list->list[index] = ob;
    list->nrEntries++;
  }

  result = 1;
done:
  return result;
}

int RaveList_size(RaveList_t* list)
{
  RAVE_ASSERT((list != NULL), "list == NULL");
  return list->nrEntries;
}

void* RaveList_get(RaveList_t* list, int index)
{
  RAVE_ASSERT((list != NULL), "list == NULL");
  if (index >= 0 && index < list->nrEntries) {
    return list->list[index];
  }
  return NULL;
}

void* RaveList_getLast(RaveList_t* list)
{
  RAVE_ASSERT((list != NULL), "list == NULL");
  if (list->nrEntries > 0) {
    return list->list[list->nrEntries-1];
  }
  return NULL;
}

void* RaveList_remove(RaveList_t* list, int index)
{
  void* result = NULL;
  RAVE_ASSERT((list != NULL), "list == NULL");
  if (index >= 0 && index < list->nrEntries) {
    int i = 0;
    int ne = list->nrEntries-1;
    result = list->list[index];
    for (i = index; i < ne; i++) {
      list->list[i] = list->list[i+1];
    }
    list->nrEntries--;
  }
  return result;
}

void* RaveList_removeLast(RaveList_t* list)
{
  void* result = NULL;
  RAVE_ASSERT((list != NULL), "list == NULL");
  if (list->nrEntries > 0) {
    result = list->list[--list->nrEntries];
  }
  return result;
}

void RaveList_removeObject(RaveList_t* list, void* object)
{
  int index = -1;
  int i = 0;
  RAVE_ASSERT((list != NULL), "list == NULL");
  RAVE_ASSERT((object != NULL), "object == NULL");
  for (i = 0; index == -1 && i < list->nrEntries; i++) {
    if (list->list[i] == object) {
      index = i;
    }
  }
  if (index >= 0) {
    (void)RaveList_remove(list, index);
  }
}

void* RaveList_find(RaveList_t* list, void* expected, int (*findfunc)(void*, void*))
{
  int i = 0;
  RAVE_ASSERT((list != NULL), "list == NULL");
  RAVE_ASSERT((findfunc != NULL), "findfunc == NULL");

  for (i = 0; i < list->nrEntries; i++) {
    if (findfunc(expected, list->list[i])) {
      return list->list[i];
    }
  }
  return NULL;
}


void RaveList_sort(RaveList_t* list, int (*sortfun)(const void*, const void*))
{
  RAVE_ASSERT((list != NULL), "list == NULL");
  qsort(list->list, list->nrEntries, sizeof(void*), sortfun);
}

void RaveList_freeAndDestroy(RaveList_t** list)
{
  void* ptr = NULL;
  RAVE_ASSERT((list != NULL), "list == NULL");
  if (*list != NULL) {
    while ((ptr = RaveList_removeLast(*list)) != NULL) {
      RAVE_FREE(ptr);
    }
    RAVE_OBJECT_RELEASE(*list);
  }
}
/*@} End of Interface functions */

RaveCoreObjectType RaveList_TYPE = {
    "RaveList",
    sizeof(RaveList_t),
    RaveList_constructor,
    RaveList_destructor
};
