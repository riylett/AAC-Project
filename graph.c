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

/**
 * =============================================================================
 * GRAPH SIZE IMPLEMENTATION
 * =============================================================================
 * S(G) = |V| + |E|
 * 
 * This definition is consistent with our extension cost calculation where
 * adding a vertex costs 1 and adding an edge costs 1.
 * =============================================================================
 */
int graphSize(const Graph* g) {
    if (!g) return 0;
    return g->n + g->m;
}


/**
 * =============================================================================
 * EXTENSION DISTANCE IMPLEMENTATION
 * =============================================================================
 * The extension distance from G to H is the minimal cost to extend H
 * such that G becomes a subgraph of the extended H.
 * 
 * Extension Cost = number of vertices added + number of edges added
 * 
 * This is an asymmetric measure (not a true metric) but is directly
 * related to the Graph Edit Distance when considering only additions.
 * =============================================================================
 */
int extensionDistance(const Graph* G, const Graph* H, int newVertices, int newEdges) {
    (void)G;  // Parameters included for context/documentation
    (void)H;
    return newVertices + newEdges;
}

// Simple Hungarian (assignment) implementation for integer cost matrix.
// Minimizes sum of assigned costs. Returns total minimal cost and fills
// `assignment` where assignment[i] = column assigned to row i (0-based).
static int hungarianSolve(int n, int** a, int* assignment) {
    const int INF = 1000000000;
    int i, j, col;
    int *u = calloc(n+1, sizeof(int));
    int *v = calloc(n+1, sizeof(int));
    int *p = calloc(n+1, sizeof(int));
    int *way = calloc(n+1, sizeof(int));
    if (!u || !v || !p || !way) {
        free(u); free(v); free(p); free(way);
        return INF;
    }

    for (i = 1; i <= n; ++i) {
        p[0] = i;
        int j0 = 0;
        int *minv = malloc((n+1) * sizeof(int));
        char *used = calloc(n+1, sizeof(char));
        if (!minv || !used) {
            free(minv); free(used);
            free(u); free(v); free(p); free(way);
            return INF;
        }
        for (j = 0; j <= n; ++j) minv[j] = INF, used[j] = 0;
        do {
            used[j0] = 1;
            int i0 = p[j0], delta = INF, j1 = 0;
            for (j = 1; j <= n; ++j) if (!used[j]) {
                int cur = a[i0-1][j-1] - u[i0] - v[j];
                if (cur < minv[j]) { minv[j] = cur; way[j] = j0; }
                if (minv[j] < delta) { delta = minv[j]; j1 = j; }
            }
            for (j = 0; j <= n; ++j) {
                if (used[j]) { u[p[j]] += delta; v[j] -= delta; }
                else minv[j] -= delta;
            }
            j0 = j1;
        } while (p[j0] != 0);
        do {
            int j1 = way[j0];
            p[j0] = p[j1];
            j0 = j1;
        } while (j0 != 0);

        free(minv); free(used);
    }

    // Build assignment and compute cost
    for (j = 1; j <= n; ++j) {
        if (p[j] > 0) {
            assignment[p[j]-1] = j-1;
        }
    }

    int totalCost = 0;
    for (i = 0; i < n; ++i) {
        col = assignment[i];
        if (col >= 0) totalCost += a[i][col];
    }

    free(u); free(v); free(p); free(way);
    return totalCost;
}

// Approximate graph distance using assignment (Hungarian) on adjacency rows.
// This runs in polynomial time O(n^3) due to the Hungarian solver. It's a
// simple and fast approximation: construct cost matrix where cost(i,j) is
// the Hamming distance between row i of G1 and row j of G2 after padding
// to equal sizes. The final distance is (sum_assigned_row_distances / 2)
// + |n1 - n2|, where division by 2 accounts for undirected double-counting
// of edge mismatches and the vertex difference term charges vertex add/del.
static int graphDistanceHungarian(const Graph* G1, const Graph* G2) {
    int n1 = G1 ? G1->n : 0;
    int n2 = G2 ? G2->n : 0;
    int n = n1 > n2 ? n1 : n2;
    if (n == 0) return 0;

    // allocate cost matrix
    int** cost = allocMatrix(n);
    if (!cost) return -1;

    // build padded row-hamming costs
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int h = 0;
            for (int k = 0; k < n; ++k) {
                int a = 0, b = 0;
                if (i < n1 && k < n1) a = G1->matrix[i][k];
                if (j < n2 && k < n2) b = G2->matrix[j][k];
                if (a != b) h++;
            }
            cost[i][j] = h;
        }
    }

    int* assignment = malloc(n * sizeof(int));
    if (!assignment) {
        for (int i = 0; i < n; i++) free(cost[i]); free(cost);
        return -1;
    }
    for (int i = 0; i < n; ++i) assignment[i] = -1;

    int total = hungarianSolve(n, cost, assignment);

    // free cost
    for (int i = 0; i < n; ++i) free(cost[i]); free(cost);

    if (total >= 1000000000) { free(assignment); return -1; }

    // For undirected graphs, each edge mismatch counted twice in row sums.
    int edgeMismatches = total / 2;
    int vertexDiff = n1 > n2 ? n1 - n2 : n2 - n1;
    int distance = edgeMismatches + vertexDiff;

    free(assignment);
    return distance;
}

/**
 * =============================================================================
 * GRAPH METRIC IMPLEMENTATION
 * =============================================================================
 * Computes an approximation of Graph Edit Distance between two graphs.
 * 
 * For small graphs, we use the exact structural difference.
 * For larger graphs, we use a heuristic based on size differences.
 * 
 * The metric d(G1, G2) represents the minimum edit operations needed
 * to transform G1 into G2 (or vice versa, since it's symmetric).
 * =============================================================================
 */
int graphDistance(const Graph* G1, const Graph* G2) {
    // Use the polynomial-time Hungarian-based approximation.
    // The function handles null graphs and returns an integer edit estimate.
    if (!G1 && !G2) return 0;
    if (!G1) return graphSize(G2);
    if (!G2) return graphSize(G1);

    return graphDistanceHungarian(G1, G2);
}

