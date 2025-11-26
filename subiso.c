#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "subiso.h"

static bool subgraphRec(
    const Graph* G, const Graph* H,
    int* map,        // map from G->H
    bool* usedH,     // which H vertices are used
    bool* adjG,
    bool* adjH,
    int mapped       // count of mapped G vertices
);

static void computeNewAdjG(bool* adj, const Graph* g, int v, int* map) {
    // Add neighbors of v in G (only unmapped ones according to map)
    for (int i = 0; i < g->adjSize[v]; i++) {
        int nb = g->adj[v][i];
        if (map[nb] == -1)
            adj[nb] = true;
    }
}

static void computeNewAdjH(bool* adj, const Graph* g, int v, bool* used) {
    // Add neighbors of v in H (only unused ones according to used[])
    for (int i = 0; i < g->adjSize[v]; i++) {
        int nb = g->adj[v][i];
        if (!used[nb])
            adj[nb] = true;
    }
}

bool isSubgraphIsomorphic(const Graph* G, const Graph* H) {
    // H must be at least as large as G
    if (H->n < G->n) return false;

    int* map = malloc(G->n * sizeof(int));
    bool* usedH = calloc(H->n, sizeof(bool));
    bool* adjG = calloc(G->n, sizeof(bool));
    bool* adjH = calloc(H->n, sizeof(bool));

    for (int i = 0; i < G->n; i++)
        map[i] = -1;

    bool result = subgraphRec(G, H, map, usedH, adjG, adjH, 0);

    free(map);
    free(usedH);
    free(adjG);
    free(adjH);

    return result;
}

static int chooseVertex(bool* adj, int n, int* map) {
    for (int i = 0; i < n; i++)
        if (adj[i] && map[i] == -1)
            return i;

    // no adjacency frontier → choose any unmapped vertex
    for (int i = 0; i < n; i++)
        if (map[i] == -1)
            return i;

    return -1; // shouldn't happen
}

static bool subgraphRec(
    const Graph* G, const Graph* H,
    int* map,
    bool* usedH,
    bool* adjG,
    bool* adjH,
    int mapped
) {
    if (mapped == G->n)
        return true;

    int n = chooseVertex(adjG, G->n, map);
    if (n == -1) return false;

    // Candidate set is either all of H (if map empty)
    // or Adj_H if frontier non-empty
    bool hasAdjH = false;
    for (int i = 0; i < H->n; i++)
        if (adjH[i] && !usedH[i])
            hasAdjH = true;

    // try all candidates
    for (int m = 0; m < H->n; m++) {

        if (usedH[m]) continue;
        if (hasAdjH && !adjH[m]) continue;

        // Degree check
        if (G->adjSize[n] > H->adjSize[m])
            continue;

        // Adjacency consistency check
        bool consistent = true;
        for (int g2 = 0; g2 < G->n; g2++) {
            if (map[g2] == -1) continue;
            int h2 = map[g2];

            if (G->matrix[n][g2] && !H->matrix[m][h2]) {
                consistent = false;
                break;
            }
        }
        if (!consistent) continue;

        // Neighborhood feasibility check
        bool feasible = true;

        for (int i = 0; i < G->adjSize[n]; i++) {
            int nu = G->adj[n][i];
            if (map[nu] != -1) continue;

            bool hasCandidate = false;
            for (int j = 0; j < H->adjSize[m]; j++) {
                int mu = H->adj[m][j];
                if (!usedH[mu] && H->adjSize[mu] >= G->adjSize[nu]) {
                    hasCandidate = true;
                    break;
                }
            }

            if (!hasCandidate) {
                feasible = false;
                break;
            }
        }
        if (!feasible) continue;

        // --- Accept mapping ---
        map[n] = m;
        usedH[m] = true;

        // Save old adjacency frontiers
        bool* oldAdjG = malloc(G->n * sizeof(bool));
        bool* oldAdjH = malloc(H->n * sizeof(bool));

        memcpy(oldAdjG, adjG, G->n * sizeof(bool));
        memcpy(oldAdjH, adjH, H->n * sizeof(bool));


        // Update frontier sets
        adjG[n] = false;
        computeNewAdjG(adjG, G, n, map); // 'map' marks used G vertices implicitly

        adjH[m] = false;
        computeNewAdjH(adjH, H, m, usedH);        if (subgraphRec(G, H, map, usedH, adjG, adjH, mapped + 1)) {
            free(oldAdjG);
            free(oldAdjH);
            return true;
        }

        // --- Backtrack ---
        map[n] = -1;
        usedH[m] = false;
        memcpy(adjG, oldAdjG, G->n * sizeof(bool));
        memcpy(adjH, oldAdjH, H->n * sizeof(bool));

        free(oldAdjG);
        free(oldAdjH);
    }

    return false;
}
