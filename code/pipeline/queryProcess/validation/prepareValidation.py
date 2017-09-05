#!/software/python/3.4/bin/python3

import argparse


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--file",
                        action="store",
                        dest="inFile")
    parser.add_argument("-d", "--out-dir",
                        action="store",
                        dest="outDir")
    parser.add_argument("-l", "--label-file",
                        action="store",
                        dest="labelFile")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    args = parser.parse_args()
    verbose = args.verbose
    inPath = args.inFile
    outDir = args.outDir
    labelPath = args.labelFile

    if verbose:
        print("Loading labels:")
    labels = {}
    count = 0
    with open(labelPath, "r") as lFile:
        for line in lFile:
            line = line.strip()
            labels[line] = count
            count += 1

    if verbose:
        print("Loaded ", len(labels), " labels")

    groupedHypo = {}
    with open(inPath, "r") as inFile:
        for line in inFile:
            try:
                (sub, verb, obj, year) = line.split("|")
                subIdx = labels[sub]
                objIdx = labels[obj]
                if(subIdx > objIdx):
                    (subIdx, objIdx) = (objIdx, subIdx)
                if subIdx not in groupedHypo:
                    groupedHypo[subIdx] = []
                groupedHypo[subIdx].append(objIdx)
            except:
                pass
    if verbose:
        print("Found ", len(groupedHypo), " hypo groups")

    for (source, targets) in groupedHypo.items():
        outPath = "{}/{}".format(outDir, source)
        with open(outPath, "w") as outFile:
            for t in targets:
                outFile.write("{}\n".format(t))


if __name__ == "__main__":
    main()
