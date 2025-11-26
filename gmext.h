#ifndef GMEXT_H
#define GMEXT_H

#include "graph.h"

typedef struct {
    int* newVertices;
    int newVertexCount;
    int (*newEdges)[2];
    int newEdgeCount;
    int* mapping;      // mapping from G vertices to H vertices
    int mappingSize;
} GreedyExtension;

/**
 * Calculates the Minimal Extension Cost to embed Graph G into Graph H.
 * This algorithm uses a greedy heuristic to map vertices from G to H.
 * 
 * @param G The pattern/source graph (the graph being embedded).
 * @param H The target/host graph (the graph being extended).
 * @return GreedyExtension struct with extension details, or NULL on failure.
 */
GreedyExtension* greedy_extension(Graph* G, Graph* H);

/**
 * Frees a GreedyExtension struct.
 */
void freeGreedyExtension(GreedyExtension* ext);

#endif // GMEXT_H