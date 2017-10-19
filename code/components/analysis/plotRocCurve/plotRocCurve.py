#!/software/python/3.4/bin/python

import math
import matplotlib
matplotlib.use("Agg")
import pylab as plt
from sklearn.metrics import auc
import argparse
from random import shuffle


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-r", "--real-file",
                        action="store",
                        dest="realPath",
                        help="path to the real file")
    parser.add_argument("-f", "--fake-file",
                        action="store",
                        dest="fakePath",
                        help="path to the fake file")
    parser.add_argument("-o", "--output-roc",
                        action="store",
                        dest="outPath",
                        default="roc.png",
                        help="path to the output png file")
    parser.add_argument("-t", "--plot-title",
                        action="store",
                        dest="title",
                        default="",
                        help="path to the output png file")
    parser.add_argument("-c", "--data-column",
                        action="store",
                        dest="columnIdx",
                        help="which column is important")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    args = parser.parse_args()
    verbose = args.verbose
    realPath = args.realPath
    fakePath = args.fakePath
    outPath = args.outPath
    title = args.title
    columnIdx = int(args.columnIdx)

    if verbose:
        print("Loading from", realPath)

    data = []

    with open(realPath) as rFile:
        for line in rFile:
            tokens = line.split()
            data.append((float(tokens[columnIdx]), 1))

    if verbose:
        print("Loading from", fakePath)

    with open(fakePath) as fFile:
        for line in fFile:
            tokens = line.split()
            data.append((float(tokens[columnIdx]), 0))

    data = [d for d in data if not math.isnan(d[0]) and not math.isinf(d[0])]
    shuffle(data)
    data.sort(key=lambda x: x[0], reverse=True)

    if verbose:
        print("Found:", len(data), "files.")

    ySum = [0 for d in data]
    nSum = [0 for d in data]
    fpf = [0 for d in data]
    tpf = [0 for d in data]

    totalY = sum([1 for d in data if d[1] == 1])
    totalN = sum([1 for d in data if d[1] == 0])

    if verbose:
        print("Positive:", totalY)
        print("Negative:", totalN)

    if data[0][1] == 1:
        ySum[0] = 1
    else:
        nSum[0] = 1

    for i in range(1, len(data)):
        if data[i][1] == 1:
            ySum[i] += 1
        else:
            nSum[i] += 1
        ySum[i] += ySum[i-1]
        nSum[i] += nSum[i-1]

    for i in range(len(data)):
        fpf[i] = nSum[i] / totalN
        tpf[i] = ySum[i] / totalY

    area = auc(fpf, tpf)

    if verbose:
        print("Area:", area)

    fig = plt.figure()
    fig.suptitle(title, fontsize=14, fontweight='bold')
    ax = fig.add_subplot(111)
    fig.subplots_adjust(top=0.85)
    ax.set_title('ROC Curve')
    ax = fig.add_subplot(111)
    ax.plot(fpf, tpf)
    ax.set_xlabel("False Positive Fraction")
    ax.set_ylabel("True Positive Fraction")
    ax.legend(("Area: {0:.4f}".format(area),), loc=4)
    plt.savefig(outPath)

    if verbose:
        print("Saved to:", outPath)


if __name__ == "__main__":
    main()
