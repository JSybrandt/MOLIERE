#!/software/python/3.4/bin/python
"""
Requires Python 3.4
Requires sqlite3
Finds publish dates from medline xml and adds to db
"""

import sqlite3
import argparse
from multiprocessing.pool import ThreadPool


class bagOfWords:
    def __init__(self, text):
        text = text.strip()
        self.bag = {}
        for word in text.split():
            if word not in self.bag:
                self.bag[word] = 1
            else:
                self.bag[word] += 1

    def __str__(self):
        return " ".join([w+" "+str(c) for w, c in self.bag.items()])


#class bagLoader:
#    def __init__(self, dbPath):
#        if(verbose):
#            print("Connecting:", dbPath)
#        self.connection = sqlite3.connect(dbPath)
#        self.cursor = self.connection.cursor()
#        self.pmid2bag = {}
#
#    def __del__(self):
#        self.connection.close()
#
#    def getBag(self, pmid):
#        if(verbose):
#            print("Looking for bag", pmid)
#        if pmid not in self.pmid2bag:
#            lock.acquire()
#            # double checking
#            if pmid not in self.pmid2bag:
#                self.pmid2bag[pmid] = bagOfWords(self._queryAbstract(pmid))
#            lock.release()
#        return self.pmid2bag[pmid]
#
#    def _queryAbstract(self, pmid):
#        if(verbose):
#            print("Running query for", pmid)
#        self.cursor.execute("""
#        SELECT content FROM abstracts WHERE pmid={};
#        """.format(pmid))
#        data = self.cursor.fetchone()
#        if(data):
#            return data[0]
#        return ""


class labelNum2Data:
    def __init__(self, labelPath):
        if(verbose):
            print("Loading labels from", labelPath)
        with open(labelPath, "r") as labelFile:
            self.l2d = [line.strip() for line in labelFile]

    def getData(self, labelNum):
        return self.l2d[int(labelNum)]


def getShortestPathJob(dijkPath):
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
            d["neigh"] = line2
            yield d


def processPath(path):
    global verbose, label2Data, outDirPath, pmid2bag
    if(verbose):
        print("Starting:", path["start"], path["end"])
        print("loaded", len(pmid2bag), "pmids")
    outPath = outDirPath + "/" + path["start"] + "-" + path["end"]
    if(verbose):
        print("Processing:", outPath)

    with open(outPath, "w") as outFile:
        # remember, the path has a bunch of line numbers (-1)
        for pmidId in [
                label2Data.getData(l.strip()) for l in path["neigh"].split()]:
            bag = pmid2bag[pmidId]
            outFile.write(str(bag) + "\n")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-l", "--labelPath",
                        action="store",
                        dest="labelPath",
                        help="file path of the graph label file")
    parser.add_argument("-p", "--dijkstraResPath",
                        action="store",
                        dest="dijkPath",
                        help="file path of the dijkstra results")
    parser.add_argument("-o", "--outDir",
                        action="store",
                        dest="outDirPath",
                        help="dir path of result files")
    parser.add_argument("-a", "--abstractFile",
                        action="store",
                        dest="abstractPath",
                        help="file path of the abstract file")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    args = parser.parse_args()

    global verbose, label2Data, outDirPath, pmid2bag

    verbose = args.verbose
    label2Data = labelNum2Data(args.labelPath)
    outDirPath = args.outDirPath
    if(verbose):
        print("Loading PMID bags")
    pmid2bag = {}
    with open(args.abstractPath, "r") as abFile:
        for line in abFile:
            tokens = line.split(" ", 1)
            if(len(tokens) == 2):
                (pmid, text) = tokens
                pmid2bag[pmid] = bagOfWords(text)
    with ThreadPool() as pool:
        pool.map(processPath, [d for d in getShortestPathJob(args.dijkPath)])


if __name__ == "__main__":
    main()
