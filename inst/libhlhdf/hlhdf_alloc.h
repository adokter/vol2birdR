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
 * Allocation routines for keeping track on memory
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-06-26
 */
#ifndef HLHDF_ALLOC_H
#define HLHDF_ALLOC_H
#include <stdlib.h>

/**
 * Allocates memory and keeps track on if it is released, overwritten and
 * similar.
 * @param[in] filename the name of the file the allocation occurs in
 * @param[in] lineno the linenumber
 * @param[in] sz the number of bytes to be allocated
 */
void* hlhdf_alloc_malloc(const char* filename, int lineno, size_t sz);

/**
 * Same as calloc but debugged.
 * @param[in] filename the name of the file the allocation occurs in
 * @param[in] lineno the linenumber
 * @param[in] npts number of points
 * @param[in] sz the number of bytes to be allocated
 */
void* hlhdf_alloc_calloc(const char* filename, int lineno, size_t npts, size_t sz);

/**
 * Same as realloc but debugged.
 * @param[in] filename the name of the file the allocation occurs in
 * @param[in] lineno the linenumber
 * @param[in] ptr the original pointer
 * @param[in] sz the number of bytes to be allocated
 */
void* hlhdf_alloc_realloc(const char* filename, int lineno, void* ptr, size_t sz);

/**
 * Same as strdup but debugged.
 * @param[in] filename the name of the file the allocation occurs in
 * @param[in] lineno the linenumber
 * @param[in] str the number of bytes to be allocated
 */
char* hlhdf_alloc_strdup(const char* filename, int lineno, const char* str);

/**
 * Releases the memory
 * @param[in] filename the name of the file the allocation occurs in
 * @param[in] lineno the linenumber
 * @param[in] ptr the pointer that should be freed
 */
void hlhdf_alloc_free(const char* filename, int lineno, void* ptr);

/**
 * Dumps all blocks that not has been released.
 */
void hlhdf_alloc_dump_heap(void);

/**
 * Prints the statistics for the heap
 */
void hlhdf_alloc_print_statistics(void);

#ifdef HLHDF_MEMORY_DEBUG
/**
 * @brief debugged malloc
 */
#define HLHDF_MALLOC(sz) hlhdf_alloc_malloc(__FILE__, __LINE__, sz)

/**
 * @brief debugged calloc
 */
#define HLHDF_CALLOC(npts,sz) hlhdf_alloc_calloc(__FILE__, __LINE__, npts, sz)

/**
 * @brief debugged realloc
 */
#define HLHDF_REALLOC(ptr, sz) hlhdf_alloc_realloc(__FILE__, __LINE__, ptr, sz)

/**
 * @brief debugged strdup
 */
#define HLHDF_STRDUP(x) hlhdf_alloc_strdup(__FILE__, __LINE__, x)

/**
 * @brief debugged free
 */
#define HLHDF_FREE(x) if (x != NULL) {hlhdf_alloc_free(__FILE__, __LINE__, x); x=NULL;}

#else
/**
 * @brief malloc
 */
#define HLHDF_MALLOC(sz) malloc(sz)

/**
 * @brief calloc
 */
#define HLHDF_CALLOC(npts,sz) calloc(npts, sz)

/**
 * @brief realloc
 */
#define HLHDF_REALLOC(ptr, sz) realloc(ptr, sz)

/**
 * @brief strdup
 */
#define HLHDF_STRDUP(x) strdup(x)

/**
 * @brief Frees the pointer if != NULL
 */
#define HLHDF_FREE(x) if (x != NULL) {free(x);x=NULL;}

#endif

#endif /* HLHDF_ALLOC_H */
