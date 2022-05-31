/**
 * Allocation routines for keeping track on memory
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI
)
 * @date 2009-08-15
 */
#include "rave_alloc.h"
#include "rave_debug.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * Keeps track on one allocation.
 */
typedef struct RaveHeapEntry_t {
  char* filename; /**< the filename the data was allocated in */
  int lineno; /**< the line the data was allocated at */
  size_t sz; /**< the allocated size */
  void* b;   /**< the returned ptr */
  void* ptr; /**< the internal ptr */
} RaveHeapEntry_t;

/**
 * A linked list where each entry corresponds to one allocation.
 */
typedef struct RaveHeap_t {
  RaveHeapEntry_t* entry; /**< the allocation information */
  struct RaveHeap_t* next; /**< the next entry in the linked list */
} RaveHeap_t;

static RaveHeap_t* rave_heap = NULL;

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

static RaveHeapEntry_t* rave_alloc_createHeapEntry(const char* filename, int lineno, size_t sz)
{
  RaveHeapEntry_t* result = malloc(sizeof(RaveHeapEntry_t));
  void* ptr = NULL;
  size_t ptrsize = sizeof(void*);
  if (result == NULL) {
    Rave_printf("RAVE_MEMORY_CHECK: Failed to allocate memory for heap entry\n");
    return NULL;
  }
  result->filename = strdup(filename);
  result->lineno = lineno;
  result->sz = sz;
  ptr = malloc(sz + (ptrsize*2));
  if (result->filename == NULL || ptr == NULL) {
    Rave_printf("RAVE_MEMORY_CHECK: Failed to allocate memory for filename and/or databuffer\n");
    if (result->filename != NULL) free(result->filename);
    if (ptr != NULL) free(ptr);
    free(result);
    return NULL;
  }
  result->ptr = ptr;
  if (ptrsize == 4) {
    memset(&(((unsigned char*)ptr)[0]), 0, 4);
    ((unsigned char*)ptr)[2] = 0xCA;
    ((unsigned char*)ptr)[3] = 0xFE;
    memset(&(((unsigned char*)ptr)[sz+4]), 0, 4);
    ((unsigned char*)ptr)[sz+4] = 0xCA;
    ((unsigned char*)ptr)[sz+5] = 0xFE;
    result->b = (char*)ptr + 4;
  } else {
    memset(&(((unsigned char*)ptr)[0]), 0, 8);
    ((unsigned char*)ptr)[6] = 0xCA;
    ((unsigned char*)ptr)[7] = 0xFE;
    memset(&(((unsigned char*)ptr)[sz+8]), 0, 8);
    ((unsigned char*)ptr)[sz+8] = 0xCA;
    ((unsigned char*)ptr)[sz+9] = 0xFE;
    result->b = (char*)ptr + 8;
  }
  return result;
}

static int rave_alloc_reallocateDataInEntry(RaveHeapEntry_t* entry, size_t sz)
{
  size_t ptrsize = sizeof(void*);

  if (entry == NULL) {
    Rave_printf("RAVE_MEMORY_CHECK: BAD CALL TO REALLOCATION FUNCTION, PROGRAMMING ERROR!!\n");
    RAVE_ABORT();
  }
  entry->ptr = realloc(entry->ptr, sz + (2*ptrsize));
  if (entry->ptr == NULL) {
    Rave_printf("Failed to reallocate memory...\n");
    return 0;
  }
  entry->sz = sz;
  if (ptrsize == 4) {
    memset(&(((unsigned char*)entry->ptr)[sz+4]), 0, 4);
    ((unsigned char*)entry->ptr)[sz+4] = 0xCA;
    ((unsigned char*)entry->ptr)[sz+5] = 0xFE;
    entry->b = (char*)(entry->ptr) + 4;
  } else {
    memset(&(((unsigned char*)entry->ptr)[sz+8]), 0, 8);
    ((unsigned char*)entry->ptr)[sz+8] = 0xCA;
    ((unsigned char*)entry->ptr)[sz+9] = 0xFE;
    entry->b = (char*)(entry->ptr) + 8;
  }
  return 1;
}

static RaveHeapEntry_t* rave_alloc_addHeapEntry(const char* filename, int lineno, size_t sz)
{
  RaveHeap_t* heap = NULL;
  if (rave_heap == NULL) {
    rave_heap = malloc(sizeof(RaveHeap_t));
    if (rave_heap == NULL) {
      Rave_printf("RAVE_MEMORY_CHECK: Failed to allocate root heap entry\n");
      return NULL;
    }
    rave_heap->entry = NULL;
    rave_heap->next = NULL;
  }
  heap = rave_heap;
  while (heap->entry != NULL && heap->next != NULL) {
    heap = heap->next;
  }
  if (heap->entry == NULL) {
    heap->entry = rave_alloc_createHeapEntry(filename, lineno, sz);
  } else {
    heap->next = malloc(sizeof(RaveHeap_t));
    if (heap->next == NULL) {
      Rave_printf("RAVE_MEMORY_CHECK: Failed to allocate heap node\n");
      return NULL;
    }
    heap->next->next = NULL;
    heap->next->entry = NULL;
    heap = heap->next;

    heap->entry = rave_alloc_createHeapEntry(filename, lineno, sz);
  }

  if (heap->entry == NULL) {
    Rave_printf("RAVE_MEMORY_CHECK: Failed to allocate heap entry\n");
  }
  return heap->entry;
}

static RaveHeapEntry_t* rave_alloc_findPointer(void* ptr)
{
  RaveHeap_t* heap = rave_heap;
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

static void rave_alloc_releaseMemory(const char* filename, int lineno, RaveHeap_t* heapptr)
{
  int status = 0;
  size_t ptrsize = sizeof(void*);

  if (heapptr != NULL && heapptr->entry != NULL) {
    RaveHeapEntry_t* entry = heapptr->entry;
    unsigned char* ptr = (unsigned char*)entry->ptr;
    if (ptrsize == 4) {
      if (ptr[2] == 0xCA && ptr[3] == 0xFE &&
          ptr[entry->sz+4] == 0xCA && ptr[entry->sz+5] == 0xFE) {
        // Ok, memory still intact...
        status = 1;
      }
    } else {
      if (ptr[6] == 0xCA && ptr[7] == 0xFE &&
          ptr[entry->sz+8] == 0xCA && ptr[entry->sz+9] == 0xFE) {
        // Ok, memory still intact...
        status = 1;
      }
    }
    if (status == 0) {
      Rave_printf("RAVE_MEMORY_CHECK: ---------MEMORY CORRUPTION HAS OCCURED-----------------\n");
      Rave_printf("RAVE_MEMORY_CHECK: Memory allocated from: %s:%d\n", entry->filename, entry->lineno);
      Rave_printf("RAVE_MEMORY_CHECK: Was corrupted when releasing at: %s:%d\n", filename, lineno);
      Rave_printf("RAVE_MEMORY_CHECK: Memory markers are: %x%x ... %x%x\n",
        (int)ptr[0], (int)ptr[1], (int)ptr[entry->sz+2], (int)ptr[entry->sz+3]);
    }
    free(entry->ptr);
    free(entry->filename);
    free(entry);
    heapptr->entry = NULL;
  }
}

void* rave_alloc_malloc(const char* filename, int lineno, size_t sz)
{
  RaveHeapEntry_t* entry = rave_alloc_addHeapEntry(filename, lineno, sz);
  if (entry != NULL) {
    number_of_allocations++;
    total_heap_usage += sz;
    return entry->b;
  } else {
    number_of_failed_allocations++;
    Rave_printf("RAVE_MEMORY_CHECK: Failed to allocate memory at %s:%d\n",filename,lineno);
    return NULL;
  }
}

void* rave_alloc_calloc(const char* filename, int lineno, size_t npts, size_t sz)
{
  RaveHeapEntry_t* entry = rave_alloc_addHeapEntry(filename, lineno, npts*sz);
  if (entry != NULL) {
    if (entry->b != NULL) {
      total_heap_usage += npts*sz;
      number_of_allocations++;
      memset(entry->b, 0, npts*sz);
    } else {
      number_of_failed_allocations++;
      Rave_printf("RAVE_MEMORY_CHECK: Failed to allocate data buffer at %s:%d\n",filename,lineno);
    }
    return entry->b;
  } else {
    number_of_failed_allocations++;
    Rave_printf("RAVE_MEMORY_CHECK: Failed to allocate memory at %s:%d\n",filename,lineno);
    return NULL;
  }
}

void* rave_alloc_realloc(const char* filename, int lineno, void* ptr, size_t sz)
{
  RaveHeapEntry_t* entry = NULL;
  size_t oldsz = 0;
  if (ptr == NULL) {
    return rave_alloc_malloc(filename, lineno, sz);
  }
  entry = rave_alloc_findPointer(ptr);
  if (entry == NULL) {
    number_of_failed_reallocations++;
    Rave_printf("RAVE_MEMORY_CHECK: Calling realloc without a valid pointer at %s:%d\n",filename,lineno);
    return NULL;
  }
  oldsz = entry->sz;
  if(!rave_alloc_reallocateDataInEntry(entry, sz)) {
    number_of_failed_reallocations++;
    Rave_printf("RAVE_MEMORY_CHECK: Failed to reallocate memory at %s:%d\n",filename,lineno);
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

char* rave_alloc_strdup(const char* filename, int lineno, const char* str)
{
  size_t len = 0;
  RaveHeapEntry_t* entry = NULL;
  if (str == NULL) {
    number_of_failed_strdup++;
    Rave_printf("RAVE_MEMORY_CHECK:Atempting to strdup NULL string %s:%d\n",filename,lineno);
    return NULL;
  }
  len = strlen(str) + 1;
  entry = rave_alloc_addHeapEntry(filename, lineno, len);
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
    Rave_printf("RAVE_MEMORY_CHECK: Failed to allocate memory at %s:%d\n",filename,lineno);
    return NULL;
  }
}

void rave_alloc_free(const char* filename, int lineno, void* ptr)
{
  RaveHeap_t* heapptr = rave_heap;
  if (heapptr == NULL) {
    number_of_failed_frees++;
    Rave_printf("RAVE_MEMORY_CHECK: FREE CALLED ON DATA NOT ALLOCATED BY HLHDF: %s:%d.\n",filename,lineno);
    return;
  }
  if (ptr == NULL) {
    number_of_failed_frees++;
    Rave_printf("RAVE_MEMORY_CHECK: ATEMPTING TO FREE NULL-value at %s:%d", filename, lineno);
    return;
  }
  while (heapptr != NULL) {
    if (heapptr->entry != NULL && heapptr->entry->b == ptr) {
      number_of_frees++;
      total_freed_heap_usage += heapptr->entry->sz;
      rave_alloc_releaseMemory(filename, lineno, heapptr);
      return;
    }
    heapptr = heapptr->next;
  }
  number_of_failed_frees++;
  Rave_printf("RAVE_MEMORY_CHECK: Atempting to free something that not has been allocated: %s:%d\n", filename, lineno);
}

void rave_alloc_dump_heap(void)
{
  RaveHeap_t* heapptr = rave_heap;
  int msgPrinted = 0;
  if (heapptr == NULL) {
    return;
  }
  while (heapptr != NULL) {
    if (heapptr->entry != NULL) {
      if (!msgPrinted) {
        Rave_printf("RAVE_MEMORY_CHECK: Application terminating...\n");
        msgPrinted = 1;
      }
      Rave_printf("RAVE_MEMORY_CHECK: %d bytes allocated %s:%d\n", (int)heapptr->entry->sz, heapptr->entry->filename, heapptr->entry->lineno);
    }
    heapptr = heapptr->next;
  }
}

void rave_alloc_print_statistics(void)
{
#ifdef RAVE_MEMORY_DEBUG
  size_t totalNumberOfAllocations = number_of_allocations + number_of_strdup;
  int maxNbrOfAllocs = 0;
  RaveHeap_t* heapptr = rave_heap;

  while (heapptr != NULL) {
    maxNbrOfAllocs++;
    heapptr = heapptr->next;
  }
  Rave_printf("\n\n");
  Rave_printf("RAVE HEAP STATISTICS:\n");
  Rave_printf("Number of allocations  : %ld\n",number_of_allocations);
  Rave_printf("Number of reallocations: %ld\n", number_of_reallocations);
  Rave_printf("Number of strdup       : %ld\n", number_of_strdup);
  Rave_printf("Number of frees:       : %ld\n", number_of_frees);
  Rave_printf("Total nbr allocs/frees : %ld / %ld\n", totalNumberOfAllocations, number_of_frees);
  Rave_printf("Total heap allocation  : %ld bytes\n", total_heap_usage);
  Rave_printf("Total heap deallocation: %ld bytes\n", total_freed_heap_usage);
  Rave_printf("Lost heap              : %ld bytes\n", (total_heap_usage - total_freed_heap_usage));
  Rave_printf("Max number of allocs   : %d\n", maxNbrOfAllocs);

  if (number_of_failed_allocations > 0)
    Rave_printf("Number of failed allocations     : %ld\n", number_of_failed_allocations);
  if (number_of_failed_reallocations  > 0)
    Rave_printf("Number of failed reallocations   : %ld\n", number_of_failed_reallocations);
  if (number_of_failed_frees > 0)
    Rave_printf("Number of failed frees           : %ld\n", number_of_failed_frees);
  if (number_of_failed_strdup > 0)
    Rave_printf("Number of failed strdup          : %ld\n", number_of_failed_strdup);
  Rave_printf("\n\n");
  rave_alloc_dump_heap();
#endif
}
