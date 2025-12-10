#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "graph.h"
#include "gmext.h"

// Helper struct for sorting
typedef struct {
    int id;
    int degree;
} VertexInfo;

// Comparator for qsort: Descending order by degree
static int compareVertices(const void* a, const void* b) {
    VertexInfo* va = (VertexInfo*)a;
    VertexInfo* vb = (VertexInfo*)b;
    return vb->degree - va->degree;
}

// Count how many edges would be preserved if we map v_g to v_h
static int countPreservedEdges(int v_g, int v_h, Graph* G, Graph* H, int* mapping) {
    int preserved = 0;
    
    for (int i = 0; i < G->adjSize[v_g]; i++) {
        int neighbor_g = G->adj[v_g][i];
        
        if (mapping[neighbor_g] != -1) {
            int mapped_neighbor = mapping[neighbor_g];
            
            // Check if edge exists in H
            if (mapped_neighbor < H->n && v_h < H->n) {
                if (H->matrix[v_h][mapped_neighbor]) {
                    preserved++;
                }
            }
        }
    }
    return preserved;
}

// Count how many potential edges could be preserved (for unmapped neighbors)
static int countPotentialEdges(int v_g, int v_h, Graph* G, Graph* H, int* mapping, int* used_H) {
    int potential = 0;
    
    for (int i = 0; i < G->adjSize[v_g]; i++) {
        int neighbor_g = G->adj[v_g][i];
        
        if (mapping[neighbor_g] == -1) {
            // Unmapped neighbor - check if v_h has available neighbors
            for (int j = 0; j < H->adjSize[v_h]; j++) {
                int h_neighbor = H->adj[v_h][j];
                if (!used_H[h_neighbor]) {
                    potential++;
                    break; // Count each G neighbor only once
                }
            }
        }
    }
    return potential;
}

// Improved scoring function
static int compute_improved_score(int v_g, int v_h, Graph* G, Graph* H, int* mapping, int* used_H) {
    int score = 0;
    
    // 1. Preserved edges (most important - each saves an edge addition)
    int preserved = countPreservedEdges(v_g, v_h, G, H, mapping);
    score += preserved * 100;
    
    // 2. Potential for future edge preservation
    int potential = countPotentialEdges(v_g, v_h, G, H, mapping, used_H);
    score += potential * 20;
    
    // 3. Degree compatibility (prefer H vertices with similar or higher degree)
    int deg_g = G->adjSize[v_g];
    int deg_h = H->adjSize[v_h];
    
    if (deg_h >= deg_g) {
        score += 10; // Good: H vertex can accommodate all edges
    } else {
        score -= (deg_g - deg_h) * 5; // Penalty for degree deficit
    }
    
    // 4. Neighbor degree matching (prefer similar neighborhood structure)
    int unmapped_g_neighbors = 0;
    int available_h_neighbors = 0;
    
    for (int i = 0; i < G->adjSize[v_g]; i++) {
        if (mapping[G->adj[v_g][i]] == -1) unmapped_g_neighbors++;
    }
    for (int i = 0; i < H->adjSize[v_h]; i++) {
        if (!used_H[H->adj[v_h][i]]) available_h_neighbors++;
    }
    
    if (available_h_neighbors >= unmapped_g_neighbors) {
        score += 15;
    }
    
    return score;
}

// Calculate total cost for a given mapping
static int calculateMappingCost(Graph* G, Graph* H, int* mapping, int added_vertices) {
    int missing_edges = 0;
    
    for (int u = 0; u < G->n; u++) {
        for (int i = 0; i < G->adjSize[u]; i++) {
            int v = G->adj[u][i];
            if (u < v) {
                int f_u = mapping[u];
                int f_v = mapping[v];
                
                int edge_exists = 0;
                if (f_u < H->n && f_v < H->n) {
                    edge_exists = H->matrix[f_u][f_v];
                }
                
                if (!edge_exists) missing_edges++;
            }
        }
    }
    
    return added_vertices + missing_edges;
}

// Try multiple starting orderings and keep the best result
static void tryMapping(Graph* G, Graph* H, VertexInfo* order, int* best_mapping, 
                       int* best_cost, int* best_added_vertices) {
    int* mapping = malloc(G->n * sizeof(int));
    int* used_H = calloc(H->n, sizeof(int));
    
    for (int i = 0; i < G->n; i++) mapping[i] = -1;
    
    int added_vertices = 0;
    
    // Map vertices in the given order
    for (int i = 0; i < G->n; i++) {
        int v_g = order[i].id;
        
        int best_v_h = -1;
        int best_score = INT_MIN;
        
        for (int v_h = 0; v_h < H->n; v_h++) {
            if (used_H[v_h]) continue;
            
            int score = compute_improved_score(v_g, v_h, G, H, mapping, used_H);
            
            if (score > best_score) {
                best_score = score;
                best_v_h = v_h;
            }
        }
        
        if (best_v_h != -1) {
            mapping[v_g] = best_v_h;
            used_H[best_v_h] = 1;
        } else {
            mapping[v_g] = H->n + added_vertices;
            added_vertices++;
        }
    }
    
    int cost = calculateMappingCost(G, H, mapping, added_vertices);
    
    if (cost < *best_cost) {
        *best_cost = cost;
        *best_added_vertices = added_vertices;
        memcpy(best_mapping, mapping, G->n * sizeof(int));
    }
    
    free(mapping);
    free(used_H);
}

// --- Main Algorithm ---
GreedyExtension* greedy_extension(Graph* G, Graph* H) {
    
    int* best_mapping = malloc(G->n * sizeof(int));
    int best_cost = INT_MAX;
    int best_added_vertices = 0;
    
    // Strategy 1: Sort by degree descending
    VertexInfo* order1 = malloc(G->n * sizeof(VertexInfo));
    for (int i = 0; i < G->n; i++) {
        order1[i].id = i;
        order1[i].degree = G->adjSize[i];
    }
    qsort(order1, G->n, sizeof(VertexInfo), compareVertices);
    tryMapping(G, H, order1, best_mapping, &best_cost, &best_added_vertices);
    
    // Strategy 2: Start from highest degree vertex and expand by connectivity (BFS-like)
    VertexInfo* order2 = malloc(G->n * sizeof(VertexInfo));
    int* visited = calloc(G->n, sizeof(int));
    int order_idx = 0;
    
    // Find highest degree vertex as start
    int start = 0;
    for (int i = 1; i < G->n; i++) {
        if (G->adjSize[i] > G->adjSize[start]) start = i;
    }
    
    // BFS from start
    int* queue = malloc(G->n * sizeof(int));
    int front = 0, back = 0;
    queue[back++] = start;
    visited[start] = 1;
    
    while (front < back) {
        int v = queue[front++];
        order2[order_idx].id = v;
        order2[order_idx].degree = G->adjSize[v];
        order_idx++;
        
        // Add neighbors sorted by degree
        if (G->adjSize[v] > 0) {
            VertexInfo* neighbors = malloc(G->adjSize[v] * sizeof(VertexInfo));
            int n_count = 0;
            for (int i = 0; i < G->adjSize[v]; i++) {
                int nb = G->adj[v][i];
                if (!visited[nb]) {
                    neighbors[n_count].id = nb;
                    neighbors[n_count].degree = G->adjSize[nb];
                    n_count++;
                    visited[nb] = 1;
                }
            }
            qsort(neighbors, n_count, sizeof(VertexInfo), compareVertices);
            for (int i = 0; i < n_count; i++) {
                queue[back++] = neighbors[i].id;
            }
            free(neighbors);
        }
    }
    
    // Add any remaining unvisited vertices
    for (int i = 0; i < G->n; i++) {
        if (!visited[i]) {
            order2[order_idx].id = i;
            order2[order_idx].degree = G->adjSize[i];
            order_idx++;
        }
    }
    
    tryMapping(G, H, order2, best_mapping, &best_cost, &best_added_vertices);
    
    // Strategy 3: Try a few random-ish permutations based on different starting vertices
    for (int start_v = 0; start_v < G->n && start_v < 5; start_v++) {
        memset(visited, 0, G->n * sizeof(int));
        order_idx = 0;
        front = back = 0;
        
        queue[back++] = start_v;
        visited[start_v] = 1;
        
        while (front < back) {
            int v = queue[front++];
            order2[order_idx].id = v;
            order2[order_idx].degree = G->adjSize[v];
            order_idx++;
            
            for (int i = 0; i < G->adjSize[v]; i++) {
                int nb = G->adj[v][i];
                if (!visited[nb]) {
                    queue[back++] = nb;
                    visited[nb] = 1;
                }
            }
        }
        
        for (int i = 0; i < G->n; i++) {
            if (!visited[i]) {
                order2[order_idx].id = i;
                order2[order_idx].degree = G->adjSize[i];
                order_idx++;
            }
        }
        
        tryMapping(G, H, order2, best_mapping, &best_cost, &best_added_vertices);
    }
    
    free(order1);
    free(order2);
    free(visited);
    free(queue);
    
    // --- Build edge list from best mapping ---
    int added_edges_capacity = G->m;
    int (*added_edges)[2] = malloc(added_edges_capacity * sizeof(int[2]));
    int added_edges_count = 0;
    
    for (int u = 0; u < G->n; u++) {
        for (int i = 0; i < G->adjSize[u]; i++) {
            int v = G->adj[u][i];
            if (u < v) {
                int f_u = best_mapping[u];
                int f_v = best_mapping[v];
                
                int edge_exists = 0;
                if (f_u < H->n && f_v < H->n) {
                    edge_exists = H->matrix[f_u][f_v];
                }
                
                if (!edge_exists) {
                    if (added_edges_count >= added_edges_capacity) {
                        added_edges_capacity *= 2;
                        added_edges = realloc(added_edges, added_edges_capacity * sizeof(int[2]));
                    }
                    added_edges[added_edges_count][0] = f_u;
                    added_edges[added_edges_count][1] = f_v;
                    added_edges_count++;
                }
            }
        }
    }
    
    // --- Build Result Struct ---
    GreedyExtension* result = malloc(sizeof(GreedyExtension));
    if (!result) {
        free(best_mapping);
        free(added_edges);
        return NULL;
    }
    
    result->newVertexCount = best_added_vertices;
    if (best_added_vertices > 0) {
        result->newVertices = malloc(best_added_vertices * sizeof(int));
        for (int i = 0; i < best_added_vertices; i++) {
            result->newVertices[i] = H->n + i;
        }
    } else {
        result->newVertices = NULL;
    }
    
    result->newEdgeCount = added_edges_count;
    if (added_edges_count > 0) {
        result->newEdges = malloc(added_edges_count * sizeof(int[2]));
        for (int i = 0; i < added_edges_count; i++) {
            result->newEdges[i][0] = added_edges[i][0];
            result->newEdges[i][1] = added_edges[i][1];
        }
    } else {
        result->newEdges = NULL;
    }
    
    result->mapping = best_mapping;
    result->mappingSize = G->n;
    
    free(added_edges);
    
    return result;
}

void freeGreedyExtension(GreedyExtension* ext) {
    if (!ext) return;
    free(ext->newVertices);
    free(ext->newEdges);
    free(ext->mapping);
    free(ext);
}