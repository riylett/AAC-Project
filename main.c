#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "graph.h"
#include "subiso.h"
#include "gmext.h"
#include "exact_extension.h"

typedef enum {
    ALG_GREEDY,
    ALG_EXACT,
    ALG_SUBISO_ONLY
} Algorithm;

static void printUsage(const char* programName) {
    printf("Usage: %s <graph G file> <graph H file> [algorithm]\n", programName);
    printf("\nArguments:\n");
    printf("  graph G file  - The pattern graph to embed\n");
    printf("  graph H file  - The host graph to extend\n");
    printf("  algorithm     - Optional: 'greedy' (default), 'exact', or 'subiso'\n");
    printf("\nExamples:\n");
    printf("  %s graph1.txt graph2.txt          (uses greedy)\n", programName);
    printf("  %s graph1.txt graph2.txt greedy   (uses greedy)\n", programName);
    printf("  %s graph1.txt graph2.txt exact    (uses exact)\n", programName);
    printf("  %s graph1.txt graph2.txt subiso   (subgraph isomorphism check only)\n", programName);
}

static void runGreedyExtension(Graph* G, Graph* H) {
    printf("Running GREEDY extension algorithm...\n");
    
    GreedyExtension* ext = greedy_extension(G, H);
    
    if (ext) {
        int total_cost = ext->newVertexCount + ext->newEdgeCount;
        printf("Greedy Extension Results:\n");
        printf("  New vertices: %d\n", ext->newVertexCount);
        printf("  New edges: %d\n", ext->newEdgeCount);
        printf("  Total cost: %d\n", total_cost);
        
        if (ext->newVertexCount > 0) {
            printf("\n  Vertices to add: ");
            for (int i = 0; i < ext->newVertexCount; i++) {
                printf("%d ", ext->newVertices[i]);
            }
            printf("\n");
        }
        
        if (ext->newEdgeCount > 0) {
            printf("  Edges to add: ");
            for (int i = 0; i < ext->newEdgeCount; i++) {
                printf("(%d-%d) ", ext->newEdges[i][0], ext->newEdges[i][1]);
            }
            printf("\n");
        }
        
        freeGreedyExtension(ext);
    } else {
        printf("Error computing greedy extension.\n");
    }
}

static void runExactExtension(const Graph* G, const Graph* H) {
    printf("Running EXACT extension algorithm...\n");
    
    Extension* ext = exactMinimalExtension(G, H);
    
    if (ext) {
        int total_cost = ext->newVertexCount + ext->newEdgeCount;
        printf("Exact Minimal Extension Results:\n");
        printf("  New vertices: %d\n", ext->newVertexCount);
        printf("  New edges: %d\n", ext->newEdgeCount);
        printf("  Total cost: %d\n", total_cost);
        
        if (ext->newVertexCount > 0) {
            printf("\n  Vertices to add: ");
            for (int i = 0; i < ext->newVertexCount; i++) {
                printf("%d ", ext->newVertices[i]);
            }
            printf("\n");
        }
        
        if (ext->newEdgeCount > 0) {
            printf("  Edges to add: ");
            for (int i = 0; i < ext->newEdgeCount; i++) {
                printf("(%d-%d) ", ext->newEdges[i][0], ext->newEdges[i][1]);
            }
            printf("\n");
        }
        
        freeExtensionObject(ext);
    } else {
        printf("No extension found (this shouldn't happen).\n");
    }
}

int main(int argc, char** argv) {
    const char *fileG, *fileH;
    Algorithm alg = ALG_GREEDY;

    if (argc < 3 || argc > 4) {
        printUsage(argv[0]);
        return 1;
    }

    fileG = argv[1];
    fileH = argv[2];

    if (argc == 4) {
        if (strcmp(argv[3], "exact") == 0) {
            alg = ALG_EXACT;
        } else if (strcmp(argv[3], "greedy") == 0) {
            alg = ALG_GREEDY;
        } else if (strcmp(argv[3], "subiso") == 0) {
            alg = ALG_SUBISO_ONLY;
        } else {
            printf("Unknown algorithm: %s\n", argv[3]);
            printf("Use 'greedy', 'exact', or 'subiso'\n");
            return 1;
        }
    }

    Graph* G = loadGraph(fileG);
    Graph* H = loadGraph(fileH);

    if (!G || !H) {
        printf("Error loading graphs.\n");
        if (G) freeGraph(G);
        if (H) freeGraph(H);
        return 1;
    }

    printf("Graph G: %d vertices, %d edges\n", G->n, G->m);
    printf("Graph H: %d vertices, %d edges\n\n", H->n, H->m);

    bool result = isSubgraphIsomorphic(G, H);

    if (result) {
        printf("G IS isomorphic to a subgraph of H.\n");
        if (alg != ALG_SUBISO_ONLY) {
            printf("No extension needed!\n");
        }
    } else {
        printf("G is NOT isomorphic to any subgraph of H.\n");
        
        if (alg == ALG_SUBISO_ONLY) {
            // Just the isomorphism check, no extension
        } else {
            printf("Computing minimal extension...\n\n");
            
            if (alg == ALG_GREEDY) {
                runGreedyExtension(G, H);
            } else {
                runExactExtension(G, H);
            }
        }
    }

    freeGraph(G);
    freeGraph(H);
    return 0;
}
