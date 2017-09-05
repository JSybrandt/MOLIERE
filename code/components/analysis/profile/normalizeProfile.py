#!/software/python/3.4/bin/python3
"""
Requires Python 3.4
"""

import argparse
from pprint import pprint


def getProfiles(filePath):
    profileName = ""
    profileData = {}
    with open(filePath, "r") as pFile:
        for line in pFile:
            tokens = line.split()
            if(len(tokens) == 1):
                if(profileName != ""):
                    yield (profileName, profileData)
                profileName = tokens[0]
                profileData = {}
            if(len(tokens) == 2):
                try:
                    profileData[tokens[0]] = float(tokens[1])
                except:
                    pass
            # note: if len(tokens) > 2 there has been an error.
            # there will have also been an error if Undef. is sent


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--profile-file",
                        action="store",
                        dest="profiles",
                        help="file path of the profiles.")
    parser.add_argument("-o", "--out-file",
                        action="store",
                        dest="outPath",
                        help="file path of the resulting profile file.")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    args = parser.parse_args()
    verbose = args.verbose
    outPath = args.outPath
    profilePath = args.profiles

    mins = {}
    maxes = {}
    sums = {}
    counts = {}
    for (name, data) in getProfiles(profilePath):
        for (k, v) in data.items():
            if k not in mins:
                mins[k] = v
            elif mins[k] > v:
                mins[k] = v

            if k not in maxes:
                maxes[k] = v
            elif maxes[k] < v:
                maxes[k] = v

            if k not in sums:
                sums[k] = v
            else:
                sums[k] += v

            if k not in counts:
                counts[k] = 1
            else:
                counts[k] += 1

    diffs = {k: maxes[k] - mins[k] for k in mins.keys()}
    avgs = {k: ((sums[k] / counts[k]) - mins[k]) / diffs[k] for k in mins.keys()}

    with open(outPath, "w") as outFile:
        for (name, data) in getProfiles(profilePath):
            outFile.write("{}\n".format(name))
            for (k, v) in data.items():
                normVal = (v - mins[k]) / diffs[k]
                outFile.write("\t{}\t{}\t{}\n".format(
                    k, normVal, normVal - avgs[k]
                    ))


if __name__ == "__main__":
    main()
