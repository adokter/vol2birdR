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

/**
 * Debug functions used in HLHDF. It is also provides a mechanism to route the printouts to custom
 * made report functions.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-06-12
 */
#ifndef HLHDF_DEBUG_H
#define HLHDF_DEBUG_H

#include <H5Epublic.h>

/**
 * Debug levels. The levels are defined so that if HLHDF_INFO debug level is turned on,
 * all higher levels will also be printed except HLHDF_SILENT which means turn of logging.
 * @ingroup hlhdf_c_apis
 */
typedef enum HL_Debug {
  HLHDF_SPEWDEBUG=0, /**< The most verbose printouts is turned on here. Like entering functions and similar */
  HLHDF_DEBUG, /**< Basic debug functions */
  HLHDF_DEPRECATED, /**< Print outs deprecated warnings */
  HLHDF_INFO, /**< Informational messages */
  HLHDF_WARNING, /**< Warnings */
  HLHDF_ERROR, /**< Errors can be when memory not could be allocated or a file not could be created */
  HLHDF_CRITICAL, /**< If this occurs, then something has gone very wrong or the code contains a bug, please report it to the HLHDF dev group for investigation. */
  HLHDF_SILENT /**< Turns of debugging */
} HL_Debug;

/**
 * Debug structure.
 */
typedef struct {
   HL_Debug dbgLevel; /**< Debug level */
   void (*dbgfun)(char* filename, int lineno, HL_Debug lvl, const char* fmt,...); /**< Debug function */
   int hdf5showerror; /**< If hdf5 errors should be printed or not */
   void (*hdf5fun)(unsigned n, const H5E_error_t* rowmsg); /**< the HDF5 error reporting function */
} hlhdf_debug_struct;

/**
 * The main structure used for routing errors and debug printouts.
 */
extern hlhdf_debug_struct hlhdfDbg;

/**
 * The printer function.
 * @param[in] fmt - the varargs formatter string
 * @param[in] ... - the varargs list
 */
void HL_printf(const char* fmt, ...);

/**
 * Initializes the debugger structure, must have been called before executing the code.
 * @ingroup hlhdf_c_apis
 */
void HL_InitializeDebugger(void);

/**
 * Sets the debug level.
 * @ingroup hlhdf_c_apis
 * @param[in] lvl the debug level. See @ref HL_Debug.
 */
void HL_setDebugLevel(HL_Debug lvl);

/**
 * Sets the debug function where the debug printouts should be routed.
 * @ingroup hlhdf_c_apis
 * @param[in] dbgfun The debug function.
 */
void HL_setDebugFunction(void (*dbgfun)(char* filename, int lineno, HL_Debug lvl, const char* fmt, ...));

/**
 * Sets the HDF5 error reporting function.
 * @ingroup hlhdf_c_apis
 * @param[in] hdf5fun the HDF5 error reporting function
 */
void HL_setHdf5ErrorFunction(void (*hdf5fun)(unsigned n, const H5E_error_t* val));

/**
 * Disables the HDF5 error reporting.
 * @ingroup hlhdf_c_apis
 */
void HL_disableHdf5ErrorReporting(void);

/**
 * Enables the HDF5 error reporting
 * @ingroup hlhdf_c_apis
 */
void HL_enableHdf5ErrorReporting(void);

/**
 * This is the debug function that is used to forward messages
 * to the appropriate debugger function.
 * @param[in] estack the error stack
 * @param[in] inv some data
 */
herr_t HL_hdf5_debug_function(hid_t estack, void *inv);

/**
 * @defgroup DebugMacros Macros for debugging and error reporting that is used in HLHDF.
 */
/*@{*/
#ifdef DEBUG_HLHDF
/**
 * Spewdebug macro taking one text string.
 */
#define HL_SPEWDEBUG0(msg) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_SPEWDEBUG,msg)

/**
 * Spewdebug macro taking one text string and one formatter argument.
 */
#define HL_SPEWDEBUG1(msg,arg1) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_SPEWDEBUG,msg,arg1)

#define HL_SPEWDEBUG2(msg,arg1,arg2) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_SPEWDEBUG,msg,arg1,arg2)

#define HL_SPEWDEBUG3(msg,arg1,arg2,arg3) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_SPEWDEBUG,msg,arg1,arg2,arg3)

#define HL_SPEWDEBUG4(msg,arg1,arg2,arg3,arg4) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_SPEWDEBUG,msg,arg1,arg2,arg3,arg4)

#define HL_DEBUG0(msg) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_DEBUG,msg)

#define HL_DEBUG1(msg,arg1) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_DEBUG,msg,arg1)

#define HL_DEBUG2(msg,arg1,arg2) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_DEBUG,msg,arg1,arg2)

#define HL_DEBUG3(msg,arg1,arg2,arg3) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_DEBUG,msg,arg1,arg2,arg3)

#define HL_DEBUG4(msg,arg1,arg2,arg3,arg4) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_DEBUG,msg,arg1,arg2,arg3,arg4)

#define HL_DEPRECATED0(msg) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_DEPRECATED,msg)

#define HL_DEPRECATED1(msg,arg1) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_DEPRECATED,msg,arg1)

#define HL_DEPRECATED2(msg,arg1,arg2) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_DEPRECATED,msg,arg1,arg2)

#define HL_DEPRECATED3(msg,arg1,arg2,arg3) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_DEPRECATED,msg,arg1,arg2,arg3)

#define HL_DEPRECATED4(msg,arg1,arg2,arg3,arg4) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_DEPRECATED,msg,arg1,arg2,arg3,arg4)

#define HL_INFO0(msg) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_INFO,msg)

#define HL_INFO1(msg,arg1) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_INFO,msg,arg1)

#define HL_INFO2(msg,arg1,arg2) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_INFO,msg,arg1,arg2)

#define HL_INFO3(msg,arg1,arg2,arg3) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_INFO,msg,arg1,arg2,arg3)

#define HL_INFO4(msg,arg1,arg2,arg3,arg4) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_INFO,msg,arg1,arg2,arg3,arg4)

#define HL_WARNING0(msg) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_WARNING,msg)

#define HL_WARNING1(msg,arg1) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_WARNING,msg,arg1)

#define HL_WARNING2(msg,arg1,arg2) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_WARNING,msg,arg1,arg2)

#define HL_WARNING3(msg,arg1,arg2,arg3) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_WARNING,msg,arg1,arg2,arg3)

#define HL_WARNING4(msg,arg1,arg2,arg3,arg4) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_WARNING,msg,arg1,arg2,arg3,arg4)

#define HL_ERROR0(msg) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_ERROR,msg)

#define HL_ERROR1(msg,arg1) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_ERROR,msg,arg1)

#define HL_ERROR2(msg,arg1,arg2) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_ERROR,msg,arg1,arg2)

#define HL_ERROR3(msg,arg1,arg2,arg3) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_ERROR,msg,arg1,arg2,arg3)

#define HL_ERROR4(msg,arg1,arg2,arg3,arg4) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_ERROR,msg,arg1,arg2,arg3,arg4)

#define HL_CRITICAL0(msg) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_CRITICAL,msg)

#define HL_CRITICAL1(msg,arg1) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_CRITICAL,msg,arg1)

#define HL_CRITICAL2(msg,arg1,arg2) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_CRITICAL,msg,arg1,arg2)

#define HL_CRITICAL3(msg,arg1,arg2,arg3) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_CRITICAL,msg,arg1,arg2,arg3)

#define HL_CRITICAL4(msg,arg1,arg2,arg3,arg4) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_CRITICAL,msg,arg1,arg2,arg3,arg4)

#define HL_ASSERT(expr, msg) \
if(!expr) { \
hlhdfDbg.dbgfun(__FILE__, __LINE__, HLHDF_CRITICAL, msg); \
abort(); \
}


#else
/** Spewdebug macro taking one text string.*/
#define HL_SPEWDEBUG0(msg)

/** Spewdebug macro taking one text string and one argument.*/
#define HL_SPEWDEBUG1(msg,arg1)

/** Spewdebug macro taking one text string and two arguments.*/
#define HL_SPEWDEBUG2(msg,arg1,arg2)

/** Spewdebug macro taking one text string and three arguments.*/
#define HL_SPEWDEBUG3(msg,arg1,arg2,arg3)

/** Spewdebug macro taking one text string and four arguments.*/
#define HL_SPEWDEBUG4(msg,arg1,arg2,arg3,arg4)

/** Debug macro taking one text string.*/
#define HL_DEBUG0(msg)

/** Debug macro taking one text string and one argument.*/
#define HL_DEBUG1(msg,arg1)

/** Debug macro taking one text string and two arguments.*/
#define HL_DEBUG2(msg,arg1,arg2)

/** Debug macro taking one text string and three arguments.*/
#define HL_DEBUG3(msg,arg1,arg2,arg3)

/** Debug macro taking one text string and four arguments.*/
#define HL_DEBUG4(msg,arg1,arg2,arg3,arg4)

/** Deprecated macro taking one text string.*/
#define HL_DEPRECATED0(msg)

/** Deprecated macro taking one text string and one argument.*/
#define HL_DEPRECATED1(msg,arg1)

/** Deprecated macro taking one text string and two arguments.*/
#define HL_DEPRECATED2(msg,arg1,arg2)

/** Deprecated macro taking one text string and three arguments.*/
#define HL_DEPRECATED3(msg,arg1,arg2,arg3)

/** Deprecated macro taking one text string and four arguments.*/
#define HL_DEPRECATED4(msg,arg1,arg2,arg3,arg4)

/** Info macro taking one text string.*/
#define HL_INFO0(msg) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_INFO,msg)

/** Info macro taking one text string and one argument.*/
#define HL_INFO1(msg,arg1) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_INFO,msg,arg1)

/** Info macro taking one text string and two arguments.*/
#define HL_INFO2(msg,arg1,arg2) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_INFO,msg,arg1,arg2)

/** Info macro taking one text string and three arguments.*/
#define HL_INFO3(msg,arg1,arg2,arg3) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_INFO,msg,arg1,arg2,arg3)

/** Info macro taking one text string and four arguments.*/
#define HL_INFO4(msg,arg1,arg2,arg3,arg4) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_INFO,msg,arg1,arg2,arg3,arg4)

/** Warning macro taking one text string.*/
#define HL_WARNING0(msg) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_WARNING,msg)

/** Warning macro taking one text string and one argument.*/
#define HL_WARNING1(msg,arg1) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_WARNING,msg,arg1)

/** Warning macro taking one text string and two arguments.*/
#define HL_WARNING2(msg,arg1,arg2) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_WARNING,msg,arg1,arg2)

/** Warning macro taking one text string and three arguments.*/
#define HL_WARNING3(msg,arg1,arg2,arg3) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_WARNING,msg,arg1,arg2,arg3)

/** Warning macro taking one text string and four arguments.*/
#define HL_WARNING4(msg,arg1,arg2,arg3,arg4) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_WARNING,msg,arg1,arg2,arg3,arg4)

/** Error macro taking one text string.*/
#define HL_ERROR0(msg) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_ERROR,msg)

/** Error macro taking one text string and one argument.*/
#define HL_ERROR1(msg,arg1) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_ERROR,msg,arg1)

/** Error macro taking one text string and two arguments.*/
#define HL_ERROR2(msg,arg1,arg2) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_ERROR,msg,arg1,arg2)

/** Error macro taking one text string and three arguments.*/
#define HL_ERROR3(msg,arg1,arg2,arg3) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_ERROR,msg,arg1,arg2,arg3)

/** Error macro taking one text string and four arguments.*/
#define HL_ERROR4(msg,arg1,arg2,arg3,arg4) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_ERROR,msg,arg1,arg2,arg3,arg4)

/** Critical macro taking one text string.*/
#define HL_CRITICAL0(msg) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_CRITICAL,msg)

/** Critical macro taking one text string and one argument.*/
#define HL_CRITICAL1(msg,arg1) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_CRITICAL,msg,arg1)

/** Critical macro taking one text string and two arguments.*/
#define HL_CRITICAL2(msg,arg1,arg2) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_CRITICAL,msg,arg1,arg2)

/** Critical macro taking one text string and three arguments.*/
#define HL_CRITICAL3(msg,arg1,arg2,arg3) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_CRITICAL,msg,arg1,arg2,arg3)

/** Critical macro taking one text string and four arguments.*/
#define HL_CRITICAL4(msg,arg1,arg2,arg3,arg4) \
hlhdfDbg.dbgfun(__FILE__,__LINE__,HLHDF_CRITICAL,msg,arg1,arg2,arg3,arg4)

#ifdef NO_HLHDF_ABORT

#define HL_ASSERT(expr, msg)
#define HL_ABORT()

#else
/**
 * Precondition macro, if the expression does not evaluate to true, then an
 * CRITICAL error message will be produced and then the program will abort().
 */
#define HL_ASSERT(expr, msg) \
if(!expr) { \
hlhdfDbg.dbgfun(__FILE__, __LINE__, HLHDF_CRITICAL, msg); \
abort(); \
}

#define HL_ABORT() abort()

#endif

#endif
/*@}*/

#endif
