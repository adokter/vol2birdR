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
 * Generic implementation of an object that is used within rave. All
 * objects should use this as template for their structure.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-11-25
 */
#ifndef RAVE_OBJECT_H
#define RAVE_OBJECT_H
#include <stdlib.h>

/**
 * Always should be at top of a struct that implements a RaveObject.
 */
#define RAVE_OBJECT_HEAD \
  int roh_refCnt; \
  struct _raveobjecttype* roh_type; \
  void* roh_bindingData;

/**
 * The basic raveobject that contains the header information for all
 * rave objects.
 */
typedef struct _raveobject {
  RAVE_OBJECT_HEAD /** Always on top */
} RaveCoreObject;

/**
 * The rave object type definition.
 * If you are implementing support for the copy constructor, you must ensure that
 * all members of the object are clones as well so that there are no references
 * to other objects.
 */
typedef struct _raveobjecttype {
  const char* name; /**< the name, for printout */
  size_t type_size; /**< the size of the object, sizeof(type) */
  int (*constructor)(RaveCoreObject* obj); /**< function to be called for initialization of the object */
  void (*destructor)(RaveCoreObject* obj); /**< function to be called for release of members in the object */
  int (*copyconstructor)(RaveCoreObject* obj, RaveCoreObject* source); /**< Function when creating an clone */
} RaveCoreObjectType;

/**
 * Creates a new instance of a specified object type, should be implemented within
 * the object itself until we have a generic way to create instances by type.
 * Typically:
 * SomeObject_t* obj = RAVE_OBJECT_NEW(&SomeObject_Type);
 */
#define RAVE_OBJECT_NEW(type) \
  (void*)RaveCoreObject_new(type, __FILE__, __LINE__);

/**
 * Releases the provided object once (i.e. decrements the reference counter and
 * sets the invalue to NULL. Ie. Do not do something funny like:
 * RAVE_OBJECT_RELEASE(x[index++]) since that will mess up the memory, instead write
 * RAVE_OBJECT_RELEASE(x[index]);
 * index++
 */
#define RAVE_OBJECT_RELEASE(obj) \
  RaveCoreObject_release((RaveCoreObject*)(obj), __FILE__, __LINE__); \
  (obj) = NULL;

/**
 * Increments the reference counter once for the specified object and returns
 * the pointer. Typically used as:
 * x = RAVE_OBJECT_COPY(obj);
 * ....
 * RAVE_OBJECT_RELEASE(x);
 */
#define RAVE_OBJECT_COPY(src) \
  (void*)RaveCoreObject_copy((RaveCoreObject*)(src), __FILE__, __LINE__)

/**
 * Creates a clone of a object. Not to be confused with COPY.
 * I.e. basically the same as doing a NEW followed by a copy of all essential members.
 * Be aware that not all objects supports this function and in that case,
 * NULL will be returned. If it is interesting to ensure that everything is
 * successful, you can always do this:
 * \verbatim
 * if (RAVE_OBJECT_ISCLONEABLE(src)) {
 *   clone = RAVE_OBJECT_CLONE(src);
 *   if (clone == NULL) {
 *     // memory allocation error or something
 *   }
 * }
 * \endverbatim
 */
#define RAVE_OBJECT_CLONE(src) \
  (void*)RaveCoreObject_clone((RaveCoreObject*)(src), __FILE__, __LINE__)

/**
 * Returns if this object is cloneable or not. Is determined by verifying
 * if the type contains a copy constructor or not.
 */
#define RAVE_OBJECT_ISCLONEABLE(src) \
  RaveCoreObject_isCloneable((RaveCoreObject*)(src))

/**
 * Returns the provided objects reference count.
 */
#define RAVE_OBJECT_REFCNT(src) \
  RaveCoreObject_getRefCount((RaveCoreObject*)src)

/**
 * Binds the rave object to some sort of pointer value. E.g. a python object pointer.
 */
#define RAVE_OBJECT_BIND(this, bound) \
  RaveCoreObject_bind((RaveCoreObject*)this, bound)

/**
 * Unbinds the rave object from a pointer value. E.g. a python object pointer.
 */
#define RAVE_OBJECT_UNBIND(this, bound) \
  RaveCoreObject_unbind((RaveCoreObject*)this, bound)

/**
 * Returns if this object currently is bound to a pointer or not.
 */
#define RAVE_OBJECT_ISBOUND(this) \
  (RaveCoreObject_getBindingData((RaveCoreObject*)this) != NULL)

/**
 * Returns the current binding or NULL if there is none.
 */
#define RAVE_OBJECT_GETBINDING(this) \
  RaveCoreObject_getBindingData((RaveCoreObject*)this)

/**
 * Checks if this object is of the specified type.
 * For example RAVE_OBJECT_CHECK_TYPE(obj, &PolarScan_TYPE).
 * @param[in] this - the object
 * @param[in] type - the type to be checked against.
 * @returns true if matching, otherwise false.
 */
#define RAVE_OBJECT_CHECK_TYPE(this, type) \
  (((RaveCoreObject*)this)->roh_type == type)


/**
 * Creates a new instance of the provided type.
 * @param[in] type - the object type
 * @param[in] filename - the filename that this allocation was performed in
 * @param[in] lineno - the linenumber that this allocation was performed in
 * @returns a new instance
 */
RaveCoreObject* RaveCoreObject_new(RaveCoreObjectType* type, const char* filename, int lineno);

/**
 * Decrements the reference counter and if reference counts gets to 0, the destructor that
 * was provided when creating the instance will be called. Note, regardless on if and how the
 * destructor has been implemented, the object will be freed.
 * @param[in] obj - the object to release (including the object itself)
 * @param[in] filename - the filename that this operation was performed in
 * @param[in] lineno - the linenumber that this operation was performed in
 */
void RaveCoreObject_release(RaveCoreObject* obj, const char* filename, int lineno);

/**
 * Increments the reference counter and returns a pointer to the provided object.
 * @param[in] src - the object to be copied
 * @param[in] filename - the filename that this operation was performed in
 * @param[in] lineno - the linenumber that this operation was performed in
 * @returns a pointer to the copied object
 */
RaveCoreObject* RaveCoreObject_copy(RaveCoreObject* src, const char* filename, int lineno);

/**
 * Creates a clone of the provided object by using the types copyconstructor (if there is any).
 * @param[in] src - the object to be cloned
 * @param[in] filename - the filename that this operation was performed in
 * @param[in] lineno - the linenumber that this operation was performed in
 * @returns a pointer to the cloned object
 */
RaveCoreObject* RaveCoreObject_clone(RaveCoreObject* src, const char* filename, int lineno);

/**
 * Returns the current reference count.
 * @param[in] src - the the object that should be queried for reference count
 * @returns the reference count.
 */
int RaveCoreObject_getRefCount(RaveCoreObject* src);

/**
 * Sets the binding data field, useful when for example associating this
 * object with another object in order to manage objects from python
 * or other languages. Do not use this function directly but use
 * the macros for binding instead.
 * @param[in] src - this object
 * @param[in] bindingData - the associated object (or similar)
 */
void RaveCoreObject_bind(RaveCoreObject* src, void* bindingData);

/**
 * Removes the binding from the rave object. Do not use this function directly
 * but use the macros for binding instead. If bindingData != the stored binding data nothing will happen.
 * @param[in] src - this object
 * @param[in] bindingData - the binding to remove
 */
void RaveCoreObject_unbind(RaveCoreObject* src, void* bindingData);

/**
 * Returns the extra data. Do not use this function directly but use
 * the macros for binding objects instead.
 * @param[in] src - this object
 * @returns the binding data or NULL if none.
 */
void* RaveCoreObject_getBindingData(RaveCoreObject* src);

/**
 * Returns if this object is possible to clone or not.
 * @param[in] src - the rave core object
 * @returns 1 if the object is possible to clone.
 */
int RaveCoreObject_isCloneable(RaveCoreObject* src);

/**
 * Print current status of object creation.
 */
void RaveCoreObject_printCurrentObjectStatus(void);

/**
 * Prints the rave object statistics.
 */
void RaveCoreObject_printStatistics(void);

#endif /* RAVE_OBJECT_H */
