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
 * Used for managing attributes and handle different versions.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2022-03-30
 */
#ifndef RAVE_ATTRIBUTE_TABLE_H
#define RAVE_ATTRIBUTE_TABLE_H

#include "rave_object.h"
#include "raveobject_hashtable.h"
#include "rave_attribute.h"
#include "rave_types.h"

/**
 * Defines a attribute tablee
 */
typedef struct _RaveAttributeTable_t RaveAttributeTable_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType RaveAttributeTable_TYPE;

/**
 * Sets the default version to use for the attribute tables. Note, this does not affect the
 * internally stored attributes. They will always be latest version regardless. This affects
 * the attribute version when using the add & get without specifying version.
 * @param[in] self - self
 * @param[in] version - the version
 * @return 1 on success
 */
int RaveAttributeTable_setVersion(RaveAttributeTable_t* self, RaveIO_ODIM_Version version);

/**
 * Returns the default version to use for the attribute tables. Note, this does not affect the
 * internally stored attributes. They will always be latest version regardless. This affects
 * the attribute version when using the add & get without specifying version.
 * @param[in] self - self
 * @return the default version
 */
RaveIO_ODIM_Version RaveAttributeTable_getVersion(RaveAttributeTable_t* self);

/**
 * Adds an attribute to the attribute table.
 * @param[in] self - self
 * @param[in] attr - the attribute
 * @param[in,out] translation - if provided and attribute was translated somehow this attribute will be set. Remember to release.
 * @return 1 on success
 */
int RaveAttributeTable_addAttribute(RaveAttributeTable_t* self, RaveAttribute_t* attr, RaveAttribute_t** translation);

/**
 * Adds an attribute to the attribute table.
 * @param[in] self - self
 * @param[in] attr - the attribute
 * @param[in] version - the version of the attribute set
 * @param[in,out] translation - if provided and attribute was translated somehow this attribute will be set. Remember to release.
 * @return 1 on success
 */
int RaveAttributeTable_addAttributeVersion(RaveAttributeTable_t* self, RaveAttribute_t* attr, RaveIO_ODIM_Version version, RaveAttribute_t** translation);

/**
 * Returns an attribute from the attribute table according to default version
 * @param[in] self - self
 * @param[in] attrname - the attribute name
 * @return the attribute if found or if it could be derived from information in table, otherwise NULL
 */
RaveAttribute_t* RaveAttributeTable_getAttribute(RaveAttributeTable_t* self, const char* attrname);

/**
 * Returns an attribute from the attribute table according to specified version
 * @param[in] self - self
 * @param[in] attrname - the attribute name
 * @param[in] version - the version of the attribute to return
 * @return the attribute if found or if it could be derived from information in table, otherwise NULL
 */
RaveAttribute_t* RaveAttributeTable_getAttributeVersion(RaveAttributeTable_t* self, const char* attrname, RaveIO_ODIM_Version version);

/**
 * Returns the number of items in this table.
 * @param[in] self - the self
 * @returns the number of items in this table.
 */
int RaveAttributeTable_size(RaveAttributeTable_t* self);

/**
 * Returns if the specified key exists or not.
 * @param[in] table - self
 * @param[in] key - the key to search for
 * @returns 1 if the key exists, 0 if it doesn't
 */
int RaveAttributeTable_hasAttribute(RaveAttributeTable_t* self, const char* key);

/**
 * Removes the item with the specified key and returns it.
 * @param[in] self - the self
 * @param[in] key - the key
 * @returns the object (or NULL if none found)
 */
RaveAttribute_t* RaveAttributeTable_removeAttribute(RaveAttributeTable_t* self, const char* key);

/**
 * Clears all entries in the table.
 * @param[in] self - the self
 */
void RaveAttributeTable_clear(RaveAttributeTable_t* self);

/**
 * Returns the keys for the table at the current state. Note,
 * remember to deallocate keys appropriately after retrival or
 * use the function \@ref #RaveList_freeAndDestroy that will
 * take care of it for you.
 * @param[in] self - self
 * @returns a list containing char* pointers.
 */
RaveList_t* RaveAttributeTable_getAttributeNames(RaveAttributeTable_t* self);

/**
 * Returns the keys for the table at the current state. Note,
 * remember to deallocate keys appropriately after retrival or
 * use the function \@ref #RaveList_freeAndDestroy that will
 * take care of it for you.
 * @param[in] self - self
 * @param[in] version - the version of the attribute names to return
 * @returns a list containing char* pointers.
 */
RaveList_t* RaveAttributeTable_getAttributeNamesVersion(RaveAttributeTable_t* self, RaveIO_ODIM_Version version);

/**
 * Returns the values for the table. Note, it is cloned values
 * but references to them.
 * @param[in] self - self
 * @returns a list of values
 */
RaveObjectList_t* RaveAttributeTable_getValues(RaveAttributeTable_t* self);

/**
 * Returns the values for the table. Note, it is cloned values
 * but references to them.
 * @param[in] self - self
 * @param[in] version - the version of the attribute names to return
 * @returns a list of values
 */
RaveObjectList_t* RaveAttributeTable_getValuesVersion(RaveAttributeTable_t* self, RaveIO_ODIM_Version version);


/**
 * Returns the internal values for the table. Note, it is not cloned values but references to them.
 * @param[in] self - self
 * @returns a list of values
 */
RaveObjectList_t* RaveAttributeTable_getInternalValues(RaveAttributeTable_t* self);

/**
 * Performs a circular shift of an array attribute. if nx < 0, then shift is performed counter clockwise, if nx > 0, shift is performed clock wise, if 0, no shift is performed.
 * @param[in] self - self
 * @param[in] name - attribute to shift
 * @param[in] nx - number of positions to shift
 * return 1 if successful, 0 if trying to shift an attribute that isn't an array or an error occurs during shift.
 */
int RaveAttributeTable_shiftAttribute(RaveAttributeTable_t* self, const char* name, int nx);

/**
 * Performs a circular shift of an array attribute if it exists. if nx < 0, then shift is performed counter clockwise, if nx > 0, shift is performed clock wise, if 0, no shift is performed.
 * @param[in] self - self
 * @param[in] name - attribute to shift
 * @param[in] nx - number of positions to shift
 * return 1 if successful, 0 if trying to shift an attribute that isn't an array or an error occurs during shift.
 */
int RaveAttributeTable_shiftAttributeIfExists(RaveAttributeTable_t* self, const char* name, int nx);

/**
 * Helper function to destroy the returned list of keys.
 * @param[in] l - the list to destroy
 */
void RaveAttributeTable_destroyKeyList(RaveList_t* l);


#endif /* RAVE_ATTRIBUTE_TABLE_H */
