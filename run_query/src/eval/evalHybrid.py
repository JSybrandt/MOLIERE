#!/usr/bin/env python3
import argparse

default_param = [
    -0.9746115311616966,   # score_a_c coef
    1.5492325735580643,    # score_a_c exp
    0.513138625868795,     # score_topic_center coef
    2.665854971611072,     # score_topic_center exp
    0.7017497381282372,    # score_topic_points coef
    2.761011067353779,     # score_topic_points exp
    -0.22752657252005895,  # score_topic_corr coef
    1.9070034217853005,    # score_topic_corr exp
    -0.3946766150567381,   # score_topic_net_btwn coef
    1.1746781019546093,    # score_topic_net_btwn exp
    -0.4087055911895488,   # score_topic_net_ccoef coef
    2.022834231799606      # score_topic_net_ccoef exp
]

default_scale = [
    13.1183,   # score_a_c
    0.62918,   # score_topic_center
    0.869151,  # score_topic_points
    0.999596,  # score_topic_corr
    3019.73,   # score_topic_net_btwn
    0.607236   # score_topic_net_ccoef
]

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

    def scale(self, max_val=default_scale):
        self.vec = [v / max_val[i] for i, v in enumerate(self.toVec())]

    def toScore(self, params=default_param):
        vec = self.toVec()
        p_tup = list(zip(*2*[iter(params)]))
        partials = [p_tup[i][0] * (vec[i] ** p_tup[i][1])
                    for i in range(Data.num_features)]
        score = sum(partials)
        return score + offset

    def isDone(self):
        return None not in self.toVec()

    def __str__(self):
        if self.isDone():
            return "{} {}".format(self.pair, self.toScore())
        else:
            return "{} N/A".format(self.pair)


def lineToPair(line):
    c1, c2, _ = line.split(" ", 2)
    if c2 < c1:
        c1, c2 = c2, c1
    return "{} {}".format(c1, c2)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("files",
                        nargs="+",
                        help="list of sub-metric files (*.{metric}.eval)")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")
    args = parser.parse_args()

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
        data.scale()

    # print
    for _, data in pair2data.items():
        print(data)


if __name__ == '__main__':
    main()
