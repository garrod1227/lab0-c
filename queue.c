#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *q = malloc(sizeof(struct list_head));
    if (q)
        INIT_LIST_HEAD(q);
    return q;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    element_t *e;

    while (l && !list_empty(l)) {
        e = container_of(l->next, element_t, list);
        list_del(&e->list);
        q_release_element(e);
    }
    if (l)
        free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    element_t *e;

    if (!head)
        return false;
    e = malloc(sizeof(element_t));
    if (!e)
        goto err_ele;
    e->value = strdup(s);
    if (!e->value)
        goto err_str;
    list_add(&e->list, head);
    return true;
err_str:
    if (e->value)
        free(e->value);
err_ele:
    if (e)
        free(e);
    return false;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    element_t *e;

    if (!head)
        return false;
    e = malloc(sizeof(element_t));
    if (!e)
        goto err_ele;
    e->value = strdup(s);
    if (!e->value)
        goto err_str;
    list_add_tail(&e->list, head);
    return true;
err_str:
    if (e->value)
        free(e->value);
err_ele:
    if (e)
        free(e);
    return false;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *e = container_of(head->next, element_t, list);
    if (sp) {
        strncpy(sp, e->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(head->next);
    return e;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *e = container_of(head->prev, element_t, list);
    if (sp) {
        strncpy(sp, e->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(head->prev);
    return e;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    struct list_head *n;
    int l = 0;

    if (!head || list_empty(head))
        return 0;
    list_for_each (n, head)
        l++;
    return l;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    struct list_head *f, *b;

    if (!head || list_empty(head))
        return false;

    for (f = (head)->next, b = (head)->prev;; f = f->next, b = b->prev) {
        if (f == b || f == b->prev)
            break;
    }

    list_del(f);
    q_release_element(container_of(f, element_t, list));
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    struct list_head *l;
    element_t *b, *c;
    bool del_cur = false;

    if (!head)
        return false;

    l = head->next;
    while (l->next != head) {
        b = list_entry(l, element_t, list);
        c = list_entry(l->next, element_t, list);
        if (strcmp(b->value, c->value) == 0) {
            del_cur = true;
            list_del(&c->list);
            q_release_element(c);
        } else {
            l = l->next;
            if (del_cur) {
                del_cur = false;
                list_del(&b->list);
                q_release_element(b);
            }
        }
    }
    if (del_cur) {
        list_del(&b->list);
        q_release_element(b);
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    struct list_head *n;

    if (!head || list_empty(head))
        return;

    list_for_each (n, head) {
        if (n->next == head)
            break;
        struct list_head *fwn = n->next;
        list_del(n);
        list_add(n, fwn);
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *n, *t;

    n = head;
    do {
        t = n->next;
        n->next = n->prev;
        n->prev = t;
        n = t;
    } while (n != head);
}

struct list_head *mergeTwoLists(struct list_head *L1, struct list_head *L2)
{
    struct list_head *head = NULL, **ptr = &head;

    for (; L1 && L2; ptr = &(*ptr)->next) {
        if (strcmp(list_entry(L1, element_t, list)->value,
                   list_entry(L2, element_t, list)->value) <= 0) {
            *ptr = L1;
            L1 = L1->next;
        } else {
            *ptr = L2;
            L2 = L2->next;
        }
    }
    *ptr = (struct list_head *) ((uintptr_t) L1 | (uintptr_t) L2);
    return head;
}

void list_mergesort(struct list_head *head)
{
    if (list_empty(head) || list_is_singular(head))
        return;

    int top = 0;
    int listsSize = 0;
    struct list_head *stack[1000] = {NULL};
    struct list_head *lists[100000] = {NULL};

    head->prev->next = NULL;
    stack[top] = head->next;

    while (top >= 0) {
        struct list_head *left = stack[top--];
        //        bool equalSeq = false;

        if (left) {
            struct list_head *slow = left;
            struct list_head *fast;

            for (fast = left->next; fast && fast->next;
                 fast = fast->next->next) {
#if 0
                if (slow->next &&
                    strcmp(list_entry(slow, element_t, list)->value,
                           list_entry(slow->next, element_t, list)->value) ==
                        0) {
                    while (
                        slow->next &&
                        strcmp(
                            list_entry(slow, element_t, list)->value,
                            list_entry(slow->next, element_t, list)->value) ==
                            0) {
                        slow = slow->next;
                        if (fast == slow)
                            slowahead = true;
                    }
                    if (slowahead)
                        fast = slow->next;
                } else {
                    slow = slow->next;
                }
                if (!slow->next)
                    break;
#else
                if (strcmp(list_entry(slow, element_t, list)->value,
                           list_entry(slow->next, element_t, list)->value) ==
                    0) {
                    while (
                        slow->next &&
                        strcmp(
                            list_entry(slow, element_t, list)->value,
                            list_entry(slow->next, element_t, list)->value) ==
                            0) {
                        slow = slow->next;
                    }
                    break;
                }
                slow = slow->next;
            }
#endif
                struct list_head *right = slow->next;
                slow->next = NULL;

                stack[++top] = left;
                stack[++top] = right;
            }
            else lists[listsSize++] = stack[top--];
        }

        while (listsSize > 1) {
            for (int i = 0, j = listsSize - 1; i < j; i++, j--)
                lists[i] = mergeTwoLists(lists[i], lists[j]);
            listsSize = (listsSize + 1) / 2;
        }

        head->next = lists[0];

        struct list_head *i = head;
        while (i->next) {
            i->next->prev = i;
            i = i->next;
        }
        head->prev = i;
        i->next = head;
    }

    void list_qsort(struct list_head * head)
    {
        struct list_head list_less, list_greater, list_equal;
        element_t *pivot;
        element_t *item = NULL, *is = NULL;

        if (list_empty(head) || list_is_singular(head))
            return;

        INIT_LIST_HEAD(&list_less);
        INIT_LIST_HEAD(&list_equal);
        INIT_LIST_HEAD(&list_greater);

        pivot = list_first_entry(head, element_t, list);
        list_del_init(&pivot->list);

        list_for_each_entry_safe (item, is, head, list) {
            if (strcmp(item->value, pivot->value) == 0)
                list_move_tail(&item->list, &list_equal);
            else if (strcmp(item->value, pivot->value) < 0)
                list_move_tail(&item->list, &list_less);
            else
                list_move(&item->list, &list_greater);
        }

        list_qsort(&list_less);
        list_qsort(&list_greater);

        list_add(&pivot->list, head);
        list_splice(&list_equal, head);
        list_splice(&list_less, head);
        list_splice_tail(&list_greater, head);
    }

    /*
     * Sort elements of queue in ascending order
     * No effect if q is NULL or empty. In addition, if q has only one
     * element, do nothing.
     */
    void q_sort(struct list_head * head)
    {
        if (!head || list_empty(head))
            return;
        // list_qsort(head);
        list_mergesort(head);
    }
