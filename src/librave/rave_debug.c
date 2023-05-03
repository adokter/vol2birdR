/* --------------------------------------------------------------------
Copyright (C) 2009 Swedish Meteorological and Hydrological Institute, SMHI,

This file is part of HLHDF.

HLHDF is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HLHDF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with HLHDF.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------*/

#include "rave_debug.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static rave_dbgfun raveDebugFunction = NULL;
static Rave_Debug raveDebugLevel = RAVE_SILENT;
static int initialized = 0;

/*@{ Private functions */
static void setLogTime(char* strtime, int len)
{
  time_t cur_time;
  struct tm* tu_time;

  time(&cur_time);
  tu_time = gmtime(&cur_time);
  strftime(strtime, len, "%Y/%m/%d %H:%M:%S", tu_time);
}

void Rave_printf(const char* fmt, ...)
{
  va_list alist;
  va_start(alist,fmt);
  char msgbuff[4096];
  int n = vsnprintf(msgbuff, 4096, fmt, alist);
  va_end(alist);
  if (n < 0 || n >= 1024) {
    return;
  }
#ifndef NO_RAVE_PRINTF
  fprintf(stderr, "%s", msgbuff);
#endif
}

static void Rave_defaultDebugFunction(const char* filename, int lineno, Rave_Debug lvl,
  const char* fmt, ...)
{
  char msgbuff[512];
  char dbgtype[20];
  char strtime[24];
  char infobuff[120];

  va_list alist;
  va_start(alist,fmt);

  Rave_initializeDebugger(); /* So that we always have an initialized debugger */

  if (raveDebugLevel == RAVE_SILENT && lvl != RAVE_CRITICAL)
    return;

  setLogTime(strtime, 24);

  strcpy(dbgtype, "");

  if (lvl >= raveDebugLevel || lvl == RAVE_CRITICAL) {
    switch (lvl) {
    case RAVE_SPEWDEBUG:
      snprintf(dbgtype, 20, "SDEBUG");
      break;
    case RAVE_DEBUG:
      snprintf(dbgtype, 20, "DEBUG");
      break;
    case RAVE_DEPRECATED:
      snprintf(dbgtype, 20, "DEPRECATED");
      break;
    case RAVE_INFO:
      snprintf(dbgtype, 20, "INFO");
      break;
    case RAVE_WARNING:
      snprintf(dbgtype, 20, "WARNING");
      break;
    case RAVE_ERROR:
      snprintf(dbgtype, 20, "ERROR");
      break;
    case RAVE_CRITICAL:
      snprintf(dbgtype, 20, "CRITICAL");
      break;
    default:
      snprintf(dbgtype, 20, "UNKNOWN");
      break;
    }
  } else {
    return;
  }
  snprintf(infobuff, 120, "%20s : %11s", strtime, dbgtype);
  vsnprintf(msgbuff, 512, fmt, alist);

  Rave_printf("%s : %s (%s:%d)\n", infobuff, msgbuff, filename, lineno);
}
/*@} End of Private functions */

/*@{ Interface functions */
void Rave_initializeDebugger(void)
{
  if (initialized == 0) {
    initialized = 1;
    raveDebugLevel = RAVE_SILENT;
    raveDebugFunction = Rave_defaultDebugFunction;
  }
}

void Rave_setDebugLevel(Rave_Debug lvl)
{
  Rave_initializeDebugger();
  if (lvl >= RAVE_SPEWDEBUG && lvl <= RAVE_SILENT) {
    raveDebugLevel = lvl;
  }
}

Rave_Debug Rave_getDebugLevel(void)
{
  Rave_initializeDebugger();
  return raveDebugLevel;
}

void Rave_setDebugFunction(rave_dbgfun dbgfun)
{
  Rave_initializeDebugger();
  raveDebugFunction = dbgfun;
}

rave_dbgfun Rave_getDebugFunction(void)
{
  Rave_initializeDebugger();
  return raveDebugFunction;
}

/*@} End of Interface functions */
