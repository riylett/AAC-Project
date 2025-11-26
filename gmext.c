#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "graph.h"

// Helper struct for Phase 1 sorting
typedef struct {
    int id;
    int degree;
} VertexInfo;

// Comparator for qsort: Descending order by degree
int compareVertices(const void* a, const void* b) {
    VertexInfo* va = (VertexInfo*)a;
    VertexInfo* vb = (VertexInfo*)b;
    return vb->degree - va->degree; // Descending
}

// --- Phase 1 Helper: Compute Score ---
// Logic: Penalty for degree deficit + Penalty for missing edges to already mapped neighbors
int compute_extension_score(int v_g, int v_h, Graph* G, Graph* H, int* mapping) {
    int score = 0;

    // 1. Heuristic: Degree Deficit Penalty
    // We only care if H is smaller than G (necessitating future edges)
    int deg_g = G->adjSize[v_g];
    int deg_h = H->adjSize[v_h];
    
    int deficit = (deg_g > deg_h) ? (deg_g - deg_h) : 0;
    score -= deficit;

    // 2. Exact Cost: Check mapped neighbors
    for (int i = 0; i < G->adjSize[v_g]; i++) {
        int neighbor_g = G->adj[v_g][i];

        // Check if this neighbor is already mapped
        if (mapping[neighbor_g] != -1) {
            int mapped_target = mapping[neighbor_g];

            // Check existence of edge in H between v_h and mapped_target
            // careful: mapped_target might be a "New" vertex (index >= H->n)
            int edge_exists = 0;
            
            if (mapped_target < H->n) {
                // Both are inside original H, check matrix
                edge_exists = H->matrix[v_h][mapped_target];
            } 
            // If mapped_target >= H->n, it's a new vertex, so edge definitely doesn't exist in H

            if (edge_exists) {
                score += 10; // Reward
            } else {
                score -= 10; // Penalty (Cost will be incurred later)
            }
        }
    }

    return score;
}

// --- Main Algorithm ---
// Returns the total cost (New Vertices + Added Edges)
int greedy_extension(Graph* G, Graph* H) {
    
    // --- Setup ---
    int* mapping = malloc(G->n * sizeof(int));
    int* used_H = calloc(H->n, sizeof(int)); // 0 = free, 1 = used
    
    for(int i=0; i<G->n; i++) mapping[i] = -1;

    int added_vertices_count = 0;
    int added_edges_count = 0;

    // --- Phase 1: Sorting G vertices by degree ---
    VertexInfo* sorted_G = malloc(G->n * sizeof(VertexInfo));
    for (int i = 0; i < G->n; i++) {
        sorted_G[i].id = i;
        sorted_G[i].degree = G->adjSize[i];
    }
    qsort(sorted_G, G->n, sizeof(VertexInfo), compareVertices);

    // --- Phase 1 & 2: Greedy Mapping ---
    for (int i = 0; i < G->n; i++) {
        int v_g = sorted_G[i].id;
        
        int best_v_h = -1;
        int best_score = INT_MIN;

        // Iterate through available vertices in H
        // NOTE: We removed the "if (deg < deg)" check here, as per the revised algorithm
        for (int v_h = 0; v_h < H->n; v_h++) {
            if (used_H[v_h]) continue;

            int score = compute_extension_score(v_g, v_h, G, H, mapping);
            
            if (score > best_score) {
                best_score = score;
                best_v_h = v_h;
            }
        }

        // Map to best existing vertex, OR create new if H is exhausted
        if (best_v_h != -1) {
            mapping[v_g] = best_v_h;
            used_H[best_v_h] = 1;
        } else {
            // Create "New" vertex
            // We assign an index >= H->n to represent new vertices
            mapping[v_g] = H->n + added_vertices_count;
            added_vertices_count++;
        }
    }

    // --- Phase 3: Identify Missing Edges ---
    // Iterate over all unique edges in G
    for (int u = 0; u < G->n; u++) {
        for (int i = 0; i < G->adjSize[u]; i++) {
            int v = G->adj[u][i];

            // Process each edge only once (u < v)
            if (u < v) {
                int f_u = mapping[u];
                int f_v = mapping[v];

                int edge_exists_in_H = 0;

                // Check if edge exists in H
                // Condition: Both endpoints must be within H's original range
                if (f_u < H->n && f_v < H->n) {
                    edge_exists_in_H = H->matrix[f_u][f_v];
                }
                // If either f_u or f_v is >= H->n, it's a new vertex, so edge is missing by definition

                if (!edge_exists_in_H) {
                    added_edges_count++;
                    // Optional: Print the added edge
                    // printf("Adding edge: (%d -> %d) maps to (%d, %d)\n", u, v, f_u, f_v);
                }
            }
        }
    }

    // --- Output Results ---
    printf("Mapping Result:\n");
    for(int i=0; i<G->n; i++) {
        if (mapping[i] < H->n)
            printf("  G[%d] -> H[%d]\n", i, mapping[i]);
        else
            printf("  G[%d] -> NEW VERTEX %d\n", i, mapping[i]);
    }
    printf("\nExtension Cost Calculation:\n");
    printf("  Added Vertices: %d\n", added_vertices_count);
    printf("  Added Edges:    %d\n", added_edges_count);
    printf("  Total Cost:     %d\n", added_vertices_count + added_edges_count);

    // --- Cleanup ---
    free(mapping);
    free(used_H);
    free(sorted_G);

    return added_vertices_count + added_edges_count;
}