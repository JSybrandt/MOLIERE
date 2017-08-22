#!/software/python/3.4/bin/python3
"""
Requires Python 3.4
Requires networkit module
Requires networkx module
Takes a graph file and produces centrality measures for each node
"""

import argparse
import os
import math
import itertools
import powerlaw
from pprint import pprint
from collections import Counter
import multiprocessing
from multiprocessing.pool import ThreadPool
from multiprocessing import Lock, Pool
from networkit import *
from networkit.globals import ClusteringCoefficient
import networkx as nx
import scipy.stats

import numpy as np
np.seterr(divide='ignore', invalid='ignore')


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


class Topic:
    def __init__(self, totalFreq):
        self.totalFreq = totalFreq
        self.words = {}

    def addWord(self, word, freq):
        self.words[word] = freq


def getTopicModels(topicFilePath):
    models = []
    currTopic = Topic(0)
    with open(topicFilePath, "r") as topFile:
        for line in topFile:
            tokens = line.split()
            if len(tokens) > 1:
                if "TOPIC:" == tokens[0]:
                    if currTopic.totalFreq != 0:
                        models.append(currTopic)
                    totalTopicFreq = float(tokens[2])
                    currTopic = Topic(totalTopicFreq)
                else:
                    currTopic.addWord(tokens[0], float(tokens[1]))
        models.append(currTopic)
    return models


def topicInfoContent(topicModels):
    res = []
    for topic in topicModels:
        currInfo = 0.0
        for freq in topic.words.values():
            p = freq / topic.totalFreq
            currInfo += -1 * p * math.log2(p)
        res .append(currInfo)
    return res


def topicCoherence(topicModels, dataFilePath):
    global coherenceCutoff
    beta = 1
    word2DocSet = {}
    docId = 0
    with open(dataFilePath, "r") as docFile:
        for line in docFile:
            docId += 1
            tokens = line.split()
            for word in tokens[::2]:  # every other token is a word
                if word not in word2DocSet:
                    word2DocSet[word] = set()
                word2DocSet[word].add(docId)
    res = []
    for topic in topicModels:
        coherence = 0
        wordList = list(topic.words.keys())[:coherenceCutoff]
        for (wordA, wordB) in itertools.combinations(wordList, 2):
            if wordA in word2DocSet and wordB in word2DocSet:
                setA = word2DocSet[wordA]
                setB = word2DocSet[wordB]
                #  invariant: A has more elements than B
                if len(setB) > len(setA):
                    (setA, setB) = (setB, setA)
                score = len(setA.intersection(setB)) + beta / len(setA)
                coherence += math.log2(score)
        res.append(coherence)
    return res


def profile(dijkData):
    global labels, fullGraph, centralities
    global outFile, outLock, verbose, viewDirPath
    global dataDirPath

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

    hypoName = "{}---{}".format(startLbl, endLbl)
    subGraph = fullGraph.subgraphFromNodes(cloudIdx)
    numEdges = subGraph.numberOfEdges()
    try:
        density = subGraph.density()
    except Exception as e:
        density = "ERR " + str(e)
    try:
        data = [subGraph.degree(x) for x in cloudIdx]
#        data = [x for x in
#                centrality.DegreeCentrality(subGraph).run().scores()
#                if x > 0]
        degDist = sorted(data, reverse=True)
        fit = powerlaw.Fit(degDist)
        degAlpha = fit.alpha
        degSig = fit.sigma
    except Exception as e:
        degAlpha = "ERR " + str(e)
        degSig = "ERR " + str(e)

    try:
        centDist = sorted([centralities[n] for n in
                           subGraph.nodes()], reverse=True)
        fit = powerlaw.Fit(centDist)
        btwnFitAlpha = fit.alpha
        btwnFitSig = fit.sigma
    except Exception as e:
        btwnFitAlpha = "ERR " + str(e)
        btwnFitSig = "ERR " + str(e)

#    try:
#        # expected from casos.cs.cmu.edu/publications/papers/CMU-ISR-08-103.pdf
#        btwnIC = 0
#        expectedDistMu = (1 - density) / cloudSize
#        expectedDistSig = 0
#        if density < 0.35:
#            btwnIC = "Undef."  # there is nothing we can learn
#        elif density < 0.45:
#            expectedDistSig = 0.0019
#        elif density < 0.6:
#            expectedDistSig = 0.0011
#        elif density < 0.8:
#            expectedDistSig = 0.0004
#        else:
#            expectedDistSig = 0.0001
#        if expectedDistSig > 0:
#            expectedDistSig *= 3 ** math.log2(100 / cloudSize)
#            normDist = scipy.stats.norm(expectedDistMu, expectedDistSig)
#            for node in subGraph.nodes():
#                cent = centralities[node]
#                p = normDist.pdf(cent)
#                btwnIC += -1 * p * math.log2(p)
#    except Exception as e:
#        btwnIC = "ERR " + str(e)

    try:
        degreeIC = 0
        for deg, count in Counter(degDist).items():
            p = count / numEdges
            degreeIC += -1 * p * math.log2(p)
    except Exception as e:
        degreeIC = "ERR " + str(e)

    nxGraph = nxadapter.nk2nx(subGraph)
    try:
        #clusterCoef = ClusteringCoefficient.approxGlobal(subGraph, 100)
        clusterCoef = globals.clustering(subGraph)
        # clusterCoef = nx.clustering(nxGraph)
    except Exception as e:
        clusterCoef = "ERR " + str(e)
    try:
        assort = nx.degree_assortativity_coefficient(nxGraph)
    except Exception as e:
        assort = "ERR " + str(e)
    try:
        triangles = 0
        for node, count in nx.triangles(nxGraph).items():
            triangles += count
        triangles /= 3
    except Exception as e:
        triangles = "ERR " + str(e)
#    try:
#        transitivity = nx.transitivity(nxGraph)
#    except Exception as e:
#        transitivity = "ERR " + str(e)

#    try:
#        dispersion = nx.dispersion(nxGraph, u=startIdx, v=endIdx)
#    except Exception as e:
#        dispersion = "ERR " + str(e)

    topicModels = getTopicModels(os.path.join(viewDirPath, hypoName))
    try:
        infoContent = topicInfoContent(topicModels)
        minInfo = min(infoContent)
        maxInfo = max(infoContent)
        meanInfo = sum(infoContent) / len(infoContent)
    except Exception as e:
        minInfo = maxInfo = meanInfo = "ERR " + str(e)

    try:
        coherence = topicCoherence(topicModels,
                                   os.path.join(dataDirPath, hypoName))
        minCoherence = min(coherence)
        maxCoherence = max(coherence)
        meanCoherence = sum(coherence) / len(coherence)
    except Exception as e:
        minCoherence = maxCoherence = meanCoherence = "ERR " + str(e)

    profile = """{hypoName}
    Num_Hops               {nHops}
    Path_Length            {weight}
    Topic_Min_IC           {minInfo}
    Topic_Max_IC           {maxInfo}
    Topic_Mean_IC          {meanInfo}
    Topic_Min_Coherence    {minCoherence}
    Topic_Max_Coherence    {maxCoherence}
    Topic_Mean_Coherence   {meanCoherence}
    Cloud_Size             {cloudSize}
    Cloud_Density          {density}
    Clustering_Coef        {clusterCoef}
    Assortativity          {assort}
    Num_Triangles          {triangles}
    Deg_Fit_Alpha          {degFitAlpha}
    Deg_Fit_Sigma          {degFitSigma}
    Deg_IC                 {degreeIC}
    Btwn_Fit_Alpha         {btwnFitAlpha}
    Btwn_Fit_Sigma         {btwnFitSig}
""".format(
           hypoName=hypoName,
           nHops=nHops,
           weight=weight,
           minInfo=minInfo,
           maxInfo=maxInfo,
           meanInfo=meanInfo,
           minCoherence=minCoherence,
           maxCoherence=maxCoherence,
           meanCoherence=meanCoherence,
           cloudSize=cloudSize,
           density=density,
           clusterCoef=clusterCoef,
           assort=assort,
           triangles=triangles,
           degFitAlpha=degAlpha,
           degFitSigma=degSig,
           degreeIC=degreeIC,
           btwnFitAlpha=btwnFitAlpha,
           btwnFitSig=btwnFitSig,
          )
    if verbose:
        print(profile)
    outLock.acquire()
    outFile.write(profile)
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
    parser.add_argument("-V", "--view-dir",
                        action="store",
                        dest="viewDirPath",
                        help="directory path of topic model views.")
    parser.add_argument("-D", "--data-dir",
                        action="store",
                        dest="dataDirPath",
                        help="directory path of bag of words files.")
    parser.add_argument("-o", "--out-file",
                        action="store",
                        dest="outPath",
                        help="file path of the resulting profile file.")
    parser.add_argument("-C", "--coherence-cutoff",
                        action="store",
                        dest="coherenceCutoff",
                        default=30,
                        help="Number of words to consider when calculating coherence.")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    global labels, fullGraph, centralities
    global outFile, outLock, verbose, viewDirPath
    global dataDirPath, coherenceCutoff

    args = parser.parse_args()
    verbose = args.verbose
    outPath = args.outPath
    edgePath = args.edgePath
    labelPath = args.labelPath
    dijkPath = args.dijkPath
    centralityPath = args.centralityPath
    viewDirPath = args.viewDirPath
    dataDirPath = args.dataDirPath
    coherenceCutoff = args.coherenceCutoff

    if verbose:
        print("Loading Labels...")
    with open(labelPath, "r") as labelFile:
        labels = [l.strip() for l in labelFile]

    if verbose:
        print("Loading Centralities...")
    with open(centralityPath, "r") as centFile:
        centralities = [float(l.split()[1]) for l in centFile]
    if verbose:
        print("Num Centralities: ", len(centralities))

    if verbose:
        print("Loading edge file...")
    fullGraph = readGraph(edgePath, Format.EdgeListSpaceZero)

    if verbose:
        overview(fullGraph)

    outFile = open(outPath, "w")
    outLock = Lock()

    #  nThreads = multiprocessing.cpu_count() * 2
    with Pool() as pool:
        pool.map(profile, [d for d in DijkData(dijkPath)])

    outFile.close()


if __name__ == "__main__":
    main()
