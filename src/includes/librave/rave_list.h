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
 * This object does NOT support \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-11-20
 */
#ifndef RAVE_LIST_H
#define RAVE_LIST_H
#include "rave_object.h"

/**
 * Defines a list
 */
typedef struct _RaveList_t RaveList_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType RaveList_TYPE;

/**
 * Add one instance to the list.
 * @param[in] list - the list
 * @param[in] ob - the object
 * @returns 1 on success, otherwise 0
 */
int RaveList_add(RaveList_t* list, void* ob);

/**
 * Inserts the object at the specified index, if index < 0 or
 * index > size, then this function will add the object to the
 * end of the list.
 * @param[in] list - the list
 * @param[in] index - the index where to insert the object
 * @param[in] ob - the object to insert.
 */
int RaveList_insert(RaveList_t* list, int index, void* ob);

/**
 * Returns the number of items in this list.
 * @param[in] list - the list
 * @returns the number of items in this list.
 */
int RaveList_size(RaveList_t* list);

/**
 * Returns the item at the specified position.
 * @param[in] list - the list
 * @param[in] index - the index of the requested item
 * @returns the object
 */
void* RaveList_get(RaveList_t* list, int index);

/**
 * Returns the item at the end.
 * @param[in] list - the list
 * @returns the object
 */
void* RaveList_getLast(RaveList_t* list);

/**
 * Removes the item at the specified position and returns it.
 * @param[in] list - the list
 * @param[in] index - the index of the requested item
 * @returns the object
 */
void* RaveList_remove(RaveList_t* list, int index);

/**
 * Removes the last item.
 * @param[in] list - the list
 * @returns the object or NULL if there are no objects
 */
void* RaveList_removeLast(RaveList_t* list);

/**
 * Removes the object that is equal to the provided object. I.e.
 * pointer comparission is made.
 * @param[in] list - the list
 * @param[in] object - the object to remove
 */
void RaveList_removeObject(RaveList_t* list, void* object);

/**
 * Finds the object in the list. If findfunc returns 1, then it is assumed
 * to be a match and that object will be returned.
 * The prototype for findfunc is int findfunc(void* expected, void* datavalue).
 * where expected is passed to findfunc for each item (datavalue) in the list
 * until a match is found.
 * @param[in] list - the list
 * @param[in] expected - the expected data value
 * @param[in] findfunc - the finder function that takes expected value and data value as arguments.
 * @returns the found item or NULL if none was found.
 */
void* RaveList_find(RaveList_t* list, void* expected, int (*findfunc)(void*, void*));

/**
 * Sorts the list according to the provided sort function.
 * The sort function should return an integer less than,
 * equal to or greater than zero depending on how the first
 * argument is in relation to the second argument.
 *
 * @param[in] list - the list
 * @param[in] sortfun - the sorting function.
 */
void RaveList_sort(RaveList_t* list, int (*sortfun)(const void*, const void*));

/**
 * This is a specialized version that can be used to destroy the list
 * when it contains single pointers allocated with RAVE_MALLOC. For example
 * char arrays and similar. NOTE! Before using this function, ensure that
 * the list you are atempting to destroy really contains entries that
 * has been allocated with RAVE_MALLOC/RAVE_STRDUP/...
 * @param[in,out] list - self, when leaving this function, *list will be NULL.
 */
void RaveList_freeAndDestroy(RaveList_t** list);

#endif /* RAVE_LIST_H */
