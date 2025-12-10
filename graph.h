#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>

typedef struct {
    int n;          // number of vertices
    int m;          // number of edges

    int** adj;      // adjacency lists
    int* adjSize;

    int** matrix;   // adjacency matrix (0/1)
} Graph;

int** allocMatrix(int n);
Graph* loadGraph(const char* filename);  // Load single graph from file (adjacency matrix format)
Graph* loadGraphFromFile(FILE* f);  // Load one graph from an open file (adjacency matrix format)
int loadBothGraphs(const char* filename, Graph** G, Graph** H);  // Load both graphs from single file
void freeGraph(Graph* g);

#endif
