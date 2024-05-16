#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Self-defined functions for q_sort
 */
struct list_head *mergesort(struct list_head *, bool);
struct list_head *merge2SortedLists(struct list_head *,
                                    struct list_head *,
                                    bool);


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head) * 1);
    if (!head) {
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

    list_for_each_safe (iter, safe, head) {
        list_del_init(iter);
        element_t *e = list_entry(iter, element_t, list);
        q_release_element(e);
        // free(iter);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, const char *s)
{
    if (!head)
        return false;
    element_t *new_e = (element_t *) malloc(sizeof(element_t) * 1);
    char *sdup = strdup(s);
    if (!new_e || !sdup) {
        free(new_e);
        free(sdup);
        return false;
    }
    new_e->value = sdup;
    list_add(&new_e->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, const char *s)
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
    list_del(head->next);

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
    // free(list_entry(next, element_t, list));
    q_release_element(list_entry(next, element_t, list));
    // free(next);

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
                q_release_element(remv_ele);
                check = true;
            } else {
                // remove = remove->next;
                break;
            }
        }
        if (check) {
            list_del(current);
            current = current->next;
            q_release_element(cur_ele);
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

    // int size = q_size(head);
    struct list_head *node, *safe;

    // for (int i = 0; i < size - 1; i++) {
    //     struct list_head *node = prev;
    //     prev = node->prev;

    //     list_del(node);
    //     list_add(node, cur);

    //     cur = cur->next;
    // }
    list_for_each_safe (node, safe, head) {
        list_move(node, head);
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

            list_move(node, cur);

            cur = cur->next;
        }

        size -= k;
        cur = new_head->prev;
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    // NULL, empty or only one element exists->do nothing.
    if (!head || (head->next == head->prev))
        return;

    head->prev->next = NULL;

    head->next = mergesort(head->next, descend);

    struct list_head *cur = head, *next = head->next;
    while (next) {
        next->prev = cur;
        cur = next;
        next = next->next;
    }
    cur->next = head;
    head->prev = cur;
}

struct list_head *mergesort(struct list_head *head, bool descend)
{
    if (!head || !head->next)
        return head;

    struct list_head *fast = head, *slow = head;

    while (fast && fast->next) {
        fast = fast->next->next;
        slow = slow->next;
    }
    // struct list_head* mid = slow;m
    slow->prev->next = NULL;

    struct list_head *left = mergesort(head, descend);
    struct list_head *right = mergesort(slow, descend);

    return merge2SortedLists(left, right, descend);
    // return left;
}

struct list_head *merge2SortedLists(struct list_head *left,
                                    struct list_head *right,
                                    bool descend)
{
    struct list_head *head = NULL;
    struct list_head **indir = &head;
    int cmpr = descend ? -1 : 1;

    for (struct list_head **node = NULL; left && right;
         (*node) = (*node)->next) {
        const element_t *l = list_entry(left, element_t, list);
        const element_t *r = list_entry(right, element_t, list);
        node = (cmpr * strcmp(l->value, r->value)) <= 0 ? &left : &right;


        *indir = *node;
        indir = &(*indir)->next;
    }

    *indir = (struct list_head *) ((u_int64_t) left | (u_int64_t) right);

    return head;
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/

    if (!head)
        return -1;
    if (head->next == head)
        return 0;
    if (head->next == head->prev)
        return 1;
    element_t *standard, *compare, *safe;

    for (standard = list_entry(head->prev, element_t, list),
        compare = list_entry(standard->list.prev, element_t, list),
        safe = list_entry(compare->list.prev, element_t, list);
         &compare->list != head;
         compare = safe, safe = list_entry(safe->list.prev, element_t, list)) {
        if (strcmp(compare->value, standard->value) > 0) {
            list_del(&compare->list);
            q_release_element(compare);
        } else {
            standard = compare;
        }
    }

    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head)
        return -1;
    if (head->next == head)
        return 0;
    if (head->next == head->prev)
        return 1;

    element_t *standard, *compare, *safe;

    for (standard = list_entry(head->prev, element_t, list),
        compare = list_entry(standard->list.prev, element_t, list),
        safe = list_entry(compare->list.prev, element_t, list);
         &compare->list != head;
         compare = safe, safe = list_entry(safe->list.prev, element_t, list)) {
        if (strcmp(compare->value, standard->value) < 0) {
            list_del(&compare->list);
            q_release_element(compare);
        } else {
            standard = compare;
        }
    }

    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/

    if (!head)
        return 0;
    if (head->next == head)
        return 0;
    if (head->next == head->prev)
        return q_size(head->next);

    int cmpr = descend ? -1 : 1;

    struct list_head *first = head->next;
    queue_contex_t *first_qctx = list_entry(first, queue_contex_t, chain);
    for (struct list_head *next = first->next; next != head;
         next = next->next) {
        queue_contex_t *next_qctx = list_entry(next, queue_contex_t, chain);

        struct list_head *cur = first_qctx->q->next;
        //  *merged = next_qctx->q->next;

        while (cur != first_qctx->q && !list_empty(next_qctx->q)) {
            const element_t *ele = list_entry(cur, element_t, list);
            element_t *m_ele = list_first_entry(next_qctx->q, element_t, list);

            if ((cmpr * strcmp(m_ele->value, ele->value)) <= 0) {
                list_move(&m_ele->list, cur->prev);
                cur = cur->prev;
            } else {
                cur = cur->next;
            }
        }

        list_splice_tail_init(next_qctx->q, first_qctx->q);
    }


    return q_size(list_entry(head->next, queue_contex_t, chain)->q);
}
