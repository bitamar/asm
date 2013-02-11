/* List node pointer. */
typedef struct _node* ListNodePtr;

/* Alias to list node pointer, for defining lists. */
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
 */
void list_append(List list, void* data);

/**
 * Print a list.
 * 
 * @param list
 *   The list head.
 * @param _print
 *   Callback function for printing a node.
 */
void list_print(List list, void(*_print)(void*));
