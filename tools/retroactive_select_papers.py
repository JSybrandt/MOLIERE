#!/usr/bin/env python3

import os
from glob import glob
import argparse
import subprocess

HOME_ENV = "MOLIERE_HOME"
DATA_ENV = "MOLIERE_DATA"
global VERBOSE


def vprint(*args, **kargs):
    global VERBOSE
    if VERBOSE:
        print(*args, **kargs)


def main():

    if HOME_ENV not in os.environ:
        raise ValueError(HOME_ENV + " not set")

    if DATA_ENV not in os.environ:
        raise ValueError(DATA_ENV + " not set")

    home_path = os.environ[HOME_ENV]
    data_path = os.environ[DATA_ENV]

    parser = argparse.ArgumentParser()
    parser.add_argument("-r", "--result-dir",
                        action="store",
                        help="specifies which result to get papers for.")
    parser.add_argument("-d", "--data-path",
                        help="specifies a data directory")
    parser.add_argument("-f", "--fix-bow",
                        action="store_true",
                        help="if set, the bow file needs to be recalculated")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        help="if set, run pipeline with verbose flags.")

    args = parser.parse_args()

    global VERBOSE
    VERBOSE = args.verbose

    vprint(args)

    if args.data_path is not None:
        data_path = data_path

    if args.result_dir is None:
        raise ValueError("Must supply result file")

    while args.result_dir[-1] == '/':
        args.result_dir = args.result_dir[:-1]

    if not os.path.isdir(args.result_dir):
        raise ValueError("Failed to supply a real result dir.")

    query_name = os.path.basename(args.result_dir)
    query_elem = query_name.split("---")
    if len(query_elem) < 2:
        raise ValueError("Query Result name does not contain id split by ---")

    abstract_path = "{}/processedText/abstracts.txt".format(data_path)
    if not os.path.isfile(abstract_path):
        raise ValueError("Data path does not contain abstract file")

    label_path = "{}/network/final.labels".format(data_path)
    if not os.path.isfile(label_path):
        raise ValueError("Failed to supply valid label file.")

    graph_path = "{}/network/final.bin.edges".format(data_path)
    if not os.path.isfile(graph_path):
        raise ValueError("Failed to supply valid graph file.")

    ngram_vec_path = "{}/fastText/ngram.vec".format(data_path)
    if not os.path.isfile(ngram_vec_path):
        raise ValueError("Failed to supply valid ngram file.")

    pmid_vec_path = "{}/fastText/pmid.vec".format(data_path)
    if not os.path.isfile(pmid_vec_path):
        raise ValueError("Failed to supply valid pmid vec file.")

    umls_vec_path = "{}/fastText/umls.vec".format(data_path)
    if not os.path.isfile(umls_vec_path):
        raise ValueError("Failed to supply valid umls vec file.")

    cloud_path = glob("{}/*.cloud".format(args.result_dir))

    if len(cloud_path) != 1:
        raise ValueError("Result dir " +
                         args.result_dir +
                         " must contain exactly one cloud file")
    cloud_path = cloud_path[0]

    bow_path = glob("{}/*.bag".format(args.result_dir))
    if len(bow_path) != 1:
        bow_path = "{}/{}.bag".format(args.result_dir, query_name)
        args.fix_bow = True
    else:
        bow_path = bow_path[0]

    topic_view_path = glob("{}/*.view".format(args.result_dir))
    if len(topic_view_path) != 1:
        raise ValueError("Result dir " +
                         args.result_dir +
                         " must contain exactly one view file")
    topic_view_path = topic_view_path[0]

    num_topics = topic_view_path.split('.')[-2]

    if args.fix_bow:
        vprint("CREATING BOW")
        cmd = "{}/run_query/bin/cloud2Bag".format(home_path)
        if not os.path.isfile(cmd):
            raise RuntimeError(cmd + " does not exist. Rebuild?")
        subprocess.call([
            cmd,
            '-c', cloud_path,
            '-o', bow_path,
            '-l', label_path,
            '-a', abstract_path,
            '-v' if VERBOSE else ''
        ])

    vprint("CREATING TOPIC MODEL RAW")
    topic_model_path = "{}/{}.model".format(args.result_dir, query_name)
    cmd = "{}/tools/bin/ldaView2Model".format(home_path)
    if not os.path.isfile(cmd):
        raise RuntimeError(cmd + " does not exist. Rebuild?")
    subprocess.call([
        cmd,
        '-i', topic_view_path,
        '-o', topic_model_path,
        '-v' if VERBOSE else ''
    ])

    vprint("CREATING INFERENCE ON {} TOPICS".format(num_topics))
    inference_path = "{}/{}.inf".format(args.result_dir, query_name)
    cmd = "{}/external/plda/infer".format(home_path)
    if not os.path.isfile(cmd):
        raise RuntimeError(cmd + " does not exist. Rebuild?")
    subprocess.call([
        cmd,
        '--num_topics', num_topics,
        '--alpha', '1',
        '--beta', '0.01',
        '--training_data_file', bow_path,
        '--inference_data_file', bow_path,
        '--inference_result_file', inference_path,
        '--model_file', topic_model_path,
        '--total_iterations', '500',
        '--burn_in_iterations', '50'
    ])

    vprint("GETTING PAPERS")
    papers_path = "{}/papers.txt".format(args.result_dir)
    cmd = "{}/tools/bin/getPapers".format(home_path)
    if not os.path.isfile(cmd):
        raise RuntimeError(cmd + " does not exist. Rebuild?")

    subprocess.call([
        cmd,
        '-c', cloud_path,
        '-i', inference_path,
        '-a', abstract_path,
        '-l', label_path,
        '-g', graph_path,
        '-o', papers_path,
        '-m', topic_view_path,
        '-N', ngram_vec_path,
        '-P', pmid_vec_path,
        '-U', umls_vec_path,
        '-s', query_elem[0],
        '-t', query_elem[-1],
        '-v' if VERBOSE else ""
    ])


if __name__ == "__main__":
    main()
