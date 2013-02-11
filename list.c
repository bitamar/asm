#include "error.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>

List* list_append(List* list, void* data) {
	/* Create a new node. */
	ListNodePtr node, p;
	node = (ListNodePtr)malloc(sizeof(ListNode));
	if (!node)
		error_fatal(ErrorMemoryAlloc);
	node->data = data;

	p = list;
	
	if (!p)
		/* Empty list; The head */
		list = node;
	else {
		/* The list is not empty; Find the tail. */	
		while (p->next) 
			p = p->next;
		
		p = node;
	}
	
	return list;
}

void list_print(List* list, void(*_print)(void*)) {
	ListNodePtr p = list;
	if (!p)
		return;
	
	/* Iterate the list calling _print for each node. */
	do {		
		_print(p->data);
	} while(p = p->next);
}