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
from multiprocessing import Pool

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
                        default="{}/data/rawData".format(homePath),
                        help="location to store raw XML files")
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

    global VERBOSE
    VERBOSE = args.verbose

    if not args.skip_download:
        vprint("Downloading medline into", args.xml_dir)
        downloadMedline(args.xml_dir)

    abstractPath = "{}/processedText/abstracts.txt".format(args.data_home)
    if not os.path.isfile(abstractPath) or args.rebuild:
        tmp_dir = "{}/processedText/tmp".format(args.data_home)
        os.makedirs(tmp_dir, exist_ok=True)
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
        with open(abstractPath, 'w') as file:
            for path in os.listdir(tmp_dir):
                subprocess.call([
                    'cat', os.path.join(tmp_dir, path)
                ], stdout=file)

    else:
        vprint("Reusing", abstractPath)
    checkFile(abstractPath)
