#!/software/python/3.4/bin/python
"""
Requires Python 3.4
Requires sqlite3
Creates a database given a file of abstracts

./populateAbstractDB.py
-d /zfs/safrolab/users/jsybran/moliere/data/database/abstracts.db
-a /zfs/safrolab/users/jsybran/moliere/data/processedText/abstracts.txt
"""

import sqlite3
import argparse


class abstract:
    def __init__(self, text):
        (self.pmid, self.text) = text.split(" ", 1)

    def __str__(self):
        return "PMID: {} TEXT: {}".format(self.pmid, self.text)


def createTable(cursor):
    cursor.execute("""
    CREATE TABLE IF NOT EXISTS abstracts (
        pmid UNSIGNED BIG INT PRIMARY KEY,
        content TEXT,
        year INT
    );""")


def dropTable(cursor):
    cursor.execute("""
    DROP TABLE abstracts;
    """)


def addAbstract(cursor, abstract):
    cursor.execute("""
    INSERT OR REPLACE INTO abstracts(pmid, content) VALUES ({}, "{}");
    """.format(abstract.pmid, abstract.text))


def getAbstracts(abFile):
    for line in abFile:
        # if line has multiple words
        if " " in line:
            yield abstract(line)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--database",
                        action="store",
                        dest="databasePath",
                        help="file path of the sqlite database")
    parser.add_argument("-a", "--abstracts",
                        action="store",
                        dest="abstractPath",
                        help="file path of the abstract file")
    parser.add_argument("-r", "--refresh",
                        action="store_true",
                        dest="refresh",
                        help="if refresh, drop abstract table")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    args = parser.parse_args()

    connection = sqlite3.connect(args.databasePath)
    cursor = connection.cursor()

    if(args.refresh):
        dropTable(cursor)
    createTable(cursor)

    with open(args.abstractPath, "r") as abFile:
        for ab in getAbstracts(abFile):
            if(args.verbose):
                print(ab)
            addAbstract(cursor, ab)
    connection.commit()
    connection.close()


if __name__ == "__main__":
    main()
