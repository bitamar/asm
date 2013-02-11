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
 * @param _cmp
 *   Pointer to a callback function receiving two data structures and returning
 *   an integer specifying which one of them supposed to be first.
 * 
 * @return 
 *   Pointer to the head of the list.
 */
List list_add_ordered(List list, void* data, int(*_cmp)(void*, void*));

/**
 * Print a list.
 * 
 * @param list
 *   The list head.
 * @param _print
 *   Callback function for printing node's data.
 */
void list_print(List list, void(*_print)(void*));

/**
 * Free all the nodes and data from a list.
 * 
 * @param list
 *   The list head.
 */
void list_destruct(List list);

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