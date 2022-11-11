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

#include "hlhdf_debug.h"
#include "hdf5.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

hlhdf_debug_struct hlhdfDbg;
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

static void HL_DefaultDebugFunction(char* filename, int lineno, HL_Debug lvl,
  const char* fmt, ...)
{
  char msgbuff[512];
  char dbgtype[20];
  char strtime[24];
  char infobuff[120];

  va_list alist;
  va_start(alist,fmt);

  if (hlhdfDbg.dbgLevel == HLHDF_SILENT) {
    return;
  }

  setLogTime(strtime, 24);

  strcpy(dbgtype, "");

  if (lvl >= hlhdfDbg.dbgLevel) {
    switch (lvl) {
    case HLHDF_SPEWDEBUG:
      sprintf(dbgtype, "SDEBUG");
      break;
    case HLHDF_DEBUG:
      sprintf(dbgtype, "DEBUG");
      break;
    case HLHDF_DEPRECATED:
      sprintf(dbgtype, "DEPRECATED");
      break;
    case HLHDF_INFO:
      sprintf(dbgtype, "INFO");
      break;
    case HLHDF_WARNING:
      sprintf(dbgtype, "WARNING");
      break;
    case HLHDF_ERROR:
      sprintf(dbgtype, "ERROR");
      break;
    case HLHDF_CRITICAL:
      sprintf(dbgtype, "CRITICAL");
      break;
    default:
      sprintf(dbgtype, "UNKNOWN");
      break;
    }
  } else {
    return;
  }
  sprintf(infobuff, "%20s : %11s", strtime, dbgtype);
  vsprintf(msgbuff, fmt, alist);
#ifndef NO_HLHDF_PRINTF
  fprintf(stderr, "%s : %s (%s:%d)\n", infobuff, msgbuff, filename, lineno);
#endif
}

static void HL_DefaultHdf5ErrorFunction(unsigned n, const H5E_error_t* rowmsg)
{
  if (hlhdfDbg.hdf5showerror) {
    char* minorError = NULL;
    HL_printf("  HDF5-ERROR: #%03d: %s line %d in %s: %s\n", n,
            rowmsg->file_name, rowmsg->line, rowmsg->func_name, rowmsg->desc);
    HL_printf("    major(%ld): %s\n", rowmsg->maj_num,
            H5Eget_major(rowmsg->maj_num));
    minorError = H5Eget_minor(rowmsg->min_num);
    if (minorError != NULL) {
      HL_printf("    minor(%ld): %s\n", rowmsg->min_num,
              minorError);
      free(minorError);
    }
  }
}
/*@} End of Private functions */

/*@{ Interface functions */
void HL_printf(const char* fmt, ...)
{
  va_list alist;
  va_start(alist,fmt);
  char msgbuff[4096];
  int n = vsnprintf(msgbuff, 4096, fmt, alist);
  va_end(alist);
  if (n < 0 || n >= 1024) {
    return;
  }
#ifndef NO_HLHDF_PRINTF
  fprintf(stderr, "%s", msgbuff);
#endif
}

void HL_InitializeDebugger(void)
{
  if (initialized == 0) {
    initialized = 1;
    hlhdfDbg.dbgLevel = HLHDF_SILENT;
    hlhdfDbg.dbgfun = HL_DefaultDebugFunction;
    hlhdfDbg.hdf5showerror = 1;
    hlhdfDbg.hdf5fun = HL_DefaultHdf5ErrorFunction;
  }
}

void HL_setDebugLevel(HL_Debug lvl)
{
  hlhdfDbg.dbgLevel = lvl;
}

void HL_setDebugFunction(void(*dbgfun)(char* filename, int lineno,
  HL_Debug lvl, const char* fmt, ...))
{
  hlhdfDbg.dbgfun = dbgfun;
}

void HL_disableHdf5ErrorReporting(void)
{
  hlhdfDbg.hdf5showerror = 0;
}

void HL_enableHdf5ErrorReporting(void)
{
  hlhdfDbg.hdf5showerror = 1;
}

void HL_setHdf5ErrorFunction(void(*hdf5fun)(unsigned n, const H5E_error_t*))
{
  hlhdfDbg.hdf5fun = hdf5fun;
}


static herr_t HDF5ErrorWalker(unsigned n, const H5E_error_t *err_desc, void *client_data)
{
  hlhdfDbg.hdf5fun(n, err_desc);
  return 0;
}

herr_t HL_hdf5_debug_function(hid_t estack, void *inv)
{
  H5Ewalk(H5E_DEFAULT, H5E_WALK_DOWNWARD, HDF5ErrorWalker, NULL);
  return 0;
}

/*@} End of Interface functions */
