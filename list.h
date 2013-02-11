/* List node pointer. */
typedef struct _node* ListNodePtr;

typedef struct _node {
  void* data;
  ListNodePtr next;
} ListNode;

/* Alias to list node, for defining lists. */
typedef ListNode List;

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
List* list_append(List* list, void* data);

/**
 * Print a list.
 * 
 * @param list
 *   The list head.
 * @param _print
 *   Callback function for printing node's data.
 */
void list_print(List* list, void(*_print)(void*));

/**
 * Free all the nodes and data from a list.
 * 
 * @param list
 *   The list head.
 */
void list_destruct(List* list);