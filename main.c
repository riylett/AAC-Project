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
    printf("Usage:\n");
    printf("  %s <input_file> [algorithm]           - Single file with both graphs\n", programName);
    printf("  %s <graph_G> <graph_H> [algorithm]    - Two separate graph files\n", programName);
    printf("\nArguments:\n");
    printf("  input_file  - File containing both graphs (G and H) in adjacency matrix format\n");
    printf("  graph_G     - File containing graph G (pattern) in adjacency matrix format\n");
    printf("  graph_H     - File containing graph H (host) in adjacency matrix format\n");
    printf("  algorithm   - Optional: 'greedy' (default), 'exact', or 'subiso'\n");
    printf("\nAlgorithms:\n");
    printf("  greedy  - Approximate Minimal Extension Algorithm\n");
    printf("  exact   - Exact Minimal Extension Algorithm\n");
    printf("  subiso  - Only check subgraph isomorphism\n");
    printf("\nExamples:\n");
    printf("  %s combined.txt              (single file, uses greedy)\n", programName);
    printf("  %s combined.txt exact        (single file, uses exact)\n", programName);
    printf("  %s graphG.txt graphH.txt     (two files, uses greedy)\n", programName);
    printf("  %s graphG.txt graphH.txt exact (two files, uses exact)\n", programName);
}

static void printSeparator(void) {
    printf("------------------------------------------------\n");
}

// Helper function to print the extended graph's adjacency matrix
// Only prints if the extended graph has 20 or fewer vertices
// Highlights new vertices and new edges
static void printExtendedMatrix(const Graph* H, int newVertexCount, 
                                 int (*newEdges)[2], int newEdgeCount) {
    int newN = H->n + newVertexCount;
    
    // Count edges first (we need this for the summary)
    int edgeCount = H->m + newEdgeCount;
    
    // Always print the summary
    printf("\n  [EXTENDED GRAPH H']\n\n");
    printf("    H' has %d vertices and %d edges\n", newN, edgeCount);
    printf("    (Added %d new vertices and %d new edges)\n", newVertexCount, newEdgeCount);
    
    // Only print adjacency matrix for small graphs (20 or fewer vertices)
    if (newN > 20) {
        printf("\n    (Adjacency matrix not shown for graphs with more than 20 vertices)\n");
        return;
    }
    
    // Allocate extended matrix
    int** extMatrix = malloc(newN * sizeof(int*));
    for (int i = 0; i < newN; i++) {
        extMatrix[i] = calloc(newN, sizeof(int));
    }
    
    // Track which edges are new (for highlighting)
    int** isNewEdge = malloc(newN * sizeof(int*));
    for (int i = 0; i < newN; i++) {
        isNewEdge[i] = calloc(newN, sizeof(int));
    }
    
    // Copy original H matrix
    for (int i = 0; i < H->n; i++) {
        for (int j = 0; j < H->n; j++) {
            extMatrix[i][j] = H->matrix[i][j];
        }
    }
    
    // Add new edges and mark them
    for (int i = 0; i < newEdgeCount; i++) {
        int u = newEdges[i][0];
        int v = newEdges[i][1];
        extMatrix[u][v] = 1;
        extMatrix[v][u] = 1;
        isNewEdge[u][v] = 1;
        isNewEdge[v][u] = 1;
    }
    
    // Print the matrix
    printf("\n    Adjacency Matrix:\n");
    
    // Print column headers
    printf("       ");
    for (int j = 0; j < newN; j++) {
        printf("%3d ", j);
    }
    printf("\n       ");
    for (int j = 0; j < newN; j++) {
        printf("----");
    }
    printf("\n");
    
    // Print matrix rows with row labels
    for (int i = 0; i < newN; i++) {
        printf("   %2d| ", i);
        
        for (int j = 0; j < newN; j++) {
            if (extMatrix[i][j] && isNewEdge[i][j]) {
                printf(" %d* ", extMatrix[i][j]);  // Mark new edges with *
            } else {
                printf(" %d  ", extMatrix[i][j]);
            }
        }
        
        if (i >= H->n) {
            printf(" <- NEW");
        }
        printf("\n");
    }
    
    // Print legend
    printf("\n    * = new edge, NEW = new vertex\n");
    
    // Free matrices
    for (int i = 0; i < newN; i++) {
        free(extMatrix[i]);
        free(isNewEdge[i]);
    }
    free(extMatrix);
    free(isNewEdge);
}

// Print adjacency matrix for an input graph (with an optional title)
static void printAdjacencyMatrix(const Graph* g, const char* title) {
    if (!g) return;

    printf("\n  +-------------------------------------------+\n");
    printf("  |  GRAPH %s (Adjacency Matrix)\n", title);
    printf("  +-------------------------------------------+\n");
    printf("    Vertices: %d    Edges: %d\n\n", g->n, g->m);

    // Avoid printing extremely large matrices
    if (g->n > 80) {
        printf("    (Adjacency matrix not shown for graphs with more than 80 vertices)\n");
        return;
    }

    // Print column headers
    printf("       ");
    for (int j = 0; j < g->n; j++) {
        printf("%3d ", j);
    }
    printf("\n       ");
    for (int j = 0; j < g->n; j++) printf("----");
    printf("\n");

    for (int i = 0; i < g->n; i++) {
        printf("   %2d| ", i);
        for (int j = 0; j < g->n; j++) {
            printf(" %d  ", g->matrix[i][j]);
        }
        printf("\n");
    }

}

static void runGreedyExtension(Graph* G, Graph* H) {
    printf("\n");
    printSeparator();
    printf("  GREEDY EXTENSION ALGORITHM\n");
    printSeparator();
    
    GreedyExtension* ext = greedy_extension(G, H);
    
    if (ext) {
        int total_cost = ext->newVertexCount + ext->newEdgeCount;
        
        printf("\n  [RESULTS]\n\n");
        printf("    New vertices needed : %d\n", ext->newVertexCount);
        printf("    New edges needed    : %d\n", ext->newEdgeCount);
        printf("    -------------------------\n");
        printf("    TOTAL COST          : %d\n", total_cost);
        
        // Display metric information
        printf("\n  [GRAPH METRICS]\n\n");
        printf("    Size of G (|V|+|E|)       : %d\n", graphSize(G));
        printf("    Size of H (|V|+|E|)       : %d\n", graphSize(H));
        printf("    Size of H' (extended)     : %d\n", graphSize(H) + ext->newVertexCount + ext->newEdgeCount);
        printf("    Extension Distance d(H,H'): %d\n", extensionDistance(G, H, ext->newVertexCount, ext->newEdgeCount));
        
        if (ext->newVertexCount > 0) {
            printf("\n  [VERTICES TO ADD]\n\n    ");
            for (int i = 0; i < ext->newVertexCount; i++) {
                printf("v%d ", ext->newVertices[i]);
            }
            printf("\n");
        }
        
        if (ext->newEdgeCount > 0) {
            printf("\n  [EDGES TO ADD]\n\n    ");
            for (int i = 0; i < ext->newEdgeCount; i++) {
                printf("(%d, %d)", ext->newEdges[i][0], ext->newEdges[i][1]);
                if (i < ext->newEdgeCount - 1) printf(", ");
            }
            printf("\n");
        }
        
        // Print the extended graph's adjacency matrix
        printExtendedMatrix(H, ext->newVertexCount, 
                           ext->newEdges, ext->newEdgeCount);
        
        printf("\n");
        printSeparator();
        freeGreedyExtension(ext);
    } else {
        printf("\n  [ERROR] Failed to compute greedy extension.\n");
        printSeparator();
    }
}

static void runExactExtension(const Graph* G, const Graph* H) {
    printf("\n");
    printSeparator();
    printf("  EXACT MINIMAL EXTENSION ALGORITHM\n");
    printSeparator();
    
    Extension* ext = exactMinimalExtension(G, H);
    
    if (ext) {
        int total_cost = ext->newVertexCount + ext->newEdgeCount;
        
        printf("\n  [RESULTS]\n\n");
        printf("    New vertices needed : %d\n", ext->newVertexCount);
        printf("    New edges needed    : %d\n", ext->newEdgeCount);
        printf("    -------------------------\n");
        printf("    TOTAL COST          : %d\n", total_cost);
        
        // Display metric information
        printf("\n  [GRAPH METRICS]\n\n");
        printf("    Size of G (|V|+|E|)       : %d\n", graphSize(G));
        printf("    Size of H (|V|+|E|)       : %d\n", graphSize(H));
        printf("    Size of H' (extended)     : %d\n", graphSize(H) + ext->newVertexCount + ext->newEdgeCount);
        printf("    Extension Distance d(H,H'): %d\n", extensionDistance(G, H, ext->newVertexCount, ext->newEdgeCount));
        
        if (ext->newVertexCount > 0) {
            printf("\n  [VERTICES TO ADD]\n\n    ");
            for (int i = 0; i < ext->newVertexCount; i++) {
                printf("v%d ", ext->newVertices[i]);
            }
            printf("\n");
        }
        
        if (ext->newEdgeCount > 0) {
            printf("\n  [EDGES TO ADD]\n\n    ");
            for (int i = 0; i < ext->newEdgeCount; i++) {
                printf("(%d, %d)", ext->newEdges[i][0], ext->newEdges[i][1]);
                if (i < ext->newEdgeCount - 1) printf(", ");
            }
            printf("\n");
        }
        
        // Print the extended graph's adjacency matrix
        printExtendedMatrix(H, ext->newVertexCount, 
                           ext->newEdges, ext->newEdgeCount);
        
        printf("\n");
        printSeparator();
        freeExtensionObject(ext);
    } else {
        printf("\n  [ERROR] No extension found.\n");
        printSeparator();
    }
}

int main(int argc, char** argv) {
    Algorithm alg = ALG_GREEDY;
    Graph *G = NULL, *H = NULL;
    const char *inputInfo = NULL;
    int twoFileMode = 0;

    if (argc < 2 || argc > 4) {
        printUsage(argv[0]);
        return 1;
    }

    // Determine if we're using single file or two file mode
    // Check if argv[2] is an algorithm name or a filename
    if (argc >= 3) {
        if (strcmp(argv[2], "exact") == 0 || strcmp(argv[2], "greedy") == 0 || strcmp(argv[2], "subiso") == 0) {
            // Single file mode: argv[1] = combined file, argv[2] = algorithm
            twoFileMode = 0;
        } else {
            // Two file mode: argv[1] = G, argv[2] = H
            twoFileMode = 1;
        }
    }

    if (twoFileMode) {
        // Two separate files mode
        const char *fileG = argv[1];
        const char *fileH = argv[2];
        
        // Parse algorithm if provided
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
        
        G = loadGraph(fileG);
        if (!G) {
            printf("[ERROR] Failed to load graph G from file: %s\n", fileG);
            return 1;
        }
        
        H = loadGraph(fileH);
        if (!H) {
            printf("[ERROR] Failed to load graph H from file: %s\n", fileH);
            freeGraph(G);
            return 1;
        }
        
        // Create input info string for display
        static char infoBuffer[512];
        snprintf(infoBuffer, sizeof(infoBuffer), "%s (G), %s (H)", fileG, fileH);
        inputInfo = infoBuffer;
    } else {
        // Single combined file mode
        const char *inputFile = argv[1];
        
        // Parse algorithm if provided
        if (argc == 3) {
            if (strcmp(argv[2], "exact") == 0) {
                alg = ALG_EXACT;
            } else if (strcmp(argv[2], "greedy") == 0) {
                alg = ALG_GREEDY;
            } else if (strcmp(argv[2], "subiso") == 0) {
                alg = ALG_SUBISO_ONLY;
            } else {
                printf("Unknown algorithm: %s\n", argv[2]);
                printf("Use 'greedy', 'exact', or 'subiso'\n");
                return 1;
            }
        }
        
        if (loadBothGraphs(inputFile, &G, &H) != 0) {
            printf("[ERROR] Failed to load graphs from file: %s\n", inputFile);
            return 1;
        }
        
        inputInfo = inputFile;
    }

    printf("\n");
    printf("================================================\n");
    printf("     SUBGRAPH ISOMORPHISM & GRAPH EXTENSION     \n");
    printf("================================================\n");
    printf("\n");
    printf("  Input: %s\n", inputInfo);
    printf("  Algorithm : %s\n", alg == ALG_EXACT ? "Exact" : (alg == ALG_GREEDY ? "Greedy" : "Subiso only"));
    printf("\n");
    printf("  +-------------------------------------------+\n");
    printf("  |  GRAPH G (Pattern)                       |\n");
    printf("  |    Vertices: %-4d    Edges: %-4d         |\n", G->n, G->m);
    printf("  |    Size S(G) = |V|+|E| = %-4d            |\n", graphSize(G));
    printf("  +-------------------------------------------+\n");
    printf("  |  GRAPH H (Host)                          |\n");
    printf("  |    Vertices: %-4d    Edges: %-4d         |\n", H->n, H->m);
    printf("  |    Size S(H) = |V|+|E| = %-4d            |\n", graphSize(H));
    printf("  +-------------------------------------------+\n");
    printf("  |  GRAPH DISTANCE METRIC                   |\n");
    printf("  |    d(G, H) = %-4d                        |\n", graphDistance(G, H));
    printf("  +-------------------------------------------+\n");
    printf("\n");

    // Print adjacency matrices for the input graphs
    printAdjacencyMatrix(G, "G");
    printAdjacencyMatrix(H, "H");
    printf("\n");

    printf("  Checking subgraph isomorphism...\n");
    bool result = isSubgraphIsomorphic(G, H);

    if (result) {
        printf("\n  [OK] G is isomorphic to a subgraph of H!\n");
        if (alg != ALG_SUBISO_ONLY) {
            printf("       No extension needed.\n");
        }
        printf("\n================================================\n");
    } else {
        printf("\n  [X] G is NOT isomorphic to any subgraph of H.\n");
        
        if (alg == ALG_SUBISO_ONLY) {
            printf("\n================================================\n");
        } else {
            printf("      Computing minimal extension to make H contain G...\n");
            
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
