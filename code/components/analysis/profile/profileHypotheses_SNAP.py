#!/software/python/2.7.13/bin/python
"""
Requires Python 2.7
Requires networkit module
Takes a graph file and produces centrality measures for each node
"""

import argparse
from pprint import pprint
from multiprocessing.pool import ThreadPool
from multiprocessing import Lock
import snap


def DijkData(dijkPath):
    with open(dijkPath, "r") as dijkFile:
        while(True):
            line1 = dijkFile.readline()
            line2 = dijkFile.readline()
            if(not line2):
                break
            pathTokens = line1.split()
            d = {}
            d["start"] = pathTokens[1]
            d["end"] = pathTokens[3]
            d["weight"] = pathTokens[-1]
            d["path"] = pathTokens[5:-2]
            d["neigh"] = line2
            yield d


def profile(dijkData):
    global labels, fullGraph, centralities, outFile, outLock, verbose

    if verbose:
        print("Processing:")
        pprint(dijkData)

    startIdx = int(dijkData["start"])
    endIdx = int(dijkData["end"])
    weight = dijkData["weight"]
    pathIdx = [int(c) for c in dijkData["path"]]
    cloudIdx = [int(c) for c in dijkData["neigh"].split()]

    nodes = snap.TIntV()
    for idx in cloudIdx:
        nodes.Add(idx)

    startLbl = labels[startIdx]
    endLbl = labels[endIdx]
    nHops = len(pathIdx)
    cloudSize = len(cloudIdx)

    subGraph = snap.GetSubGraph(fullGraph, nodes)
    snap.PrintInfo(subGraph, startLbl+"---"+endLbl, "/dev/stdout", False)

#    if verbose:
#        print(profile)
#    outLock.acquire()
#    outFile.write(profile)
#    outLock.release()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-e", "--edge-file",
                        action="store",
                        dest="edgePath",
                        help="file path of the FULL edge list.")
    parser.add_argument("-l", "--label-file",
                        action="store",
                        dest="labelPath",
                        help="file path of the FULL label file.")
    parser.add_argument("-d", "--dijkstra-file",
                        action="store",
                        dest="dijkPath",
                        help="file path of the dijkstra file.")
    parser.add_argument("-c", "--centrality-file",
                        action="store",
                        dest="centralityPath",
                        help="file path of the dijkstra file.")
    parser.add_argument("-o", "--out-file",
                        action="store",
                        dest="outPath",
                        help="file path of the resulting profile file.")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    global labels, fullGraph, centralities, outFile, outLock, verbose

    args = parser.parse_args()
    verbose = args.verbose
    outPath = args.outPath
    edgePath = args.edgePath
    labelPath = args.labelPath
    dijkPath = args.dijkPath
    centralityPath = args.centralityPath

    outFile = open(outPath, "w")
    outLock = Lock()

    with open(labelPath, "r") as labelFile:
        labels = [l.strip() for l in labelFile]
    with open(centralityPath, "r") as centFile:
        centralities = [float(l.split()[1]) for l in centFile]
    fullGraph = snap.LoadEdgeList(snap.PUNGraph, edgePath, 0, 1, " ")

    if verbose:
        snap.PrintInfo(fullGraph, "FULL GRAPH", "/dev/stdout", False)

    for d in DijkData(dijkPath):
        profile(d)

    # with ThreadPool() as pool:
    #   pool.map(profile, [d for d in DijkData(dijkPath)])

    outFile.close()


if __name__ == "__main__":
    main()
