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

global VERBOSE
HOME_ENV = "MOLIERE_HOME"


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
        print("FAILED TO MAKE " + path, file=sys.stderr)
        raise
    if os.stat(path).st_size == 0:
        print("EMPTY: " + path, file=sys.stderr)
        raise


def unzip(name):
    if name.endswith(".gz"):
        vprint("Unzipping", name)
        subprocess.call(["gunzip", os.path.join(name)])


def cleanTmp(tmp_dir):
    vprint("cleaning temp")
    # cleanup
    subprocess.call([
        'rm', '-rf', tmp_dir
    ])
    os.makedirs(tmp_dir, exist_ok=True)


def downloadMedline(dir):
    MEDLINE_URL = "ftp.ncbi.nlm.nih.gov"
    FTP_DIR = "/pubmed/baseline/"
    vprint("Making", dir)
    os.makedirs(dir, exist_ok=True)
    ftp = FTP(MEDLINE_URL)
    ftp.login()
    ftp.cwd(FTP_DIR)

    for file_name in ftp.nlst():
        break
        if file_name.endswith(".xml.gz"):
            vprint("Found:", file_name)
            with open(os.path.join(dir, file_name), 'wb') as file:
                ftp.retrbinary('RETR ' + file_name, file.write)
    ftp.quit()
    ftp.close()

    with Pool() as p:
        p.map(unzip, [os.path.join(dir, f) for f in os.listdir(dir)])


if __name__ == "__main__":
    if HOME_ENV not in os.environ:
        print("ERROR: ", HOME_ENV, " not set")
        exit(1)

    homePath = os.environ[HOME_ENV]
    linkPath = "{}/code/build_network/bin".format(homePath)

    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--data-home",
                        action="store",
                        dest="data_home",
                        default="{}/data".format(homePath),
                        help="specifies a data directory")
    parser.add_argument("-x", "--xml-dir",
                        action="store",
                        dest="xml_dir",
                        default="{}/rawData",
                        help="location to store raw XML files. {} -> data")
    parser.add_argument("-u", "--umls-dir",
                        action="store",
                        dest="umls_dir",
                        help="location of UMLS installation")
    parser.add_argument("-V", "--vector-dim",
                        action="store",
                        dest="vector_dim",
                        default="100",
                        help="dimensioality of word embedding")
    parser.add_argument("--skip-download",
                        action="store_true",
                        dest="skip_download",
                        help="If set, don't download xml data")
    parser.add_argument("--rebuild",
                        action="store_true",
                        dest="rebuild",
                        help="If set, remake existing files")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="if set, run pipeline with verbose flags.")

    args = parser.parse_args()

    if len([x for x in string.Formatter().parse(args.xml_dir)]) > 0:
        args.xml_dir = args.xml_dir.format(args.data_home)

    if not os.path.isdir(args.xml_dir):
        print(args.xml_dir)
        raise ValueError("Failed to supply valid xml dir")

    if not os.path.isdir(args.umls_dir):
        print(args.umls_dir)
        raise ValueError("Failed to supply valid umls dir")

    if not os.path.isdir(args.data_home):
        print(args.data_home)
        raise ValueError("Failed to supply valid data dir")

    if int(args.vector_dim) <= 0:
        print(args.vector_dim)
        raise ValueError("Vector dimensionality must be postive")

    global VERBOSE
    VERBOSE = args.verbose

    num_threads = str(multiprocessing.cpu_count())
    tmp_dir = "{}/tmp".format(args.data_home)
    cleanTmp(tmp_dir)

    # make dirs
    os.makedirs("{}/processedText".format(args.data_home), exist_ok=True)
    os.makedirs("{}/fastText".format(args.data_home), exist_ok=True)
    os.makedirs("{}/network".format(args.data_home), exist_ok=True)

    if not args.skip_download:
        vprint("Downloading medline into", args.xml_dir)
        downloadMedline(args.xml_dir)

    # ABSTRACT FILE
    ab_raw_path = "{}/processedText/abstract.raw.txt".format(args.data_home)
    if not os.path.isfile(ab_raw_path) or args.rebuild:
        cmd = getCmdStr(linkPath, "parseXML")

        def parse(path):
            if path.endswith(".xml"):
                vprint("Parsing", path)
                subprocess.call([
                    cmd,
                    '-f', os.path.join(args.xml_dir, path),
                    '-o', os.path.join(tmp_dir, path),
                    '-k',  # include author supplied keywords
                    '-v' if VERBOSE else ''
                ])

        # Parse each xml file in parallel
        with Pool() as p:
            p.map(parse, os.listdir(args.xml_dir))

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

    # AUTOPHRASE Expert Phrases
    phrase_path = "{}/processedText/expertPhrases.txt" .format(args.data_home)
    if not os.path.isfile(phrase_path) or args.rebuild:
        mrconso_path = "{}/META/MRCONSO.RRF".format(args.umls_dir)
        checkFile(mrconso_path)
        cmd = getCmdStr(linkPath, "parseXML")

        def parse(path):
            if path.endswith(".xml"):
                vprint("parsing", path)
                subprocess.call([
                    cmd,
                    '-f', os.path.join(args.xml_dir, path),
                    '-o', os.path.join(tmp_dir, path),
                    '-O',  # only print author supplied keywords
                    '-N',  # no meta
                    '-v' if VERBOSE else ''
                ])
        tmp_path = os.path.join(tmp_dir, 'temp_phrases.txt')
        with open(tmp_path, "w") as file:
            vprint("Getting phrase data from MRCONSO")
            subprocess.call(
                ['awk', 'BEGIN{FS="|"}{print $15}', mrconso_path],
                stdout=file)
            for path in os.listdir(tmp_dir):
                if path.endswith(".xml"):
                    vprint("Getting phrase data from", path)
                    subprocess.call([
                        'cat', os.path.join(tmp_dir, path)
                    ], stdout=file)

        vprint("cleaning phrase file")
        cmd = getCmdStr(linkPath, "cleanRawText")
        subprocess.call([cmd, '-i', tmp_path, '-o', phrase_path,
                         '-v' if VERBOSE else ''])
        vprint('removing trailing periods')
        subprocess.call(['sed', '-i', r's/\..*$//g', phrase_path])
        vprint('finding unique phrases')
        subprocess.call(['uniq', phrase_path],
                        stdout=open(tmp_path, 'w'))
        subprocess.call(['mv', tmp_path, phrase_path])
        subprocess.call(['sort', '-u', phrase_path],
                        stdout=open(tmp_path, 'w'))
        subprocess.call(['mv', tmp_path, phrase_path])
        cleanTmp(tmp_dir)
    else:
        vprint("Reusing", phrase_path)
    checkFile(phrase_path)

    # AUTOPHRASE Training and parsing
    ab_w_phrases_path = "{}/processedText/abstract.parsed.txt"\
                        .format(args.data_home)
    if not os.path.isfile(ab_w_phrases_path) or args.rebuild:
        vprint("Running autophrase")
        cmd = getCmdStr(linkPath, "../../external/runAutoPhrase.sh")
        subprocess.call([cmd, ab_raw_path, num_threads,
                         phrase_path, ab_w_phrases_path],
                        stdout=sys.stdout if VERBOSE else
                        open("/dev/null", 'w'))
        vprint("Processing autophrase")
        cmd = getCmdStr(linkPath, "apHighlight2Underscore")
        tmp_path = os.path.join(tmp_dir, "tmp.txt")
        subprocess.call([cmd, '-i', ab_w_phrases_path, '-o', tmp_path,
                         "-v" if VERBOSE else ""])
        subprocess.call(['mv', tmp_path, ab_w_phrases_path])
        cleanTmp(tmp_dir)
    else:
        vprint("Reusing", ab_w_phrases_path)
    checkFile(ab_w_phrases_path)

    # word 2 vec
    ngram_vec_path = "{}/fastText/ngram.vec".format(args.data_home)
    if not os.path.isfile(ngram_vec_path) or args.rebuild:
        ngram_bin_path = "{}/fastText/ngram.bin".format(args.data_home)
        cmd = getCmdStr(linkPath, "fasttext")
        vprint("Running fasttext")
        subprocess.call([
            cmd,
            "skipgram",
            "-input", ab_w_phrases_path,
            "-output", os.path.splitext(ngram_vec_path)[0],
            "-thread", num_threads,
            "-dim", args.vector_dim
        ], stdout=sys.stdout if VERBOSE else open("/dev/null", 'w'))
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

    # trash all "." from abstracts
    vprint("Updating abstrat file, removing stop token")
    subprocess.call(['sed', '-i', r's/\s\.//g', ab_w_phrases_path])
