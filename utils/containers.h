#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    size_t len;
    void* vals[];
} array_t;

typedef struct LINKEDLIST_T{
    void* val;
    struct LINKEDLIST_T* next;
    struct LINKEDLIST_T* prev;
} linkedlist_t;

array_t* Array_New(size_t len, size_t type_size);

array_t* Array_Resize(array_t* list, size_t len, size_t type_size);

void Array_Delete(array_t* list);



// Allocates a new linkedlist node
// Returns: the newly allocated node or NULL
linkedlist_t* LinkedList_New(void);

// Takes a node (node_new) and inserts it after another (node_list)
linkedlist_t* LinkedList_InsertNodeAfter(linkedlist_t* list_node, linkedlist_t* node_new);

// Takes a list (list_new) and inserts the entire list after a node (list_node)
// WARNING: Attempting to insert a cyclical list will result in an infinite loop
linkedlist_t* LinkedList_InsertListAfter(linkedlist_t* list_node, linkedlist_t* list_new);

// Appends a node (node_toappend) to the end of a list (list)
// WARNING: Attempting to append into a cyclical list will result in an infinite loop
linkedlist_t* LinkedList_AppendNode(linkedlist_t* list, linkedlist_t* node_toappend);

// Appends a list (list_toappend) to the end of a list (list)
linkedlist_t* LinkedList_AppendList(linkedlist_t* list, linkedlist_t* list_toappend);

linkedlist_t* LinkedList_PrependNode(linkedlist_t* list, linkedlist_t* node_toprepend);

linkedlist_t* LinkedList_PrependList(linkedlist_t* list, linkedlist_t* list_toprepend);

linkedlist_t* LinkedList_RemoveNode(linkedlist_t* node);

linkedlist_t* LinkedList_DeleteNode(linkedlist_t* node);

void LinkedList_DeleteList(linkedlist_t* list);

// Moves a node in a list to the first element of the list
// Returns: the node passed in
// WARNING: Calling this on a node in a cyclically linked list will cause an infinite loop
linkedlist_t* LinkedList_MoveToHead(linkedlist_t* node);

// Moves a node in a list to the last element of the list
// Returns: the node passed in
// WARNING: Calling this on a node in a cyclically linked list will cause an infinite loop
linkedlist_t* LinkedList_MoveToTail(linkedlist_t* node);

// Gets the head node of a list
// Returns: the head node
// WARNING: Calling this on a node in a cyclically linked list will cause an infinite loop
linkedlist_t* LinkedList_HeadOf(linkedlist_t* list);

// Gets the tail node of a list
// Returns: the tail node
// WARNING: Calling this on a node in a cyclically linked list will cause an infinite loop
linkedlist_t* LinkedList_TailOf(linkedlist_t* list);


#endif
