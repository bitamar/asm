/*
 * list.h
 */

#ifndef LIST_H_
#define LIST_H_

#include <stdio.h>

/* List node pointer. */
typedef struct _node* ListNodePtr;

/* Alias to list node, for defining lists. */
typedef ListNodePtr List;

typedef struct _node {
  void* data;
  ListNodePtr next;
} ListNode;

/**
 * Append a node to the tail of a list.
 * 
 * @param list
 *   The list head.
 * @param data
 *   Pointer to the data structure.
 * 
 * @return
 *   Pointer to the head of the list.
 */
List list_append(List list, void* data);

/**
 * Add a node to an ordered list.
 * 
 * @param list
 *   The list head.
 * @param data
 *   Pointer to the data structure.
 * @param _compare
 *   Pointer to a callback function receiving two data structures and returning
 *   an integer specifying which one of them supposed to be first.
 * @param _duplicate
 *   Optional pointer to function that will be triggered if a duplicated
 *   element will be inserted.
 * 
 * @return 
 *   Pointer to the head of the list.
 */
List list_add_ordered(List list, void* data, int(*_compare)(void*, void*), void(*_duplicate)(void*));

/**
 * Print a list.
 * 
 * @param list
 *   The list head.
 * @param stream
 *   File to print to.
 * @param _print
 *   Call-back function for printing node's data.
 */
void list_print(List list, FILE* stream, void(*_print)(void*, FILE*));

/**
 * Iterate a list and perform a call-back function on each node.
 *
 * @param list
 *   The list head.
 * @param _callback
 *   Call-back function to perform on the node's data.
 */
void list_foreach(List list, void(*_callback)(void*));

/**
 * Free all the nodes and data from a list.
 * 
 * @param list
 *   The list head.
 */
void list_destruct(List list);

/**
 * Find identical item to the given item.
 * @param
 */
void* list_find_item(List list, void* item, int(*_compare)(void*, void*));

/**
 * Create a new node.
 * 
 * @param data
 *   Data to be set on the node.
 * 
 * @return 
 *   Pointer to the node.
 */
ListNodePtr _list_create_node(void* data);

#endif
