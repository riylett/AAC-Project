#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void generate_path(const char* filename, int n) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Error opening file.\n");
        exit(1);
    }

    fprintf(f, "%d %d\n", n, n - 1);
    for (int i = 0; i < n - 1; i++)
        fprintf(f, "%d %d\n", i, i + 1);

    fclose(f);
}

void generate_complete(const char* filename, int n) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Error opening file.\n");
        exit(1);
    }

    int edges = n * (n - 1) / 2;
    fprintf(f, "%d %d\n", n, edges);

    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            fprintf(f, "%d %d\n", i, j);

    fclose(f);
}

void generate_random_connected(const char* filename, int n, int k) {
    if (k < n - 1 || k > (n * (n - 1)) / 2) {
        printf("Invalid k: must be between n-1 and n(n-1)/2.\n");
        exit(1);
    }

    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Error opening file.\n");
        exit(1);
    }

    int max_edges = n * (n - 1) / 2;
    int edges_added = 0;

    // adjacency matrix to avoid duplicates
    int** used = malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        used[i] = calloc(n, sizeof(int));
    }

    // Write header later
    fprintf(f, "%d %d\n", n, k);

    // 1. Generate a random tree to guarantee connectivity
    for (int i = 1; i < n; i++) {
        int parent = rand() % i;
        fprintf(f, "%d %d\n", parent, i);
        used[parent][i] = used[i][parent] = 1;
        edges_added++;
    }

    // 2. Add random edges until reaching k
    while (edges_added < k) {
        int u = rand() % n;
        int v = rand() % n;
        if (u == v) continue;
        if (used[u][v]) continue;

        used[u][v] = used[v][u] = 1;
        fprintf(f, "%d %d\n", u, v);
        edges_added++;
    }

    // Cleanup
    for (int i = 0; i < n; i++) free(used[i]);
    free(used);

    fclose(f);
}

void print_usage() {
    printf("Usage:\n");
    printf("  ./aac path <output_file> <n>\n");
    printf("  ./aac complete <output_file> <n>\n");
    printf("  ./aac random <output_file> <n> <k>\n");
}

int main(int argc, char** argv) {
    srand(time(NULL));

    if (argc < 4) {
        print_usage();
        return 1;
    }

    char* mode = argv[1];
    char* filename = argv[2];
    int n = atoi(argv[3]);

    if (n <= 1) {
        printf("n must be >= 2\n");
        return 1;
    }

    if (strcmp(mode, "path") == 0) {
        if (argc != 4) {
            print_usage();
            return 1;
        }
        generate_path(filename, n);
    }
    else if (strcmp(mode, "complete") == 0) {
        if (argc != 4) {
            print_usage();
            return 1;
        }
        generate_complete(filename, n);
    }
    else if (strcmp(mode, "random") == 0) {
        if (argc != 5) {
            print_usage();
            return 1;
        }
        int k = atoi(argv[4]);
        generate_random_connected(filename, n, k);
    }
    else {
        print_usage();
        return 1;
    }

    return 0;
}