#!/software/python/3.4/bin/python
import argparse
import random


def getRand(s):
    return random.sample(s, 1)[0]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--out-file",
                        action="store",
                        dest="outPath",
                        help="file path of the resulting fake hypo.")
    parser.add_argument("-n", "--num-fakes",
                        action="store",
                        dest="numFake",
                        help="Number of fake hypotheses to generate")
    parser.add_argument("-i", "--existing-hypo",
                        action="store",
                        dest="inPath",
                        help="file path to the data regarding existing hypo. should be sqlite3 output.")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    args = parser.parse_args()
    verbose = args.verbose
    outPath = args.outPath
    inPath = args.inPath
    numFake = int(args.numFake)

    hypo = set()
    entities = set()
    relations = set()
    if verbose:
        print("Loading", inPath)
    with open(inPath, "r") as inFile:
        for line in inFile:
            tokens = line.split("|")
            if len(tokens) == 4:
                entities.add(tokens[0])
                relations.add(tokens[1])
                entities.add(tokens[2])
                hypo.add((tokens[0], tokens[1], tokens[2]))
                # note: tokens[3] is year
    if verbose:
        print("Loaded:", len(hypo))
        print("Making Fake Hypo")
    fakeHypo = set()
    while len(fakeHypo) < numFake:
        randHypo = (getRand(entities), getRand(relations), getRand(entities))
        if(randHypo[0] != randHypo[2] and
           randHypo not in hypo and
           randHypo not in fakeHypo):
            fakeHypo.add(randHypo)
    if verbose:
        print("Made ", len(fakeHypo), " fake hypos.")
        print("Writing to ", outPath)

    with open(outPath, "w") as outFile:
        for h in fakeHypo:
            outFile.write("{}|{}|{}|0000\n".format(h[0], h[1], h[2]))




if __name__ == "__main__":
    main()

