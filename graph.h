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

/**
 * =============================================================================
 * GRAPH SIZE DEFINITION
 * =============================================================================
 * Graph Size S(G) = |V| + |E| (number of vertices + number of edges)
 * 
 * This definition treats vertices and edges as equally weighted components
 * of the graph structure. It provides a natural measure of graph complexity
 * that is consistent with the minimal extension cost calculation.
 * =============================================================================
 */
int graphSize(const Graph* g);

/**
 * =============================================================================
 * GRAPH METRIC / DISTANCE DEFINITION
 * =============================================================================
 * The distance between two graphs G1 and G2 is defined as the Graph Edit 
 * Distance (GED), which represents the minimum cost of edit operations 
 * needed to transform G1 into G2.
 * 
 * For efficiency, we compute an approximation because exact GED computation
 * would be too expensive (GED is NP-hard).
 * =============================================================================
 */
int graphDistance(const Graph* G1, const Graph* G2);

/**
 * Computes the extension distance - the minimum number of operations
 * (vertex additions + edge additions) needed to make H contain G as a subgraph.
 * This is a specialized asymmetric distance used in the extension problem.
 */
int extensionDistance(const Graph* G, const Graph* H, int newVertices, int newEdges);

#endif
