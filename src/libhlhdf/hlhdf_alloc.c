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
#include "hlhdf_alloc.h"
#include "hlhdf_debug.h"
#include <stdlib.h>
#include <string.h>

/**
 * Keeps track on one allocation.
 */
typedef struct HlhdfHeapEntry_t {
  char* filename; /**< the filename the data was allocated in */
  int lineno; /**< the line the data was allocated at */
  size_t sz; /**< the allocated size */
  void* b;   /**< the returned ptr */
  void* ptr; /**< the internal ptr */
} HlhdfHeapEntry_t;

/**
 * A linked list where each entry corresponds to one allocation.
 */
typedef struct HlhdfHeap_t {
  HlhdfHeapEntry_t* entry; /**< the allocation information */
  struct HlhdfHeap_t* next; /**< the next entry in the linked list */
} HlhdfHeap_t;

static HlhdfHeap_t* hlhdf_heap = NULL;

static size_t number_of_allocations = 0;
static size_t number_of_failed_allocations = 0;
static size_t number_of_frees = 0;
static size_t number_of_failed_frees = 0;
static size_t number_of_reallocations = 0;
static size_t number_of_failed_reallocations = 0;
static size_t number_of_strdup = 0;
static size_t number_of_failed_strdup = 0;
static size_t total_heap_usage = 0;
static size_t total_freed_heap_usage = 0;

static HlhdfHeapEntry_t* hlhdf_alloc_createHeapEntry(const char* filename, int lineno, size_t sz)
{
  HlhdfHeapEntry_t* result = malloc(sizeof(HlhdfHeapEntry_t));
  void* ptr = NULL;
  if (result == NULL) {
    HL_printf("HLHDF_MEMORY_CHECK: Failed to allocate memory for heap entry\n");
    return NULL;
  }
  result->filename = strdup(filename);
  result->lineno = lineno;
  result->sz = sz;
  ptr = malloc(sz + 4);
  if (result->filename == NULL || ptr == NULL) {
    HL_printf("HLHDF_MEMORY_CHECK: Failed to allocate memory for filename and/or databuffer\n");
    if (result->filename != NULL) free(result->filename);
    if (ptr != NULL) free(ptr);
    free(result);
    return NULL;
  }
  ((unsigned char*)ptr)[0] = 0xCA;
  ((unsigned char*)ptr)[1] = 0xFE;
  ((unsigned char*)ptr)[sz+2] = 0xCA;
  ((unsigned char*)ptr)[sz+3] = 0xFE;
  result->ptr = ptr;
  result->b = ptr + 2;
  return result;
}

static int hlhdf_alloc_reallocateDataInEntry(HlhdfHeapEntry_t* entry, size_t sz)
{
  if (entry == NULL) {
    HL_printf("BAD CALL TO REALLOCATION FUNCTION, PROGRAMMING ERROR!!\n");
    HL_ABORT();
  }
  entry->ptr = realloc(entry->ptr, sz + 4);
  if (entry->ptr == NULL) {
    HL_printf("Failed to reallocate memory...\n");
    return 0;
  }
  entry->sz = sz;
  ((unsigned char*)entry->ptr)[sz+2] = 0xCA;
  ((unsigned char*)entry->ptr)[sz+3] = 0xFE;
  entry->b = entry->ptr + 2;
  return 1;
}

static HlhdfHeapEntry_t* hlhdf_alloc_addHeapEntry(const char* filename, int lineno, size_t sz)
{
  HlhdfHeap_t* heap = NULL;
  if (hlhdf_heap == NULL) {
    hlhdf_heap = malloc(sizeof(HlhdfHeap_t));
    if (hlhdf_heap == NULL) {
      HL_printf("HLHDF_MEMORY_CHECK: Failed to allocate root heap entry\n");
      return NULL;
    }
    hlhdf_heap->entry = NULL;
    hlhdf_heap->next = NULL;
  }
  heap = hlhdf_heap;
  while (heap->entry != NULL && heap->next != NULL) {
    heap = heap->next;
  }
  if (heap->entry == NULL) {
    heap->entry = hlhdf_alloc_createHeapEntry(filename, lineno, sz);
  } else {
    heap->next = malloc(sizeof(HlhdfHeap_t));
    if (heap->next == NULL) {
      HL_printf("HLHDF_MEMORY_CHECK: Failed to allocate heap node\n");
      return NULL;
    }
    heap->next->next = NULL;
    heap->next->entry = NULL;
    heap = heap->next;

    heap->entry = hlhdf_alloc_createHeapEntry(filename, lineno, sz);
  }

  if (heap->entry == NULL) {
    HL_printf("HLHDF_MEMORY_CHECK: Failed to allocate heap entry\n");
  }
  return heap->entry;
}

static HlhdfHeapEntry_t* hlhdf_alloc_findPointer(void* ptr)
{
  HlhdfHeap_t* heap = hlhdf_heap;
  if (heap == NULL) {
    return NULL;
  }
  while (heap != NULL) {
    if (heap->entry != NULL && heap->entry->b == ptr) {
      return heap->entry;
    }
    heap = heap->next;
  }
  return NULL;
}

static void hlhdf_alloc_releaseMemory(const char* filename, int lineno, HlhdfHeap_t* heapptr)
{
  int status = 0;
  if (heapptr != NULL && heapptr->entry != NULL) {
    HlhdfHeapEntry_t* entry = heapptr->entry;
    unsigned char* ptr = (unsigned char*)entry->ptr;
    if (ptr[0] == 0xCA && ptr[1] == 0xFE &&
        ptr[entry->sz+2] == 0xCA && ptr[entry->sz+3] == 0xFE) {
      // Ok, memory still intact...
      status = 1;
    }
    if (status == 0) {
      HL_printf("HLHDF_MEMORY_CHECK: ---------MEMORY CORRUPTION HAS OCCURED-----------------\n");
      HL_printf("HLHDF_MEMORY_CHECK: Memory allocated from: %s:%d\n", entry->filename, entry->lineno);
      HL_printf("HLHDF_MEMORY_CHECK: Was corrupted when releasing at: %s:%d\n", filename, lineno);
      HL_printf("HLHDF_MEMORY_CHECK: Memory markers are: %x%x ... %x%x\n",
        (int)ptr[0], (int)ptr[1], (int)ptr[entry->sz+2], (int)ptr[entry->sz+3]);
    }
    free(entry->ptr);
    free(entry->filename);
    free(entry);
    heapptr->entry = NULL;
  }
}

void* hlhdf_alloc_malloc(const char* filename, int lineno, size_t sz)
{
  HlhdfHeapEntry_t* entry = hlhdf_alloc_addHeapEntry(filename, lineno, sz);
  if (entry != NULL) {
    number_of_allocations++;
    total_heap_usage += sz;
    return entry->b;
  } else {
    number_of_failed_allocations++;
    HL_printf("HLHDF_MEMORY_CHECK: Failed to allocate memory at %s:%d\n",filename,lineno);
    return NULL;
  }
}

void* hlhdf_alloc_calloc(const char* filename, int lineno, size_t npts, size_t sz)
{
  HlhdfHeapEntry_t* entry = hlhdf_alloc_addHeapEntry(filename, lineno, npts*sz);
  if (entry != NULL) {
    if (entry->b != NULL) {
      total_heap_usage += npts*sz;
      number_of_allocations++;
      memset(entry->b, 0, npts*sz);
    } else {
      number_of_failed_allocations++;
      HL_printf("Failed to allocate data buffer at %s:%d\n",filename,lineno);
    }
    return entry->b;
  } else {
    number_of_failed_allocations++;
    HL_printf("HLHDF_MEMORY_CHECK: Failed to allocate memory at %s:%d\n",filename,lineno);
    return NULL;
  }
}

void* hlhdf_alloc_realloc(const char* filename, int lineno, void* ptr, size_t sz)
{
  HlhdfHeapEntry_t* entry = NULL;
  size_t oldsz = 0;
  if (ptr == NULL) {
    return hlhdf_alloc_malloc(filename, lineno, sz);
  }
  entry = hlhdf_alloc_findPointer(ptr);
  if (entry == NULL) {
    number_of_failed_reallocations++;
    HL_printf("HLHDF_MEMORY_CHECK: Calling realloc without a valid pointer at %s:%d\n",filename,lineno);
    return NULL;
  }
  oldsz = entry->sz;
  if(!hlhdf_alloc_reallocateDataInEntry(entry, sz)) {
    number_of_failed_reallocations++;
    HL_printf("HLHDF_MEMORY_CHECK: Failed to reallocate memory at %s:%d\n",filename,lineno);
  } else {
    number_of_reallocations++;
    if (sz > oldsz) {
      total_heap_usage += (sz - oldsz);
    } else {
      total_heap_usage -= (oldsz - sz);
    }
  }
  return entry->b;
}

char* hlhdf_alloc_strdup(const char* filename, int lineno, const char* str)
{
  size_t len = 0;
  HlhdfHeapEntry_t* entry = NULL;
  if (str == NULL) {
    number_of_failed_strdup++;
    HL_printf("HLHDF_MEMORY_CHECK:Atempting to strdup NULL string %s:%d\n",filename,lineno);
    return NULL;
  }
  len = strlen(str) + 1;
  entry = hlhdf_alloc_addHeapEntry(filename, lineno, len);
  if (entry != NULL) {
    if (entry->b != NULL) {
      total_heap_usage += len;
      number_of_strdup++;
      memcpy(entry->b, str, len);
    } else {
      number_of_failed_strdup++;
    }
    return entry->b;
  } else {
    number_of_failed_strdup++;
    HL_printf("HLHDF_MEMORY_CHECK: Failed to allocate memory at %s:%d\n",filename,lineno);
    return NULL;
  }
}

void hlhdf_alloc_free(const char* filename, int lineno, void* ptr)
{
  HlhdfHeap_t* heapptr = hlhdf_heap;
  if (heapptr == NULL) {
    number_of_failed_frees++;
    HL_printf("HLHDF_MEMORY_CHECK: FREE CALLED ON DATA NOT ALLOCATED BY HLHDF: %s:%d.\n",filename,lineno);
    return;
  }
  if (ptr == NULL) {
    number_of_failed_frees++;
    HL_printf("HLHDF_MEMORY_CHECK: ATEMPTING TO FREE NULL-value at %s:%d", filename, lineno);
    return;
  }
  while (heapptr != NULL) {
    if (heapptr->entry != NULL && heapptr->entry->b == ptr) {
      number_of_frees++;
      total_freed_heap_usage += heapptr->entry->sz;
      hlhdf_alloc_releaseMemory(filename, lineno, heapptr);
      return;
    }
    heapptr = heapptr->next;
  }
  number_of_failed_frees++;
  HL_printf("HLHDF_MEMORY_CHECK: Atempting to free something that not has been allocated: %s:%d\n", filename, lineno);
}

void hlhdf_alloc_dump_heap(void)
{
  HlhdfHeap_t* heapptr = hlhdf_heap;
  int msgPrinted = 0;
  if (heapptr == NULL) {
    return;
  }
  while (heapptr != NULL) {
    if (heapptr->entry != NULL) {
      if (!msgPrinted) {
        HL_printf("HLHDF_MEMORY_CHECK: Application terminating...\n");
        msgPrinted = 1;
      }
      HL_printf("HLHDF_MEMORY_CHECK: %d bytes allocated %s:%d\n", (int)heapptr->entry->sz, heapptr->entry->filename, heapptr->entry->lineno);
    }
    heapptr = heapptr->next;
  }
}

void hlhdf_alloc_print_statistics(void)
{
  size_t totalNumberOfAllocations = number_of_allocations + number_of_strdup;
  int maxNbrOfAllocs = 0;
  HlhdfHeap_t* heapptr = hlhdf_heap;

  while (heapptr != NULL) {
    maxNbrOfAllocs++;
    heapptr = heapptr->next;
  }

  HL_printf("HLHDF HEAP STATISTICS:\n");
  HL_printf("Number of allocations  : %ld\n",number_of_allocations);
  HL_printf("Number of reallocations: %ld\n", number_of_reallocations);
  HL_printf("Number of strdup       : %ld\n", number_of_strdup);
  HL_printf("Number of frees:       : %ld\n", number_of_frees);
  HL_printf("Total nbr allocs/frees : %ld / %ld\n", totalNumberOfAllocations, number_of_frees);
  HL_printf("Total heap allocation  : %ld bytes\n", total_heap_usage);
  HL_printf("Total heap deallocation: %ld bytes\n", total_freed_heap_usage);
  HL_printf("Lost heap              : %ld bytes\n", (total_heap_usage - total_freed_heap_usage));
  HL_printf("Max number of allocs   : %d\n", maxNbrOfAllocs);

  if (number_of_failed_allocations > 0)
    HL_printf("Number of failed allocations     : %ld\n", number_of_failed_allocations);
  if (number_of_failed_reallocations  > 0)
    HL_printf("Number of failed reallocations   : %ld\n", number_of_failed_reallocations);
  if (number_of_failed_frees > 0)
    HL_printf("Number of failed frees           : %ld\n", number_of_failed_frees);
  if (number_of_failed_strdup > 0)
    HL_printf("Number of failed strdup          : %ld\n", number_of_failed_strdup);
}
