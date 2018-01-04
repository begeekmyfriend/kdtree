/*
 * Copyright (C) 2017, Leo Ma <begeekmyfriend@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>

#include "kdtree.h"

static inline int is_leaf(struct kdnode *node)
{
        return node->left == node->right;
}

static inline void swap(long *a, long *b)
{
        long tmp = *a;
        *a = *b;
        *b = tmp;
}

static inline double square(double d)
{
        return d * d;
}

static inline double distance(double *c1, double *c2, int dim)
{
        double distance = 0;
        while (dim-- > 0) {
                distance += square(*c1++ - *c2++);
        }
        return distance;
}

static inline double knn_max(struct kdtree *tree)
{
        return tree->knn_list_head.prev->distance;
}

static inline double D(struct kdtree *tree, long index, int r)
{
        return tree->coord_table[index][r];
}

static inline int kdnode_passed(struct kdtree *tree, struct kdnode *node)
{
        return node != NULL ? tree->coord_passed[node->coord_index] : 1;
}

static inline int knn_search_on(struct kdtree *tree, int k, double value, double target)
{
        return tree->knn_num < k || square(target - value) < knn_max(tree);
}

static inline void coord_index_reset(struct kdtree *tree)
{
        long i;
        for (i = 0; i < tree->capacity; i++) {
                tree->coord_indexes[i] = i;
        }
}

static inline void coord_table_reset(struct kdtree *tree)
{
        long i;
        for (i = 0; i < tree->capacity; i++) {
                tree->coord_table[i] = tree->coords + i * tree->dim;
        }
}

static inline void coord_deleted_reset(struct kdtree *tree)
{
        memset(tree->coord_deleted, 0, tree->capacity);
}

static inline void coord_passed_reset(struct kdtree *tree)
{
        memset(tree->coord_passed, 0, tree->capacity);
}

static void coord_dump_all(struct kdtree *tree)
{
        long i, j;
        for (i = 0; i < tree->count; i++) {
                long index = tree->coord_indexes[i];
                double *coord = tree->coord_table[index];
                printf("(");
                for (j = 0; j < tree->dim; j++) {
                        if (j != tree->dim - 1) {
                                printf("%.2f,", coord[j]);
                        } else {
                                printf("%.2f)\n", coord[j]);
                        }
                }
        }
}

static void coord_dump_by_indexes(struct kdtree *tree, long low, long high, int r)
{
        long i;
        printf("r=%d:", r);
        for (i = 0; i <= high; i++) {
                if (i < low) {
                        printf("%8s", " ");
                } else {
                        long index = tree->coord_indexes[i];
                        printf("%8.2f", tree->coord_table[index][r]);
                }
        }
        printf("\n");
}

static void bubble_sort(struct kdtree *tree, long low, long high, int r)
{
        long i, flag = high + 1;
        long *indexes = tree->coord_indexes;
        while (flag > 0) {
                long len = flag;
                flag = 0;
                for (i = low + 1; i < len; i++) {
                        if (D(tree, indexes[i], r) < D(tree, indexes[i - 1], r)) {
                                swap(indexes + i - 1, indexes + i);
                                flag = i;
                        }
                }
        }
}

static void insert_sort(struct kdtree *tree, long low, long high, int r)
{
        long i, j;
        long *indexes = tree->coord_indexes;
        for (i = low + 1; i <= high; i++) {
                long tmp_idx = indexes[i];
                double tmp_value = D(tree, indexes[i], r);
                j = i - 1;
                for (; j >= low && D(tree, indexes[j], r) > tmp_value; j--) {
                        indexes[j + 1] = indexes[j];
                }
                indexes[j + 1] = tmp_idx;
        }
}

static void quicksort(struct kdtree *tree, long low, long high, int r)
{
        if (high - low <= 32) {
                insert_sort(tree, low, high, r);
                //bubble_sort(tree, low, high, r);
                return;
        }

        long *indexes = tree->coord_indexes;
        /* median of 3 */
        long mid = low + (high - low) / 2;
        if (D(tree, indexes[low], r) > D(tree, indexes[mid], r)) {
                swap(indexes + low, indexes + mid);
        }
        if (D(tree, indexes[low], r) > D(tree, indexes[high], r)) {
                swap(indexes + low, indexes + high);
        }
        if (D(tree, indexes[high], r) > D(tree, indexes[mid], r)) {
                swap(indexes + high, indexes + mid);
        }

        /* D(indexes[low]) <= D(indexes[high]) <= D(indexes[mid]) */
        double pivot = D(tree, indexes[high], r);

        /* 3-way partition
         * +---------+-----------+---------+-------------+---------+
         * |  pivot  |  <=pivot  |   ?     |  >=pivot    |  pivot  |
         * +---------+-----------+---------+-------------+---------+
         * low     lt             i       j               gt    high
         */
        long i = low - 1;
        long lt = i;
        long j = high;
        long gt = j;
        for (; ;) {
                while (D(tree, indexes[++i], r) < pivot) {}
                while (D(tree, indexes[--j], r) > pivot && j > low) {}
                if (i >= j) break;
                swap(indexes + i, indexes + j);
                if (D(tree, indexes[i], r) == pivot) swap(&indexes[++lt], &indexes[i]);
                if (D(tree, indexes[j], r) == pivot) swap(&indexes[--gt], &indexes[j]);
        }
        /* i == j or j + 1 == i */
        swap(indexes + i, indexes + high);

        /* Move equal elements to the middle of array */
        long x, y;
        for (x = low, j = i - 1; x <= lt && j > lt; x++, j--) swap(indexes + x, indexes + j);
        for (y = high, i = i + 1; y >= gt && i < gt; y--, i++) swap(indexes + y, indexes + i);

        quicksort(tree, low, j - lt + x - 1, r);
        quicksort(tree, i + y - gt, high, r);
}

static struct kdnode *kdnode_alloc(double *coord, long index, int r)
{
        struct kdnode *node = malloc(sizeof(*node));
        if (node != NULL) {
                memset(node, 0, sizeof(*node));
                node->coord = coord;
                node->coord_index = index;
                node->r = r;
        }
        return node;
}

static void kdnode_free(struct kdnode *node)
{
        free(node);
}

static int coord_cmp(double *c1, double *c2, int dim)
{
        int i;
        double ret;
        for (i = 0; i < dim; i++) {
                ret = *c1++ - *c2++;
                if (fabs(ret) >= DBL_EPSILON) {
                        return ret > 0 ? 1 : -1;
                }
        }

        if (fabs(ret) < DBL_EPSILON) {
                return 0;
        } else {
                return ret > 0 ? 1 : -1;
        }
}

static void knn_list_add(struct kdtree *tree, struct kdnode *node, double distance)
{
        if (node == NULL) return;

        struct knn_list *head = &tree->knn_list_head;
        struct knn_list *p = head->prev;
        if (tree->knn_num == 1) {
                if (p->distance > distance) {
                        p = p->prev;
                }
        } else {
                while (p != head && p->distance > distance) {
                        p = p->prev;
                }
        }

        if (p == head || coord_cmp(p->node->coord, node->coord, tree->dim)) {
                struct knn_list *log = malloc(sizeof(*log));
                if (log != NULL) {
                        log->node = node;
                        log->distance = distance;
                        log->prev = p;
                        log->next = p->next;
                        p->next->prev = log;
                        p->next = log;
                        tree->knn_num++;
                }
        }
}

static void knn_list_adjust(struct kdtree *tree, struct kdnode *node, double distance)
{
        if (node == NULL) return;

        struct knn_list *head = &tree->knn_list_head;
        struct knn_list *p = head->prev;
        if (tree->knn_num == 1) {
                if (p->distance > distance) {
                        p = p->prev;
                }
        } else {
                while (p != head && p->distance > distance) {
                        p = p->prev;
                }
        }

        if (p == head || coord_cmp(p->node->coord, node->coord, tree->dim)) {
                struct knn_list *log = head->prev;
                /* Replace the original max one */
                log->node = node;
                log->distance = distance;
                /* Remove from the max position */
                head->prev = log->prev;
                log->prev->next = head;
                /* insert as a new one */
                log->prev = p;
                log->next = p->next;
                p->next->prev = log;
                p->next = log;
        }
}

static void knn_list_clear(struct kdtree *tree)
{
        struct knn_list *head = &tree->knn_list_head;
        struct knn_list *p = head->next;
        while (p != head) {
                struct knn_list *prev = p;
                p = p->next;
                free(prev);
        }
        tree->knn_num = 0;
}

static void resize(struct kdtree *tree)
{
        tree->capacity *= 2;
        tree->coords = realloc(tree->coords, tree->dim * sizeof(double) * tree->capacity);
        tree->coord_table = realloc(tree->coord_table, sizeof(double *) * tree->capacity);
        tree->coord_indexes = realloc(tree->coord_indexes, sizeof(long) * tree->capacity);
        tree->coord_deleted = realloc(tree->coord_deleted, sizeof(char) * tree->capacity);
        tree->coord_passed = realloc(tree->coord_passed, sizeof(char) * tree->capacity);
        coord_table_reset(tree);
        coord_index_reset(tree);
        coord_deleted_reset(tree);
        coord_passed_reset(tree);
}

static void kdnode_dump(struct kdnode *node, int dim)
{
        int i;
        if (node->coord != NULL) {
                printf("(");
                for (i = 0; i < dim; i++) {
                        if (i != dim - 1) {
                                printf("%.2f,", node->coord[i]);
                        } else {
                                printf("%.2f)\n", node->coord[i]);
                        }
                }
        } else {
                printf("(none)\n");
        }
}

void kdtree_insert(struct kdtree *tree, double *coord)
{
        if (tree->count + 1 > tree->capacity) {
                resize(tree);
        }
        memcpy(tree->coord_table[tree->count++], coord, tree->dim * sizeof(double));
}

static void knn_pickup(struct kdtree *tree, struct kdnode *node, double *target, int k)
{
        double dist = distance(node->coord, target, tree->dim);
        if (tree->knn_num < k) {
                knn_list_add(tree, node, dist);
        } else {
                if (dist < knn_max(tree)) {
                        knn_list_adjust(tree, node, dist);
                } else if (fabs(dist - knn_max(tree)) < DBL_EPSILON) {
                        knn_list_add(tree, node, dist);
                }
        }
}

static void kdtree_search_recursive(struct kdtree *tree, struct kdnode *node, double *target, int k, int *pickup)
{
        if (node == NULL || kdnode_passed(tree, node)) {
                return;
        }

        int r = node->r;
        if (!knn_search_on(tree, k, node->coord[r], target[r])) {
                return;
        }

        if (*pickup) {
                tree->coord_passed[node->coord_index] = 1;
                knn_pickup(tree, node, target, k);
                kdtree_search_recursive(tree, node->left, target, k, pickup);
                kdtree_search_recursive(tree, node->right, target, k, pickup);
        } else {
                if (is_leaf(node)) {
                        *pickup = 1;
                } else {
                        if (target[r] <= node->coord[r]) {
                                kdtree_search_recursive(tree, node->left, target, k, pickup);
                                kdtree_search_recursive(tree, node->right, target, k, pickup);
                        } else {
                                kdtree_search_recursive(tree, node->right, target, k, pickup);
                                kdtree_search_recursive(tree, node->left, target, k, pickup);
                        }
                }
                /* back track and pick up  */
                if (*pickup) {
                        tree->coord_passed[node->coord_index] = 1;
                        knn_pickup(tree, node, target, k);
                }
        }
}

void kdtree_knn_search(struct kdtree *tree, double *target, int k)
{
        if (k > 0) {
                int pickup = 0;
                kdtree_search_recursive(tree, tree->root, target, k, &pickup);
        }
}

void kdtree_delete(struct kdtree *tree, double *coord)
{
        int r = 0;
        struct kdnode *node = tree->root;
        struct kdnode *parent = node;

        while (node != NULL) {
                if (node->coord == NULL) {
                        if (parent->right->coord == NULL) {
                                break;
                        } else {
                                node = parent->right;
                                continue;
                        }
                }

                if (coord[r] < node->coord[r]) {
                        parent = node;
                        node = node->left;
                } else if (coord[r] > node->coord[r]) {
                        parent = node;
                        node = node->right;
                } else {
                        int ret = coord_cmp(coord, node->coord, tree->dim);
                        if (ret < 0) {
                                parent = node;
                                node = node->left;
                        } else if (ret > 0) {
                                parent = node;
                                node = node->right;
                        } else {
                                node->coord = NULL;
                                break;
                        }
                }
                r = (r + 1) % tree->dim;
        }
}

static void kdnode_build(struct kdtree *tree, struct kdnode **nptr, int r, long low, long high)
{
        if (low == high) {
                long index = tree->coord_indexes[low];
                *nptr = kdnode_alloc(tree->coord_table[index], index, r);
        } else if (low < high) {
                /* Sort and fetch the median to build a balanced BST */
                quicksort(tree, low, high, r);
                long median = low + (high - low) / 2;
                long median_index = tree->coord_indexes[median];
                struct kdnode *node = *nptr = kdnode_alloc(tree->coord_table[median_index], median_index, r);
                r = (r + 1) % tree->dim;
                kdnode_build(tree, &node->left, r, low, median - 1);
                kdnode_build(tree, &node->right, r, median + 1, high);
        }
}

static void kdtree_build(struct kdtree *tree)
{
        kdnode_build(tree, &tree->root, 0, 0, tree->count - 1);
}

void kdtree_rebuild(struct kdtree *tree)
{
        long i, j;
        size_t size_of_coord = tree->dim * sizeof(double);
        for (i = 0, j = 0; j < tree->count; i++, j++) {
                while (j < tree->count && tree->coord_deleted[j]) {
                        j++;
                }
                if (i != j && j < tree->count) {
                        memcpy(tree->coord_table[i], tree->coord_table[j], size_of_coord);
                        tree->coord_deleted[i] = 0;
                }
        }
        tree->count = i;
        coord_index_reset(tree);
        kdtree_build(tree);
}

struct kdtree *kdtree_init(int dim)
{
        struct kdtree *tree = malloc(sizeof(*tree));
        if (tree != NULL) {
                tree->root = NULL;
                tree->dim = dim;
                tree->count = 0;
                tree->capacity = 65536;
                tree->knn_list_head.next = &tree->knn_list_head;
                tree->knn_list_head.prev = &tree->knn_list_head;
                tree->knn_list_head.node = NULL;
                tree->knn_list_head.distance = 0;
                tree->knn_num = 0;
                tree->coords = malloc(dim * sizeof(double) * tree->capacity);
                tree->coord_table = malloc(sizeof(double *) * tree->capacity);
                tree->coord_indexes = malloc(sizeof(long) * tree->capacity);
                tree->coord_deleted = malloc(sizeof(char) * tree->capacity);
                tree->coord_passed = malloc(sizeof(char) * tree->capacity);
                coord_index_reset(tree);
                coord_table_reset(tree);
                coord_deleted_reset(tree);
                coord_passed_reset(tree);
        }
        return tree;
}

static void kdnode_destroy(struct kdnode *node)
{
        if (node == NULL) return;
        kdnode_destroy(node->left);
        kdnode_destroy(node->right);
        kdnode_free(node);
}

void kdtree_destroy(struct kdtree *tree)
{
        kdnode_destroy(tree->root);
        knn_list_clear(tree);
        free(tree->coords);
        free(tree->coord_table);
        free(tree->coord_indexes);
        free(tree->coord_deleted);
        free(tree->coord_passed);
        free(tree);
}

#define _KDTREE_DEBUG

#ifdef _KDTREE_DEBUG
struct kdnode_backlog {
        struct kdnode *node;
        int next_sub_idx;
};

void kdtree_dump(struct kdtree *tree)
{
        int level = 0;
        struct kdnode *node = tree->root;
        struct kdnode_backlog nbl, *p_nbl = NULL;
        struct kdnode_backlog nbl_stack[KDTREE_MAX_LEVEL];
        struct kdnode_backlog *top = nbl_stack;

        for (; ;) {
                if (node != NULL) {
                        /* Fetch the pop-up backlogged node's sub-id.
                         * If not backlogged, fetch the first sub-id. */
                        int sub_idx = p_nbl != NULL ? p_nbl->next_sub_idx : KDTREE_RIGHT_INDEX;

                        /* Backlog should be left in next loop */
                        p_nbl = NULL;

                        /* Backlog the node */
                        if (is_leaf(node) || sub_idx == KDTREE_LEFT_INDEX) {
                                top->node = NULL;
                                top->next_sub_idx = KDTREE_RIGHT_INDEX;
                        } else {
                                top->node = node;
                                top->next_sub_idx = KDTREE_LEFT_INDEX;
                        }
                        top++;
                        level++;

                        /* Draw lines as long as sub_idx is the first one */
                        if (sub_idx == KDTREE_RIGHT_INDEX) {
                                int i;
                                for (i = 1; i < level; i++) {
                                        if (i == level - 1) {
                                                printf("%-8s", "+-------");
                                        } else {
                                                if (nbl_stack[i - 1].node != NULL) {
                                                        printf("%-8s", "|");
                                                } else {
                                                        printf("%-8s", " ");
                                                }
                                        }
                                }
                                kdnode_dump(node, tree->dim);
                        }

                        /* Move down according to sub_idx */
                        node = sub_idx == KDTREE_LEFT_INDEX ? node->left : node->right;
                } else {
                        p_nbl = top == nbl_stack ? NULL : --top;
                        if (p_nbl == NULL) {
                                /* End of traversal */
                                break;
                        }
                        node = p_nbl->node;
                        level--;
                }
        }
}
#endif
