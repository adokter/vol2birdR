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
 * Implementation of a rave object hashtable that maps between strings and
 * rave core objects.
 * This object supports \ref #RAVE_OBJECT_CLONE with an exception, if any members
 * of the list is not possible to clone, they will not be added to the list which
 * means that the returned list might have fewer entries.
 *
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-01-21
 */
#include "raveobject_hashtable.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include <string.h>
#include <stdio.h>

typedef struct RaveHash_bucket {
  char* key;
  struct RaveHash_bucket* next;
  RaveCoreObject* object;
} RaveHash_bucket;

/**
 * Represents a hash table
 */
struct _RaveObjectHashTable_t {
  RAVE_OBJECT_HEAD /** Always on top */
  int bucketCount; /** Number of top list buckets */
  RaveHash_bucket** buckets; /** The array of buckets */
};

#define INITIAL_BUCKET_COUNT 10

/*@{ Private functions */
/**
 * Creates a hash value from a string.
 * @param[in] str - the string to create a hash from
 * @returns a hash value
 */
static unsigned long roht_createhash(const char* str)
{
  unsigned long hash = 0;
  int c = 0;

  while ((c = *str++)) {
    hash = c + (hash << 6) + (hash << 16) - hash;
  }

  return hash;
}

/**
 * Creates a bucket for insertion in the table.
 * @param[in] key - the key
 * @param[in] object - the object
 * @returns a new bucket instance on success, otherwise NULL
 */
static RaveHash_bucket* roht_createbucket(const char* key, RaveCoreObject* object)
{
  RaveHash_bucket* result = RAVE_MALLOC(sizeof(RaveHash_bucket));
  if (result == NULL) {
    return NULL;
  }
  result->key = RAVE_STRDUP(key);
  if (result->key == NULL) {
    RAVE_FREE(result);
    return NULL;
  }
  result->object = RAVE_OBJECT_COPY(object);
  result->next = NULL;
  return result;
}

/**
 * Destroys the bucket with it's members.
 * @param[in] bucket - the bucket to be destroyed
 */
static void roht_destroybucket(RaveHash_bucket* bucket)
{
  if (bucket != NULL) {
    roht_destroybucket(bucket->next);
    RAVE_FREE(bucket->key);
    RAVE_OBJECT_RELEASE(bucket->object);
    RAVE_FREE(bucket);
  }
}

/**
 * Finds a bucket with the specified key.
 * @param[in] bucket - the top bucket
 * @param[in] key - the key name
 * @param[out] lastbucket - the last bucket in the linked list
 * @returns the bucket if found, otherwise NULL
 */
static RaveHash_bucket* roht_findbucket(RaveHash_bucket* bucket, const char* key, RaveHash_bucket** lastbucket)
{
  if (lastbucket != NULL) {
    *lastbucket = NULL;
  }
  while (bucket != NULL) {
    if (bucket->key != NULL && strcmp(key, bucket->key) == 0) {
      return bucket;
    }
    if (bucket->next == NULL && lastbucket != NULL) {
      *lastbucket = bucket;
    }
    bucket = bucket->next;
  }
  return NULL;
}

/**
 * Adds all keys from this chain of buckets.
 * @param[in] bucket - the bucket to start adding keys from
 * @param[in] l - the list to add the keys to
 * @returns 1 on success, otherwise 0.
 */
static int roht_addkeyfrombuckets(RaveHash_bucket* bucket, RaveList_t* l)
{
  RAVE_ASSERT((l != NULL), "l == NULL");
  if (bucket != NULL) {
    if (bucket->key != NULL) {
      char* tmpstr = RAVE_STRDUP(bucket->key);
      if (tmpstr == NULL) {
        return 0;
      }
      if (!RaveList_add(l, tmpstr)) {
        RAVE_FREE(tmpstr);
        return 0;
      }
    }
    return roht_addkeyfrombuckets(bucket->next, l);
  }
  return 1;
}

/**
 * Adds all values from this chain of buckets.
 * @param[in] bucket - the bucket to start adding values from
 * @param[in] l - the list to add the values to
 * @returns 1 on success, otherwise 0.
 */
static int roht_addvaluefrombuckets(RaveHash_bucket* bucket, RaveObjectList_t* l)
{
  RAVE_ASSERT((l != NULL), "l == NULL");
  if (bucket != NULL) {
    if (bucket->object != NULL) {
      if (!RaveObjectList_add(l, bucket->object)) {
        return 0;
      }
    }
    return roht_addvaluefrombuckets(bucket->next, l);
  }
  return 1;
}

/**
 * Counts the number of buckets including self.
 * @param[in] bucket - the start bucket (may be NULL and in that case, 0 is returned)
 * @returns the number of buckets
 */
static int roht_countbuckets(RaveHash_bucket* bucket)
{
  int sz = 0;
  while (bucket != NULL) {
    sz++;
    bucket = bucket->next;
  }
  return sz;
}

/**
 * Clones a bucket and its childs.
 * @param[in] src - the bucket to be cloned
 */
static RaveHash_bucket* roht_clonebucket(RaveHash_bucket* src)
{
  RaveHash_bucket* clone = NULL;
  if (src == NULL || src->key == NULL || src->object == NULL) {
    return NULL;
  }
  if (!RAVE_OBJECT_ISCLONEABLE(src->object)) {
    RAVE_ERROR1("Atempting to clone a non cloneable object: %s", src->object->roh_type->name);
    goto fail;
  }
  clone = RAVE_MALLOC(sizeof(RaveHash_bucket));
  if (clone == NULL) {
    goto fail;
  }
  clone->key = RAVE_STRDUP(src->key);
  clone->object = RAVE_OBJECT_CLONE(src->object);
  clone->next = NULL;
  if (clone->key == NULL || clone->object == NULL) {
    RAVE_ERROR0("Error allocating memory for clone");
    goto fail;
  }
  if (src->next != NULL) {
    clone->next = roht_clonebucket(src->next);
    if (clone->next == NULL) {
      goto fail;
    }
  }
  return clone;
fail:
  if (clone != NULL) {
    roht_destroybucket(clone);
  }
  return NULL;
}

static void roht_clearbuckets(RaveObjectHashTable_t* this)
{
  int i = 0;
  for (i = 0; i < this->bucketCount; i++) {
    roht_destroybucket(this->buckets[i]);
    this->buckets[i] = NULL;
  }
}

/**
 * Constructor.
 * @param[in] obj - the created object
 */
static int RaveObjectHashTable_constructor(RaveCoreObject* obj)
{
  RaveObjectHashTable_t* roht = (RaveObjectHashTable_t*)obj;
  int i = 0;
  roht->bucketCount = INITIAL_BUCKET_COUNT;
  roht->buckets = RAVE_MALLOC(sizeof(RaveHash_bucket*) * roht->bucketCount);
  if (roht->buckets == NULL) {
    return 0;
  }
  for (i = 0; i < roht->bucketCount; i++) {
    roht->buckets[i] = NULL;
  }
  return 1;
}

/**
 * Destructor
 * @param[in] obj - the object to destroy
 */
static void RaveObjectHashTable_destructor(RaveCoreObject* obj)
{
  RaveObjectHashTable_t* this = (RaveObjectHashTable_t*)obj;
  roht_clearbuckets(this);
  RAVE_FREE(this->buckets);
  this->buckets = NULL;
}

/**
 * Constructor.
 * @param[in] obj - the created object
 */
static int RaveObjectHashTable_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  RaveObjectHashTable_t* this = (RaveObjectHashTable_t*)obj;
  RaveObjectHashTable_t* src = (RaveObjectHashTable_t*)srcobj;
  int i = 0;

  this->bucketCount = src->bucketCount;
  this->buckets = RAVE_MALLOC(sizeof(RaveHash_bucket*) * this->bucketCount);
  if (this->buckets == NULL) {
    goto fail;
  }
  memset(this->buckets, 0, sizeof(RaveHash_bucket*) * this->bucketCount);

  for (i = 0; i < src->bucketCount; i++) {
    if (src->buckets[i] != NULL) {
      this->buckets[i] = roht_clonebucket(src->buckets[i]);
      if (this->buckets[i] == NULL) {
        goto fail;
      }
    }
  }
  return 1;
fail:
  RaveObjectHashTable_destructor((RaveCoreObject*)this);
  return 0;
}


/*@} End of Private functions */

/*@{ Interface functions */
int RaveObjectHashTable_put(RaveObjectHashTable_t* table, const char* key, RaveCoreObject* obj)
{
  long index = 0;
  RAVE_ASSERT((table != NULL), "table == NULL");
  if (key == NULL || obj == NULL) {
    return 0;
  }
  index = roht_createhash(key) % table->bucketCount;
  if (table->buckets[index] != NULL) {
    RaveHash_bucket* bucket = NULL;
    RaveHash_bucket* lastbucket = NULL;
    bucket = roht_findbucket(table->buckets[index], key, &lastbucket);

    if (bucket == NULL) {
      // No match found, then we hopefully should have the last bucket at least.
      if (lastbucket != NULL) {
        if (lastbucket->next == NULL) {
          lastbucket->next = roht_createbucket(key, obj);
          if (lastbucket->next == NULL) {
            goto fail;
          }
        } else {
          goto fail;
        }
      }
    } else {
      RAVE_OBJECT_RELEASE(bucket->object);
      bucket->object = RAVE_OBJECT_COPY(obj);
    }
  } else {
    table->buckets[index] = roht_createbucket(key, obj);
    if (table->buckets[index] == NULL) {
      goto fail;
    }
  }

  return 1;
fail:
  return 0;
}

RaveCoreObject* RaveObjectHashTable_get(RaveObjectHashTable_t* table, const char* key)
{
  long index = 0;
  RaveHash_bucket* bucket = NULL;

  RAVE_ASSERT((table != NULL), "table == NULL");
  if (key != NULL) {
    index = roht_createhash(key) % table->bucketCount;
    if (table->buckets[index] != NULL) {
      bucket = roht_findbucket(table->buckets[index], key, NULL);
    }
  }

  if (bucket != NULL) {
    return RAVE_OBJECT_COPY(bucket->object);
  }
  return NULL;
}

int RaveObjectHashTable_size(RaveObjectHashTable_t* table)
{
  int sz = 0;
  int i = 0;
  RAVE_ASSERT((table != NULL), "table == NULL");
  for (i = 0; i < table->bucketCount; i++) {
    sz += roht_countbuckets(table->buckets[i]);
  }
  return sz;
}

int RaveObjectHashTable_exists(RaveObjectHashTable_t* table, const char* key)
{
  long index = 0;
  RAVE_ASSERT((table != NULL), "table == NULL");
  index = roht_createhash(key) % table->bucketCount;
  return roht_findbucket(table->buckets[index], key, NULL)==NULL?0:1;
}

RaveCoreObject* RaveObjectHashTable_remove(RaveObjectHashTable_t* table, const char* key)
{
  long index = 0;
  RaveHash_bucket* bucket = NULL;
  RaveCoreObject* result = NULL;

  RAVE_ASSERT((table != NULL), "table == NULL");
  if (key == NULL) {
    return NULL;
  }
  index = roht_createhash(key) % table->bucketCount;
  if (table->buckets[index] != NULL) {
    RaveHash_bucket* this = table->buckets[index];
    if (this->key != NULL && strcmp(key, this->key) == 0) {
      table->buckets[index] = this->next;
      bucket = this;
      bucket->next = NULL;
    }

    while (bucket == NULL && this != NULL) {
      RaveHash_bucket* nextbucket = this->next;
      if (nextbucket != NULL) {
        if (nextbucket->key != NULL && strcmp(key, nextbucket->key) == 0) {
          this->next = nextbucket->next;
          bucket = nextbucket;
          bucket->next = NULL;
        }
      }
      this = this->next;
    }
  }

  if (bucket != NULL) {
    result = RAVE_OBJECT_COPY(bucket->object);
    roht_destroybucket(bucket);
  }
  return result;
}

void RaveObjectHashTable_clear(RaveObjectHashTable_t* table)
{
  RAVE_ASSERT((table != NULL), "table == NULL");
  roht_clearbuckets(table);
}


RaveList_t* RaveObjectHashTable_keys(RaveObjectHashTable_t* table)
{
  RaveList_t* result = NULL;
  int i = 0;

  RAVE_ASSERT((table != NULL), "table == NULL");

  result = RAVE_OBJECT_NEW(&RaveList_TYPE);
  if (result != NULL) {
    for (i = 0; i < table->bucketCount; i++) {
      if (!roht_addkeyfrombuckets(table->buckets[i], result)) {
        goto fail;
      }
    }
  }

  return result;
fail:
  RaveObjectHashTable_destroyKeyList(result);
  return NULL;
}

RaveObjectList_t* RaveObjectHashTable_values(RaveObjectHashTable_t* table)
{
  RaveObjectList_t* result = NULL;
  int i = 0;
  RAVE_ASSERT((table != NULL), "table == NULL");
  result = RAVE_OBJECT_NEW(&RaveObjectList_TYPE);
  if (result != NULL) {
    for (i = 0; i < table->bucketCount; i++) {
      if (!roht_addvaluefrombuckets(table->buckets[i], result)) {
        goto fail;
      }
    }
  }
  return result;
fail:
  RAVE_OBJECT_RELEASE(result);
  return NULL;
}

void RaveObjectHashTable_destroyKeyList(RaveList_t* l)
{
  if (l != NULL) {
    char* ptr = NULL;
    while ((ptr = RaveList_removeLast(l)) != NULL) {
      RAVE_FREE(ptr);
    }
    RAVE_OBJECT_RELEASE(l);
  }
}

/*@} End of Interface functions */

RaveCoreObjectType RaveObjectHashTable_TYPE = {
    "HashTable",
    sizeof(RaveObjectHashTable_t),
    RaveObjectHashTable_constructor,
    RaveObjectHashTable_destructor,
    RaveObjectHashTable_copyconstructor
};
