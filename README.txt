================================================================================
                              AAC-PROJECT
         Subgraph Isomorphism Algorithm and Minimal Graph Extension
================================================================================

BUILDING
--------

Compile all source files with gcc:

    gcc -Wall -O2 -o aac.exe main.c graph.c subiso.c gmext.c exact_extension.c

Or use make:

    make

To compile the graph generator:

    gcc -Wall -O2 -o graphgen.exe graphgen.c


USAGE
-----

Single file mode (both graphs in one file):

    ./aac.exe <input_file> [algorithm]

Two file mode (separate files for each graph):

    ./aac.exe <graph_G_file> <graph_H_file> [algorithm]

Arguments:
    input_file    - File containing both graphs (G and H) in adjacency matrix format
    graph_G_file  - File containing graph G (pattern) in adjacency matrix format
    graph_H_file  - File containing graph H (host) in adjacency matrix format
    algorithm     - Optional: 'greedy' (default), 'exact', or 'subiso'

Algorithms:
    greedy  - Approximate Minimal Extension Algorithm
    exact   - Exact Minimal Extension Algorithm
    subiso  - Only check subgraph isomorphism


EXAMPLES
--------

    # Single file mode
    ./aac.exe input.txt              # Uses greedy
    ./aac.exe input.txt exact        # Uses exact algorithm
    ./aac.exe input.txt subiso       # Only isomorphism check

    # Two file mode
    ./aac.exe graphG.txt graphH.txt          # Uses greedy
    ./aac.exe graphG.txt graphH.txt exact    # Uses exact algorithm


INPUT FILE FORMAT
-----------------

The input file contains descriptions of two graphs in adjacency matrix format:

    <number_of_vertices_G>
    <adjacency_matrix_row_1 for G>
    <adjacency_matrix_row_2 for G>
    ...
    <adjacency_matrix_row_n for G>
    <number_of_vertices_H>
    <adjacency_matrix_row_1 for H>
    <adjacency_matrix_row_2 for H>
    ...
    <adjacency_matrix_row_m for H>


Example (G = 3-vertex path, H = 4-vertex cycle):

    3
    0 1 0
    1 0 1
    0 1 0
    4
    0 1 0 1
    1 0 1 0
    0 1 0 1
    1 0 1 0


GRAPH GENERATOR
---------------

The graph generator outputs graphs in adjacency matrix format.

Generate a single graph:

    ./graphgen.exe single <type> <output_file> <n> [k]

Generate combined input file (two graphs for aac):

    ./graphgen.exe combined <output_file> <type1> <n1> [k1] <type2> <n2> [k2]

Graph Types:
    path     - Path graph with n vertices (n-1 edges)
    cycle    - Cycle graph with n vertices (n edges)
    complete - Complete graph K_n (n(n-1)/2 edges)
    random   - Random connected graph with n vertices and k edges
               Constraint: n-1 <= k <= n(n-1)/2

Examples:

    # Single graph generation
    ./graphgen.exe single path out.txt 5
    ./graphgen.exe single cycle out.txt 6
    ./graphgen.exe single complete out.txt 4
    ./graphgen.exe single random out.txt 8 12

    # Combined input file generation (for use with aac)
    ./graphgen.exe combined input.txt path 4 cycle 6
    ./graphgen.exe combined input.txt random 5 7 complete 6
    ./graphgen.exe combined input.txt complete 4 random 8 15

================================================================================
