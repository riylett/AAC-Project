#ifndef EXTENSION_H
#define EXTENSION_H

#include "graph.h"

/**
 * Calculates the Minimal Extension Cost to embed Graph G into Graph H.
 * * This algorithm uses a greedy heuristic to map vertices from G to H.
 * It returns the "Extension Cost", which is the sum of:
 * 1. New vertices created (when H is exhausted).
 * 2. Missing edges added (when G requires an edge that H lacks).
 * * @param G The pattern/source graph (the graph being embedded).
 * @param H The target/host graph (the graph being extended).
 * @return The total cost (integer).
 */
int greedy_extension(Graph* G, Graph* H);

#endif // EXTENSION_H