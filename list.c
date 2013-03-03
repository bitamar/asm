#include "error.h"
#include "list.h"
#include <stdio.h>
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

void list_foreach(List list, void(*_callback)(void*)) {
	ListNodePtr p = list;

	if (!list)
		return;

	/* Iterate the list calling _callback for each node. */
	do {
		_callback(p->data);
	} while((p = p->next));
}

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

ListNodePtr _list_create_node(void* data) {
	ListNodePtr node;
	
	node = (ListNodePtr)malloc(sizeof(ListNode));
	if (!node)
		error_fatal(ErrorMemoryAlloc);
	node->data = data;
	node->next = NULL;
	return node;
}
