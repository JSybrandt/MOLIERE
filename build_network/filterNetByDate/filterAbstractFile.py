#!/software/python/3.4/bin/python


import argparse


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--yearFile",
                        action="store",
                        dest="yearPath",
                        help="file path of the pmid-year file")
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
    yearPath = args.yearPath

    validPMID = set()
    with open(yearPath, "r") as yearFile:
        for line in yearFile:
            tokens = line.split()
            if len(tokens) == 2:
                pmid = tokens[0]
                year = int(tokens[1])
                if(year < filterYear):
                    validPMID.add(pmid)
            else:
                print("ERR: line is invalid:", line)

    with open(absFilePath, "r") as absFile, open(outFilePath, "w") as outFile:
        for line in absFile:
            tokens = line.split(" ", 1)
            pmid = tokens[0].strip()
            data = ""
            if len(tokens) > 1:
                data = tokens[1].strip()
            if pmid in validPMID:
                outFile.write("{} {}\n".format(pmid, data))


if __name__ == "__main__":
    main()
