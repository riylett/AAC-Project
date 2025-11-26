# AAC-Project

Subgraph Isomorphism and Graph Extension Tool

## Building

Compile all source files with gcc:

```
gcc -Wall -O2 -o aac.exe main.c graph.c subiso.c gmext.c exact_extension.c
```

To compile the graph generator:

```
gcc -Wall -O2 -o graphgen.exe graphgen.c
```

## Usage

```
./aac.exe <graph_G> <graph_H> [algorithm]
```

**Arguments:**
- `graph_G` - Pattern graph file
- `graph_H` - Host graph file  
- `algorithm` - Optional: `greedy` (default), `exact`, or `subiso`

**Algorithms:**
- `greedy` - Approximate Minimal Extension Algorithm
- `exact` - Exact Minimal Extension Algorithm
- `subiso` - Only check subgraph isomorphism

## Examples

```
./aac.exe graph1.txt graph2.txt              # Uses greedy
./aac.exe graph1.txt graph2.txt exact        # Uses exact algorithm
./aac.exe graph1.txt graph2.txt subiso       # Only isomorphism check
```

## Graph File Format

```
<number_of_vertices> <number_of_edges>
<vertex1> <vertex2>
<vertex1> <vertex2>
...
```

Example (a triangle):
```
3 3
0 1
1 2
2 0
```

## Graph Generator

```
./graphgen.exe <type> <output_file> <n> [extra_param]
```

Types: `path`, `cycle`, `complete`, `random`

Examples:
```
./graphgen.exe path graph.txt 10         # 10-vertex path
./graphgen.exe cycle graph.txt 8         # 8-vertex cycle
./graphgen.exe complete graph.txt 5      # Complete K5
./graphgen.exe random graph.txt 10 15    # 10 vertices, 15 edges
```