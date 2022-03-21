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
#include "rave_utilities.h"
#include "rave_object.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include <string.h>
#include <stdio.h>

int RaveUtilities_addLongAttributeToList(RaveObjectList_t* l, const char* name, long value)
{
  int result = 0;
  RaveAttribute_t* attr = NULL;
  RAVE_ASSERT((l != NULL), "l == NULL");
  attr = RaveAttributeHelp_createLong(name, value);
  if (attr != NULL) {
    if (RaveObjectList_add(l, (RaveCoreObject*)attr)) {
      result = 1;
    }
  }
  RAVE_OBJECT_RELEASE(attr);
  return result;
}

int RaveUtilities_addDoubleAttributeToList(RaveObjectList_t* l, const char* name, double value)
{
  int result = 0;
  RaveAttribute_t* attr = NULL;
  RAVE_ASSERT((l != NULL), "l == NULL");
  attr = RaveAttributeHelp_createDouble(name, value);
  if (attr != NULL) {
    if (RaveObjectList_add(l, (RaveCoreObject*)attr)) {
      result = 1;
    }
  }
  RAVE_OBJECT_RELEASE(attr);
  return result;
}

int RaveUtilities_addStringAttributeToList(RaveObjectList_t* l, const char* name, const char* value)
{
  int result = 0;
  RaveAttribute_t* attr = NULL;
  RAVE_ASSERT((l != NULL), "l == NULL");
  attr = RaveAttributeHelp_createString(name, value);
  if (attr != NULL) {
    if (RaveObjectList_add(l, (RaveCoreObject*)attr)) {
      result = 1;
    }
  }
  RAVE_OBJECT_RELEASE(attr);
  return result;
}

int RaveUtilities_replaceLongAttributeInList(RaveObjectList_t* l, const char* name, long value)
{
  int result = 0;
  RaveAttribute_t* attr = NULL;
  int n = 0;
  int i = 0;
  RAVE_ASSERT((l != NULL), "l == NULL");
  RAVE_ASSERT((name != NULL), "name == NULL");

  n = RaveObjectList_size(l);

  for (i = 0; result == 0 && i < n; i++) {
    attr = (RaveAttribute_t*)RaveObjectList_get(l, i);
    if (attr != NULL && RaveAttribute_getName(attr) != NULL && strcmp(name, RaveAttribute_getName(attr)) == 0) {
      RaveAttribute_setLong(attr, value);
      result = 1;
    }
    RAVE_OBJECT_RELEASE(attr);
  }

  if (result == 0) {
    result = RaveUtilities_addLongAttributeToList(l, name, value);
  }

  return result;

}

int RaveUtilities_replaceDoubleAttributeInList(RaveObjectList_t* l, const char* name, double value)
{
  int result = 0;
  RaveAttribute_t* attr = NULL;
  int n = 0;
  int i = 0;
  RAVE_ASSERT((l != NULL), "l == NULL");
  RAVE_ASSERT((name != NULL), "name == NULL");

  n = RaveObjectList_size(l);

  for (i = 0; result == 0 && i < n; i++) {
    attr = (RaveAttribute_t*)RaveObjectList_get(l, i);
    if (attr != NULL && RaveAttribute_getName(attr) != NULL && strcmp(name, RaveAttribute_getName(attr)) == 0) {
      RaveAttribute_setDouble(attr, value);
      result = 1;
    }
    RAVE_OBJECT_RELEASE(attr);
  }

  if (result == 0) {
    result = RaveUtilities_addDoubleAttributeToList(l, name, value);
  }

  return result;
}

int RaveUtilities_replaceStringAttributeInList(RaveObjectList_t* l, const char* name, const char* value)
{
  int result = 0;
  RaveAttribute_t* attr = NULL;
  int n = 0;
  int i = 0;
  int found = 0;
  RAVE_ASSERT((l != NULL), "l == NULL");
  RAVE_ASSERT((name != NULL), "name == NULL");

  n = RaveObjectList_size(l);

  for (i = 0; found == 0 && i < n; i++) {
    attr = (RaveAttribute_t*)RaveObjectList_get(l, i);
    if (attr != NULL && RaveAttribute_getName(attr) != NULL && strcmp(name, RaveAttribute_getName(attr)) == 0) {
      result = RaveAttribute_setString(attr, value);
      found = 1;
    }
    RAVE_OBJECT_RELEASE(attr);
  }

  if (found == 0) {
    result = RaveUtilities_addStringAttributeToList(l, name, value);
  }

  return result;
}

void RaveUtilities_removeAttributeFromList(RaveObjectList_t* l, const char* name)
{
  int n = 0;
  int i;
  int index = -1;
  RAVE_ASSERT((l != NULL), "l == NULL");
  n = RaveObjectList_size(l);
  for (i = 0; index == -1 && i < n; i++) {
   RaveAttribute_t* attribute = (RaveAttribute_t*)RaveObjectList_get(l, i);
   if (strcmp(RaveAttribute_getName(attribute), name)==0) {
    index = i;
   }
   RAVE_OBJECT_RELEASE(attribute);
  }
  if (index >= 0) {
    RaveAttribute_t* attr = (RaveAttribute_t*)RaveObjectList_remove(l, index);
    RAVE_OBJECT_RELEASE(attr);
  }
}

int RaveUtilities_getRaveAttributeDoubleFromHash(RaveObjectHashTable_t* h, const char* name, double* v)
{
  RaveAttribute_t* attr = NULL;
  int result = 0;

  RAVE_ASSERT((h != NULL), "h == NULL");
  RAVE_ASSERT((v != NULL), "v == NULL");

  attr = (RaveAttribute_t*)RaveObjectHashTable_get(h, name);
  if (attr != NULL) {
    result = RaveAttribute_getDouble(attr, v);
  }

  RAVE_OBJECT_RELEASE(attr);
  return result;
}

int RaveUtilities_iswhitespace(char c)
{
  return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
}


char* RaveUtilities_trimText(const char* str, int len)
{
  int index = 0;
  int startpos = 0;
  char* result = NULL;
  int newsize = 0;

  if (str != NULL) {
    index = 0;
    while (index < len && RaveUtilities_iswhitespace(str[index])) {
      index++;
    }
    startpos = index;

    index = len-1;
    while (index > 0 && RaveUtilities_iswhitespace(str[index])) {
      index--;
    }
    index++;

    newsize = (index-startpos);
    if (newsize < 0) {
      newsize = 0;
    }

    result = RAVE_MALLOC((newsize + 1)*sizeof(char));
    if (result != NULL) {
      if (startpos < len && startpos + newsize < (len + 1)) {
        strncpy(result, &str[startpos], newsize);
      }
      result[newsize]='\0';
    }
  }

  return result;
}

RaveList_t* RaveUtilities_getTrimmedTokens(const char* str, int c)
{
  RaveList_t* result = RAVE_OBJECT_NEW(&RaveList_TYPE);
  int status = 0;

  if (str != NULL && result != NULL) {
    char* startptr = (char*)str;
    char* endptr = NULL;

    if (*startptr == '\0') {
      status = 1; /* this is ok, but there are no tokens */
      goto done;
    }

    while (*startptr != '\0') {
      endptr = strchr(startptr, c);
      if (endptr != NULL) {
        int len = (endptr - startptr);
        char* tmp = RaveUtilities_trimText(startptr, len);
        if (tmp == NULL || !RaveList_add(result, tmp)) {
          RAVE_FREE(tmp);
          RAVE_ERROR0("Failed to tokenize string");
          goto done;
        }

        startptr += len;
        /* We might have a delimiter at end of string which
         * should result it yet another token.
         */
        if (*startptr == c && *(startptr+1) == '\0') {
          tmp = RAVE_MALLOC(sizeof(char));
          if (tmp != NULL) {
            tmp[0] = '\0';
          }
          if (tmp == NULL || !RaveList_add(result, tmp)) {
            RAVE_FREE(tmp);
            RAVE_ERROR0("Failed to allocate empty string");
            goto done;
          }
        }
        startptr++;
      } else {
        int len = strlen(startptr);
        char* tmp = RaveUtilities_trimText(startptr, len);
        if (tmp == NULL || !RaveList_add(result, tmp)) {
          RAVE_FREE(tmp);
          RAVE_ERROR0("Failed to tokenize string");
          goto done;
        }
        startptr += len;
      }
    }
  }
  status = 1;
done:
  if (status == 0) {
    RaveList_freeAndDestroy(&result);
  }
  return result;
}

int RaveUtilities_isXmlSupported(void)
{
#ifdef RAVE_XML_SUPPORTED
  return 1;
#else
  return 0;
#endif
}

int RaveUtilities_isCFConventionSupported(void)
{
#ifdef RAVE_CF_SUPPORTED
  return 1;
#else
  return 0;
#endif
}

int RaveUtilities_isLegacyProjEnabled(void)
{
#ifdef USE_PROJ4_API
  return 1;
#else
  return 0;
#endif
}

char* RaveUtilities_handleSourceVersion(const char* source, RaveIO_ODIM_Version version)
{
  char* result = NULL;
  RaveList_t* sourceTokens = NULL;
  if (source != NULL) {
    result = RAVE_STRDUP(source);
    if (result == NULL) {
      goto done;
    }
    if (version < RaveIO_ODIM_Version_2_3) {
      char* p = strstr(result, "WIGOS:");
      if (p != NULL) {
        sourceTokens = RaveUtilities_getTrimmedTokens(result, (int)',');
        if (sourceTokens != NULL) {
          int nlen = RaveList_size(sourceTokens);
          int i = 0;
          for (i = nlen - 1; i >= 0; i--) {
            char* pToken = (char*)RaveList_get(sourceTokens, i);
            if (pToken != NULL && strstr(pToken, "WIGOS")) {
              pToken = (char*)RaveList_remove(sourceTokens, i);
              RAVE_FREE(pToken);
            }
          }
          nlen = RaveList_size(sourceTokens);
          strcpy(result, "");
          for (i = 0; i < nlen; i++) {
            char* pToken = (char*)RaveList_get(sourceTokens, i);
            if (i > 0) {
              strcat(result, ",");
            }
            strcat(result, pToken);
          }
        }
      }
    }
  }
done:
  if (sourceTokens != NULL) {
    RaveList_freeAndDestroy(&sourceTokens);
  }
  return result;
}

int RaveUtilities_isSourceValid(const char* source, RaveIO_ODIM_Version version)
{
  int result = 0;
  if (source != NULL) {
    if (version >= RaveIO_ODIM_Version_2_4) {
      if (strstr(source, "NOD:") != NULL || strstr(source, "ORG:") != NULL) {
        result = 1;
      } else {
        RAVE_WARNING2("Source is not valid according to rules for version=%d, source=%s", version, source);
      }
    } else {
      result = 1;
    }
  }
  return result;
}
