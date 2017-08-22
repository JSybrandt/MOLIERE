#!/software/python/3.4/bin/python3

import argparse
import os
import math
import itertools
from pprint import pprint
import scipy.stats
from sklearn.utils import shuffle
import numpy as np
import pandas as pd
from tempfile import mkdtemp
from sklearn.ensemble import RandomForestClassifier
from sklearn.tree import _tree

labelName = "IsLegit"


class DecisionTree:
    def __init__(self, feature, threshold, result=None):
        self.feature = feature
        self.threshold = threshold
        self.result = result
        self.leftChild = None
        self.rightChild = None
        self.parent = None

    def isLeaf(self):
        return self.leftChild is None and self.rightChild is None

    def classify(self, data):
        leaf = self.getLeaf(data)
        return leaf.result

    def setChild(self, child, isLeft=True):
        if isLeft:
            self.leftChild = child
        else:
            self.rightChild = child
        child.parent = self

    def trimLeaf(self, data, newVal):
        leaf = self.getLeaf(data)
        parent = leaf.parent
        parent.leftChild = None
        parent.rightChild = None
        parent.result = newVal

    def getLeaf(self, data):
        if self.isLeaf():
            return self
        if data[self.feature] <= self.threshold:
            if self.leftChild is not None:
                return self.leftChild.getLeaf(data)
        else:
            if self.rightChild is not None:
                return self.rightChild.getLeaf(data)
        return self


def getProb(forrest, data, classidx):
    results = [tree.classify(data) for tree in forrest]
    prob = 0
    for result in results:
        if np.argmax(result) == classidx:
            prob += 1
    return prob / len(forrest)


# change leaf nodes so all trees classify this point the same way
def silence(forrest, data, classIdx):
    for tree in forrest:
        result = tree.classify(data)
        if np.argmax(result) != classIdx:
            tree.trimLeaf(data, [0, 1])


def skTreeToMyTree(skDTree, features):
    skTree = skDTree.tree_

    feature_name = [
        features[i] if i != _tree.TREE_UNDEFINED else "undefined!"
        for i in skTree.feature
    ]

    def recursiveAdd(skNode):
        if skTree.feature[skNode] != _tree.TREE_UNDEFINED:
            feature = feature_name[skNode]
            threshold = skTree.threshold[skNode]
            treeNode = DecisionTree(feature, threshold)
            treeNode.setChild(recursiveAdd(skTree.children_left[skNode]), True)
            treeNode.setChild(recursiveAdd(skTree.children_right[skNode]), False)
            return treeNode
        else:
            return DecisionTree(None, None, skTree.value[skNode][0])
    return recursiveAdd(0)


def tree_to_code(tree, feature_names):
    tree_ = tree.tree_
    feature_name = [
        feature_names[i] if i != _tree.TREE_UNDEFINED else "undefined!"
        for i in tree_.feature
    ]

    print("def tree({}):".format(", ".join(feature_names)))

    def recurse(node, depth):
        indent = "  " * depth
        if tree_.feature[node] != _tree.TREE_UNDEFINED:
            name = feature_name[node]
            threshold = tree_.threshold[node]
            print("{}if {} <= {}:".format(indent, name, threshold))
            recurse(tree_.children_left[node], depth + 1)
            print("{}else:  # if {} > {}".format(indent, name, threshold))
            recurse(tree_.children_right[node], depth + 1)
        else:
            print("{}return ({}, {})".format(indent, tree_.value[node][0][0], tree_.value[node][0][1]))

    recurse(0, 1)


def yieldProfiles(profilePath):
    with open(profilePath, "r") as profileFile:
        hypoTitle = ""
        data = []
        index = []
        for line in profileFile:
            tokens = line.split()
            if len(tokens) == 1:
                if len(data) > 0:
                    index.append(labelName)
                    data.append(1)
                    yield (hypoTitle, pd.Series(data, index=index))
                hypoTitle = tokens[0]
                data = []
                index = []
            elif len(tokens) == 2:
                try:
                    attrName = tokens[0]
                    value = float(tokens[1])
                    data.append(value)
                    index.append(attrName)
                except:
                    pass  # do nothing


def main():
    np.random.seed()
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--profile-path",
                        action="store",
                        dest="profilePath",
                        help="file path of the profile file.")
    parser.add_argument("-m", "--model-dir-path",
                        action="store",
                        dest="modelDirPath",
                        default="",
                        help="directory path of the resulting model files.")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    args = parser.parse_args()
    verbose = args.verbose
    profilePath = args.profilePath
    modelDirPath = args.modelDirPath
    if modelDirPath == "":
        modelDirPath = mkdtemp()

    d = {}
    for (hypoName, series) in yieldProfiles(profilePath):
        d[hypoName] = series

    realFrame = pd.DataFrame(d).transpose()

    # scale 0-1
    for feature in realFrame.columns:
        maxVal = realFrame[feature].max()
        minVal = realFrame[feature].min()
        if maxVal - minVal > 0:
            realFrame[feature] = \
                (realFrame[feature] - minVal) / (maxVal - minVal)
        if verbose:
            print("Feature: {}\t [{}, {}] -> [0, 1]".format(
                   feature, minVal, maxVal))

    if verbose:
        print("Loaded:")
        print(realFrame.iloc[0:5])

    synthFrame = \
        realFrame.reindex(["SYNTH{}".format(i) for i in range(len(realFrame))])
    for feature in synthFrame.columns:
        if feature == labelName:
            synthFrame[feature] = 0
        else:
            synthFrame[feature] = \
                np.random.uniform(size=len(synthFrame[feature]))

    totalFrame = pd.concat([realFrame, synthFrame])
    totalFrame = totalFrame.reindex(np.random.permutation(totalFrame.index))

    model = RandomForestClassifier(n_jobs=-1, n_estimators=20)
    features = [x for x in totalFrame.columns if x != labelName]
    model.fit(totalFrame[features], totalFrame[labelName])

    activeSet = set()
    res = model.predict_proba(realFrame[features])
    data = list(zip(realFrame.index, [x[0] for x in res]))
    for dat in data:
        if dat[1] > 0:
            activeSet.add(dat[0])

    if verbose:
        print("Important Features:")
        for (feat, level) in zip(features, model.feature_importances_):
            print(feat, " ", level)

    myForrest = [skTreeToMyTree(t, features) for t in model.estimators_]

    def getProbs():
        data = [(d, getProb(myForrest, realFrame.loc[d, features], 0))
                for d in activeSet]
        for rm in filter(lambda x: x[1] == 0, data):
            activeSet.remove(rm[0])
        data.sort(key=lambda x: x[1], reverse=True)
        return data

    """
    res = model.predict_proba(realFrame[features])
    data = list(zip(realFrame.index, [x[0] for x in res]))
    data.sort(key=lambda x: x[1], reverse=True)
    if verbose:
        print("THEM")
        for dat in data[:5]:
            if dat[1] > 0:
                pprint(dat)
        print("ME")
        for dat in myData[:5]:
            if dat[1] > 0:
                pprint(dat)

    if verbose:
        for tree in model.estimators_:
            print("TREE:")
            tree_to_code(tree, features)
    """

    data = getProbs()
    while True:
        os.system("clear")
        print("Highest Rated Hypotheses:")
        pprint(data[:15])
        userInput = input(">>> ")
        tokens = userInput.lower().split()
        if len(tokens) == 2:
            cmd = tokens[0]
            query = tokens[1]
            if cmd == "show":
                matches = list(filter(lambda x: query in x[0], data))
                pprint(matches)
                input("Press Enter to Continue")
            if cmd == "silence":
                matches = list(filter(lambda x: (query in x[0] and x[1] > 0), data))
                if len(matches) > 0:
                    print("Silence the following?")
                    pprint(matches)
                    if input("[y/n]:")[0].lower() == 'y':
                        arr = [list(realFrame.loc[match[0], features])
                               for match in matches]
                        for val in arr:
                            valDict = {features[i]: val[i] for i in range(len(val))}
                            silence(myForrest, valDict, 1)
                            data = getProbs()
                    else:
                        print("Canceling")
                else:
                    print("ERR: Did not find anomalies related to ", query)
                    input("Press Enter to Continue")
        elif len(tokens) == 1 and tokens[0] == "all":
            os.system("clear")
            print("Showing All Anomalies")
            pprint(data)
            input("Press Enter to Continue")
        elif len(tokens) == 1 and tokens[0] == "quit":
            break
        else:
            print("Expecting 'quit', 'show <>', or 'silence <>'")


if __name__ == "__main__":
    main()
