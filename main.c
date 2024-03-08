//#define DEBUG

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "list.h"

typedef struct __node {
    long value;
    struct list_head list;
} node_t;

int list_length(struct list_head *head);
struct list_head *list_new();
void list_free(struct list_head *head);
void list_dump(struct list_head *head);

static bool list_is_ordered(struct list_head *head);
void shuffle(int *array, size_t n);
void quick_sort(struct list_head **list);


int main(int argc, char **argv)
{
    struct list_head *list = list_new();
    size_t count = 100000;
    int *test_arr = malloc(sizeof(int) * count);

    for (int i = 0; i < count; ++i)
        test_arr[i] = i;
    shuffle(test_arr, count);

    while (count--) {
        node_t *entry = malloc(sizeof(node_t));
        entry->value = test_arr[count];
        list_add(&entry->list, list);
    }
    list_dump(list);

    quick_sort(&list);
    assert(list_is_ordered(list));
    list_free(list);
    free(test_arr);
    return 0;
}


int list_length(struct list_head *head)
{
    if (!head)
        return 0;

    int n = 0;
    struct list_head *node;
    list_for_each (node, head)
        ++n;
    return n;
}

struct list_head *list_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

void list_free(struct list_head *head)
{
    if (!head)
        return;

    node_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        list_del(&entry->list);
        free(entry);
    }
    free(head);
    head = NULL;
}

#if defined DEBUG
void list_dump(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    node_t *entry;
    list_for_each_entry (entry, head, list)
        printf("%ld ", entry->value);
    printf("\n");
}
#else
void list_dump(struct list_head *head) {}
#endif

/* Verify if list is order */
static bool list_is_ordered(struct list_head *head)
{       
    bool first = true;
    int value;
    node_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        if (first) {
            value = entry->value;
            first = false;
        } else {
            if (entry->value < value)
                return false;
            value = entry->value;
        }
    }
    return true;
}

/* shuffle array, only work if n < RAND_MAX */
void shuffle(int *array, size_t n)
{
    if (n <= 0)
        return;

    for (size_t i = 0; i < n - 1; i++) {
        size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
        int t = array[j];
        array[j] = array[i];
        array[i] = t;
    }
}


void quick_sort(struct list_head **list)
{
    if (!(*list) || list_empty(*list))
        return;

    int n = list_length(*list);
    int value;
    int i = 0;
    int max_level = 2 * n;
    struct list_head begin[max_level];
    for (int i = 1; i < max_level; ++i)
        INIT_LIST_HEAD(&begin[i]);
    INIT_LIST_HEAD(&begin[0]);
    list_splice_init(*list, &begin[0]);

    struct list_head *result = list_new();
    struct list_head *left = list_new();
    struct list_head *middle = list_new();
    struct list_head *right = list_new();


    while (i >= 0) {
        struct list_head *target_list = &begin[i];
        if (!list_empty(target_list) &&
            !list_is_singular(target_list)) { /* list length >= 2 */
            list_dump(target_list);
            node_t *pivot = list_entry(target_list->next, node_t, list);
            list_del(&pivot->list);
            list_add(&pivot->list, middle);
            value = pivot->value;
            node_t *entry, *safe;
            list_for_each_entry_safe (entry, safe, target_list, list) {
                list_del(&entry->list);
                list_add(&entry->list, entry->value > value ? right : left);
            }

            list_splice_init(left, &begin[i]);
            list_splice_init(middle, &begin[i + 1]);
            list_splice_init(right, &begin[i + 2]);
            list_dump(&begin[i]);
            list_dump(&begin[i + 1]);
            list_dump(&begin[i + 2]);
            i += 2;
            // getc(stdin);
        } else if (list_is_singular(target_list)) { /* list length == 1 */
            list_splice_init(target_list, result);
            i--;
        } else { /* list length == 0 */
            i--;
        }
    }

    list_free(left);
    list_free(middle);
    list_free(right);
    *list = result;
}

