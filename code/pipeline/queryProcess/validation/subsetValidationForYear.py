#!/software/python/3.4/bin/python3

import argparse


parser = argparse.ArgumentParser()
parser.add_argument("-V", "--validation-pairs",
                    action="store",
                    dest="validationPairs")
parser.add_argument("-o", "--output-pairs",
                    action="store",
                    dest="outPairs")
parser.add_argument("-l", "--label-file",
                    action="store",
                    dest="labelFile")
parser.add_argument("-v", "--verbose",
                    action="store_true",
                    dest="verbose",
                    help="print debug info")

args = parser.parse_args()
verbose = args.verbose
inPairsPath = args.validationPairs
outPairsPath = args.outPairs
labelPath = args.labelFile

labels = set()
with open(labelPath, "r") as lFile:
    for line in lFile:
        labels.add(line.strip())

with open(inPairsPath, "r") as iFile, open(outPairsPath, "w") as oFile:
    for line in iFile:
        (c1, v, c2, y) = line.split("|")
        if c1 in labels and c2 in labels:
            oFile.write(line)

