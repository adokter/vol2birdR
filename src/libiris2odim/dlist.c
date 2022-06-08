/**
 * Description: This code comes from the book "Mastering Algorithms with C"
 * by Kyle Loudon, published by O'Reilly & Associates.  This code is under 
 * copyright and cannot be included in any other book, publication, or  
 * educational product  without  permission  from  O'Reilly & Associates.  
 * No warranty is attached.
 * These functions are used to store and retrieve data from doubly-linked
 * lists, and they are identical to the code found in the above book, 
 * except for the free() function being replaced by the RAVE_FREE macro.
 * @file dlist.c
 * @author Kyle Loudon
 * @date 1999-08-01
 */
/*****************************************************************************
*                                                                            *
*  ------------------------------- dlist.c --------------------------------  *
*                                                                            *
*****************************************************************************/
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#include <ctype.h>
#include <stdio.h>
#include <math.h>       // M_PI
#include <stddef.h> // size_t, NULL
#include <stdlib.h> // malloc, calloc, exit, free
#include <string.h>
#include <time.h>       // localtime_r
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "dlist.h"
#include "iris2odim.h" // this includes rave_alloc and other rave related


/*****************************************************************************
*                                                                            *
*  ------------------------------ dlist_init ------------------------------  *
*                                                                            *
*****************************************************************************/

void dlist_init(DList *list, void (*destroy)(void *data)) {

/*****************************************************************************
*                                                                            *
*  Initialize the list.                                                      *
*                                                                            *
*****************************************************************************/

list->size = 0;
list->destroy = destroy;
list->head = NULL;
list->tail = NULL;

return;

}

/*****************************************************************************
*                                                                            *
*  ---------------------------- dlist_destroy -----------------------------  *
*                                                                            *
*****************************************************************************/

void dlist_destroy(DList *list) {

void               *data;

/*****************************************************************************
*                                                                            *
*  Remove each element.                                                      *
*                                                                            *
*****************************************************************************/

while (dlist_size(list) > 0) {

   if (dlist_remove(list, dlist_tail(list), (void **)&data) == 0 && list->
      destroy != NULL) {

      /***********************************************************************
      *                                                                      *
      *  Call a user-defined function to free dynamically allocated data.    *
      *                                                                      *
      ***********************************************************************/

      list->destroy(data);

   }

}

/*****************************************************************************
*                                                                            *
*  No operations are allowed now, but clear the structure as a precaution.   *
*                                                                            *
*****************************************************************************/

memset(list, 0, sizeof(DList));

return;

}

/*****************************************************************************
*                                                                            *
*  ---------------------------- dlist_ins_next ----------------------------  *
*                                                                            *
*****************************************************************************/

int dlist_ins_next(DList *list, DListElmt *element, const void *data) {

DListElmt          *new_element;

/*****************************************************************************
*                                                                            *
*  Do not allow a NULL element unless the list is empty.                     *
*                                                                            *
*****************************************************************************/

if (element == NULL && dlist_size(list) != 0)
   return -1;

/*****************************************************************************
*                                                                            *
*  Allocate storage for the element.                                         *
*                                                                            *
*****************************************************************************/

if ((new_element = (DListElmt *) RAVE_MALLOC(sizeof(DListElmt))) == NULL)
   return -1;

/*****************************************************************************
*                                                                            *
*  Insert the new element into the list.                                     *
*                                                                            *
*****************************************************************************/

new_element->data = (void *)data;

if (dlist_size(list) == 0) {

   /**************************************************************************
   *                                                                         *
   *  Handle insertion when the list is empty.                               *
   *                                                                         *
   **************************************************************************/

   list->head = new_element;
   list->head->prev = NULL;
   list->head->next = NULL;
   list->tail = new_element;

   }

else {

   /**************************************************************************
   *                                                                         *
   *  Handle insertion when the list is not empty.                           *
   *                                                                         *
   **************************************************************************/

   new_element->next = element->next;
   new_element->prev = element;

   if (element->next == NULL)
      list->tail = new_element;
   else
      element->next->prev = new_element;

   element->next = new_element;

}

/*****************************************************************************
*                                                                            *
*  Adjust the size of the list to account for the inserted element.          *
*                                                                            *
*****************************************************************************/

list->size++;

return 0;

}

/*****************************************************************************
*                                                                            *
*  ---------------------------- dlist_ins_prev ----------------------------  *
*                                                                            *
*****************************************************************************/


int dlist_ins_prev(DList *list, DListElmt *element, const void *data) {

DListElmt          *new_element;

/*****************************************************************************
*                                                                            *
*  Do not allow a NULL element unless the list is empty.                     *
*                                                                            *
*****************************************************************************/

if (element == NULL && dlist_size(list) != 0)
   return -1;

/*****************************************************************************
*                                                                            *
*  Allocate storage to be managed by the abstract data type.                 *
*                                                                            *
*****************************************************************************/
if ((new_element = (DListElmt *) RAVE_MALLOC(sizeof(DListElmt))) == NULL)
   return -1;

/*****************************************************************************
*                                                                            *
*  Insert the new element into the list.                                     *
*                                                                            *
*****************************************************************************/

new_element->data = (void *)data;

if (dlist_size(list) == 0) {

   /**************************************************************************
   *                                                                         *
   *  Handle insertion when the list is empty.                               *
   *                                                                         *
   **************************************************************************/

   list->head = new_element;
   list->head->prev = NULL;
   list->head->next = NULL;
   list->tail = new_element;

   }


else {

   /**************************************************************************
   *                                                                         *
   *  Handle insertion when the list is not empty.                           *
   *                                                                         *
   **************************************************************************/

   new_element->next = element; 
   new_element->prev = element->prev;

   if (element->prev == NULL)
      list->head = new_element;
   else
      element->prev->next = new_element;

   element->prev = new_element;

}


/*****************************************************************************
*                                                                            *
*  Adjust the size of the list to account for the new element.               *
*                                                                            *
*****************************************************************************/

list->size++;

return 0;

}

/*****************************************************************************
*                                                                            *
*  ----------------------------- dlist_remove -----------------------------  *
*                                                                            *
*****************************************************************************/

int dlist_remove(DList *list, DListElmt *element, void **data) {

/*****************************************************************************
*                                                                            *
*  Do not allow a NULL element or removal from an empty list.                *
*                                                                            *
*****************************************************************************/

if (element == NULL || dlist_size(list) == 0)
   return -1;

/*****************************************************************************
*                                                                            *
*  Remove the element from the list.                                         *
*                                                                            *
*****************************************************************************/

*data = element->data;

if (element == list->head) {

   /**************************************************************************
   *                                                                         *
   *  Handle removal from the head of the list.                              *
   *                                                                         *
   **************************************************************************/

   list->head = element->next;

   if (list->head == NULL)
      list->tail = NULL;
   else
      element->next->prev = NULL;

   }

else {

   /**************************************************************************
   *                                                                         *
   *  Handle removal from other than the head of the list.                   *
   *                                                                         *
   **************************************************************************/

   element->prev->next = element->next;

   if (element->next == NULL)
      list->tail = element->prev;
   else
      element->next->prev = element->prev;

}

/*****************************************************************************
*                                                                            *
*  Free the storage allocated by the abstract data type.                     *
*                                                                            *
*****************************************************************************/

RAVE_FREE(element);

/*****************************************************************************
*                                                                            *
*  Adjust the size of the list to account for the removed element.           *
*                                                                            *
*****************************************************************************/

list->size--;

return 0;

}
