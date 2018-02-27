#!/software/python/3.4/bin/python
"""
Requires Python 3.4
Requires sqlite3
Finds publish dates from medline xml and adds to db
"""

import argparse
from multiprocessing import Lock, Pool
from multiprocessing.pool import ThreadPool
import xml.etree.cElementTree as ElementTree
import os
import sys
from pprint import pprint


def getAbstractYear(xmlFile):
    lastSeenPMID = None
    lastSeenYear = None
    for event, elem in ElementTree.iterparse(xmlFile, events=("start", "end")):
        if event == "end" and elem.tag == "MedlineCitation":
            if lastSeenPMID and lastSeenYear:
                yield (lastSeenPMID.strip(), lastSeenYear)
                lastSeenPMID = None
                lastSeenYear = None
            else:
                print("ERR: completed citation without finding pmid / year")
                if lastSeenPMID:
                    print(lastSeenPMID)
        if elem.tag == "Year":
            if elem.text is not None and elem.text != "":
                if lastSeenYear is None:
                    lastSeenYear = int(elem.text)
                elif lastSeenYear > int(elem.text):
                    lastSeenYear = int(elem.text)
        if elem.tag == "PMID":
            lastSeenPMID = elem.text


def processFile(xmlPath):
    global verbose, lock, outFile, numFinished, numTotal

    pmid2year = []
    with open(xmlPath, "r") as xmlFile:
        for pmid, year in getAbstractYear(xmlFile):
            pmid2year.append((pmid, year))

    lock.acquire()
    if(verbose):
        sys.stdout.write('\r')
        sys.stdout.flush()
        sys.stdout.write("{} : Writing: {}".format(100*numFinished/numTotal, xmlPath))
        sys.stdout.flush()
    for pmid, year in pmid2year:
        outFile.write("PMID{} {}\n".format(pmid, year))
    numFinished += 1
    if(verbose):
        sys.stdout.write('\r')
        sys.stdout.flush()
        sys.stdout.write("{} : Wrote: {}".format(100*numFinished/numTotal, xmlPath))
        sys.stdout.flush()
    lock.release()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--outFile",
                        action="store",
                        dest="outPath",
                        help="file path of the resulting file")
    parser.add_argument("-x", "--xmldir",
                        action="store",
                        dest="xmlDPath",
                        help="file path of the xml directory")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    args = parser.parse_args()

    global verbose, lock, outFile, numFinished, numTotal

    verbose = args.verbose
    outPath = args.outPath
    lock = Lock()
    outFile = open(outPath, "w")
    numFinished = 0
    paths = [args.xmlDPath+"/"+x
             for x in os.listdir(args.xmlDPath)
             if x.endswith(".xml")]
    numTotal = len(paths)

    with Pool() as pool:
        pool.map(processFile, paths)

    outFile.close()


if __name__ == "__main__":
    main()
