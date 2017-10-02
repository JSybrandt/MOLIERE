'''
First step for clustering is converting the Moliere IDs for each hypothesis to a binary feature
    Then, calculate the distaces to produce the similarity matrix

The output should save to file.

Next, we use XXX script to cluster them (hierarchical)    
'''
# coding: utf-8
import argparse
import numpy as np
import scipy as sp
from scipy.cluster.hierarchy import linkage
import timeit



def calc_linkage(input_file, output_file, verbose):
	start = timeit.default_timer()
	data = np.load(input_file)

	stop = timeit.default_timer()
	if(verbose):
		print("loading the similarity matrix takes %.1f seconds" % (stop - start))
		print('data shape:{}x{}'.format(data.shape[0], data.shape[1]))

	start = timeit.default_timer()
	Z = linkage(data, 'ward')
	stop = timeit.default_timer()

	if(verbose):
		print("calculate the linkage takes: %.1f seconds" % (stop - start))

	np.save(output_file, Z)

	#check the quality
	if(verbose):
		from scipy.cluster.hierarchy import cophenet
	from scipy.spatial.distance import pdist
	c, coph_dists = cophenet(Z, pdist(data))
	print('cophenet quality measure is {}'.format(c))

  
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--inputPath",
                        action="store",
                        dest="inPath",
                        help="file path of the inputs")
    parser.add_argument("-o", "--outPath",
                        action="store",
                        dest="outPath",
                        help="dir path of output files")
    parser.add_argument("-f", "--similarityFile",
                        action="store",
                        dest="similarityFile",
                        help="similarity file name")
    parser.add_argument("-e", "--linkageFile",
                        action="store",
                        dest="linkageFile",
                        help="export linkage file")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    args = parser.parse_args()

    
    if(args.verbose):
        print("Loading similarity matrix from {}".format(args.inPath + args.similarityFile))

    if(not args.outPath):
        args.outPath = args.inPath

    calc_linkage(args.inPath + args.similarityFile, args.outPath + args.linkageFile, True)



if __name__ == "__main__":
    main()
