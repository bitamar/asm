#include "error.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Create a new node.
 *
 * @param data
 *   Data to be set on the node.
 *
 * @return
 *   Pointer to the node.
 */
ListNodePtr _list_create_node(void* data) {
	ListNodePtr node;

	node = (ListNodePtr)malloc(sizeof(ListNode));
	if (!node)
		error_fatal(ErrorMemoryAlloc);
	node->data = data;
	node->next = NULL;
	return node;
}

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
List list_append(List list, void* data) {
	ListNodePtr node, p;
	
	node = _list_create_node(data);

	p = list;
	if (!list)
		/* Empty list; Let the new node be its head. */
		list = node;
	else {
		/* The list is not empty; Find the tail. */	
		while (p->next)
			p = p->next;
		
		p->next = node;
	}
	
	return list;
}

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
List list_add_ordered(List list, void* data, int(*_compare)(void*, void*), void(*_duplicate)(void*)) {
	ListNodePtr node, p = list, prev = NULL;
	int difference;
	
	node = _list_create_node(data);

	while(p) {
		difference = _compare(data, p->data);
		if(difference < 0)
			break;
		
		if (_duplicate && !difference)
			_duplicate(data);
			
		prev = p;
		p = p->next;
	}
	
	if(!prev) {
		node->next = list;
		list = node;
	}
	else {
		prev->next = node;
		node->next = p;
	}
	
	return list;
} 

/**
 * Iterate a list and perform a call-back function on each node.
 *
 * @param list
 *   The list head.
 * @param _callback
 *   Call-back function to perform on the node's data.
 */
void list_foreach(List list, void(*_callback)(void*)) {
	ListNodePtr p = list;

	if (!list)
		return;

	/* Iterate the list calling _callback for each node. */
	do {
		_callback(p->data);
	} while((p = p->next));
}

/**
 * Free all the nodes and data from a list.
 *
 * @param list
 *   The list head.
 * @param _free_data
 *   Call-back function freeing the data of each node.
 */
void list_destruct(List list, void(*_free_data)(void*)) {
	ListNodePtr p = list, tmp;

	while (p) {
		/* Store the next node aside, to be able to free the current node. */
		tmp = p;
	    p = p->next;

	    /* Free the node's data and then the node itself. */
	    _free_data(tmp->data);
	    tmp->data = NULL;
	    tmp->next = NULL;
	    free(tmp);
	}
	return;
}

/**
 * Find identical item to the given item.
 * @param
 */
void* list_find_item(List list, void* data, int(*_compare)(void*, void*)) {
	ListNodePtr p = list;

	while(p) {
		if (!_compare(data, p->data)) {
			return p->data;
		}

		p = p->next;
	}

	return NULL;
}
