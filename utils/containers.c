#include "utils/containers.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

array_t* Array_New(size_t len, size_t type_size) {
    array_t* array = calloc(1, sizeof(array_t) + type_size * len);
    array->len     = len;
    return array;
}

array_t* Array_Resize(array_t* arr, size_t len, size_t type_size) {
    array_t* arrNew = Array_New(len, type_size);
    memcpy(arrNew->vals, arr->vals, len * type_size);
    free(arr);
    return arrNew;
}

void Array_Delete(array_t* arr) {
    free(arr);
    return;
}

// Allocates a new linkedlist node
// Returns: the newly allocated node or NULL
linkedlist_t* LinkedList_New(void) {
    linkedlist_t* node = calloc(1, sizeof(linkedlist_t));
    return node;
}

// Takes a node (node_new) and inserts it after another (node_list)
linkedlist_t* LinkedList_InsertNodeAfter(linkedlist_t* list_node, linkedlist_t* node_new) {
    LinkedList_RemoveNode(node_new);

    if (list_node->next != NULL) {
        list_node->next->prev = node_new;
    }

    node_new->next = list_node->next;
    node_new->prev = list_node;

    list_node->next = node_new;

    return node_new;
}

// Takes a list (list_new) and inserts the entire list after a node (list_node)
// WARNING: Attempting to insert a cyclical list will result in an infinite loop
linkedlist_t* LinkedList_InsertListAfter(linkedlist_t* list_node, linkedlist_t* list_new) {
    linkedlist_t* prev = list_node;
    linkedlist_t* next = list_node->next;

    linkedlist_t* head = LinkedList_HeadOf(list_new);
    linkedlist_t* tail = LinkedList_TailOf(list_new);

    if (prev != NULL) {
        prev->next = head;
    }

    head->prev = prev;
    tail->next = next;

    if (next != NULL) {
        next->prev = tail;
    }

    return list_new;
}

// Appends a node (node_toappend) to the end of a list (list)
// WARNING: Attempting to append into a cyclical list will result in an infinite loop
linkedlist_t* LinkedList_AppendNode(linkedlist_t* list, linkedlist_t* node_toappend) {
    return LinkedList_InsertNodeAfter(list, node_toappend);
}

// Appends a list (list_toappend) to the end of a list (list)
linkedlist_t* LinkedList_AppendList(linkedlist_t* list, linkedlist_t* list_toappend) {
    return LinkedList_InsertListAfter(LinkedList_TailOf(list), list_toappend);
}

linkedlist_t* LinkedList_PrependNode(linkedlist_t* list, linkedlist_t* node_toprepend) {
    LinkedList_RemoveNode(node_toprepend);
    LinkedList_AppendList(node_toprepend, list);
    return node_toprepend;
}

linkedlist_t* LinkedList_PrependList(linkedlist_t* list, linkedlist_t* list_toprepend) {
    LinkedList_AppendList(list_toprepend, list);
    return list_toprepend;
}

linkedlist_t* LinkedList_RemoveNode(linkedlist_t* node) {
    linkedlist_t* next = node->next;
    linkedlist_t* prev = node->prev;
    linkedlist_t* list = NULL;

    if (next != NULL) {
        next->prev = prev;
        list       = next;
    }

    if (prev != NULL) {
        prev->next = next;
        list       = prev;
    }

    node->next = NULL;
    node->prev = NULL;

    return list;
}

linkedlist_t* LinkedList_DeleteNode(linkedlist_t* node) {
    linkedlist_t* list = LinkedList_RemoveNode(node);
    free(node);
    return list;
}

void LinkedList_DeleteList(linkedlist_t* list) {
    // break the list, could be cyclic which would cause inf loop
    linkedlist_t* node_prev = list->prev;

    if (node_prev != NULL) {
        node_prev->next = NULL;
        list->prev      = NULL;

        // then delete the previous portion of the list
        linkedlist_t *prev, *sel;
        sel  = node_prev;
        prev = node_prev->prev;

        while (sel != NULL) {
            free(sel);
            sel = prev;
            if (prev != NULL) {
                prev = prev->prev;
            } else {
                prev = NULL;
            }
        }
    }

    // delete the node and everything after
    linkedlist_t *next, *sel;
    sel  = list;
    next = list->next;

    while (sel != NULL) {
        free(sel);
        sel = next;
        if (next != NULL) {
            next = next->next;
        } else {
            next = NULL;
        }
    }

    return;
}

// Moves a node in a list to the first element of the list
// Returns: the node passed in
// WARNING: Calling this on a node in a cyclically linked list will cause an infinite loop
linkedlist_t* LinkedList_MoveToHead(linkedlist_t* node) {
    linkedlist_t* head = LinkedList_HeadOf(node);
    if (head == node) {
        return node;
    }

    linkedlist_t* list = LinkedList_RemoveNode(node);
    return LinkedList_AppendList(node, list);
}

// Moves a node in a list to the last element of the list
// Returns: the node passed in
// WARNING: Calling this on a node in a cyclically linked list will cause an infinite loop
linkedlist_t* LinkedList_MoveToTail(linkedlist_t* node) {
    linkedlist_t* tail = LinkedList_TailOf(node);
    if (tail == node) {
        return node;
    }

    linkedlist_t* list = LinkedList_RemoveNode(node);
    return LinkedList_AppendNode(list, node);
}

// Gets the head node of a list
// Returns: the head node
// WARNING: Calling this on a node in a cyclically linked list will cause an infinite loop
linkedlist_t* LinkedList_HeadOf(linkedlist_t* list) {
    linkedlist_t *sel, *prev;
    prev = list->prev;
    sel  = list;

    while (prev != NULL) {
        sel  = prev;
        prev = prev->prev;
    }

    return sel;
}

// Gets the tail node of a list
// Returns: the tail node
// WARNING: Calling this on a node in a cyclically linked list will cause an infinite loop
linkedlist_t* LinkedList_TailOf(linkedlist_t* list) {
    linkedlist_t *sel, *next;
    next = list->next;
    sel  = list;

    while (next != NULL) {
        sel  = next;
        next = next->next;
    }

    return sel;
}
