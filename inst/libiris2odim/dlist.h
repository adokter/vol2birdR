/**
 * Description: The code comes from the  book  "Mastering Algorithms with C"  
 * by Kyle Loudon,  published by O'Reilly & Associates.  This code is under 
 * copyright and cannot be included in any other book, publication, or  
 * educational product  without  permission  from  O'Reilly & Associates.  
 * No warranty is attached.
 * These structures,  macros, and interfaces are used to store and retrieve 
 * data from doubly-linked lists, and they are identical to the code found 
 * in the above book. Part of program iris2odim.
 * @file dlist.h
 * @author Kyle Loudon
 * @date 1999-08-01
 */
/*****************************************************************************
*                                                                            *
*  ------------------------------- dlist.h --------------------------------  *
*                                                                            *
*****************************************************************************/

#ifndef DLIST_H
#define DLIST_H
/*****************************************************************************
*                                                                            *
*  Define a structure for doubly-linked list elements.                       *
*                                                                            *
*****************************************************************************/

typedef struct DListElmt_ {

void               *data;
struct DListElmt_  *prev;
struct DListElmt_  *next;

} DListElmt;

/*****************************************************************************
*                                                                            *
*  Define a structure for doubly-linked lists.                               *
*                                                                            *
*****************************************************************************/

typedef struct DList_ {

int                size;

int                (*match)(const void *key1, const void *key2);
void               (*destroy)(void *data);

DListElmt          *head;
DListElmt          *tail;

} DList;

/*****************************************************************************
*                                                                            *
*  --------------------------- Public Interface ---------------------------  *
*                                                                            *
*****************************************************************************/

void dlist_init(DList *list, void (*destroy)(void *data));

void dlist_destroy(DList *list);

int dlist_ins_next(DList *list, DListElmt *element, const void *data);

int dlist_ins_prev(DList *list, DListElmt *element, const void *data);

int dlist_remove(DList *list, DListElmt *element, void **data);

#define dlist_size(list) ((list)->size)

#define dlist_head(list) ((list)->head)

#define dlist_tail(list) ((list)->tail)

#define dlist_is_head(element) ((element)->prev == NULL ? 1 : 0)

#define dlist_is_tail(element) ((element)->next == NULL ? 1 : 0)

#define dlist_data(element) ((element)->data)

#define dlist_next(element) ((element)->next)

#define dlist_prev(element) ((element)->prev)

#endif
