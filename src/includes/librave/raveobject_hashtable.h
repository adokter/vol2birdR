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
 * Implementation of a rave object hashtable that maps between strings and
 * rave core objects.
 * This object currently does not support \ref #RAVE_OBJECT_CLONE.
 *
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-01-21
 */
#ifndef RAVEOBJECT_HASHTABLE_H_
#define RAVEOBJECT_HASHTABLE_H_

#include "rave_object.h"
#include "rave_list.h"
#include "raveobject_list.h"

/**
 * Defines a hash table
 */
typedef struct _RaveObjectHashTable_t RaveObjectHashTable_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType RaveObjectHashTable_TYPE;

/**
 * Inserts a key - object binding in the table.
 * @param[in] table - the table
 * @param[in] key - the key
 * @param[in] obj - the object
 * @returns 1 on success, otherwise 0
 */
int RaveObjectHashTable_put(RaveObjectHashTable_t* table, const char* key, RaveCoreObject* obj);

/**
 * Returns the object referred to by key
 * @param[in] table - the table
 * @param[in] key - the key
 * @returns the object (or NULL if not found or error)
 */
RaveCoreObject* RaveObjectHashTable_get(RaveObjectHashTable_t* table, const char* key);

/**
 * Returns the number of items in this table.
 * @param[in] table - the table
 * @returns the number of items in this table.
 */
int RaveObjectHashTable_size(RaveObjectHashTable_t* table);

/**
 * Returns if the specified key exists or not.
 * @param[in] table - self
 * @param[in] key - the key to search for
 * @returns 1 if the key exists, 0 if it doesn't
 */
int RaveObjectHashTable_exists(RaveObjectHashTable_t* table, const char* key);

/**
 * Removes the item with the specified key and returns it.
 * @param[in] table - the table
 * @param[in] key - the key
 * @returns the object (or NULL if none found)
 */
RaveCoreObject* RaveObjectHashTable_remove(RaveObjectHashTable_t* table, const char* key);

/**
 * Clears all entries in the table.
 * @param[in] table - the table
 */
void RaveObjectHashTable_clear(RaveObjectHashTable_t* table);

/**
 * Returns the keys for the table at the current state. Note,
 * remember to deallocate keys appropriately after retrival or
 * use the function \@ref #RaveList_freeAndDestroy that will
 * take care of it for you.
 * @param[in] table - self
 * @returns a list containing char* pointers.
 */
RaveList_t* RaveObjectHashTable_keys(RaveObjectHashTable_t* table);

/**
 * Returns the values for the table. Note, it is not cloned values
 * but references to them.
 * @param[in] table - self
 * @returns a list of values
 */
RaveObjectList_t* RaveObjectHashTable_values(RaveObjectHashTable_t* table);

/**
 * Helper function to destroy the returned list of keys.
 * @param[in] l - the list to destroy
 */
void RaveObjectHashTable_destroyKeyList(RaveList_t* l);

#endif /* RAVE_HASHTABLE_H */
