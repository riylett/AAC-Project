#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include "subiso.h"
#include "exact_extension.h"


static Graph* cloneGraphWithExtraVertices(const Graph* H, int kv) {
    int newN = H->n + kv;
    Graph* g = malloc(sizeof(Graph));
    if (!g) return NULL;

    g->n = newN;
    g->m = H->m;

    g->adj = malloc(newN * sizeof(int*));
    g->adjSize = calloc(newN, sizeof(int));
    g->matrix = allocMatrix(newN);
    if (!g->adj || !g->adjSize || !g->matrix) {
        free(g->adj); free(g->adjSize);
        if (g->matrix) {
            for (int i = 0; i < newN; i++) free(g->matrix[i]);
            free(g->matrix);
        }
        free(g);
        return NULL;
    }

    for (int i = 0; i < H->n; i++) {
        g->adjSize[i] = H->adjSize[i];
        if (H->adjSize[i] > 0) {
            g->adj[i] = malloc(H->adjSize[i] * sizeof(int));
            memcpy(g->adj[i], H->adj[i], H->adjSize[i] * sizeof(int));
        } else {
            g->adj[i] = NULL;
        }
        memcpy(g->matrix[i], H->matrix[i], H->n * sizeof(int));
    }

    // Initialize new vertices with empty adjacency lists and zeroed matrix rows
    for (int i = H->n; i < newN; i++) {
        g->adjSize[i] = 0;
        g->adj[i] = NULL; // will realloc when edges are added
    }

    return g;
}

static void addEdgesToGraph(Graph* g, int (*edgeSet)[2], int ke) {
    if (!g || ke <= 0) return;

    for (int i = 0; i < ke; i++) {
        int u = edgeSet[i][0];
        int v = edgeSet[i][1];
        if (u < 0 || v < 0 || u >= g->n || v >= g->n) continue;
        if (g->matrix[u][v]) continue; // avoid duplicate

        g->matrix[u][v] = g->matrix[v][u] = 1;

        g->adj[u] = realloc(g->adj[u], (g->adjSize[u] + 1) * sizeof(int));
        g->adj[u][g->adjSize[u]++] = v;

        g->adj[v] = realloc(g->adj[v], (g->adjSize[v] + 1) * sizeof(int));
        g->adj[v][g->adjSize[v]++] = u;

        g->m += 1;
    }
}

static Extension* buildExtension(const int* newVertices, int kv,
                                 int (*edgeSet)[2], int ke) {
    Extension* ext = malloc(sizeof(Extension));
    if (!ext) return NULL;

    ext->newVertexCount = kv;
    if (kv > 0) {
        ext->newVertices = malloc(kv * sizeof(int));
        if (!ext->newVertices) { free(ext); return NULL; }
        memcpy(ext->newVertices, newVertices, kv * sizeof(int));
    } else {
        ext->newVertices = NULL;
    }

    ext->newEdgeCount = ke;
    if (ke > 0) {
        ext->newEdges = malloc(ke * sizeof(int[2]));
        if (!ext->newEdges) { free(ext->newVertices); free(ext); return NULL; }
        memcpy(ext->newEdges, edgeSet, ke * sizeof(int[2]));
    } else {
        ext->newEdges = NULL;
    }

    return ext;
}

static void freeExtension(Extension* e) {
    if (!e) return;
    free(e->newVertices);
    free(e->newEdges);
    free(e);
}

// Core try: build H' and test subgraph iso
static Extension* tryExtension(const Graph* G, const Graph* H,
                               const int* newVertices, int kv,
                               int (*edgeSet)[2], int ke) {
    Graph* Hprime = cloneGraphWithExtraVertices(H, kv);
    if (!Hprime) return NULL;

    addEdgesToGraph(Hprime, edgeSet, ke);

    bool iso = isSubgraphIsomorphic(G, Hprime);

    Extension* ext = NULL;
    if (iso) {
        ext = buildExtension(newVertices, kv, edgeSet, ke);
    }

    freeGraph(Hprime);
    return ext;
}

// Generate all pairs excluding edges that already exist in H
static int generateAllPairs(const Graph* H, int* vertices, int total, int (*edges)[2]) {
    int idx = 0;
    for (int i = 0; i < total; i++) {
        for (int j = i + 1; j < total; j++) {
            int u = vertices[i];
            int v = vertices[j];
            // Skip edges that already exist in H
            if (u < H->n && v < H->n && H->matrix[u][v])
                continue;
            edges[idx][0] = u;
            edges[idx][1] = v;
            idx++;
        }
    }
    return idx;
}

static void onEdgeCombination(int (*curr)[2], int k,
                              const Graph* G, const Graph* H,
                              int* newVertices, int kv,
                              int* found, Extension** result) {
    if (*found) return;
    Extension* ext = tryExtension(G, H, newVertices, kv, curr, k);
    if (ext) {
        *result = ext;
        *found = 1;
    }
}

static void combEdges(int (*edges)[2], int n, int k, int start, int depth,
                      int (*curr)[2], int* found,
                      const Graph* G, const Graph* H, int* newVertices, int kv,
                      Extension** result) {
    if (*found) return;
    if (depth == k) {
        onEdgeCombination(curr, k, G, H, newVertices, kv, found, result);
        return;
    }
    for (int i = start; i < n; i++) {
        curr[depth][0] = edges[i][0];
        curr[depth][1] = edges[i][1];
        combEdges(edges, n, k, i + 1, depth + 1, curr, found, G, H, newVertices, kv, result);
        if (*found) return;
    }
}

// Try a specific (kv, ke) combination
static Extension* tryKVKE(const Graph* G, const Graph* H, int kv, int ke) {
    int* newVertices = NULL;
    int* allVertices = NULL;
    int (*edges)[2] = NULL;
    int (*curr)[2] = NULL;
    Extension* result = NULL;

    if (ke > G->m) {
        return NULL;  // Can't possibly need more edges than G has
    }

    if (kv > 0) {
        newVertices = malloc(kv * sizeof(int));
        for (int i = 0; i < kv; i++) newVertices[i] = H->n + i;
    }

    int totalV = H->n + kv;
    allVertices = malloc(totalV * sizeof(int));
    for (int i = 0; i < H->n; i++) allVertices[i] = i;
    for (int i = 0; i < kv; i++) allVertices[H->n + i] = H->n + i;

    int maxEdges = totalV * (totalV - 1) / 2;
    edges = malloc(maxEdges * sizeof(int[2]));
    int edgeCount = generateAllPairs(H, allVertices, totalV, edges);

    // Handle ke=0 case separately (no edges to add, just vertices)
    if (ke == 0) {
        result = tryExtension(G, H, newVertices, kv, NULL, 0);
    } else if (ke <= edgeCount) {
        curr = malloc(ke * sizeof(int[2]));
        int found = 0;
        combEdges(edges, edgeCount, ke, 0, 0, curr, &found, G, H, newVertices, kv, &result);
    }

    free(newVertices);
    free(allVertices);
    free(edges);
    free(curr);

    return result;
}

// Public find
Extension* findExtension(const Graph* G, const Graph* H, int maxSize) {
    int min_new_vertices = (G->n > H->n) ? (G->n - H->n) : 0;

    for (int kv = min_new_vertices; kv <= maxSize; kv++) {
        int ke = maxSize - kv;
        Extension* ext = tryKVKE(G, H, kv, ke);
        if (ext) return ext;
    }
    return NULL;
}

Extension* exactMinimalExtension(const Graph* G, const Graph* H) {
    int maxSize = G->n + G->m;
    for (int k = 0; k <= maxSize; k++) {
        Extension* ext = findExtension(G, H, k);
        if (ext) return ext;
    }
    return NULL;
}

void freeExtensionObject(Extension* e) {
    freeExtension(e);
}