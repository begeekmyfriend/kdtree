#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "kdtree.h"

#define N 1024 * 1024

static inline double rd(void)
{
        return (double) rand() / RAND_MAX * 20 - 10;
}

static void kdtree_knn_dump(struct kdtree *tree)
{
        int i;
        struct knn_list *p = tree->knn_list_head.next;
        while (p != &tree->knn_list_head) {
                putchar('(');
                for (i = 0; i < tree->dim; i++) {
                        if (i == tree->dim - 1) {
                                printf("%.2lf) Distance:%lf\n", p->node->coord[i], sqrt(p->distance));
                        } else {
                                printf("%.2lf, ", p->node->coord[i]);
                        }
                }
                p = p->next;
        }
}

int main(void)
{
        int i, j, dim = 2;
        struct timespec start, end;
        struct kdtree *tree = kdtree_init(dim);
        if (tree == NULL) {
                exit(-1);
        }

        double sample1[] = { 6.27, 5.50 };
        double sample2[] = { 1.24, -2.86 };
        double sample3[] = { 17.05, -12.79 };
        double sample4[] = { -6.88, -5.40 };
        double sample5[] = { -2.96, -0.50 };
        double sample6[] = { 7.75, -22.68 };
        double sample7[] = { 10.80, -5.03 };
        double sample8[] = { -4.60, -10.55 };
        double sample9[] = { -4.96, 12.61 };
        double sample10[] = { 1.75, 12.26 };
        double sample11[] = { 15.31, -13.16 };
        double sample12[] = { 7.83, 15.70 };
        double sample13[] = { 14.63, -0.35 };

        kdtree_insert(tree, sample1);
        kdtree_insert(tree, sample2);
        kdtree_insert(tree, sample3);
        kdtree_insert(tree, sample4);
        kdtree_insert(tree, sample5);
        kdtree_insert(tree, sample6);
        kdtree_insert(tree, sample7);
        kdtree_insert(tree, sample8);
        kdtree_insert(tree, sample9);
        kdtree_insert(tree, sample10);
        kdtree_insert(tree, sample11);
        kdtree_insert(tree, sample12);
        kdtree_insert(tree, sample13);

        kdtree_rebuild(tree);

        kdtree_dump(tree);

        int k = 3;
        double target[] = { -1, -5 };
        kdtree_knn_search(tree, target, k);
        printf("%d nearest neighbors of sample(", k);
        for (i = 0; i < dim; i++) {
                if (i == dim - 1) {
                        printf("%.2lf):\n", target[i]);
                } else {
                        printf("%.2lf, ", target[i]);
                }
        }
        kdtree_knn_dump(tree);
        kdtree_destroy(tree);

        /* Performance test */
        printf("\n>>> Performance test: kNN search for %d samples\n\n", N);
        dim = 12;
        tree = kdtree_init(dim);
        if (tree == NULL) {
                exit(-1);
        }

        /* Insert test */
        printf("Add %d nodes...\n", N);
        srandom(time(NULL));
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (i = 0; i < N; i++) {
                double *sample = malloc(dim * sizeof(double));
                for (j = 0; j < dim; j++) {
                        sample[j] = rd();
                }
                kdtree_insert(tree, sample);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        printf("time span: %ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_nsec - start.tv_nsec)/1000000);

        /* Build test */
        printf("Build KD tree...\n");
        clock_gettime(CLOCK_MONOTONIC, &start);
        kdtree_rebuild(tree);
        clock_gettime(CLOCK_MONOTONIC, &end);
        printf("time span: %ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_nsec - start.tv_nsec)/1000000);

        /* Search test */
        k = 20;
        srandom(time(NULL));
        double *t = malloc(dim * sizeof(double));
        for (i = 0; i < dim; i++) {
                t[i] = rd();
        }
        printf("Search KD tree...\n");
        srandom(time(NULL));
        clock_gettime(CLOCK_MONOTONIC, &start);
        kdtree_knn_search(tree, t, k);
        clock_gettime(CLOCK_MONOTONIC, &end);
        printf("time span: %ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_nsec - start.tv_nsec)/1000000);
        printf("%d nearest neighbors of sample(", k);
        for (i = 0; i < dim; i++) {
                if (i == dim - 1) {
                        printf("%.2lf):\n", t[i]);
                } else {
                        printf("%.2lf, ", t[i]);
                }
        }
        kdtree_knn_dump(tree);

        /* Destroy test */
        printf("Destroy KD tree...\n");
        srandom(time(NULL));
        clock_gettime(CLOCK_MONOTONIC, &start);
        kdtree_destroy(tree);
        clock_gettime(CLOCK_MONOTONIC, &end);
        printf("time span: %ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_nsec - start.tv_nsec)/1000000);

        return 0;
}
