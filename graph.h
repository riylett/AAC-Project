#ifndef GRAPH_H
#define GRAPH_H

typedef struct {
    int n;          // number of vertices
    int m;          // number of edges

    int** adj;      // adjacency lists
    int* adjSize;

    int** matrix;   // adjacency matrix (0/1)
} Graph;

Graph* loadGraph(const char* filename);
void freeGraph(Graph* g);

#endif
