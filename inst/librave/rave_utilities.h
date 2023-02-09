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
 * Contains various utility functions that makes life easier when working
 * with the rave framework.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-06-10
 */
#ifndef RAVE_UTILITIES_H
#define RAVE_UTILITIES_H
#include "rave_attribute.h"
#include "raveobject_list.h"
#include "raveobject_hashtable.h"

#ifdef _MSC_VER
#ifndef strncasecmp
#define strcasecmp(x, y) _stricmp(x,y)
#endif
#endif

/**
 * Adds a long attribute to an object list.
 * @param[in] l - the list
 * @param[in] name - the name of the attribute
 * @param[in] value - the long
 * @returns 1 on success otherwise 0
 */
int RaveUtilities_addLongAttributeToList(RaveObjectList_t* l, const char* name, long value);

/**
 * Adds a double attribute to an object list.
 * @param[in] l - the list
 * @param[in] name - the name of the attribute
 * @param[in] value - the double
 * @returns 1 on success otherwise 0
 */
int RaveUtilities_addDoubleAttributeToList(RaveObjectList_t* l, const char* name, double value);

/**
 * Adds a string attribute to an object list.
 * @param[in] l - the list
 * @param[in] name - the name of the attribute
 * @param[in] value - the string
 * @returns 1 on success otherwise 0
 */
int RaveUtilities_addStringAttributeToList(RaveObjectList_t* l, const char* name, const char* value);

/**
 * Replaces the content of a attribute in the object list. If the attribute does not exist a new
 * one will be created.
 * @param[in] l - the list
 * @param[in] name - the name of the attribute
 * @param[in] value - the long
 * @returns 1 on success otherwise 0
 */
int RaveUtilities_replaceLongAttributeInList(RaveObjectList_t* l, const char* name, long value);

/**
 * Replaces the content of a attribute in the object list. If the attribute does not exist a new
 * one will be created.
 * @param[in] l - the list
 * @param[in] name - the name of the attribute
 * @param[in] value - the double
 * @returns 1 on success otherwise 0
 */
int RaveUtilities_replaceDoubleAttributeInList(RaveObjectList_t* l, const char* name, double value);

/**
 * Replaces the content of a attribute in the object list. If the attribute does not exist a new
 * one will be created.
 * @param[in] l - the list
 * @param[in] name - the name of the attribute
 * @param[in] value - the string
 * @returns 1 on success otherwise 0
 */
int RaveUtilities_replaceStringAttributeInList(RaveObjectList_t* l, const char* name, const char* value);

/**
 * Removes the rave attribute with specified name from the list
 * @param[in] l - the list
 * @param[in] name - the name
 */
void RaveUtilities_removeAttributeFromList(RaveObjectList_t* l, const char* name);

/**
 * Gets the double value from a rave attribute that resides in a hash table
 * @param[in] h - the hash table
 * @param[in] name - the name
 * @param[in,out] v - the value
 * @returns 1 on success otherwise 0
 */
int RaveUtilities_getRaveAttributeDoubleFromHash(RaveObjectHashTable_t* h, const char* name, double* v);

/**
 * Returns if the character is a whitespace character or not,
 * i.e. ' ', '\t', '\r' or '\n'
 * @param[in] c - the character to check
 * @returns true if character is a whitespace otherwise 0
 */
int RaveUtilities_iswhitespace(char c);

/**
 * Trims the text from all leading and trailing whitespaces.
 * @param[in] str - the string to trim
 * @param[in] len - the length of the string to trim
 * @returns a new trimmed string, release with RAVE_FREE
 */
char* RaveUtilities_trimText(const char* str, int len);

/**
 * Returns a list of tokens delimited by 'c'. The tokens will
 * be trimmed from any leading and trailing whitespaces.
 * @param[in] str - the string to tokenize
 * @param[in] c - the delimiter
 * @returns a list of tokens, use @ref RaveList_freeAndDestroy to delete
 */
RaveList_t* RaveUtilities_getTrimmedTokens(const char* str, int c);

/**
 * Returns if xml support is activated or not since expat support
 * is optional and ought to be tested.
 * @returns 0 if xml isn't supported in the build, otherwise 1
 */
int RaveUtilities_isXmlSupported(void);

/**
 * Returns if CF convention IO support is activated or not.
 * @returns 0 if CF convention IO isn't supported in the build, otherwise 1
 */
int RaveUtilities_isCFConventionSupported(void);

/**
 * Returns if legacy PROJ (PROJ.4 and PROJ 5) is enabled or not
 * @returns 0 if legacy PROJ isn't enabled in the build, otherwise 1
 */
int RaveUtilities_isLegacyProjEnabled(void);

/**
 * Handles the source value according to version. For example, if version < 2.3, then WIGOS will be removed.
 * @param[in] source - source to be verified
 * @param[in] version - what version we want to validate against.
 * @returns the modified version. Note, should be freed with RAVE_FREE.
 */
char* RaveUtilities_handleSourceVersion(const char* source, RaveIO_ODIM_Version version);

/**
 * Verifies if the source is valid according to ODIM version rules. Currently
 * the only restriction is for version >= 2.4 which says that either NOD or ORG must
 * exist in the file depending on object type. We will not care about object type
 * and only check if one or both of NOD or ORG exists.
 * @param[in] source - the source to check
 * @param[in] version - the version we are testing against.
 * @returns 1 if source is valid, otherwise 0
 */
int RaveUtilities_isSourceValid(const char* source, RaveIO_ODIM_Version version);

#endif /* RAVE_UTILITIES_H */
