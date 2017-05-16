import sys
from collections import Counter
mapperPath = sys.argv[1]
unStemmer = sys.argv[2]
phraseUnstemmer = sys.argv[3]
partition = sys.argv[4]
newPartition = sys.argv[5]
f = open(mapperPath, 'r')
unMapper = {}
for line in f:
    word, number = line.strip().split('\t')
    unMapper[int(number)] = word
unStem = {}
f = open(unStemmer, 'r')
for line in f:
    line = line.strip().split(":")
    base = line[0].strip()
    rest = line[1].strip().split('\t')
    maximum_count = -float('inf')
    unstemmed = None
    for word_count in rest:
        word, count = word_count.strip().split(" ")
        if int(count)>maximum_count:
            maximum_count = int(count)
            unstemmed = word
    unStem[base] = unstemmed
f = open(phraseUnstemmer, 'r')
phraseUndo = {}
for line in f:
    line = line.strip().split('\t')
    num_phrase = map(int, line[0].split(" "))
    if len(line) < 2:
        line.append("ERR")
    phrase = line[1]
    phraseUndo[tuple(num_phrase)]=phrase
##############
f = open(partition, 'r')
g = open(newPartition, 'w')
f.readline()
for line in f:
    line = line.strip().split(',')
    phrases = []
    for cand in line:
        cand=cand.strip()
        cand = map(int, cand.split(" "))
        if len(cand) == 1:
            phrases.append(unStem[unMapper[cand[0]]])
        else:
            phrases.append(phraseUndo[tuple(cand)])
    g.write(','.join([p for p in phrases if p != "ERR"])+"\n")



