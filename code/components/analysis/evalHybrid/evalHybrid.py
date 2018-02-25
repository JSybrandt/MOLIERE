#!/usr/bin/env python3
import argparse

global default_scale
global default_param

offset = 1


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
        self.vec = None

    def add(self, file_path, line):
        file_type = file_path.split(".")[-2]
        tokens = line.strip().split()
        try:
            if file_type == "l2":
                self.score_a_c = float(tokens[2])
                self.score_topic_center = float(tokens[3])
            elif file_type == "tpw":
                self.score_topic_points = float(tokens[2])
            elif file_type == "twe":
                self.score_topic_corr = float(tokens[2])
            elif file_type == "path":
                self.score_topic_net_btwn = float(tokens[6])
                self.score_topic_net_ccoef = float(tokens[9])
            else:
                print("NO IDEA WHAT TO DO WITH", file_path)
                raise ValueError(file_type)
        except:
            print("FAILED TO PARSE", file_path, line)
            raise ValueError(file_path)

    def toVec(self):
        if self.vec is None:
            self.vec = [
                        self.score_a_c,
                        self.score_topic_center,
                        self.score_topic_points,
                        self.score_topic_corr,
                        self.score_topic_net_btwn,
                        self.score_topic_net_ccoef,
                       ]
            self.vec = [0 if v == float('inf') else v for v in self.vec]
            self.vec = [0 if v == float('-inf') else v for v in self.vec]
            self.vec = [0 if v < 0 else v for v in self.vec]
        return self.vec

    def scale(self, max_val):
        self.vec = [v / max_val[i] for i, v in enumerate(self.toVec())]

    def toScore(self, params):
        vec = self.toVec()
        p_tup = list(zip(*2*[iter(params)]))
        partials = [p_tup[i][0] * (vec[i] ** p_tup[i][1])
                    for i in range(Data.num_features)]
        score = sum(partials)
        return score + offset

    def isDone(self):
        return None not in self.toVec()

    def __str__(self):
        global default_param
        if self.isDone():
            return "{} {}".format(self.pair, self.toScore(default_param))
        else:
            return "{} N/A".format(self.pair)


def lineToPair(line):
    c1, c2, _ = line.split(" ", 2)
    if c2 < c1:
        c1, c2 = c2, c1
    return "{} {}".format(c1, c2)


def main():
    global default_param, default_scale
    global param, scale
    parser = argparse.ArgumentParser()
    parser.add_argument("files",
                        nargs="+",
                        help="list of sub-metric files (*.{metric}.eval)")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")
    parser.add_argument("-p", "--param_file",
                        action="store",
                        dest="param_file",
                        default=None,
                        help="path to param file")
    args = parser.parse_args()

    if args.param_file:
        with open(args.param_file) as pFile:
            exec(pFile.read())
            default_param = param
            default_scale = scale
        if(args.verbose):
            print(default_param)
            print(default_scale)

    pair2data = {}
    for file_name in args.files:
        if args.verbose:
            print("Loading", file_name)
        with open(file_name) as file:
            for line in file:
                pair = lineToPair(line)
                if pair not in pair2data:
                    pair2data[pair] = Data(pair)
                pair2data[pair].add(file_name, line)

    # check and scale
    for pair, data in pair2data.items():
        if not data.isDone():
            print("ERROR:", pair, "does not have all metric values")
            exit(1)
        data.scale(default_scale)

    # print
    for _, data in pair2data.items():
        print(data)


if __name__ == '__main__':
    main()
