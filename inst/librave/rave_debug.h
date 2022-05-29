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
 * Defines the functions for debugging rave
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-10-15
 */
#ifndef RAVE_DEBUG_H
#define RAVE_DEBUG_H

/**
 * Debug levels. The levels are defined so that if RAVE_INFO debug level is turned on,
 * all higher levels will also be printed except RAVE_SILENT which means turn of logging.
 * @ingroup rave_c_apis
 */
typedef enum Rave_Debug {
  RAVE_SPEWDEBUG=0, /**< The most verbose printouts is turned on here. Like entering functions and similar */
  RAVE_DEBUG, /**< Basic debug functions */
  RAVE_DEPRECATED, /**< Print outs deprecated warnings */
  RAVE_INFO, /**< Informational messages */
  RAVE_WARNING, /**< Warnings */
  RAVE_ERROR, /**< Errors can be when memory not could be allocated or a file not could be created */
  RAVE_CRITICAL, /**< If this occurs, then something has gone very wrong or the code contains a bug, please report it to the HLHDF dev group for investigation. */
  RAVE_SILENT /**< Turns of debugging */
} Rave_Debug;

/**
 * The debugger function.
 * @param[in] filename - the name of the file
 * @param[in] lineno - the line number
 * @param[in] lvl - the debug level for this message
 * @param[in] fmt - the varargs formatter string
 * @param[in] ... - the varargs list
 */
typedef void(*rave_dbgfun)(const char* filename, int lineno, Rave_Debug lvl, const char* fmt, ...);

/**
 * The printer function.
 * @param[in] fmt - the varargs formatter string
 * @param[in] ... - the varargs list
 */
void Rave_printf(const char* fmt, ...);

/**
 * Initializes the debugger structure, must have been called before executing the code.
 * @ingroup rave_c_apis
 */
void Rave_initializeDebugger(void);

/**
 * Sets the debug level.
 * @ingroup hlhdf_c_apis
 * @param[in] lvl the debug level. See @ref Rave_Debug.
 */
void Rave_setDebugLevel(Rave_Debug lvl);

/**
 * @returns the current rave debug level
 */
Rave_Debug Rave_getDebugLevel(void);

/**
 * Sets the debug function where the debug printouts should be routed.
 * @ingroup hlhdf_c_apis
 * @param[in] rave_dbgfun The debug function.
 */
void Rave_setDebugFunction(rave_dbgfun dbgfun);

/**
 * @returns the currently set debugger function
 */
rave_dbgfun Rave_getDebugFunction(void);

/**
 * @defgroup DebugMacros Macros for debugging and error reporting that is used in RAVE.
 */
/*@{*/
#ifdef DEBUG_RAVE
/**
 * Spewdebug macro taking one text string.
 */
#define RAVE_SPEWDEBUG0(msg) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_SPEWDEBUG,msg)

/**
 * Spewdebug macro taking one text string and one formatter argument.
 */
#define RAVE_SPEWDEBUG1(msg,arg1) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_SPEWDEBUG,msg,arg1)

#define RAVE_SPEWDEBUG2(msg,arg1,arg2) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_SPEWDEBUG,msg,arg1,arg2)

#define RAVE_SPEWDEBUG3(msg,arg1,arg2,arg3) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_SPEWDEBUG,msg,arg1,arg2,arg3)

#define RAVE_SPEWDEBUG4(msg,arg1,arg2,arg3,arg4) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_SPEWDEBUG,msg,arg1,arg2,arg3,arg4)

#define RAVE_DEBUG0(msg) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_DEBUG,msg)

#define RAVE_DEBUG1(msg,arg1) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_DEBUG,msg,arg1)

#define RAVE_DEBUG2(msg,arg1,arg2) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_DEBUG,msg,arg1,arg2)

#define RAVE_DEBUG3(msg,arg1,arg2,arg3) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_DEBUG,msg,arg1,arg2,arg3)

#define RAVE_DEBUG4(msg,arg1,arg2,arg3,arg4) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_DEBUG,msg,arg1,arg2,arg3,arg4)

#define RAVE_DEBUG7(msg,arg1,arg2,arg3,arg4,arg5,arg6,arg7) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_DEBUG,msg,arg1,arg2,arg3,arg4,arg5,arg6,arg7)

#define RAVE_DEPRECATED0(msg) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_DEPRECATED,msg)

#define RAVE_DEPRECATED1(msg,arg1) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_DEPRECATED,msg,arg1)

#define RAVE_DEPRECATED2(msg,arg1,arg2) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_DEPRECATED,msg,arg1,arg2)

#define RAVE_DEPRECATED3(msg,arg1,arg2,arg3) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_DEPRECATED,msg,arg1,arg2,arg3)

#define RAVE_DEPRECATED4(msg,arg1,arg2,arg3,arg4) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_DEPRECATED,msg,arg1,arg2,arg3,arg4)

#define RAVE_INFO0(msg) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_INFO,msg)

#define RAVE_INFO1(msg,arg1) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_INFO,msg,arg1)

#define RAVE_INFO2(msg,arg1,arg2) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_INFO,msg,arg1,arg2)

#define RAVE_INFO3(msg,arg1,arg2,arg3) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_INFO,msg,arg1,arg2,arg3)

#define RAVE_INFO4(msg,arg1,arg2,arg3,arg4) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_INFO,msg,arg1,arg2,arg3,arg4)

#define RAVE_INFO7(msg,arg1,arg2,arg3,arg4,arg5,arg6,arg7) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_INFO,msg,arg1,arg2,arg3,arg4,arg5,arg6,arg7)

#define RAVE_WARNING0(msg) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_WARNING,msg)

#define RAVE_WARNING1(msg,arg1) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_WARNING,msg,arg1)

#define RAVE_WARNING2(msg,arg1,arg2) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_WARNING,msg,arg1,arg2)

#define RAVE_WARNING3(msg,arg1,arg2,arg3) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_WARNING,msg,arg1,arg2,arg3)

#define RAVE_WARNING4(msg,arg1,arg2,arg3,arg4) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_WARNING,msg,arg1,arg2,arg3,arg4)

#define RAVE_ERROR0(msg) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_ERROR,msg)

#define RAVE_ERROR1(msg,arg1) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_ERROR,msg,arg1)

#define RAVE_ERROR2(msg,arg1,arg2) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_ERROR,msg,arg1,arg2)

#define RAVE_ERROR3(msg,arg1,arg2,arg3) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_ERROR,msg,arg1,arg2,arg3)

#define RAVE_ERROR4(msg,arg1,arg2,arg3,arg4) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_ERROR,msg,arg1,arg2,arg3,arg4)

#define RAVE_CRITICAL0(msg) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_CRITICAL,msg)

#define RAVE_CRITICAL1(msg,arg1) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_CRITICAL,msg,arg1)

#define RAVE_CRITICAL2(msg,arg1,arg2) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_CRITICAL,msg,arg1,arg2)

#define RAVE_CRITICAL3(msg,arg1,arg2,arg3) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_CRITICAL,msg,arg1,arg2,arg3)

#define RAVE_CRITICAL4(msg,arg1,arg2,arg3,arg4) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_CRITICAL,msg,arg1,arg2,arg3,arg4)

#define RAVE_ASSERT(expr, msg) \
if(!expr) { \
Rave_getDebugFunction()(__FILE__, __LINE__, RAVE_CRITICAL, msg); \
abort(); \
}

#define RAVE_ABORT() abort()

#else
/** Spewdebug macro taking one text string.*/
#define RAVE_SPEWDEBUG0(msg)

/** Spewdebug macro taking one text string and one argument.*/
#define RAVE_SPEWDEBUG1(msg,arg1)

/** Spewdebug macro taking one text string and two arguments.*/
#define RAVE_SPEWDEBUG2(msg,arg1,arg2)

/** Spewdebug macro taking one text string and three arguments.*/
#define RAVE_SPEWDEBUG3(msg,arg1,arg2,arg3)

/** Spewdebug macro taking one text string and four arguments.*/
#define RAVE_SPEWDEBUG4(msg,arg1,arg2,arg3,arg4)

/** Debug macro taking one text string.*/
#define RAVE_DEBUG0(msg)

/** Debug macro taking one text string and one argument.*/
#define RAVE_DEBUG1(msg,arg1)

/** Debug macro taking one text string and two arguments.*/
#define RAVE_DEBUG2(msg,arg1,arg2)

/** Debug macro taking one text string and three arguments.*/
#define RAVE_DEBUG3(msg,arg1,arg2,arg3)

/** Debug macro taking one text string and four arguments.*/
#define RAVE_DEBUG4(msg,arg1,arg2,arg3,arg4)

/** Debug macro taking one text string and seven arguments.*/
#define RAVE_DEBUG7(msg,arg1,arg2,arg3,arg4,arg5,arg6,arg7)

/** Deprecated macro taking one text string.*/
#define RAVE_DEPRECATED0(msg)

/** Deprecated macro taking one text string and one argument.*/
#define RAVE_DEPRECATED1(msg,arg1)

/** Deprecated macro taking one text string and two arguments.*/
#define RAVE_DEPRECATED2(msg,arg1,arg2)

/** Deprecated macro taking one text string and three arguments.*/
#define RAVE_DEPRECATED3(msg,arg1,arg2,arg3)

/** Deprecated macro taking one text string and four arguments.*/
#define RAVE_DEPRECATED4(msg,arg1,arg2,arg3,arg4)

/** Info macro taking one text string.*/
#define RAVE_INFO0(msg) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_INFO,msg)

/** Info macro taking one text string and one argument.*/
#define RAVE_INFO1(msg,arg1) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_INFO,msg,arg1)

/** Info macro taking one text string and two arguments.*/
#define RAVE_INFO2(msg,arg1,arg2) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_INFO,msg,arg1,arg2)

/** Info macro taking one text string and three arguments.*/
#define RAVE_INFO3(msg,arg1,arg2,arg3) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_INFO,msg,arg1,arg2,arg3)

/** Info macro taking one text string and four arguments.*/
#define RAVE_INFO4(msg,arg1,arg2,arg3,arg4) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_INFO,msg,arg1,arg2,arg3,arg4)

/** Info macro taking one text string and seven arguments.*/
#define RAVE_INFO7(msg,arg1,arg2,arg3,arg4,arg5,arg6,arg7) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_INFO,msg,arg1,arg2,arg3,arg4,arg5,arg6,arg7)

/** Warning macro taking one text string.*/
#define RAVE_WARNING0(msg) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_WARNING,msg)

/** Warning macro taking one text string and one argument.*/
#define RAVE_WARNING1(msg,arg1) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_WARNING,msg,arg1)

/** Warning macro taking one text string and two arguments.*/
#define RAVE_WARNING2(msg,arg1,arg2) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_WARNING,msg,arg1,arg2)

/** Warning macro taking one text string and three arguments.*/
#define RAVE_WARNING3(msg,arg1,arg2,arg3) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_WARNING,msg,arg1,arg2,arg3)

/** Warning macro taking one text string and four arguments.*/
#define RAVE_WARNING4(msg,arg1,arg2,arg3,arg4) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_WARNING,msg,arg1,arg2,arg3,arg4)

/** Error macro taking one text string.*/
#define RAVE_ERROR0(msg) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_ERROR,msg)

/** Error macro taking one text string and one argument.*/
#define RAVE_ERROR1(msg,arg1) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_ERROR,msg,arg1)

/** Error macro taking one text string and two arguments.*/
#define RAVE_ERROR2(msg,arg1,arg2) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_ERROR,msg,arg1,arg2)

/** Error macro taking one text string and three arguments.*/
#define RAVE_ERROR3(msg,arg1,arg2,arg3) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_ERROR,msg,arg1,arg2,arg3)

/** Error macro taking one text string and four arguments.*/
#define RAVE_ERROR4(msg,arg1,arg2,arg3,arg4) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_ERROR,msg,arg1,arg2,arg3,arg4)

/** Critical macro taking one text string.*/
#define RAVE_CRITICAL0(msg) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_CRITICAL,msg)

/** Critical macro taking one text string and one argument.*/
#define RAVE_CRITICAL1(msg,arg1) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_CRITICAL,msg,arg1)

/** Critical macro taking one text string and two arguments.*/
#define RAVE_CRITICAL2(msg,arg1,arg2) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_CRITICAL,msg,arg1,arg2)

/** Critical macro taking one text string and three arguments.*/
#define RAVE_CRITICAL3(msg,arg1,arg2,arg3) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_CRITICAL,msg,arg1,arg2,arg3)

/** Critical macro taking one text string and four arguments.*/
#define RAVE_CRITICAL4(msg,arg1,arg2,arg3,arg4) \
Rave_getDebugFunction()(__FILE__,__LINE__,RAVE_CRITICAL,msg,arg1,arg2,arg3,arg4)

#ifdef NO_RAVE_ABORT

#define RAVE_ASSERT(expr, msg)
#define RAVE_ABORT()

#else

#define RAVE_ASSERT(expr, msg) \
if(!expr) { \
Rave_getDebugFunction()(__FILE__, __LINE__, RAVE_CRITICAL, msg); \
abort(); \
}
#define RAVE_ABORT() abort()

#endif

#endif
/*@}*/

#endif
