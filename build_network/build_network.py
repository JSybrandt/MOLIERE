#!/usr/bin/env python3

'''
This software is designed to build the Moliere Knowledge Network
(MKN) from a release of MEDLINE.
'''

import argparse
import os
import os.path
import subprocess
import sys
from ftplib import FTP
import multiprocessing
from multiprocessing import Pool
import string
import json

global VERBOSE
HOME_ENV = "MOLIERE_HOME"
DATA_ENV = "MOLIERE_DATA"
NUM_THREADS_ENV = "OMP_NUM_THREADS"

MEDLINE_FTP_DOMAIN = "ftp.ncbi.nlm.nih.gov"


def vprint(*args, **kargs):
    global VERBOSE
    if VERBOSE:
        print(*args, **kargs)


def getCmdStr(linkPath, cmd):
    s = os.path.join(linkPath, cmd)
    if os.path.isfile(s):
        return s
    else:
        raise ValueError(s + " is not a file")


def checkFile(path):
    if not os.path.isfile(path):
        print("FAILED TO FIND " + path, file=sys.stderr)
        raise
    if os.stat(path).st_size == 0:
        print("EMPTY: " + path, file=sys.stderr)
        raise


def shouldRemake(path, args):
    if not os.path.isfile(path):
        return True
    if os.stat(path).st_size == 0:
        return True
    if args.rebuild:
        return True
    return False


def unzip(file_path):
    if file_path.endswith(".gz") and os.path.isfile(file_path):
        vprint("Unzipping", file_path)
        subprocess.call(["gunzip", file_path])
    else:
        raise ValueError("Attempted to unzip the wrong file.")


def untar(file_path, to_dir):
    if file_path.endswith(".tar.gz") and os.path.isfile(file_path):
        vprint("Unzipping", file_path, "to", to_dir)
        os.makedirs(to_dir, exist_ok=True)
        subprocess.call(["tar", "xf", file_path, '-C', to_dir])
    else:
        raise ValueError("Attempted to untar the wrong file.")


def cleanTmp(tmp_dir):
    vprint("cleaning temp")
    # cleanup
    subprocess.call([
        'rm', '-rf', tmp_dir
    ])
    os.makedirs(tmp_dir, exist_ok=True)


def downloadPhraseData(dir):
    vprint("Making", dir)
    os.makedirs(dir, exist_ok=True)

    FTP_DIR = "/pub/wilbur/PHRASES/"
    FILE_NAME = "PubMed_Phrases.tar.gz"
    vprint("Connecting", MEDLINE_FTP_DOMAIN)
    ftp = FTP(MEDLINE_FTP_DOMAIN)
    ftp.login()
    ftp.cwd(FTP_DIR)
    vprint("Downloading", FILE_NAME)
    with open(os.path.join(dir, FILE_NAME), 'wb') as file:
        ftp.retrbinary('RETR ' + FILE_NAME, file.write)
    ftp.quit()
    ftp.close()
    vprint("Disconnected")
    untar(os.path.join(dir, FILE_NAME), dir)
    phrase_path = os.path.join(dir, "all_dictionary.txt")
    if not os.path.isfile(phrase_path):
        raise ValueError("Failed to find all_dictionary.txt in ", dir)
    return phrase_path


def downloadMedline(dir):
    FTP_DIR = "/pubmed/baseline/"
    vprint("Making", dir)
    os.makedirs(dir, exist_ok=True)
    ftp = FTP(MEDLINE_FTP_DOMAIN)
    ftp.login()
    ftp.cwd(FTP_DIR)

    for file_name in ftp.nlst():
        if file_name.endswith(".xml.gz"):
            vprint("Found:", file_name)
            with open(os.path.join(dir, file_name), 'wb') as file:
                ftp.retrbinary('RETR ' + file_name, file.write)
    ftp.quit()
    ftp.close()

    with Pool(int(os.environ[NUM_THREADS_ENV])) as p:
        p.map(unzip, [os.path.join(dir, f) for f in os.listdir(dir)])


if __name__ == "__main__":
    if HOME_ENV not in os.environ:
        raise ValueError(HOME_ENV + " not set")
    home_path = os.environ[HOME_ENV]
    data_path = None
    if(DATA_ENV in os.environ):
        data_path = os.environ[DATA_ENV]

    if(NUM_THREADS_ENV not in os.environ):
        raise ValueError("Needs " + NUM_THREADS_ENV + " set.")

    linkPath = "{}/build_network/bin".format(home_path)

    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--data-home",
                        action="store",
                        dest="data_path",
                        help="specifies a data directory")
    parser.add_argument("-u", "--umls-dir",
                        action="store",
                        dest="umls_dir",
                        help="location of UMLS installation")
    parser.add_argument("--filter-year",
                        action="store",
                        dest="filter_year",
                        help="train the network on historical data.")
    parser.add_argument("-V", "--vector-dim",
                        action="store",
                        dest="vector_dim",
                        default="100",
                        help="dimensionality of word embedding")
    parser.add_argument("-N", "--num-nearest-neighbors",
                        action="store",
                        dest="num_nn",
                        default="100",
                        help="number of neighbors per entity")
    parser.add_argument("-m", "--min-count",
                        action="store",
                        dest="min_count",
                        default="5",
                        help="min number of times for a word to occur")
    parser.add_argument("--plugin-config",
                        action="store",
                        dest="plugin_config",
                        help="json config file describing additional data.")
    parser.add_argument("--download",
                        action="store_true",
                        dest="download",
                        help="If set, download xml data")
    parser.add_argument("--skip-phrase-training",
                        action="store_true",
                        dest="skip_phrase_training",
                        help="If set, we parse with autophrase,\
                              but don't train a phrase model")
    parser.add_argument("--rebuild",
                        action="store_true",
                        dest="rebuild",
                        help="If set, remake existing files")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="if set, run pipeline with verbose flags.")

    args = parser.parse_args()

    if args.data_path is not None:
        data_path = args.data_path
    if data_path is None:
        raise ValueError("Must either supply MOLIERE_DATA through env or cmd")

    if not os.path.isdir(data_path):
        print(data_path)
        raise ValueError("Failed to supply valid data dir")

    if int(args.vector_dim) <= 0:
        print(args.vector_dim)
        raise ValueError("Vector dimensionality must be positive")

    if int(args.num_nn) <= 0:
        print(args.num_nn)
        raise ValueError("num nearest neighbors must be positive")

    if int(args.min_count) <= 0:
        print(args.min_count)
        raise ValueError("min_count must be positive")

    if args.filter_year and int(args.filter_year) <= 0:
        print(args.filter_year)
        raise ValueError("If you supply a filter year, it must be positive")

    global VERBOSE
    VERBOSE = args.verbose

    plugin_data = None
    if args.plugin_config:
        vprint("Checking plugin config", args.plugin_config)
        with open(args.plugin_config) as file:
            plugin_data = json.load(file)
        assert("text-plugins" in plugin_data)
        assert("edge-plugins" in plugin_data)
        for path in plugin_data["text-plugins"]:
            vprint("Adding text-plugin data:", path)
            checkFile(path)
        for path, weight in plugin_data["edge-plugins"]:
            vprint("Adding edge-plugin data:", path, ":", weight)
            checkFile(path)
            assert(weight >= 0)

    num_threads = str(multiprocessing.cpu_count())
    tmp_dir = "{}/tmp".format(data_path)
    cleanTmp(tmp_dir)

    # make dirs
    os.makedirs("{}/processedText".format(data_path), exist_ok=True)
    os.makedirs("{}/fastText".format(data_path), exist_ok=True)
    os.makedirs("{}/network".format(data_path), exist_ok=True)
    xml_dir = "{}/xml".format(data_path)
    os.makedirs(xml_dir, exist_ok=True)

    if args.umls_dir is not None:
        mrconso_path = "{}/META/MRCONSO.RRF".format(args.umls_dir)
        mrrel_path = "{}/META/MRREL.RRF".format(args.umls_dir)
        checkFile(mrconso_path)
        checkFile(mrrel_path)

    if args.download:
        vprint("Downloading medline into", xml_dir)
        downloadMedline(xml_dir)
    elif os.listdir(xml_dir) == []:
        raise ValueError("There are no files in " + xml_dir +
                         ". Run again with '--download'")

    # ABSTRACT FILE
    ab_raw_path = "{}/processedText/abstract.raw.txt".format(data_path)
    if shouldRemake(ab_raw_path, args):
        cmd = getCmdStr(linkPath, "parseXML")

        def parse(path):
            if path.endswith(".xml"):
                vprint("Parsing", path)
                subprocess.call([
                    cmd,
                    '-i', os.path.join(xml_dir, path),
                    '-o', os.path.join(tmp_dir, path),
                    '-k',  # include author supplied keywords
                    '-v' if VERBOSE else '',
                    '-y' if args.filter_year else '',
                    args.filter_year if args.filter_year else '',
                ])

        # Parse each xml file in parallel
        with Pool(int(os.environ[NUM_THREADS_ENV])) as p:
            p.map(parse, os.listdir(xml_dir))

        # cat all abstracts together
        with open(ab_raw_path, 'w') as file:
            for path in os.listdir(tmp_dir):
                subprocess.call([
                    'cat', os.path.join(tmp_dir, path)
                ], stdout=file)

        cleanTmp(tmp_dir)
    else:
        vprint("Reusing", ab_raw_path)
    checkFile(ab_raw_path)

    if plugin_data:
        # we are going to append all the text plugins
        # to our parsed abstracts
        with open(ab_raw_path, 'a') as write_file:
            for path in plugin_data['text-plugins']:
                vprint("Adding", path, "to", ab_raw_path)
                with open(path) as read_file:
                    for line in read_file:
                        write_file.write(line)

    # AUTOPHRASE Expert Phrases
    phrase_path = "{}/processedText/expertPhrases.txt" .format(data_path)
    phrase_download_path = "{}/processedText/phrase_data".format(data_path)
    if shouldRemake(phrase_path, args):
        phrase_download_path = "{}/processedText/phrase_data".format(data_path)
        raw_phrase_file = downloadPhraseData(phrase_download_path)
        cmd = getCmdStr(linkPath, "cleanRawText")
        subprocess.call([
            cmd,
            '-i', raw_phrase_file,
            '-o', phrase_path,
            '-v' if VERBOSE else ''])
        vprint("Removing trailing periods from phrase file")
        subprocess.call([
            'sed',
            '-i', '-E', r"'s/\s\.//g'",
            phrase_path])
    else:
        vprint("Reusing", phrase_path)
    checkFile(phrase_path)

    # AUTOPHRASE Training and parsing
    ab_pre_path = "{}/processedText/abstracts.pre.txt".format(data_path)
    if shouldRemake(ab_pre_path, args):
        if(not args.skip_phrase_training):
            vprint("Running autophrase")
            cmd = getCmdStr(linkPath, "../../external/trainAutoPhrase.sh")
            subprocess.call([cmd, ab_raw_path, num_threads,
                             phrase_path, ab_pre_path],
                            stdout=sys.stdout if VERBOSE else
                            open("/dev/null", 'w'))
        cmd = getCmdStr(linkPath, "../../external/parseAutoPhrase.sh")
        subprocess.call([cmd, ab_raw_path, num_threads,
                         phrase_path, ab_pre_path],
                        stdout=sys.stdout if VERBOSE else
                        open("/dev/null", 'w'))
        vprint("Processing autophrase")
        cmd = getCmdStr(linkPath, "apHighlight2Underscore")
        tmp_path = os.path.join(tmp_dir, "tmp.txt")
        subprocess.call([cmd, '-i', ab_pre_path, '-o', tmp_path,
                         "-v" if VERBOSE else ""])
        subprocess.call(['mv', tmp_path, ab_pre_path])
        cleanTmp(tmp_dir)
    else:
        vprint("Reusing", ab_pre_path)
    checkFile(ab_pre_path)

    # word 2 vec
    ngram_vec_path = "{}/fastText/ngram.vec".format(data_path)
    if shouldRemake(ngram_vec_path, args):
        ngram_bin_path = "{}/fastText/ngram.bin".format(data_path)
        cmd = getCmdStr(linkPath, "fasttext")
        vprint("Running fasttext")
        subprocess.call([
            cmd,
            "skipgram",
            "-input", ab_pre_path,
            "-output", os.path.splitext(ngram_vec_path)[0],
            "-thread", num_threads,
            "-dim", args.vector_dim,
            "-minCount", args.min_count
        ], stdout=sys.stdout if VERBOSE else open("/dev/null", 'w'))
        checkFile(ngram_vec_path)
        vprint("Removing", ngram_bin_path)
        subprocess.call(['rm', '-f', ngram_bin_path])
        vprint("cleaning", ngram_vec_path)
        # trash first line (status line)
        subprocess.call(['sed', '-i', '1d', ngram_vec_path])
        # trash period
        subprocess.call(['sed', '-i', r'/^.\s.*/d', ngram_vec_path])
        # trash space
        subprocess.call(['sed', '-i', r'/^<\/s>\s.*/d', ngram_vec_path])

    else:
        vprint("Reusing", ngram_vec_path)
    checkFile(ngram_vec_path)

    abstract_path = "{}/processedText/abstracts.txt".format(data_path)
    if shouldRemake(abstract_path, args):
        cmd = getCmdStr(linkPath, "filterWordsByCount")
        vprint("Filtering words and removeing periods")
        subprocess.call([
            cmd,
            '-i', ab_pre_path,
            '-o', abstract_path,
            '-m', args.min_count,
            '--skip-second',
            '--remove-period',
            '-v' if VERBOSE else ''
        ])
    else:
        vprint("Reusing", abstract_path)
    checkFile(abstract_path)

    pmid_vec_path = "{}/fastText/pmid.vec".format(data_path)
    if shouldRemake(pmid_vec_path, args):
        cmd = getCmdStr(linkPath, "makeCentroid")
        vprint("Abstract file 2 centroids:")
        subprocess.call([
            cmd,
            '-i', abstract_path,
            '--skip-second',
            '-V', ngram_vec_path,
            '-o', pmid_vec_path,
            '-v' if VERBOSE else ''
        ])
    else:
        vprint("Reusing", pmid_vec_path)
    checkFile(pmid_vec_path)

    pmid_network_path = "{}/network/pmid.edges".format(data_path)
    if shouldRemake(pmid_network_path, args):
        cmd = getCmdStr(linkPath, "makeApproxNNN")
        vprint("Running FLANN to make pmid net")
        subprocess.call([
            cmd,
            '-i', pmid_vec_path,
            '-o', pmid_network_path,
            '-n',
            '-k', args.num_nn,
            '-v' if VERBOSE else ''
        ])
    else:
        vprint("Reusing", pmid_network_path)
    checkFile(pmid_network_path)

    ngram_network_path = "{}/network/ngram.edges".format(data_path)
    if shouldRemake(ngram_network_path, args):
        cmd = getCmdStr(linkPath, "makeApproxNNN")
        vprint("Running FLANN to make pmid net")
        subprocess.call([
            cmd,
            '-i', ngram_vec_path,
            '-o', ngram_network_path,
            '-n',
            '-k', args.num_nn,
            '-v' if VERBOSE else ''
        ])
    else:
        vprint("Reusing", ngram_network_path)
    checkFile(ngram_network_path)

    pmid_ngram_edges_path = "{}/network/pmid2ngram.edges"\
                            .format(data_path)
    if(shouldRemake(pmid_ngram_edges_path, args)):
        cmd = getCmdStr(linkPath, "makeDocumentEdges")
        vprint("Creating edges from docs to keywords")
        subprocess.call([
            cmd,
            '-i', abstract_path,
            '-o', pmid_ngram_edges_path,
            '--skip-second',
            '-v' if VERBOSE else ''
        ])
    else:
        vprint("Reusing", pmid_ngram_edges_path)
    checkFile(pmid_ngram_edges_path)

    if args.umls_dir is not None:
        umls_text_path = "{}/processedText/umls.txt".format(data_path)
        if shouldRemake(umls_text_path, args):
            cmd = getCmdStr(linkPath, "parseUmlsText")
            subprocess.call([
                cmd,
                '-i', mrconso_path,
                '-o', umls_text_path,
                '-v' if VERBOSE else ''
            ])
            tmp_path = os.path.join(tmp_dir, "umls_tmp.txt")
            cmd = getCmdStr(linkPath, "../../external/parseAutoPhrase.sh")
            subprocess.call([cmd, umls_text_path, num_threads,
                             phrase_path, tmp_path],
                            stdout=sys.stdout if VERBOSE else
                            open("/dev/null", 'w'))
            subprocess.call(['mv', tmp_path, umls_text_path])
            vprint("making phrases")
            cmd = getCmdStr(linkPath, "apHighlight2Underscore")
            subprocess.call([cmd, '-i', umls_text_path, '-o', tmp_path,
                             "-v" if VERBOSE else ""])
            subprocess.call(['mv', tmp_path, umls_text_path])
            vprint("Removing stop token")
            subprocess.call(['sed', '-i', r's/\s\.//g', umls_text_path])
            cleanTmp(tmp_path)
        else:
            vprint("Reusing", umls_text_path)
        checkFile(umls_text_path)

        umls_vec_path = "{}/fastText/umls.vec".format(data_path)
        if shouldRemake(umls_vec_path, args):
            cmd = getCmdStr(linkPath, "makeCentroid")
            vprint("Abstract file 2 centroids:")
            subprocess.call([
                cmd,
                '-i', umls_text_path,
                '-V', ngram_vec_path,
                '-o', umls_vec_path,
                '-v' if VERBOSE else ''
            ])
        else:
            vprint("Reusing", umls_vec_path)
        checkFile(umls_vec_path)

        umls_ngram_edges_path = "{}/network/umls2ngram.edges"\
                                .format(data_path)
        if(shouldRemake(umls_ngram_edges_path, args)):
            cmd = getCmdStr(linkPath, "makeDocumentEdges")
            vprint("Creating edges from docs to keywords")
            subprocess.call([
                cmd,
                '-i', umls_text_path,
                '-o', umls_ngram_edges_path,
                '-v' if VERBOSE else ''
            ])
        else:
            vprint("Reusing", umls_ngram_edges_path)
        checkFile(umls_ngram_edges_path)

        umls_network_path = "{}/network/umls2umls.edges"\
                            .format(data_path)
        if shouldRemake(umls_network_path, args):
            cmd = getCmdStr(linkPath, "parseUmlsEdges")
            vprint("Extraning umls curated edges")
            subprocess.call([
                cmd,
                '-i', mrrel_path,
                '-o', umls_network_path,
                '-v' if VERBOSE else ''
            ])
        else:
            vprint("Reusing", umls_network_path)
        checkFile(umls_network_path)
    else:
        vprint("Skipping UMLS data")

    final_network_path = "{}/network/final".format(data_path)
    final_network_edge_path = final_network_path + ".bin.edges"
    final_network_label_path = final_network_path + ".labels"
    if shouldRemake(final_network_edge_path, args):
        cmd = [getCmdStr(linkPath, "finalizeNetwork"),
               '-o', final_network_path,
               '-v' if VERBOSE else '',
               pmid_network_path, '1',
               ngram_network_path, '1',
               pmid_ngram_edges_path, '1']
        if args.umls_dir is not None:
            cmd += [umls_ngram_edges_path, '1.5',
                    umls_network_path, '2']
        if plugin_data:
            # we are going to add edge-plugins
            for path, weight in plugin_data['edge-plugins']:
                vprint("Adding", path, ":", weight)
                cmd += [path, str(weight)]
        subprocess.call(cmd)
    else:
        vprint("Reusing", final_network_path)
    checkFile(final_network_edge_path)
    checkFile(final_network_label_path)
