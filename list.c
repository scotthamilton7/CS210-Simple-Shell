#include "list.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Node* new_node(char* new_value) {
    int len = strlen(new_value);

    Node* n = malloc(sizeof(Node));
    if (n == NULL) {
        return NULL;
    }

    n->value = malloc(len + 1);
    if (n->value == NULL) {
        free(n);
        return NULL;
    }

    strcpy(n->value, new_value);
    n->next = NULL;
    return n;
}

void delete_node(Node* n) {
    free(n);
}

void push(List list, char* value) {
    // create new node
    Node* n = new_node(value);

    // if list is empty set head to new node
    if (*list == NULL) {
        *list = n;
        return;
    }
    
    // else, add new node to end of list
    Node* current = *list;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = n;
}

List new_list() {
    List list = malloc(sizeof(Node*));
    if (list == NULL) {
        return NULL;
    }
    *list = NULL;
    return list;
}

char* rem(List list) {
    // the list is empty
    if (*list == NULL) {
        return NULL;
    }
    
    Node* head = *list;
    
    // save value to return
    char* value = head->value;
    
    if (head->next == NULL) {
        // if list only has the one item
        *list = NULL;
    } else {
        // list has more than one item, 'head' pointer moved to next item
        *list = head->next;
    }
    
    delete_node(head);
    return value;
}

int is_empty(List list) {
    return *list == NULL;
}

int size(List list) {
    if (is_empty(list)) {
        return 0;
    }

    int count = 0;
    Node* n = *list;
    while (n->next != NULL) {
        count++;
        n = n->next;
    }
    count++;
    return count;
}

void clear(List list) {
    // check if list is empty
    if (is_empty(list)) {
        return;
    }
    
    // remove first element
    rem(list);
    
    // recursive call to clear rest of list
    clear(list);
}

int index_of(List list, char* value) {
    // check if list is empty
    if (is_empty(list)) {
        return -1;
    }
    
    Node* current = *list;
    
    for (int i = 0; i < size(list); i++) {
        if (strcmp(current->value, value) == 0) {
            return i;
        }
        current = current->next;
    }
    
    return -1;
}

int contains(List list, char* value) {
    return index_of(list, value) != -1;
}