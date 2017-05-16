#!/software/python/3.4/bin/python
"""
Note: Requires python/3.4 module
Command Line Arguments:
    <xmlFile>
    <resultFile>
"""
import sys
import re
import codecs
from unidecode import unidecode
from nltk.corpus import wordnet
import nltk

wnl = nltk.wordnet.WordNetLemmatizer()
splitReg = re.compile("(\W+)")


def addDummySentence(text):
    return "quick brown fox. " + text


def stem(text):
    def getWordnetPos(tag):
        if tag.startswith('J'):
            return wordnet.ADJ
        elif tag.startswith('V'):
            return wordnet.VERB
        elif tag.startswith('N'):
            return wordnet.NOUN
        elif tag.startswith('R'):
            return wordnet.ADV
        elif tag.startswith('S'):
            return wordnet.ADJ
        else:
            return None
    res = []
    tokens = [x for x in splitReg.split(text) if x]
    for tagObj in nltk.pos_tag(tokens):
        pos = getWordnetPos(tagObj[1])
        if(pos):
            res += wnl.lemmatize(tagObj[0], pos)
        else:
            res += tagObj[0]
    return "".join(res)


def parseXML(xmlFile):
    pmidReg = re.compile("\\</?PMID.*?\\>", re.UNICODE)
    titleReg = re.compile("\\</?ArticleTitle.*?\\>", re.UNICODE)
    textReg = re.compile("\\</?AbstractText.*?\\>", re.UNICODE)
    endReg = re.compile("\\</MedlineCitation.*?\\>", re.UNICODE)

    currPmid = u""
    currText = u""

    for line in xmlFile:
        if(pmidReg.search(line)):
            currPmid = pmidReg.sub(u"", line).strip()
        elif(titleReg.search(line)):
            currText = titleReg.sub(u"", line).strip()
            if(currText[-1] != u"."):
                currText += u"."
        elif(textReg.search(line)):
            currText += u" " + textReg.sub(u"", line).strip()
        elif(endReg.search(line)):
            yield (currPmid, currText)
            currPmid = u""
            currText = u""


def main():
    if(len(sys.argv) < 3):
        sys.stderr.write("Failed to supply <xmlFile> and <resultFile>")
        exit(1)
    xmlFilePath = sys.argv[1]
    resultFilePath = sys.argv[2]
    with codecs.open(xmlFilePath, encoding="utf-8", mode="r") as xmlFile, \
            open(resultFilePath, "w") as resultFile:
        for (pmid, text) in parseXML(xmlFile):
            text = unidecode(text)
            text = text.lower()
            text = stem(text)
            text = addDummySentence(text)
            resultFile.write("{} {}\n".format(pmid,text))


if(__name__ == "__main__"):
    main()

