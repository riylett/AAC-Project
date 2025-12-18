#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"

int** allocMatrix(int n) {
    int** m = malloc(n * sizeof(int*));
    if (!m) return NULL;
    for (int i = 0; i < n; i++) {
        m[i] = calloc(n, sizeof(int));
        if (!m[i]) {
            for (int j = 0; j < i; j++) free(m[j]);
            free(m);
            return NULL;
        }
    }
    return m;
}

// Load a graph from an open file in adjacency matrix format
// Format: first line is number of vertices, then n lines of adjacency matrix
Graph* loadGraphFromFile(FILE* f) {
    if (!f) return NULL;

    Graph* g = malloc(sizeof(Graph));
    if (!g) return NULL;

    // Read number of vertices
    if (fscanf(f, "%d", &g->n) != 1) {
        fprintf(stderr, "Error: could not read number of vertices from input.\n");
        free(g);
        return NULL;
    }

    g->matrix = allocMatrix(g->n);
    if (!g->matrix) {
        free(g);
        return NULL;
    }

    g->adj = malloc(g->n * sizeof(int*));
    g->adjSize = calloc(g->n, sizeof(int));
    if (!g->adj || !g->adjSize) {
        for (int i = 0; i < g->n; i++) free(g->matrix[i]);
        free(g->matrix);
        free(g->adj);
        free(g->adjSize);
        free(g);
        return NULL;
    }

    // Read adjacency matrix
    int edgeCount = 0;
    for (int i = 0; i < g->n; i++) {
        for (int j = 0; j < g->n; j++) {
            if (fscanf(f, "%d", &g->matrix[i][j]) != 1) {
                fprintf(stderr, "Error: could not read adjacency matrix element at row %d column %d.\n", i, j);
                for (int k = 0; k < g->n; k++) free(g->matrix[k]);
                free(g->matrix);
                free(g->adj);
                free(g->adjSize);
                free(g);
                return NULL;
            }
            // Count edges (only upper triangle for undirected)
            if (i < j && g->matrix[i][j] > 0) {
                edgeCount++;
            }
        }
    }
    
    // Validate input is an undirected graph: matrix must be symmetric
    // and no self-loops (diagonal must be zero). If input violates this,
    // free allocated resources and return NULL.
    for (int i = 0; i < g->n; i++) {
        if (g->matrix[i][i] != 0) {
            fprintf(stderr, "Invalid input: self-loop detected at vertex %d. Expected an undirected simple graph (no self-loops).\n", i);
            for (int k = 0; k < g->n; k++) free(g->matrix[k]);
            free(g->matrix);
            free(g->adj);
            free(g->adjSize);
            free(g);
            return NULL;
        }
        for (int j = i + 1; j < g->n; j++) {
            if (g->matrix[i][j] != g->matrix[j][i]) {
                fprintf(stderr, "Invalid input: adjacency matrix is not symmetric at (%d,%d): %d vs %d. Expected undirected graph.\n",
                        i, j, g->matrix[i][j], g->matrix[j][i]);
                for (int k = 0; k < g->n; k++) free(g->matrix[k]);
                free(g->matrix);
                free(g->adj);
                free(g->adjSize);
                free(g);
                return NULL;
            }
        }
    }
    g->m = edgeCount;

    // Count degrees for adjacency list allocation
    int* deg = calloc(g->n, sizeof(int));
    for (int i = 0; i < g->n; i++) {
        for (int j = 0; j < g->n; j++) {
            if (g->matrix[i][j] > 0) {
                deg[i]++;
            }
        }
    }

    // Allocate adjacency lists
    for (int i = 0; i < g->n; i++) {
        if (deg[i] > 0) {
            g->adj[i] = malloc(deg[i] * sizeof(int));
        } else {
            g->adj[i] = NULL;
        }
        g->adjSize[i] = 0;
    }

    // Fill adjacency lists
    for (int i = 0; i < g->n; i++) {
        for (int j = 0; j < g->n; j++) {
            if (g->matrix[i][j] > 0) {
                g->adj[i][g->adjSize[i]++] = j;
            }
        }
    }

    free(deg);
    return g;
}

// Load both graphs from a single file
int loadBothGraphs(const char* filename, Graph** G, Graph** H) {
    FILE* f = fopen(filename, "r");
    if (!f) return -1;

    *G = loadGraphFromFile(f);
    if (!*G) {
        fclose(f);
        return -1;
    }

    *H = loadGraphFromFile(f);
    if (!*H) {
        freeGraph(*G);
        *G = NULL;
        fclose(f);
        return -1;
    }

    fclose(f);
    return 0;
}

// Load single graph from file (adjacency matrix format)
Graph* loadGraph(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return NULL;

    Graph* g = loadGraphFromFile(f);
    fclose(f);
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
