import sys
from collections import Counter
pathWords = sys.argv[1]
pathTopics = sys.argv[2]
numTopics = sys.argv[3]
#######################
f = open(pathWords, 'r')
g = open(pathTopics, 'r')
topics = [Counter() for i in xrange(int(numTopics))]
topicUnigrams = [Counter() for i in xrange(int(numTopics))]
f.readline()
for line in f:
    line = line.strip().split(',')
    topicLine = g.readline().strip().split(',')
    for i in xrange(len(line)):
        phrase = line[i].strip()
        phrase = tuple(phrase.split(" "))
        topic = int(topicLine[i])
        if len(phrase)>1:
            topics[topic][phrase]+=1
            for word in phrase:
                topicUnigrams[topic][tuple([word])]+=1
        else:
            topicUnigrams[topic][phrase]+=1
h = open('topPhrases.txt','w')
k = open('topUnigrams.txt','w')
for i in xrange(len(topics)):
    topic = topics[i]
    unigramTopic = topicUnigrams[i]
    topPhrases = [[phrase,topic[phrase]] for phrase in topic]
    topPhrases.sort(key = lambda x: x[1], reverse = True)
    topUnigrams = [[phrase,unigramTopic[phrase]] for phrase in unigramTopic]
    topUnigrams.sort(key = lambda x: x[1], reverse = True)
    numPhrases = 20
    h.write("Topic: "+str(i)+"\n")
    k.write("Topic: "+str(i)+"\n")
    for i in xrange(min(numPhrases, len(topPhrases))):
        phrase, count = topPhrases[i]
        h.write(" ".join(phrase)+", ")
    h.write("\n")
    for i in xrange(min(numPhrases, len(topUnigrams))):
        phrase, count = topUnigrams[i]
        k.write(phrase[0]+", ")
    k.write("\n")

