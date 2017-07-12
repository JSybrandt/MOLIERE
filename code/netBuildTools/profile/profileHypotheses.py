#!/software/python/3.4/bin/python3
"""
Requires Python 3.4
Requires networkit module
Takes a graph file and produces centrality measures for each node
"""

import argparse
from pprint import pprint
from multiprocessing.pool import ThreadPool
from multiprocessing import Lock
from networkit import *


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

    startLbl = labels[startIdx]
    endLbl = labels[endIdx]
    nHops = len(pathIdx)
    cloudSize = len(cloudIdx)

    subGraph = fullGraph.subgraphFromNodes(cloudIdx)

    density = subGraph.density()

    clusterCoef = globals.ClusteringCoefficient.approxGlobal(subGraph, 100)

    outLock.acquire()
    outFile.write("""{start}---{end}
    Num_Hops:           {nHops}
    Path_Length:        {weight}
    Info_Content:       {infoContent}
    Cloud_Size:         {cloudSize}
    Cloud_Density:      {density}
    Clustering_Coef:    {clusterCoef}
    Num_Triangles:      {triangles}
    Shannon_Entropy:    {shannon}
    Mutual_Information: {mutualInfo}
                  """.format(
                      start=startLbl,
                      end=endLbl,
                      nHops=nHops,
                      weight=weight,
                      infoContent="",
                      cloudSize=cloudSize,
                      density=density,
                      clusterCoef=clusterCoef,
                      triangles="",
                      shannon="",
                      mutualInfo="",
                      ))
    outLock.release()


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
    fullGraph = readGraph(edgePath, Format.EdgeListSpaceZero)

    if verbose:
        overview(fullGraph)

    with ThreadPool(1 if verbose else 0) as pool:
        pool.map(profile, [d for d in DijkData(dijkPath)])

    outFile.close()


if __name__ == "__main__":
    main()
