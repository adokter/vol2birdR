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
 * Object for managing date and time.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-12-16
 */
#include "rave_datetime.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include <string.h>
#define __USE_XOPEN
#define _XOPEN_SOURCE
#include <time.h>
#include <stdio.h>

/**
 * Represents a date time instance
 */
struct _RaveDateTime_t {
  RAVE_OBJECT_HEAD /** Always on top */
  char date[9];    /**< the date string, format is YYYYMMDD */
  char time[7];    /**< the time string, format is HHmmss */
};

/*@{ Private functions */
/**
 * Constructor.
 */
static int RaveDateTime_constructor(RaveCoreObject* obj)
{
  RaveDateTime_t* dt = (RaveDateTime_t*)obj;
  strcpy(dt->date,"");
  strcpy(dt->time,"");
  return 1;
}

/**
 * Copy constructor.
 */
static int RaveDateTime_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  RaveDateTime_t* dt = (RaveDateTime_t*)obj;
  RaveDateTime_t* srcdt = (RaveDateTime_t*)srcobj;
  strcpy(dt->date, srcdt->date);
  strcpy(dt->time, srcdt->time);
  return 1;
}

/**
 * Destructor.
 */
static void RaveDateTime_destructor(RaveCoreObject* obj)
{
  //RaveDateTime_t* scan = (RaveDateTime_t*)obj;
}

/**
 * Verifies that the string only contains digits.
 * @param[in] value - the null terminated string
 * @returns 1 if the string only contains digits, otherwise 0
 */
static int RaveDateTimeInternal_isDigits(const char* value)
{
  int result = 0;
  if (value != NULL) {
    int len = strlen(value);
    int i = 0;
    result = 1;
    for (i = 0; result == 1 && i < len; i++) {
      if (value[i] < 0x30 || value[i] > 0x39) {
        result = 0;
      }
    }
  }
  return result;
}
/*@} End of Private functions */

/*@{ Interface functions */
int RaveDateTime_setTime(RaveDateTime_t* dt, const char* value)
{
  int result = 0;
  RAVE_ASSERT((dt != NULL), "dt was NULL");
  if (value == NULL) {
    strcpy(dt->time, "");
    result = 1;
  } else {
    if (strlen(value) == 6 && RaveDateTimeInternal_isDigits(value)) {
      strcpy(dt->time, value);
      result = 1;
    }
  }
  return result;
}

const char* RaveDateTime_getTime(RaveDateTime_t* dt)
{
  RAVE_ASSERT((dt != NULL), "dt was NULL");
  if (strcmp(dt->time, "") == 0) {
    return NULL;
  }
  return (const char*)dt->time;
}

int RaveDateTime_setDate(RaveDateTime_t* dt, const char* value)
{
  int result = 0;
  RAVE_ASSERT((dt != NULL), "dt was NULL");
  if (value == NULL) {
    strcpy(dt->date, "");
    result = 1;
  } else {
    if (strlen(value) == 8 && RaveDateTimeInternal_isDigits(value)) {
      strcpy(dt->date, value);
      result = 1;
    }
  }
  return result;
}

const char* RaveDateTime_getDate(RaveDateTime_t* dt)
{
  RAVE_ASSERT((dt != NULL), "dt was NULL");
  if (strcmp(dt->date, "") == 0) {
    return NULL;
  }
  return (const char*)dt->date;
}

int RaveDateTime_strptime(char* yyyymmddHHMMSS, struct tm* ts)
{
  if (sscanf(yyyymmddHHMMSS, "%4d%2d%2d%2d%2d%2d", &ts->tm_year, &ts->tm_mon, &ts->tm_mday, &ts->tm_hour, &ts->tm_min, &ts->tm_sec) != 6) {
    return 0;
  }
  ts->tm_year += 1900;
  ts->tm_mon -= 1;
  return 1;
}

int RaveDateTime_compare(RaveDateTime_t* self, RaveDateTime_t* other)
{
  RAVE_ASSERT((self != NULL), "self was NULL");
  RAVE_ASSERT((other != NULL), "other was NULL");
  int result = -1;
  if (strlen(RaveDateTime_getDate(self)) == 8 &&
      strlen(RaveDateTime_getTime(self)) == 6 &&
      strlen(RaveDateTime_getDate(other)) == 8 &&
      strlen(RaveDateTime_getTime(other)) == 6) {
    char selfdatestr[32], otherdatestr[32];
    struct tm selft, othert;
    strcpy(selfdatestr, RaveDateTime_getDate(self));
    strcat(selfdatestr, RaveDateTime_getTime(self));
    strcpy(otherdatestr, RaveDateTime_getDate(other));
    strcat(otherdatestr, RaveDateTime_getTime(other));
    memset(&selft, 0, sizeof(struct tm));
    memset(&othert, 0, sizeof(struct tm));

    if (!RaveDateTime_strptime(selfdatestr, &selft) || !RaveDateTime_strptime(otherdatestr, &othert)) {
      RAVE_WARNING2("Failed to convert either %s or %s into a time_t structure", selfdatestr, otherdatestr);
    } else {
      time_t ot = mktime(&othert);
      time_t st = mktime(&selft);
      double d = difftime(ot, st);
      if (d > 0.0)
        return -1;
      else if (d < 0.0)
        return 1;
      else
        return 0;
    }
  } else {
    RAVE_WARNING0("When comparing datetime either self or other is not initialized with both date and time");
  }
  return result;
}

/*@} End of Interface functions */
RaveCoreObjectType RaveDateTime_TYPE = {
    "RaveDateTime",
    sizeof(RaveDateTime_t),
    RaveDateTime_constructor,
    RaveDateTime_destructor,
    RaveDateTime_copyconstructor
};
