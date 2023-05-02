/**
 * Allocation routines for keeping track on memory
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-08-15
 */
#ifndef RAVE_ALLOC_H
#define RAVE_ALLOC_H
#include <stdlib.h>

/**
 * Allocates memory and keeps track on if it is released, overwritten and
 * similar.
 * @param[in] filename the name of the file the allocation occurs in
 * @param[in] lineno the linenumber
 * @param[in] sz the number of bytes to be allocated
 */
void* rave_alloc_malloc(const char* filename, int lineno, size_t sz);

/**
 * Same as calloc but debugged.
 * @param[in] filename the name of the file the allocation occurs in
 * @param[in] lineno the linenumber
 * @param[in] npts number of points
 * @param[in] sz the number of bytes to be allocated
 */
void* rave_alloc_calloc(const char* filename, int lineno, size_t npts, size_t sz);

/**
 * Same as realloc but debugged.
 * @param[in] filename the name of the file the allocation occurs in
 * @param[in] lineno the linenumber
 * @param[in] ptr the original pointer
 * @param[in] sz the number of bytes to be allocated
 */
void* rave_alloc_realloc(const char* filename, int lineno, void* ptr, size_t sz);

/**
 * Same as strdup but debugged.
 * @param[in] filename the name of the file the allocation occurs in
 * @param[in] lineno the linenumber
 * @param[in] str the number of bytes to be allocated
 */
char* rave_alloc_strdup(const char* filename, int lineno, const char* str);

/**
 * Releases the memory
 * @param[in] filename the name of the file the allocation occurs in
 * @param[in] lineno the linenumber
 * @param[in] ptr the pointer that should be freed
 */
void rave_alloc_free(const char* filename, int lineno, void* ptr);

/**
 * Dumps all blocks that not has been released.
 */
void rave_alloc_dump_heap(void);

/**
 * Prints the statistics for the heap
 */
void rave_alloc_print_statistics(void);

#ifdef RAVE_MEMORY_DEBUG
/**
 * @brief debugged malloc
 */
#define RAVE_MALLOC(sz) rave_alloc_malloc(__FILE__, __LINE__, sz)

/**
 * @brief debugged calloc
 */
#define RAVE_CALLOC(npts,sz) rave_alloc_calloc(__FILE__, __LINE__, npts, sz)

/**
 * @brief debugged realloc
 */
#define RAVE_REALLOC(ptr, sz) rave_alloc_realloc(__FILE__, __LINE__, ptr, sz)

/**
 * @brief debugged strdup
 */
#define RAVE_STRDUP(x) rave_alloc_strdup(__FILE__, __LINE__, x)

/**
 * @brief debugged free
 */
#define RAVE_FREE(x) if (x != NULL) {rave_alloc_free(__FILE__, __LINE__, x); x=NULL;}

#else
/**
 * @brief malloc
 */
#define RAVE_MALLOC(sz) malloc(sz)

/**
 * @brief calloc
 */
#define RAVE_CALLOC(npts,sz) calloc(npts, sz)

/**
 * @brief realloc
 */
#define RAVE_REALLOC(ptr, sz) realloc(ptr, sz)

/**
 * @brief strdup
 */
#define RAVE_STRDUP(x) strdup(x)

/**
 * @brief Frees the pointer if != NULL
 */
#define RAVE_FREE(x) if (x != NULL) {free(x);x=NULL;}

#endif


#endif /* RAVE_ALLOC_H */
