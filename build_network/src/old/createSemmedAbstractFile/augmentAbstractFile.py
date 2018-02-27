#!/software/python/3.4/bin/python3
"""
Requires Python 3.4
Requires networkit module
Requires networkx module
Takes a graph file and produces centrality measures for each node
"""

import argparse
from enum import IntEnum

# Example line from predicate file:
# C0021311|Infection|PROCESS_OF|C0020114|Human|1967|5585326
# CUID1    NAME1     VERB       CUID2    NAME2 YEAR PMID


class ColName(IntEnum):
    NAME1 = 0
    VERB = 1
    NAME2 = 2
    PMID = 3
    EXPECTED_PRED_COL = 4

    def parse(line):
        global verbose
        line = line.strip().lower().replace(" ", "_")
        tokens = line.split(":=:")
        if(verbose):
            print(tokens)
        if len(tokens) != ColName.EXPECTED_PRED_COL:
            print("ERROR: MALFORMED PREDICATE FILE")
            print(tokens)
            print("Found:", len(tokens), "columns.")
            print("Expected:", ColName.EXPECTED_PRED_COL, "columns.")
        else:
            return {k: v for k, v in zip(ColName, tokens)}


def predicates(predicatePath):
    with open(predicatePath, "r") as pFile:
        for line in pFile:
            d = ColName.parse(line)
            if(d):
                yield (d[ColName.PMID], " ".join((d[ColName.NAME1],
                                                  d[ColName.VERB],
                                                  d[ColName.NAME2])))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-a", "--abstract-file",
                        action="store",
                        dest="abstractPath",
                        help="file path of the abstract file.")
    parser.add_argument("-p", "--predicate-file",
                        action="store",
                        dest="predicatePath",
                        help="file path of the predicate (output from SEMMEDDB) file.")
    parser.add_argument("-o", "--out-file",
                        action="store",
                        dest="outPath",
                        help="file path of the resulting file.")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    global verbose

    args = parser.parse_args()
    verbose = args.verbose
    abstractPath = args.abstractPath
    predicatePath = args.predicatePath
    outPath = args.outPath

    pmid2predicates = {}
    for p, s in predicates(predicatePath):
        if p not in pmid2predicates:
            pmid2predicates[p] = []
        pmid2predicates[p].append(s)

    with open(abstractPath, 'r') as abFile, open(outPath, 'w') as outFile:
        for line in abFile:
            pmid, text = line.strip().split(" ", 1)
            aug = " ".join(pmid2predicates[p])
            outFile.write("{} {} {}\n".format(pmid, text, aug))


if __name__ == "__main__":
    main()
