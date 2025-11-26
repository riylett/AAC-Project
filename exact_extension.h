#ifndef EXACT_EXTENSION_H
#define EXACT_EXTENSION_H

#include "graph.h"

typedef struct {
    int* newVertices;
    int newVertexCount;
    int (*newEdges)[2];
    int newEdgeCount;
} Extension;

Extension* exactMinimalExtension(const Graph* G, const Graph* H);
Extension* findExtension(const Graph* G, const Graph* H, int maxSize);
void freeExtensionObject(Extension* e);

#endif
