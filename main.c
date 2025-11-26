#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "graph.h"
#include "subiso.h"
#include "gmext.h"

int main(int argc, char** argv) {
    const char *fileG, *fileH;

    if (argc == 3) {
        fileG = argv[1];
        fileH = argv[2];
    } else {
        printf("Usage: %s <graph G file> <graph H file>\n", argv[0]);
        printf("Checks if G is isomorphic to a subgraph of H.\n");
        return 1;
    }

    Graph* G = loadGraph(fileG);
    Graph* H = loadGraph(fileH);

    if (!G || !H) {
        printf("Error loading graphs.\n");
        return 1;
    }

    bool result = isSubgraphIsomorphic(G, H);

    if (result)
        printf("G IS isomorphic to a subgraph of H.\n");
    else
    {
        printf("G is NOT isomorphic to any subgraph of H.\n");
        int extension_cost = greedy_extension(G, H);
        printf("Minimal Extension Cost: %d\n", extension_cost);
    }

    freeGraph(G);
    freeGraph(H);
    return 0;
}
