#include "error.h"
#include "list.h"
#include <stdlib.h>

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

List list_add_ordered(List list, void* data, int(*_cmp)(void*, void*)) {
	ListNodePtr node, p = list, prev = NULL;
	
	node = _list_create_node(data);

	if (!list)
		/* Empty list; Let the new node be its head. */
		list = node;
	else {
		/* The list is not empty; Find the first node which is "larger" then 
		 * the new node according to _cmp. */	
		while (p->next && _cmp(data, p->data) > 0) {
			prev = p;
			p = p->next;
		}
		/* Attach the new node. */
		node->next = p;
		if (!prev)
			/* There's one node and the new node needs to be prepended. */
			list = node;
		else
			prev->next = node;
	}
	
	return list;	
} 

void list_print(List list, void(*_print)(void*)) {
	ListNodePtr p = list;
	
	if (!list)
		return;
	
	/* Iterate the list calling _print for each node. */
	do {		
		_print(p->data);
	} while((p = p->next));
}

void list_destruct(List list) {
	ListNodePtr p = list, tmp;
	
	if (!list)
		return;
	
	/* Iterate the list calling _free_data for each node. */
	do {
		/* Store the next node aside, to be able to free the current node. */
		tmp = p->next;
		/* Free the node's data and then the node itself. */
		free(p->data);
		free(p);
	} while((p = tmp));
}

ListNodePtr _list_create_node(void* data) {
	ListNodePtr node;
	
	node = (ListNodePtr)malloc(sizeof(ListNode));
	if (!node)
		error_fatal(ErrorMemoryAlloc);
	node->data = data;
	return node;
}
