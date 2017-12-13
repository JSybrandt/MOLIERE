#!/software/python/3.4/bin/python
import argparse
import os
import sys
import subprocess


HOME_ENV = "MOLIERE_HOME"

FIND_PATH = "{}/findPath"
FIND_CLOUD = "{}/findCloud"
CLOUD2BAG = "{}/cloud2Bag"
PLDA = "{}/mpi_lda"
VIEW_MODEL = "{}/view_model.py"
EVAL = "{}/evalHybrid"
EVAL_PARAM = "0 1.5203 .29332 1.9804 .87312 2.2369 .38104 1.0092"


def createOrRecoverFile(args, extension):
    path = "{}/{}---{}.{}".format(args.cache,
                                  args.wordA,
                                  args.wordB,
                                  extension)
    if os.path.isfile(path) and not args.reconstruct:
        return (path, True)
    else:
        return (path, False)


def main():

    if HOME_ENV not in os.environ:
        print("ERROR: ", HOME_ENV, " not set")
        return 1

    homePath = os.environ[HOME_ENV]
    linkPath = "{}/code/components/links".format(homePath)

    parser = argparse.ArgumentParser()
    parser.add_argument("-c", "--cache",
                        action="store",
                        dest="cache",
                        default="/tmp",
                        help="specifies where to store cached files.")
    parser.add_argument("-d", "--data_home",
                        action="store",
                        dest="data_home",
                        default="{}/data".format(homePath),
                        help="specifies an anternate data directory")
    parser.add_argument("-n", "--num_topics",
                        action="store",
                        dest="num_topics",
                        default="100",
                        help="specifies the number of topics to generate.")
    parser.add_argument("-e", "--ellipse_constant",
                        action="store",
                        dest="ellipse_constant",
                        default="1.4",
                        help="size of ellipse optimization")
    parser.add_argument("-m", "--move_here",
                        action="store_true",
                        dest="move_here",
                        help="move topic / analysis files to working dir")
    parser.add_argument("-r", "--reconstruct",
                        action="store_true",
                        dest="reconstruct",
                        help="if set, do not reuse existing cached files.")
    parser.add_argument("-s", "--skip_sanitize",
                        action="store_true",
                        dest="skip_sanitize",
                        help="if set, do not check for input in labels.")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="if set, run pipeline with verbose flags.")
    parser.add_argument("wordA")
    parser.add_argument("wordB")

    args = parser.parse_args()

    graphFile = "{}/network/final.bin.edges".format(args.data_home)
    labelFile = "{}/network/final.labels".format(args.data_home)
    abstractFile = "{}/processedText/abstracts.txt".format(args.data_home)
    ngramVecs = "{}/fastText/canon.vec".format(args.data_home)
    pmidVecs = "{}/fastText/centroids.data".format(args.data_home)
    umlsVecs = "{}/fastText/umls.data".format(args.data_home)
    verboseFlag = '-v' if args.verbose else ' '

    args.wordA = args.wordA.lower()
    args.wordB = args.wordB.lower()

    # always put then in order
    if args.wordA > args.wordB:
        args.wordB, args.wordA = (args.wordA, args.wordB)

    if not args.skip_sanitize:
        if args.verbose:
            print("Validating input")

        foundA = foundB = False
        with open(labelFile) as lFile:
            for line in lFile:
                line = line.strip()
                if line == args.wordA:
                    foundA = True
                if line == args.wordB:
                    foundB = True
                if foundA and foundB:
                    break

        if not foundA:
            print("Error, failed to find", args.wordA, "in", labelFile)
            return 1

        if not foundB:
            print("Error, failed to find", args.wordB, "in", labelFile)
            return 1

    path_path, reuse = createOrRecoverFile(args, "path")
    if not reuse:
        if args.verbose:
            print("Running findPath, creating", path_path)
        subprocess.call([
            FIND_PATH.format(linkPath),
            '-g', graphFile,
            '-l', labelFile,
            '-s', args.wordA,
            '-t', args.wordB,
            '-V', ngramVecs,
            '-U', umlsVecs,
            '-e', args.ellipse_constant,
            '-o', path_path,
            verboseFlag
        ])
    elif args.verbose:
        print("reusing: ", path_path)

    cloud_path, reuse = createOrRecoverFile(args, "cloud")
    if not reuse:
        if args.verbose:
            print("Running path2cloud, creating", cloud_path)
        subprocess.call([
            FIND_CLOUD.format(linkPath),
            '-g', graphFile,
            '-l', labelFile,
            '-p', path_path,
            '-o', cloud_path,
            verboseFlag
        ])
    elif args.verbose:
        print("reusing: ", cloud_path)

    bag_path, reuse = createOrRecoverFile(args, "bag")
    if not reuse:
        if args.verbose:
            print("Running cloud2bag, creating", bag_path)
        subprocess.call([
            CLOUD2BAG.format(linkPath),
            '-c', cloud_path,
            '-o', bag_path,
            '-l', labelFile,
            '-a', abstractFile,
            verboseFlag
        ])
    elif args.verbose:
        print("reusing: ", bag_path)

    model_ext = "{}.model".format(args.num_topics)
    model_path, reuse = createOrRecoverFile(args, model_ext)
    if not reuse:
        if args.verbose:
            print("Running plda, creating", model_path)
        nullFile = open("/dev/null", 'w')
        subprocess.call([
            'mpiexec', PLDA.format(linkPath),
            '--num_topics', args.num_topics,
            '--alpha', '1',
            '--beta', '0.01',
            '--training_data_file', bag_path,
            '--model_file', model_path,
            '--total_iterations', '500',
            '--burn_in_iterations', '50'
        ], stdout=nullFile)
        nullFile.close()
    elif args.verbose:
        print("reusing: ", model_path)

    view_ext = "{}.view".format(args.num_topics)
    view_path, reuse = createOrRecoverFile(args, view_ext)
    if not reuse:
        if args.verbose:
            print("Running make view, creating", view_path)
        with open(view_path, 'w') as view_file:
            subprocess.call([
                VIEW_MODEL.format(linkPath),
                model_path
            ], stdout=view_file)
    elif args.verbose:
        print("reusing: ", view_path)

    analysis_ext = "{}.eval".format(args.num_topics)
    analysis_path, reuse = createOrRecoverFile(args, analysis_ext)
    if not reuse:
        if args.verbose:
            print("Running analysis, creating", analysis_path)
        with open(analysis_path, 'w') as analysis_file:
            subprocess.call([
                EVAL.format(linkPath),
                '-p', EVAL_PARAM,
                '-m', view_path,
                '-n', ngramVecs,
                '-c', umlsVecs,
                '-p', pmidVecs,
                '-s', args.wordA,
                '-t', args.wordB
            ], stdout=analysis_file)
    elif args.verbose:
        print("reusing: ", analysis_path)

    if args.move_here:
        if args.verbose:
            print("Moving", view_path, " to local dir")
        subprocess.call(['cp', view_path, './'])
        if args.verbose:
            print("Moving", analysis_path, " to local dir")
        subprocess.call(['cp', analysis_path, './'])


if __name__ == "__main__":
    main()
