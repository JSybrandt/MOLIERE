#!/usr/bin/env python3
import argparse
import os
import sys
import subprocess
import psutil
import re


HOME_ENV = "MOLIERE_HOME"
DATA_ENV = "MOLIERE_DATA"

FIND_PATH = "{}/findPath"
FIND_CLOUD = "{}/findCloud"
CLOUD2BAG = "{}/cloud2Bag"
PLDA = "{}/mpi_lda"
VIEW_MODEL = "{}/view_model.py"

EVAL_L2 = "{}/eval_l2"
EVAL_TPW = "{}/eval_tpw"
EVAL_TWE = "{}/eval_twe"
EVAL_TOPIC_PATH = "{}/eval_topic_path"
EVAL_HYBRID = "{}/evalHybrid.py"


def checkFile(path):
    if not os.path.isfile(path):
        print("FAILED TO MAKE " + path, file=sys.stderr)
        raise
    if os.stat(path).st_size == 0:
        print("EMPTY: " + path, file=sys.stderr)
        raise


def getQueryName(args):
    return "---".join(args.query_words)


def createOrRecoverFile(args, sub_dir, name, extension):
    dir_path = "{}/{}".format(args.cache, sub_dir)
    if not os.path.isdir(dir_path):
        os.mkdir(dir_path)
    file_path = "{}/{}.{}".format(dir_path,
                                  name,
                                  extension)
    if(os.path.isfile(file_path) and
       os.stat(file_path).st_size > 0 and
       not args.reconstruct):
        return (file_path, True)
    else:
        return (file_path, False)


def cleanInput(s):
    # if its a UMLS id
    if re.match(r'^C[0-9]+$', s):
        return s
    # if its a PMID
    if re.match(r'^PMID[0-9]+$', s):
        return s
    return s.lower()


def main():

    if HOME_ENV not in os.environ:
        raise ValueError(HOME_ENV + " not set")

    home_path = os.environ[HOME_ENV]
    data_path = None
    if(DATA_ENV in os.environ):
        data_path = os.environ[DATA_ENV]
    link_path = "{}/run_query/bin".format(home_path)

    parser = argparse.ArgumentParser()
    parser.add_argument("-c", "--cache",
                        default="/tmp",
                        help="specifies where to store cached files.")
    parser.add_argument("-d", "--data-path",
                        help="specifies a data directory")
    parser.add_argument("-n", "--num-topics",
                        default="20",
                        help="specifies the number of topics to generate.")
    parser.add_argument("-!", "--no-analysis",
                        action="store_true",
                        help="If set, don't create any analysis files.")
    parser.add_argument("-m", "--move-here",
                        action="store_true",
                        help="move topic / analysis files to working dir")
    parser.add_argument("-r", "--reconstruct",
                        action="store_true",
                        help="if set, do not reuse existing cached files.")
    parser.add_argument("-s", "--skip-sanitize",
                        action="store_true",
                        help="if set, do not check for input in labels.")
    parser.add_argument("--cloud-size",
                        action="store",
                        default="3000",
                        help="Nuber of abstracts per node in cloud.")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        help="if set, run pipeline with verbose flags.")
    parser.add_argument("query_words",
                        nargs=argparse.REMAINDER,
                        help="(at least 2) query words." +
                             "additional words specify required intermediates")

    args = parser.parse_args()

    if args.verbose:
        print(args)

    if args.data_path is not None:
        data_path = args.data_path
    if data_path is None:
        raise ValueError("Must either supply MOLIERE_DATA through env or cmd")

    if len(args.query_words) < 2:
        raise ValueError("Must supply at least 2 query words!")

    if int(args.cloud_size) <= 0:
        raise ValueError("Cloud-size must be a positive number")

    hadToRebuild = False
    graph_path = "{}/network/final.bin.edges".format(data_path)
    label_path = "{}/network/final.labels".format(data_path)
    abstract_path = "{}/processedText/abstracts.txt".format(data_path)
    ngram_vec_path = "{}/fastText/ngram.vec".format(data_path)
    pmid_vec_path = "{}/fastText/pmid.vec".format(data_path)
    umls_vec_path = "{}/fastText/umls.vec".format(data_path)
    verbose_flag = '-v' if args.verbose else ' '

    args.query_words = [cleanInput(x) for x in args.query_words]
    wordA = args.query_words[0]
    wordB = args.query_words[-1]

    # always put then in order
    if wordA > wordB:
        wordB, wordA = (wordA, wordB)

    if not args.skip_sanitize:
        if args.verbose:
            print("Validating input")

        found = [False for x in args.query_words]
        with open(label_path) as file:
            for line in file:
                line = line.strip()
                if line in args.query_words:
                    found[args.query_words.index(line)] = True
                if False not in found:
                    break

        for word, f in zip(args.query_words, found):
            if not f:
                raise ValueError("Failed to find", word)

    query_name = getQueryName(args)

    if args.verbose:
        print("Starting", query_name)

    path_path, reuse = createOrRecoverFile(args,
                                           query_name,
                                           query_name,
                                           "path")
    if not reuse or hadToRebuild:
        sub_path_paths = []
        for idx in range(len(args.query_words)-1):
            wordI = args.query_words[idx]
            wordJ = args.query_words[idx+1]
            sub_path_name = "---".join([wordI, wordJ])
            sub_path_path, reuse = createOrRecoverFile(args,
                                                       query_name,
                                                       sub_path_name,
                                                       "path")
            if not reuse or hadToRebuild:
                if args.verbose:
                    print("Running findPath, creating", sub_path_path)
                subprocess.call([
                    FIND_PATH.format(link_path),
                    '-g', graph_path,
                    '-l', label_path,
                    '-s', wordI,
                    '-t', wordJ,
                    '-o', sub_path_path,
                    verbose_flag
                ])
            elif args.verbose:
                print("reusing sub path:", sub_path_path)
            sub_path_paths.append(sub_path_path)
        if len(args.query_words) > 2:
            # need to concatinate paths together
            path_nodes = set()
            for p in sub_path_paths:
                with open(p, 'r') as p_file:
                    for line in p_file:
                        tokens = line.strip().split()
                        for t in tokens:
                            path_nodes.add(t)
            with open(path_path, "w") as file:
                file.write(" ".join(path_nodes) + "\n")

        hadToRebuild = True
    elif args.verbose:
        print("reusing: ", path_path)

    checkFile(path_path)

    cloud_path, reuse = createOrRecoverFile(args, query_name,
                                            query_name, "cloud")
    if not reuse or hadToRebuild:
        hadToRebuild = True
        if args.verbose:
            print("Running path2cloud, creating", cloud_path)
        subprocess.call([
            FIND_CLOUD.format(link_path),
            '-g', graph_path,
            '-l', label_path,
            '-A', args.cloud_size,
            '-p', path_path,
            '-o', cloud_path,
            verbose_flag
        ])
    elif args.verbose:
        print("reusing: ", cloud_path)

    checkFile(cloud_path)

    bag_path, reuse = createOrRecoverFile(args, query_name,
                                          query_name, "bag")
    if not reuse or hadToRebuild:
        hadToRebuild = True
        if args.verbose:
            print("Running cloud2bag, creating", bag_path)
        subprocess.call([
            CLOUD2BAG.format(link_path),
            '-c', cloud_path,
            '-o', bag_path,
            '-l', label_path,
            '-a', abstract_path,
            verbose_flag
        ])
    elif args.verbose:
        print("reusing: ", bag_path)

    checkFile(bag_path)

    view_ext = "{}.view".format(args.num_topics)
    view_path, reuse = createOrRecoverFile(args, query_name,
                                           query_name, view_ext)
    if not reuse or hadToRebuild:
        hadToRebuild = True

        # building the view requires the model
        model_ext = "{}.model".format(args.num_topics)
        model_path, reuse = createOrRecoverFile(args, query_name,
                                                query_name, model_ext)
        if args.verbose:
            print("Running plda, creating", model_path)
        nullFile = open("/dev/null", 'w')
        subprocess.call([
            'mpiexec', '-n', str(psutil.cpu_count()),
            PLDA.format(link_path),
            '--num_topics', args.num_topics,
            '--alpha', '1',
            '--beta', '0.01',
            '--training_data_file', bag_path,
            '--model_file', model_path,
            '--total_iterations', '500',
            '--burn_in_iterations', '50'
        ], stdout=nullFile)
        nullFile.close()

        checkFile(model_path)

        if args.verbose:
            print("Running make view, creating", view_path)
        with open(view_path, 'w') as view_file:
            subprocess.call([
                VIEW_MODEL.format(link_path),
                model_path
            ], stdout=view_file)

        checkFile(view_path)

        # cleanup, this file is big and we don't need it
        os.remove(model_path)
    elif args.verbose:
        print("reusing: ", view_path)

    checkFile(view_path)

    if args.move_here:
        if args.verbose:
            print("Moving", view_path, " to local dir")
        subprocess.call(['cp', view_path, './'])

    if args.no_analysis:
        if args.verbose:
            print("Skipping Analysis")
        return

    # intermediate analysis files
    eval_dir = "{}/eval".format(query_name)

    analysis_ext = "{}.l2.eval".format(args.num_topics)
    eval_l2_path, reuse = createOrRecoverFile(args, eval_dir,
                                              query_name, analysis_ext)
    if not reuse or hadToRebuild:
        hadToRebuild = True
        if args.verbose:
            print("Running analysis, creating", eval_l2_path)
        with open(eval_l2_path, 'w') as analysis_file:
            subprocess.call([
                EVAL_L2.format(link_path),
                '-m', view_path,
                '-N', ngram_vec_path,
                '-U', umls_vec_path,
                '-P', pmid_vec_path,
                '-s', wordA,
                '-t', wordB,
                '-e'
            ], stdout=analysis_file)
    elif args.verbose:
        print("reusing: ", eval_l2_path)

    checkFile(eval_l2_path)

    analysis_ext = "{}.tpw.eval".format(args.num_topics)
    eval_tpw_path, reuse = createOrRecoverFile(args, eval_dir,
                                               query_name, analysis_ext)
    if not reuse or hadToRebuild:
        hadToRebuild = True
        if args.verbose:
            print("Running analysis, creating", eval_tpw_path)
        with open(eval_tpw_path, 'w') as analysis_file:
            subprocess.call([
                EVAL_TPW.format(link_path),
                '-m', view_path,
                '-N', ngram_vec_path,
                '-U', umls_vec_path,
                '-P', pmid_vec_path,
                '-s', wordA,
                '-t', wordB
            ], stdout=analysis_file)
    elif args.verbose:
        print("reusing: ", eval_tpw_path)

    checkFile(eval_tpw_path)

    analysis_ext = "{}.twe.eval".format(args.num_topics)
    eval_twe_path, reuse = createOrRecoverFile(args, eval_dir,
                                               query_name, analysis_ext)
    if not reuse or hadToRebuild:
        hadToRebuild = True
        if args.verbose:
            print("Running analysis, creating", eval_twe_path)
        with open(eval_twe_path, 'w') as analysis_file:
            subprocess.call([
                EVAL_TWE.format(link_path),
                '-m', view_path,
                '-N', ngram_vec_path,
                '-U', umls_vec_path,
                '-P', pmid_vec_path,
                '-s', wordA,
                '-t', wordB
            ], stdout=analysis_file)
    elif args.verbose:
        print("reusing: ", eval_twe_path)

    checkFile(eval_twe_path)

    analysis_ext = "{}.path.eval".format(args.num_topics)
    eval_topic_path_path, reuse = createOrRecoverFile(args,
                                                      eval_dir,
                                                      query_name,
                                                      analysis_ext)
    if not reuse or hadToRebuild:
        hadToRebuild = True
        if args.verbose:
            print("Running analysis, creating", eval_topic_path_path)
        with open(eval_topic_path_path, 'w') as analysis_file:
            subprocess.call([
                EVAL_TOPIC_PATH.format(link_path),
                '-m', view_path,
                '-N', ngram_vec_path,
                '-U', umls_vec_path,
                '-P', pmid_vec_path,
                '-s', wordA,
                '-t', wordB
            ], stdout=analysis_file, stderr=subprocess.DEVNULL)
    elif args.verbose:
        print("reusing: ", eval_topic_path_path)

    checkFile(eval_topic_path_path)

    analysis_ext = "{}.hybrid.eval".format(args.num_topics)
    eval_hybrid_path, reuse = createOrRecoverFile(args, eval_dir,
                                                  query_name, analysis_ext)
    if not reuse or hadToRebuild:
        if args.verbose:
            print("Running analysis, creating", eval_hybrid_path)
        with open(eval_hybrid_path, 'w') as analysis_file:
            subprocess.call([
                EVAL_HYBRID.format(link_path),
                eval_l2_path,
                eval_tpw_path,
                eval_twe_path,
                eval_topic_path_path
            ], stdout=analysis_file)
    elif args.verbose:
        print("reusing: ", eval_hybrid_path)

    checkFile(eval_hybrid_path)

    if args.move_here:
        if args.verbose:
            print("Moving", eval_hybrid_path, " to local dir")
        subprocess.call(['cp', eval_hybrid_path, './'])


if __name__ == "__main__":
    main()
