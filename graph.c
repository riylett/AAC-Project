#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"

static int** allocMatrix(int n) {
    int** m = malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        m[i] = calloc(n, sizeof(int));
    }
    return m;
}

Graph* loadGraph(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return NULL;

    Graph* g = malloc(sizeof(Graph));

    fscanf(f, "%d %d", &g->n, &g->m);

    g->adj = malloc(g->n * sizeof(int*));
    g->adjSize = calloc(g->n, sizeof(int));
    g->matrix = allocMatrix(g->n);

    // Temporary count of neighbors before allocating final adj lists
    int* deg = calloc(g->n, sizeof(int));
    int u, v;

    int* U = malloc(g->m * sizeof(int));
    int* V = malloc(g->m * sizeof(int));

    for (int i = 0; i < g->m; i++) {
        fscanf(f, "%d %d", &u, &v);
        U[i] = u;
        V[i] = v;
        deg[u]++;
        deg[v]++;
        g->matrix[u][v] = 1;
        g->matrix[v][u] = 1;
    }
    fclose(f);

    for (int i = 0; i < g->n; i++)
        g->adj[i] = malloc(deg[i] * sizeof(int));

    memset(deg, 0, g->n * sizeof(int));

    for (int i = 0; i < g->m; i++) {
        u = U[i]; 
        v = V[i];

        g->adj[u][deg[u]++] = v;
        g->adj[v][deg[v]++] = u;
    }

    for (int i = 0; i < g->n; i++)
        g->adjSize[i] = deg[i];

    free(U);
    free(V);
    free(deg);

    return g;
}

void freeGraph(Graph* g) {
    if (!g) return;
    for (int i = 0; i < g->n; i++) {
        free(g->adj[i]);
        free(g->matrix[i]);
    }
    free(g->adj);
    free(g->adjSize);
    free(g->matrix);
    free(g);
}
