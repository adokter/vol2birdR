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
 * This object supports \ref #RAVE_OBJECT_CLONE with an exception, if any members
 * of the list is not possible to clone, they will not be added to the list which
 * means that the returned list might have fewer entries.
 *
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-11-26
 */
#ifndef RAVEOBJECT_LIST_H
#define RAVEOBJECT_LIST_H

#include "rave_object.h"

/**
 * Defines a list
 */
typedef struct _RaveObjectList_t RaveObjectList_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType RaveObjectList_TYPE;

/**
 * Add one instance to the list.
 * @param[in] list - the list
 * @param[in] obj - the object
 * @returns 1 on success, otherwise 0
 */
int RaveObjectList_add(RaveObjectList_t* list, RaveCoreObject* obj);

/**
 * Inserts the object at the specified index, if index < 0 or
 * index > size, then this function will add the object to the
 * end of the list.
 * @param[in] list - the list
 * @param[in] index - the index where to insert the object
 * @param[in] obj - the object to insert.
 */
int RaveObjectList_insert(RaveObjectList_t* list, int index, RaveCoreObject* obj);

/**
 * Returns the number of items in this list.
 * @param[in] list - the list
 * @returns the number of items in this list.
 */
int RaveObjectList_size(RaveObjectList_t* list);

/**
 * Returns the item at the specified position.
 * @param[in] list - the list
 * @param[in] index - the index of the requested item
 * @returns the object
 */
RaveCoreObject* RaveObjectList_get(RaveObjectList_t* list, int index);

/**
 * Returns the item at the end.
 * @param[in] list - the list
 * @returns the object
 */
RaveCoreObject* RaveObjectList_getLast(RaveObjectList_t* list);

/**
 * Removes the item at the specified position and returns it.
 * @param[in] list - the list
 * @param[in] index - the index of the requested item
 * @returns the object
 */
RaveCoreObject* RaveObjectList_remove(RaveObjectList_t* list, int index);

/**
 * Removes the item at the specified position and releases it.
 * @param[in] list - the list
 * @param[in] index - the index of the requested item
 * @returns the object
 */
void RaveObjectList_release(RaveObjectList_t* list, int index);

/**
 * Removes all entries from the list.
 * @param[in] list - the list
 */
void RaveObjectList_clear(RaveObjectList_t* list);

/**
 * Removes the last item.
 * @param[in] list - the list
 * @returns the object or NULL if there are no objects
 */
RaveCoreObject* RaveObjectList_removeLast(RaveObjectList_t* list);

/**
 * Sorts the list according to the provided sort function.
 * The sort function should return an integer less than,
 * equal to or greater than zero depending on how the first
 * argument is in relation to the second argument.
 *
 * @param[in] list - the list
 * @param[in] sortfun - the sorting function.
 */
void RaveObjectList_sort(RaveObjectList_t* list, int (*sortfun)(const void*, const void*));

/**
 * Locates the object at returns the index in the list. The comparision is
 * based on addresses.
 * @param[in] list - the list
 * @obj[in] the object to find
 * @return the index if found, otherwise -1
 */
int RaveObjectList_indexOf(RaveObjectList_t* list, RaveCoreObject* obj);

#endif /* RAVEOBJECT_LIST_H */
