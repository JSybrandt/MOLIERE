#!/software/python/3.4/bin/python
"""
Requires Python 3.4
Requires sqlite3
Finds publish dates from medline xml and adds to db
"""

import sqlite3
import argparse
from multiprocessing import Pool, Lock
import xml.etree.cElementTree as ElementTree
import os


class abstract:
    def __init__(self, text):
        (self.pmid, self.text) = text.split(" ", 1)

    def __str__(self):
        return "PMID: {} TEXT: {}".format(self.pmid, self.text)


def addDate(cursor, pmid, year):
    cursor.execute("""
    UPDATE abstracts SET year={} WHERE pmid={};
    """.format(year, pmid))


def getAbstractYear(xmlFile):
    inCitation = False
    inPubDate = False
    lastSeenPMID = None
    lastSeenYear = None
    for _, elem in ElementTree.iterparse(xmlFile):
        if elem.tag == "MedlineCitation":
            inCitation = not inCitation
        if elem.tag == "PubDate":
            inPubDate = not inPubDate
        if inCitation:
            if elem.tag == "PMID":
                lastSeenPMID = elem.text
            if inPubDate and elem.tag == "Year":
                lastSeenYear = elem.text
        if(lastSeenYear and lastSeenPMID):
            yield (lastSeenPMID, lastSeenYear)
            lastSeenPMID = None
            lastSeenYear = None


def processFile(xmlPath):
    global verbose, dbPath, lock, connection, cursor
    if(verbose):
        print("Processing", xmlPath)

    pmid2year = []
    with open(xmlPath, "r") as xmlFile:
        for pmid, year in getAbstractYear(xmlFile):
            pmid2year.append((pmid, year))

    lock.acquire()
    if(verbose):
        print("Saving data from", xmlPath)
    for pmid, year in pmid2year:
        addDate(cursor, pmid, year)
    connection.commit()
    lock.release()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--database",
                        action="store",
                        dest="databasePath",
                        help="file path of the sqlite database")
    parser.add_argument("-x", "--xmldir",
                        action="store",
                        dest="xmlDPath",
                        help="file path of the xml directory")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    args = parser.parse_args()

    global verbose, dbPath, connection, cursor, lock

    verbose = args.verbose
    dbPath = args.databasePath
    connection = sqlite3.connect(dbPath)
    cursor = connection.cursor()
    lock = Lock()

    with Pool() as pool:
        pool.map(processFile,
                 [args.xmlDPath+"/"+x
                  for x in os.listdir(args.xmlDPath) if x.endswith(".xml")])
    connection.close()


if __name__ == "__main__":
    main()
