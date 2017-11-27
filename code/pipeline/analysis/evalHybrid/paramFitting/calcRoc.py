#!/software/python/3.4/bin/python
import sys
import math
import time
import matplotlib
matplotlib.use('Agg')
import pylab as plt
from sklearn.metrics import auc
from random import shuffle



def calcROC(realPath, fakePath):
    data = []
    columnIdx = 2  # idx of score
    with open(realPath) as rFile:
        for line in rFile:
            tokens = line.split()
            data.append((float(tokens[columnIdx]), 1))
    with open(fakePath) as fFile:
        for line in fFile:
            tokens = line.split()
            data.append((float(tokens[columnIdx]), 0))
    data = [d for d in data if not math.isnan(d[0])
            and not math.isinf(d[0])]
    shuffle(data)
    data.sort(key=lambda x: x[0], reverse=True)
    ySum = [0 for d in data]
    nSum = [0 for d in data]
    fpf = [0 for d in data]
    tpf = [0 for d in data]
    totalY = sum([1 for d in data if d[1] == 1])
    totalN = sum([1 for d in data if d[1] == 0])
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
    return auc(fpf, tpf)


def main():
    print(calcROC(sys.argv[1], sys.argv[2]))


if __name__ == '__main__':
    main()
