#include "error.h"
#include "list.h"
#include <stdlib.h>

void list_append(List head, void* data) {
	/* Create a new node. */
	ListNodePtr node, p;
	node = (ListNodePtr)malloc(sizeof(ListNode));
	if (!node)
		error_fatal(ErrorMemoryAlloc);
	node->data = data;
	
	p = head;
	
	if (!p)
		/* Empty list; The head */
		head = node;
	else {
		/* The list is not empty; Find the tail. */	
		while (p->next) 
			p = p->next;
		
		p = node;
	}
}

void list_print(List list, void(*_print)(void*)) {
	ListNodePtr p = list;
	
	/* Iterate the list calling _print for each node. */
	do {
		_print(p->data);
	} while(p = p->next);
}