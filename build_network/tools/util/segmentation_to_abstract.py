#!/software/python/3.4/bin/python3

import os
import re

MODEL = "UMLS"
SEGMENT_FILE = "models/{}/segmentation.txt".format(MODEL)

phraseMark = re.compile(r'</?phrase>')

START_PHRASE = "<phrase>"
END_PHRASE = "</phrase>"

with open(SEGMENT_FILE, 'r') as sFile:
    for line in sFile:
        startIdx = 0
        endIdx = 0
        abstract = []
        # invariant: endIdx points to the next unread char
        while startIdx > -1:
            # find start of next phrase
            startIdx = line.find(START_PHRASE, endIdx)
            # if no phrase found
            if startIdx == -1:
                # add the rest
                abstract.append(line[endIdx:])
                break
            # add stuff until next phrase
            abstract.append(line[endIdx:startIdx])
            # move phrase ptr to after <phrase>
            startIdx += len(START_PHRASE)
            # find end of phrase
            endIdx = line.find(END_PHRASE, startIdx)
            if endIdx == -1:
                print("SOMETHING WENT HORRIBLY WRONG")
                break
            # add in phrase
            abstract.append(line[startIdx:endIdx].replace(' ', '_'))
            # move end to next usable char
            endIdx += len(END_PHRASE)
        s = "".join(abstract).strip()
        if len(s) > 1:
            print(s)
