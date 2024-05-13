#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */



/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head =
        (struct list_head *) malloc(sizeof(struct list_head) * 1);
    if (!head) {
        free(head);
        return NULL;
    }
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    // Check if head point to NULL
    if (!head)
        return;

    // Check if head point to an empty list
    // Actually, this step is not necessary because the iteration below will do
    // nothing if the list is empty.
    if (list_empty(head)) {
        free(head);  // Even list_empty return true, there is still a dummpy
                     // list_head.
        return;
    }

    struct list_head *iter = NULL, *safe = NULL;

    list_for_each_safe (iter, safe, head)
        q_release_element(list_entry(iter, element_t, list));

    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_e = malloc(sizeof(element_t));
    if (!new_e) {
        free(new_e);
        return false;
    }
    INIT_LIST_HEAD(&new_e->list);

    /*
    The error msg implies there are some problemes about the lifetime of pointer
    to char s. If I just assign the value of s to new_e->value, new_e->value
    will point to a memory location which will be free after q_insert_head
    function.
    */
    // new_e->value = s; // Error msg: Need to allocate and copy string for new

    /*
    Copy s and assign to new_e->value
    */
    size_t len = 0;
    while (s[len])
        len++;
    new_e->value = malloc(sizeof(*s) * (len + 1));
    char *iter = new_e->value;
    while (*s) {
        *iter++ = *s++;
    }
    *iter = '\0';  // Add string end


    list_add(&new_e->list, head);


    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    return q_insert_head(head->prev, s);
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *ele = list_entry(head->next, element_t, list);
    list_del_init(head->next);

    if (sp) {
        strncpy(sp, ele->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return ele;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    return q_remove_head(head->prev->prev, sp, bufsize);
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head) {
        return -1;
    }
    int size = 0;

    struct list_head *iter = NULL;
    list_for_each (iter, head)
        size++;
    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/

    if (!head || list_empty(head))
        return false;

    struct list_head *next = head->next, *prev = head->prev;

    while (next != prev && next->next != prev) {
        next = next->next;
        prev = prev->prev;
    }
    // delete next

    list_del(next);
    free(list_entry(next, element_t, list));

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/

    if (!head || list_empty(head))
        return false;

    struct list_head *current = head->next, *remove;

    while (current != head) {
        element_t *cur_ele = list_entry(current, element_t, list);

        remove = current->next;
        bool check = false;
        while (remove != head) {
            element_t *remv_ele = list_entry(remove, element_t, list);

            if (!strcmp(remv_ele->value, cur_ele->value)) {
                // element_t *node = remove;
                list_del(remove);
                remove = remove->next;
                free(remv_ele);
                check = true;
            } else {
                // remove = remove->next;
                break;
            }
        }
        if (check) {
            list_del(current);
            current = current->next;
            free(cur_ele);
        } else {
            current = current->next;
        }
    }


    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/

    if (!head || list_empty(head))
        return;

    struct list_head **indir = &head->next;

    while (*indir != head && (*indir)->next != head) {
        struct list_head *node = *indir;
        node->next->prev = node->prev;
        node->prev->next = node->next;

        (*indir)->next->prev = node;
        node->next = (*indir)->next;
        node->prev = (*indir);
        (*indir)->next = node;

        indir = &(*indir)->next->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    int size = q_size(head);
    struct list_head *prev = head->prev;
    struct list_head *cur = head;

    for (int i = 0; i < size - 1; i++) {
        struct list_head *node = prev;
        prev = node->prev;

        list_del(node);
        list_add(node, cur);

        cur = cur->next;
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/

    if (!head || list_empty(head))
        return;

    int size = q_size(head);
    struct list_head *cur = head, *new_head;

    while (size >= k) {
        struct list_head *tail = cur;
        for (int i = 0; i < k; i++)
            tail = tail->next;
        new_head = tail->next;  // Memorize the begining node of next iteration.

        for (int i = 0; i < k - 1; i++) {
            struct list_head *node = tail;
            tail = tail->prev;

            list_del(node);
            list_add(node, cur);

            cur = cur->next;
        }

        size -= k;
        cur = new_head->prev;
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend) {}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
