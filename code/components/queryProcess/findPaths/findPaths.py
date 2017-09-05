#!/software/python/3.4/bin/python3

import argparse

# for readgraph
import networkit as nk

from multiprocessing import Lock, Pool


def runProbGroup(probGroup):
    global graph, outLock, verbose

    (source, targets) = probGroup
    if len(targets) == 1:
        dijk = nk.distance.Dijkstra(graph, source, target=targets[0])
    else:
        dijk = nk.distance.Dijkstra(graph, source)

    dijk.run()

    outLock.acquire()
    for target in targets:
        path = dijk.getPath(target)
        print(" ".join([str(x) for x in path]))
    outLock.release()


def main():
    global graph, outLock, verbose
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--problem-desc",
                        action="store",
                        dest="probPath",
                        help="Pairs of labels that we want")
#    parser.add_argument("-o", "--out-file",
#                        action="store",
#                        dest="outPath")
    parser.add_argument("-e", "--graph-file-edges",
                        action="store",
                        dest="edgePath")
    parser.add_argument("-l", "--graph-file-labels",
                        action="store",
                        dest="labelPath")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    args = parser.parse_args()
    verbose = args.verbose
#    outPath = args.outPath
    edgePath = args.edgePath
    labelPath = args.labelPath
    probDescPath = args.probPath

    if verbose:
        print("Loading labels from ", labelPath)

    count = 0
    lbl2idx = {}
    with open(labelPath, "r") as lblFile:
        for line in lblFile:
            label = line.strip()
            lbl2idx[label] = count
            count += 1

    if verbose:
        print("Loaded ", len(lbl2idx), " labels")
        print("Loading problem from ", probDescPath)

    problem = {}
    count = 0
    with open(probDescPath, "r") as probFile:
        for line in probFile:
            count += 1
            lblA, lblB = line.split()
            if lblA in lbl2idx and lblB in lbl2idx:
                idxA, idxB = (lbl2idx[i] for i in line.split())
                # always go from smaller -> larger, helps merge
                if(idxA > idxB):
                    (idxA, idxB) = (idxB, idxA)
                if idxA not in problem:
                    problem[idxA] = []
                problem[idxA].append(idxB)

    if verbose:
        print("Loaded ", count, " prob pairs.")
        print("Grouped into ", len(problem), " query groups.")
        print("Constructing network from ", edgePath)

    graph = nk.readGraph(edgePath, nk.Format.EdgeListSpaceZero)
    outLock = Lock()

    with Pool() as pool:
        pool.map(runProbGroup, problem.items())


if __name__ == "__main__":
    main()

