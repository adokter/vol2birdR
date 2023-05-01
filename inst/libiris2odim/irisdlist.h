/* --------------------------------------------------------------------
Copyright (C) 2022 HENJAB AB

This file is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This file is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this file.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------*/
/**
 * Description
 * @file
 * @author Anders Henja, HENJAB AB
 * @date 2022-10-12
 */
#ifndef IRISDLIST_H
#define IRISDLIST_H

/**
 * The element building the linked structure
 */
typedef struct _IrisDListElement_t {
  struct _IrisDListElement_t* prev;
  struct _IrisDListElement_t* next;
  void* data;
} IrisDListElement_t;

/**
 * The container keeping track of one list
 */
typedef struct _IrisDList_t {
  IrisDListElement_t* head;
  IrisDListElement_t* tail;
  int size;
} IrisDList_t;

/**
 * Allocates memory for a list element and initialized the values to NULL.
 * @return the allocated memory
 */
IrisDListElement_t* IrisDListElement_create(void);

/**
 * Allocates memory for a list and initializes the values to NULL
 * @return the created list
 */
IrisDList_t* IrisDList_create(void);

/**
 * Adds the data to the front of the double linked list
 * @param[in] list - the list the item should be added to
 * @param[in] data - the data to be added to the element
 * @return the created element
 */
IrisDListElement_t* IrisDList_addFront(IrisDList_t* list, void* data);

/**
 * Adds the data to the end of the double linked list
 * @param[in] list - the list the item should be added to
 * @param[in] data - the data to be added to the element
 * @return the created element
 */

IrisDListElement_t* IrisDList_addEnd(IrisDList_t* list, void* data);

/* Below are some useful macros that makes it somewhat easier to read the code. If you prefer you can always
 * use the members in the structure directly */
#define IrisDList_size(list) ((list)->size)

#define IrisDList_head(list) ((list)->head)

#define IrisDList_tail(list) ((list)->tail)

#define IrisDListElement_prev(element) ((element)->prev)

#define IrisDListElement_next(element) ((element)->next)

#define IrisDListElement_data(element) ((element)->data)

#endif

