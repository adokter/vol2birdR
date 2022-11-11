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
#include "irisdlist.h"
#include "rave_alloc.h"
#include "rave_debug.h"
#include <stdlib.h>
#include <stdio.h>

IrisDListElement_t* IrisDListElement_create(void)
{
  IrisDListElement_t* result = RAVE_MALLOC(sizeof(IrisDListElement_t));
  result->next = NULL;
  result->prev = NULL;
  result->data = NULL;
  return result;
}

IrisDList_t* IrisDList_create(void)
{
  IrisDList_t* result = RAVE_MALLOC(sizeof(IrisDList_t));
  result->head=NULL;
  result->tail=NULL;
  result->size=0;
  return result;
}

IrisDListElement_t* IrisDList_addFront(IrisDList_t* list, void* data)
{
  IrisDListElement_t* el = NULL;
  RAVE_ASSERT((list != NULL), "list == NULL");
  el = IrisDListElement_create();
  if (el != NULL) {
    el->data = data;
    if (list->head == NULL) {
      list->head = el;
      list->tail = el;
    } else {
      list->head->prev = el;
      el->next = list->head;
      list->head = el;
    }
    list->size++;
  } else {
    RAVE_ERROR0("Failed to allocate memory for list element");
  }
  return el;
}

IrisDListElement_t* IrisDList_addEnd(IrisDList_t* list, void* data)
{
  IrisDListElement_t* el = NULL;
  RAVE_ASSERT((list != NULL), "list == NULL");
  el = IrisDListElement_create();
  if (el != NULL) {
    el->data = data;
    if (list->head == NULL) {
      list->head = el;
      list->tail = el;
    } else {
      list->tail->next = el;
      el->prev = list->tail;
      list->tail = el;
    }
    list->size++;
  } else {
    RAVE_ERROR0("Failed to allocate memory for list element");
  }
  return el;
}
