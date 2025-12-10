#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Helper function to write adjacency matrix to file
void write_adjacency_matrix(FILE* f, int** matrix, int n) {
    fprintf(f, "%d\n", n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (j > 0) fprintf(f, " ");
            fprintf(f, "%d", matrix[i][j]);
        }
        fprintf(f, "\n");
    }
}

// Allocate n x n matrix initialized to 0
int** alloc_matrix(int n) {
    int** m = malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        m[i] = calloc(n, sizeof(int));
    }
    return m;
}

void free_matrix(int** m, int n) {
    for (int i = 0; i < n; i++) free(m[i]);
    free(m);
}

// Generate path graph and return adjacency matrix
int** generate_path_matrix(int n) {
    int** matrix = alloc_matrix(n);
    for (int i = 0; i < n - 1; i++) {
        matrix[i][i + 1] = 1;
        matrix[i + 1][i] = 1;
    }
    return matrix;
}

// Generate cycle graph and return adjacency matrix
int** generate_cycle_matrix(int n) {
    int** matrix = alloc_matrix(n);
    for (int i = 0; i < n; i++) {
        int next = (i + 1) % n;
        matrix[i][next] = 1;
        matrix[next][i] = 1;
    }
    return matrix;
}

// Generate complete graph and return adjacency matrix
int** generate_complete_matrix(int n) {
    int** matrix = alloc_matrix(n);
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            matrix[i][j] = 1;
            matrix[j][i] = 1;
        }
    }
    return matrix;
}

// Generate random connected graph and return adjacency matrix
int** generate_random_matrix(int n, int k) {
    if (k < n - 1 || k > (n * (n - 1)) / 2) {
        printf("Invalid k: must be between n-1 and n(n-1)/2.\n");
        exit(1);
    }

    int** matrix = alloc_matrix(n);
    int edges_added = 0;

    // 1. Generate a random tree to guarantee connectivity
    for (int i = 1; i < n; i++) {
        int parent = rand() % i;
        matrix[parent][i] = 1;
        matrix[i][parent] = 1;
        edges_added++;
    }

    // 2. Add random edges until reaching k
    while (edges_added < k) {
        int u = rand() % n;
        int v = rand() % n;
        if (u == v) continue;
        if (matrix[u][v]) continue;

        matrix[u][v] = 1;
        matrix[v][u] = 1;
        edges_added++;
    }

    return matrix;
}

void generate_single_graph(const char* filename, const char* type, int n, int k) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Error opening file.\n");
        exit(1);
    }

    int** matrix = NULL;
    if (strcmp(type, "path") == 0) {
        matrix = generate_path_matrix(n);
    } else if (strcmp(type, "cycle") == 0) {
        matrix = generate_cycle_matrix(n);
    } else if (strcmp(type, "complete") == 0) {
        matrix = generate_complete_matrix(n);
    } else if (strcmp(type, "random") == 0) {
        matrix = generate_random_matrix(n, k);
    }

    write_adjacency_matrix(f, matrix, n);
    free_matrix(matrix, n);
    fclose(f);
}

void generate_combined_input(const char* filename, 
                              const char* type1, int n1, int k1,
                              const char* type2, int n2, int k2) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Error opening file.\n");
        exit(1);
    }

    // Generate first graph (G)
    int** matrix1 = NULL;
    if (strcmp(type1, "path") == 0) {
        matrix1 = generate_path_matrix(n1);
    } else if (strcmp(type1, "cycle") == 0) {
        matrix1 = generate_cycle_matrix(n1);
    } else if (strcmp(type1, "complete") == 0) {
        matrix1 = generate_complete_matrix(n1);
    } else if (strcmp(type1, "random") == 0) {
        matrix1 = generate_random_matrix(n1, k1);
    }

    write_adjacency_matrix(f, matrix1, n1);
    free_matrix(matrix1, n1);

    // Generate second graph (H)
    int** matrix2 = NULL;
    if (strcmp(type2, "path") == 0) {
        matrix2 = generate_path_matrix(n2);
    } else if (strcmp(type2, "cycle") == 0) {
        matrix2 = generate_cycle_matrix(n2);
    } else if (strcmp(type2, "complete") == 0) {
        matrix2 = generate_complete_matrix(n2);
    } else if (strcmp(type2, "random") == 0) {
        matrix2 = generate_random_matrix(n2, k2);
    }

    write_adjacency_matrix(f, matrix2, n2);
    free_matrix(matrix2, n2);

    fclose(f);
}

void print_usage() {
    printf("\n");
    printf("========================================\n");
    printf("    GRAPH GENERATOR (Adjacency Matrix)  \n");
    printf("========================================\n");
    printf("\n");
    printf("  MODES:\n");
    printf("  ------\n");
    printf("\n");
    printf("  1) Generate a single graph:\n");
    printf("     ./graphgen single <type> <output_file> <n> [k]\n");
    printf("\n");
    printf("  2) Generate combined input file (for aac):\n");
    printf("     ./graphgen combined <output_file> <type1> <n1> [k1] <type2> <n2> [k2]\n");
    printf("\n");
    printf("  GRAPH TYPES:\n");
    printf("  ------------\n");
    printf("    path     - Path graph (n vertices, n-1 edges)\n");
    printf("    cycle    - Cycle graph (n vertices, n edges)\n");
    printf("    complete - Complete graph K_n (n vertices, n(n-1)/2 edges)\n");
    printf("    random   - Random connected graph (requires k = edge count)\n");
    printf("               Constraint: n-1 <= k <= n(n-1)/2\n");
    printf("\n");
    printf("  EXAMPLES:\n");
    printf("  ---------\n");
    printf("    ./graphgen single path out.txt 5\n");
    printf("    ./graphgen single cycle out.txt 6\n");
    printf("    ./graphgen single complete out.txt 4\n");
    printf("    ./graphgen single random out.txt 8 12\n");
    printf("\n");
    printf("    ./graphgen combined input.txt path 4 cycle 6\n");
    printf("    ./graphgen combined input.txt random 5 7 complete 6\n");
    printf("\n");
    printf("========================================\n");
    printf("\n");
}

int parse_graph_args(int argc, char** argv, int start, const char** type, int* n, int* k) {
    if (start >= argc) return -1;
    
    *type = argv[start];
    if (strcmp(*type, "path") != 0 && strcmp(*type, "cycle") != 0 &&
        strcmp(*type, "complete") != 0 && strcmp(*type, "random") != 0) {
        printf("Unknown graph type: %s\n", *type);
        return -1;
    }
    
    if (start + 1 >= argc) return -1;
    *n = atoi(argv[start + 1]);
    
    if (*n <= 1) {
        printf("n must be >= 2\n");
        return -1;
    }
    
    *k = 0;
    if (strcmp(*type, "random") == 0) {
        if (start + 2 >= argc) {
            printf("Random graph requires k parameter\n");
            return -1;
        }
        *k = atoi(argv[start + 2]);
        return 3;  // consumed 3 arguments
    }
    
    return 2;  // consumed 2 arguments
}

int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));

    if (argc < 2) {
        print_usage();
        return 1;
    }

    char* mode = argv[1];

    if (strcmp(mode, "single") == 0) {
        if (argc < 5) {
            print_usage();
            return 1;
        }
        
        const char* type = argv[2];
        const char* filename = argv[3];
        int n = atoi(argv[4]);
        int k = 0;
        
        if (n <= 1) {
            printf("n must be >= 2\n");
            return 1;
        }
        
        if (strcmp(type, "random") == 0) {
            if (argc < 6) {
                printf("Random graph requires k parameter\n");
                return 1;
            }
            k = atoi(argv[5]);
        }
        
        generate_single_graph(filename, type, n, k);
        
        int edges = 0;
        if (strcmp(type, "path") == 0) edges = n - 1;
        else if (strcmp(type, "cycle") == 0) edges = n;
        else if (strcmp(type, "complete") == 0) edges = n * (n - 1) / 2;
        else if (strcmp(type, "random") == 0) edges = k;
        
        printf("\n");
        printf("[OK] Generated single graph successfully!\n");
        printf("\n");
        printf("  Type     : %s\n", type);
        printf("  Vertices : %d\n", n);
        printf("  Edges    : %d\n", edges);
        printf("  Output   : %s\n", filename);
        printf("\n");
    }
    else if (strcmp(mode, "combined") == 0) {
        if (argc < 7) {
            print_usage();
            return 1;
        }
        
        const char* filename = argv[2];
        const char *type1, *type2;
        int n1, k1, n2, k2;
        
        int consumed = parse_graph_args(argc, argv, 3, &type1, &n1, &k1);
        if (consumed < 0) {
            print_usage();
            return 1;
        }
        
        int next = 3 + consumed;
        consumed = parse_graph_args(argc, argv, next, &type2, &n2, &k2);
        if (consumed < 0) {
            print_usage();
            return 1;
        }
        
        generate_combined_input(filename, type1, n1, k1, type2, n2, k2);
        
        int edges1 = 0, edges2 = 0;
        if (strcmp(type1, "path") == 0) edges1 = n1 - 1;
        else if (strcmp(type1, "cycle") == 0) edges1 = n1;
        else if (strcmp(type1, "complete") == 0) edges1 = n1 * (n1 - 1) / 2;
        else if (strcmp(type1, "random") == 0) edges1 = k1;
        
        if (strcmp(type2, "path") == 0) edges2 = n2 - 1;
        else if (strcmp(type2, "cycle") == 0) edges2 = n2;
        else if (strcmp(type2, "complete") == 0) edges2 = n2 * (n2 - 1) / 2;
        else if (strcmp(type2, "random") == 0) edges2 = k2;
        
        printf("\n");
        printf("[OK] Generated combined input file successfully!\n");
        printf("\n");
        printf("  Output file: %s\n", filename);
        printf("\n");
        printf("  +----------------------------------------+\n");
        printf("  |  Graph G (Pattern)                     |\n");
        printf("  |    Type     : %-10s              |\n", type1);
        printf("  |    Vertices : %-4d                    |\n", n1);
        printf("  |    Edges    : %-4d                    |\n", edges1);
        printf("  +----------------------------------------+\n");
        printf("  |  Graph H (Host)                        |\n");
        printf("  |    Type     : %-10s              |\n", type2);
        printf("  |    Vertices : %-4d                    |\n", n2);
        printf("  |    Edges    : %-4d                    |\n", edges2);
        printf("  +----------------------------------------+\n");
        printf("\n");
        printf("  Run with: ./aac %s [greedy|exact|subiso]\n", filename);
        printf("\n");
    }
    else {
        print_usage();
        return 1;
    }

    return 0;
}