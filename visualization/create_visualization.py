#!/usr/bin/env python3

import json
import os
from os.path import join
import argparse
from glob import glob
import shutil

import requests
from html.parser import HTMLParser
from multiprocessing import Pool

HOME_ENV = "MOLIERE_HOME"


class PubMedParser(HTMLParser):
    def __init__(self):
        super().__init__(self)
        self.in_title_text = False
        self.in_abstract_div = False
        self.in_abstract_text = False
        self.title = None
        self.abstract = None

    def handle_starttag(self, tag, attr):
        if tag == "h1":
            self.in_title_text = True

    def handle_endtag(self, tag):
        if tag == "h1":
            self.in_title_text = False

    def handle_data(self, data):
        if self.in_title_text:
            self.title = data


def get_path_from_meta(res, num_topics, ext):
    files = glob("{}/*.{}.{}".format(res, num_topics, ext))
    if len(files) == 0:
        raise RuntimeError("No {} file for res:{}, #:{}".format(ext,
                                                                res,
                                                                num_topics))
    if len(files) > 1:
        raise RuntimeError("Too Many {} files res:{}, #:{}".format(ext,
                                                                   res,
                                                                   num_topics))
    return files[0]


def parse_eval(path):
    res = {}
    with open(path) as file:
        for line in file:
            tokens = line.strip().split()
            if len(tokens) == 2:
                res[tokens[0]] = (float(tokens[1]), None)
            elif len(tokens) > 2:
                topic_tokens = tokens[2:]
                topics = {}
                curr_top = topic_tokens[0]
                for tok in topic_tokens:
                    if tok[0] == 'T':
                        curr_top = tok
                    else:
                        if curr_top not in topics:
                            topics[curr_top] = []
                        else:
                            topics[curr_top].append(tok)
                res[tokens[0]] = (float(tokens[1]), topics)
    return res


def parse_papers(path):
    res = {}
    uniq_pmids = set()
    with open(path) as file:
        for line in file:
            tokens = line.strip().split()
            topic = tokens[0]
            res[topic] = {}
            res[topic]['papers'] = [
                {"pmid": pmid, "title": None}
                for pmid in tokens[1:]]
            uniq_pmids = uniq_pmids.union(set(tokens[1:]))
    pmids = list(uniq_pmids)
    with Pool() as p:
        content = p.map(get_paper_content, pmids)
    pmid2title = {pmid: c['title'] for pmid, c in zip(pmids, content)}
    for topic in res:
        for paper in res[topic]['papers']:
            paper['title'] = pmid2title[paper['pmid']]

    return res


def get_paper_content(pmid):
    URL = "https://www.ncbi.nlm.nih.gov/pubmed/"
    response = requests.get(URL, params={'term': pmid})
    response.encoding = 'utf-8'
    parser = PubMedParser()
    parser.feed(response.text)
    res = {}
    res['pmid'] = pmid
    res['title'] = parser.title
    print("Received", pmid)
    return res


def parse_topic_model(path, max_words):
    "prepares tm for flare.json"
    topics = []

    with open(path) as file:
        for line in file:
            tokens = line.strip().split()
            if len(tokens) == 3:
                if tokens[0] == "TOPIC:":
                    topics.append({
                        'name': "Topic_{}".format(tokens[1]),
                        'children': []
                    })
            elif len(tokens) == 2:
                target = topics[-1]['children']
                if len(target) < max_words:
                    target.append({
                        'name': tokens[0],
                        'size': tokens[1]
                    })

    return topics


def parse_topic_net(path):
    "parses topic net for cyto"

    def name(s):
        if s[-1] == "*":
            return s[:-1]
        else:
            return s

    def group(s):
        if s[-1] == "*":
            # path topic
            return 2
        elif s[0] == "T":
            # regular topic
            return 1
        else:
            # keyword
            return 3

    res = {
        'nodes': [],
        'links': []
    }
    nodes = set()

    with open(path) as file:
        # skip
        file.readline()
        for line in file:
            a, b, w = line.strip().split(',')
            nodes.add(a)
            nodes.add(b)
            res['links'].append({
                'source': name(a),
                'target': name(b),
                'value': float(w)
            })

    res['nodes'] = [{
        'id': name(node),
        'group': group(node)
    } for node in nodes]

    return res


if __name__ == "__main__":
    home_path = os.environ[HOME_ENV]

    parser = argparse.ArgumentParser()
    parser.add_argument("-r",
                        "--res-dir",
                        help="Specifies result dir, containing" +
                             " eval, papers, and edges file.")
    parser.add_argument('-n', '--num-topics',
                        help="number of topics to gen vis.")
    parser.add_argument('-m', '--max-words-per-topic',
                        default=7,
                        help="max words per topic in bubble vis")
    parser.add_argument('-o', '--out-dir',
                        default="./vis")
    parser.add_argument('-t', '--template-dir',
                        default=join(home_path,
                                     "visualization",
                                     "template"))

    args = parser.parse_args()

    eval_path = get_path_from_meta(args.res_dir,
                                   args.num_topics, 'eval')
    papers_path = get_path_from_meta(args.res_dir,
                                     args.num_topics, 'papers')
    topic_model_path = get_path_from_meta(args.res_dir,
                                          args.num_topics, 'view')
    topic_net_path = get_path_from_meta(args.res_dir,
                                        args.num_topics,
                                        'topic_net.edges')

    args.max_words_per_topic = int(args.max_words_per_topic)

    template_dir_path = args.template_dir
    if not os.path.isdir(template_dir_path):
        raise RuntimeError("Failed to find template dir.")

    if os.path.isdir(args.out_dir):
        print("Warning, overwritting existing vis")
    else:
        os.makedirs(args.out_dir, exist_ok=True)

    with open(join(args.out_dir, "network.json"), 'w') as file:
        file.write(json.dumps(parse_topic_net(topic_net_path)))

    with open(join(args.out_dir, "topics.json"), 'w') as file:
        file.write(json.dumps(parse_topic_model(topic_model_path,
                                                args.max_words_per_topic)))

    with open(join(args.out_dir, "papers.json"), 'w') as file:
        file.write(json.dumps(parse_papers(papers_path)))

    with open(join(args.out_dir, "eval.json"), 'w') as file:
        file.write(json.dumps(parse_eval(eval_path)))

    for template_file in glob("{}/*".format(template_dir_path)):
        shutil.copy2(template_file, args.out_dir)
