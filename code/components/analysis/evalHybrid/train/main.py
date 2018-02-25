#!/usr/bin/env python3
import blackbox as bb
from sklearn.metrics import auc
from random import shuffle, sample, random
import numpy as np
import multiprocessing as mp
from pprint import pprint
import sys

np.seterr(all='raise')

file_templates = [
#        "data/{}.cos.eval",
        "data/{}.l2.eval",
        "data/{}.tpw.eval",
        "data/{}.twe.eval",
        "data/{}.path.eval"
]

# MUST HAVE 2 AND POSITIVE EXAMPLE MUST BE IDX 1
trial_names = [
    "fake",
#    "highlyCited"
    "real"
]


class Data(object):
    num_features = 6

    def __init__(self, pair):
        self.pair = pair
        self.score_a_c = None
        self.score_topic_center = None
        self.score_topic_points = None
        self.score_topic_corr = None
        self.score_topic_net_btwn = None
        self.score_topic_net_ccoef = None
#        self.score_topic_net_mod = None
        self.vec = None

    def add(self, template, line):
        file_type = template.split(".")[1]
        tokens = line.strip().split()
        try:
            if file_type == "l2":
                self.score_a_c = float(tokens[2])
#            elif file_type == "cos":
                self.score_topic_center = float(tokens[3])
            elif file_type == "tpw":
                self.score_topic_points = float(tokens[2])
            elif file_type == "twe":
                self.score_topic_corr = float(tokens[2])
            elif file_type == "path":
                self.score_topic_net_btwn = float(tokens[6])
                self.score_topic_net_ccoef = float(tokens[9])
#                self.score_topic_net_mod = float(tokens[10])
            else:
                print("NO IDEA WHAT TO DO WITH", template)
        except:
            print("FAILED TO PARSE", template, line)

    def toVec(self):
        if self.vec is None:
            self.vec = [
                        self.score_a_c,
                        self.score_topic_center,
                        self.score_topic_points,
                        self.score_topic_corr,
                        self.score_topic_net_btwn,
                        self.score_topic_net_ccoef,
#                        self.score_topic_net_mod
                       ]
            if None in self.vec:
                print(self.vec)
                raise ValueError("Contained None")
            self.vec = [0 if np.isinf(v) else v for v in self.vec]
            self.vec = [0 if v < 0 else v for v in self.vec]
        return self.vec

    def scale(self, scales):
        self.vec = [v / scales[i] for i, v in enumerate(self.toVec())]

    def toScore(self, params):
        vec = self.toVec()
        p_tup = list(zip(*2*[iter(params)]))
        partials = [p_tup[i][0] * (vec[i] ** p_tup[i][1])
                    for i in range(Data.num_features)]
        score = sum(partials)
        return score

    def isDone(self):
        return None not in self.toVec()

    def __str__(self):
        return " ".join([str(v) for v in self.toVec()])


def lineToPair(line):
    c1, c2, _ = line.split(" ", 2)
    if c2 < c1:
        c1, c2 = c2, c1
    return "{} {}".format(c1, c2)


def getPairs(trial):
    res = []
    with open(file_templates[0].format(trial)) as file:
        for line in file:
            res.append(lineToPair(line))
    return res


def loadData(trial):
    pair2data = {r: Data(r) for r in getPairs(trial)}
    for template in file_templates:
        file_name = template.format(trial)
        with open(file_name) as file:
            for line in file:
                pair = lineToPair(line)
                pair2data[pair].add(template, line)
    for k, v in pair2data.items():
        if not v.isDone():
            raise ValueError(k + " " + str(v))
    return [v for k, v in pair2data.items()]


# Needs to be global or else pickling gets mad
trial2data = {t: loadData(t) for t in trial_names}
scales = [0 for i in range(Data.num_features)]
for _, data_list in trial2data.items():
    scales = [max(scales[i], max([d.toVec()[i] for d in data_list]))
              for i in range(Data.num_features)]

# even out data
min_data_size = min(len(v) for _, v in trial2data.items())
trial2data = {k: sample(v, min_data_size)
              for k, v in trial2data.items()}

for _, data_vec in trial2data.items():
    for data in data_vec:
        data.scale(scales)


def eval_params(params):
    score_label_list = []
    for trial_idx, trial in enumerate(trial_names):
        for data in trial2data[trial]:
            score_label_list.append((data.toScore(params), trial_idx))
    shuffle(score_label_list)
    score_label_list.sort(key=lambda x: x[0], reverse=True)
    ySum = [0 for d in score_label_list]
    nSum = [0 for d in score_label_list]
    fpf = [0 for d in score_label_list]
    tpf = [0 for d in score_label_list]
    totalY = sum([1 for d in score_label_list if d[1] == 1])
    totalN = sum([1 for d in score_label_list if d[1] == 0])
    if score_label_list[0][1] == 1:
        ySum[0] = 1
    else:
        nSum[0] = 1
    for i in range(1, len(score_label_list)):
        if score_label_list[i][1] == 1:
            ySum[i] += 1
        else:
            nSum[i] += 1
        ySum[i] += ySum[i-1]
        nSum[i] += nSum[i-1]
    for i in range(len(score_label_list)):
        fpf[i] = nSum[i] / totalN
        tpf[i] = ySum[i] / totalY
    area = auc(fpf, tpf)
    return -area


def output(params):
    for trial in trial_names:
        with open("{}.hybrid.eval".format(trial), 'w') as file:
            for data in trial2data[trial]:
                file.write("{} {}\n".format(data.pair, -data.toScore(params)))


def main():

    global default_best_point

    # coef and exp
    feature_space = [([-1, 1], [1, 3]) for i in range(Data.num_features)]
    feature_space = list(sum(feature_space, ()))


#    print("Sanity Check", eval_params([
#        1.0, 1.0,
#        0, 0,
#        0, 0,
#        0, 0,
#        0, 0,
#        0, 0
#        ]), "== 0.708?")

    # bb.search(f=eval_params,
              # box=list(sum(feature_space, ())),
              # n=10000,
              # m=0,
              # batch=24,
              # resfile='output.csv')

    num_batches = 5
    jobsPerBatch = 24
    res = []
    num_restarts = 100

    local_best_point = default_best_point
    for _ in range(num_restarts):
        search_area = feature_space[:]

        def samplePoint(area):
            return [random() * (h-l) + l for (l, h) in area]

        def shrinkSearchArea(area, point):
            radius = [(h - l)/2 for (l, h) in area]
            radius = [r - r/num_batches for r in radius]
            new_area = [(point[i]-radius[i], point[i]+radius[i])
                        for i in range(len(point))]
            new_area = [(max(area[i][0], l), min(area[i][1], h))
                        for i, (l, h) in enumerate(new_area)]
            return new_area

        search_area = shrinkSearchArea(search_area, local_best_point)
        #print(search_area)
        # search_area = shrinkSearchArea(search_area, default_best_point)

        for batch in range(num_batches):
            samples = [samplePoint(search_area) for j in range(jobsPerBatch)]
            values = [float("inf") for j in range(jobsPerBatch)]
            with mp.Pool() as pool:
                values = list(pool.map(eval_params, samples))
            best_idx = values.index(min(values))
            best_point = samples[best_idx]
            search_area = shrinkSearchArea(search_area, best_point)
            res.append((best_point, values[best_idx]))

        res.sort(key=lambda x: x[1])
        local_best_point = res[0][0]
    return res[0]


if __name__ == "__main__":
    global default_best_point

    with open(sys.argv[1]) as pFile:
        exec(pFile.read())
        default_best_point = param

    # output(default_best_point)
    pt, score = main()
    print("global param, scale")
    print("param = ", pt)
    print("scale = ", scales)
    print("score = ", score)
