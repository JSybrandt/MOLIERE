#!/software/python/3.4/bin/python3
"""
Requires Python 3.4
Requires networkit module
Takes a graph file and produces centrality measures for each node
"""

import argparse
import sys
from networkit import *


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-g", "--graph",
                        action="store",
                        dest="graphPath",
                        help="file path of the graph edge file")
    parser.add_argument("-o", "--out",
                        action="store",
                        dest="outPath",
                        help="dir path of result files")
    parser.add_argument("-s", "--samples",
                        action="store",
                        dest="samples",
                        default=100,
                        help="dir path of result files")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    args = parser.parse_args()
    verbose = args.verbose
    outPath = args.outPath
    graphPath = args.graphPath
    samples = args.samples

    if verbose:
        print("Reading", graphPath, "as EdgeListSpaceZero")
    graph = readGraph(graphPath, Format.EdgeListSpaceZero)
    if verbose:
        print("Loaded:")
        overview(graph)

    centRunner =  centrality.EstimateBetweenness(graph, samples,
                                                 normalized=True,
                                                 parallel=True)
    centRunner.run()
    with open(outPath, "w") as outFile:
        count = 0
        for score in centRunner.scores():
            outFile.write("{} {}\n".format(count, score))
            count += 1


if __name__ == "__main__":
    main()
