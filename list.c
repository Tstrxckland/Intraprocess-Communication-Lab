
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

list_t *list_alloc(void) {
    list_t *list = malloc(sizeof(list_t));
    if (list != NULL) {
        list->head = NULL;
    }
    return list;
}

void list_free(list_t *list) {
    if (list == NULL) return;

    node_t *current = list->head;

    while (current != NULL) {
        node_t *next = current->next;
        free(current);
        current = next;
    }

    free(list);
}

int list_length(list_t *list) {
    if (list == NULL) return 0;

    int count = 0;
    node_t *current = list->head;

    while (current != NULL) {
        count++;
        current = current->next;
    }

    return count;
}

void list_add_to_front(list_t *list, int value) {
    if (list == NULL) return;

    node_t *newNode = malloc(sizeof(node_t));
    if (newNode == NULL) return;

    newNode->data = value;
    newNode->next = list->head;
    list->head = newNode;
}

void list_add_to_back(list_t *list, int value) {
    if (list == NULL) return;

    node_t *newNode = malloc(sizeof(node_t));
    if (newNode == NULL) return;

    newNode->data = value;
    newNode->next = NULL;

    if (list->head == NULL) {
        list->head = newNode;
        return;
    }

    node_t *current = list->head;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = newNode;
}

void list_add_at_index(list_t *list, int value, int index) {
    if (list == NULL || index < 0 ||
        index > list_length(list)) return;

    if (index == 0) {
        list_add_to_front(list, value);
        return;
    }

    node_t *current = list->head;

    for (int i = 0; i < index - 1; i++) {
        current = current->next;
    }

    node_t *newNode = malloc(sizeof(node_t));
    if (newNode == NULL) return;

    newNode->data = value;
    newNode->next = current->next;
    current->next = newNode;
}

int list_remove_from_front(list_t *list) {
    if (list == NULL || list->head == NULL) return -1;

    node_t *temp = list->head;
    int value = temp->data;

    list->head = temp->next;
    free(temp);

    return value;
}

int list_remove_from_back(list_t *list) {
    if (list == NULL || list->head == NULL) return -1;

    if (list->head->next == NULL) {
        return list_remove_from_front(list);
    }

    node_t *current = list->head;

    while (current->next->next != NULL) {
        current = current->next;
    }

    int value = current->next->data;
    free(current->next);
    current->next = NULL;

    return value;
}

int list_remove_at_index(list_t *list, int index) {
    if (list == NULL || index < 0 ||
        index >= list_length(list)) return -1;

    if (index == 0) {
        return list_remove_from_front(list);
    }

    node_t *current = list->head;

    for (int i = 0; i < index - 1; i++) {
        current = current->next;
    }

    node_t *temp = current->next;
    int value = temp->data;

    current->next = temp->next;
    free(temp);

    return value;
}

int list_get_elem_at(list_t *list, int index) {
    if (list == NULL || index < 0) return -1;

    node_t *current = list->head;

    for (int i = 0; i < index && current != NULL; i++) {
        current = current->next;
    }

    if (current == NULL) return -1;

    return current->data;
}

char *listToString(list_t *list) {
    static char buffer[1024];
    buffer[0] = '\0';

    if (list == NULL || list->head == NULL) {
        snprintf(buffer, sizeof(buffer), "[]");
        return buffer;
    }

    size_t used = 0;
    int written = snprintf(buffer, sizeof(buffer), "[");

    if (written < 0) return buffer;
    used = (size_t)written;

    node_t *current = list->head;

    while (current != NULL && used < sizeof(buffer)) {
        written = snprintf(
            buffer + used,
            sizeof(buffer) - used,
            "%s%d",
            current == list->head ? "" : ", ",
            current->data
        );

        if (written < 0) break;

        if ((size_t)written >= sizeof(buffer) - used) {
            used = sizeof(buffer) - 1;
            break;
        }

        used += (size_t)written;
        current = current->next;
    }

    if (used < sizeof(buffer) - 1) {
        snprintf(buffer + used, sizeof(buffer) - used, "]");
    }

    return buffer;
}
