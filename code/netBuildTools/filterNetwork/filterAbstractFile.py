#!/software/python/3.4/bin/python


import sqlite3
import argparse


def getPmidYear(pmid, cursor):
    cursor.execute("""
    select year from abstracts where pmid={};
    """.format(pmid))
    return cursor.fetchone()[0]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--database",
                        action="store",
                        dest="databasePath",
                        help="file path of the sqlite database")
    parser.add_argument("-a", "--absFile",
                        action="store",
                        dest="absFile",
                        help="file path of the abstract file")
    parser.add_argument("-o", "--outFile",
                        action="store",
                        dest="outFile",
                        help="file path of the filtered file")
    parser.add_argument("-y", "--year",
                        action="store",
                        dest="year",
                        help="first year to NOT include in resulting file")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    args = parser.parse_args()

    verbose = args.verbose
    absFilePath = args.absFile
    outFilePath = args.outFile
    filterYear = int(args.year)
    connection = sqlite3.connect(args.databasePath)
    cursor = connection.cursor()

    with open(absFilePath, "r") as absFile, open(outFilePath, "w") as outFile:
        for line in absFile:
            pmid, data = line.split(" ", 1)
            pmid = pmid.strip()
            pmid = pmid[4:]  # strip off the PMID part of the label
            data = data.strip()
            if verbose:
                print("Found pmid {}".format(pmid))
            year = getPmidYear(pmid, cursor)
            if year is None:
                print("COULDN'T FIND PMID:{}".format(pmid))
                # magic number for 2013
                if int(pmid) < 22989000:
                    outFile.write("PMID{} {}\n".format(pmid, data))
                continue
            if verbose:
                print("PMID {} published in year {}".format(pmid, year))
            if int(year) < filterYear:
                outFile.write("PMID{} {}\n".format(pmid, data))


if __name__ == "__main__":
    main()
