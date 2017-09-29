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
from scipy.sparse import csr_matrix 
from sklearn.metrics.pairwise import cosine_similarity
import timeit


'''
input_file = "/scratch2/esadrfa/hiv_associate_dementia.dijkstra"
#input_file = "/home/esadrfa/test"
out_file = input_file + ".out"
'''

def check_input_file(input_file, verbose):
    start = timeit.default_timer()        
    numbers = []
    max_line_num = 0
    # new_list = []
    with open(input_file) as dijkFile:
        while(True):
            line1 = dijkFile.readline()
            line2 = dijkFile.readline()
            if(not line2):
                break
            max_line_num += 1
            new_list = (line2.split(' '))
            for i in new_list:
                try:
                    numbers.append(int(i))
                except:
                    pass 
    
    largest_num = max(numbers)
    print('number of items: {}'.format(len(numbers)))
    
    stop = timeit.default_timer()
    if(verbose):
#        print("count the number of lines and documents: %.1f seconds" % (stop - start))
        print('Largest moliere id:{}'.format(largest_num))
        print('Number of hypothesis in the file:{}'.format(max_line_num))
        print("Number of all doc ids (with duplicates):{}".format(len(numbers)))      
    
    start = timeit.default_timer()
    uniq_id = set(numbers)
    stop = timeit.default_timer()
    if(verbose):
        print("Number of uniq documents ID:{}".format(len(uniq_id)))
#        print("create a set takes: %.1f seconds" % (stop - start))
    return largest_num


'''
create a csr matrix as features
'''
def create_csr_matrix(input_file, largest_num, verbose):
    start = timeit.default_timer()
    # create the csr_matrix
    
    start = timeit.default_timer()
    row = []
    col = []
    data = []
    with open(input_file) as dijkFile:
        line_idx = 0
        while(True):
            line1 = dijkFile.readline()
            line2 = dijkFile.readline()
            if(not line2):
                break
            cloud = line2.rstrip().split(' ')
            for doc_idx in cloud:
                row.append(int(line_idx))
                col.append(int(doc_idx))
                data.append(1)
            #line_idx is updated for each line of the file, not documents
            line_idx += 1

    stop = timeit.default_timer()
    if(verbose):
        print("load the arrays takes: %.1f seconds" % (stop - start))
        print('number of lines in file (only clouds) {}'.format(line_idx))


    items=['row','col','data']
    items_name=[row,col,data]
#    for i in range(len(items_name)):
#        print("num_{} in row:{}".format(items[i],len(items_name[i])))
    print("Matrix info #row:{}, #col:{}, #data:{}".format(len(row), len(col), len(data)))
    print("Matrix dimension:{}x{}".format(line_idx, largest_num + 1))

    start = timeit.default_timer()
    m_clouds = csr_matrix((data, (row, col)), shape=(line_idx, largest_num + 1))
    stop = timeit.default_timer()
    print("create a set takes: %.1f seconds" % (stop - start))
    print('m_clouds shape {}'.format(m_clouds.shape))
    return m_clouds

    
def calc_similarity(m_clouds):
    start = timeit.default_timer()
    similarities = cosine_similarity(m_clouds)
    stop = timeit.default_timer()
    print("calculate the similarities takes: %.1f seconds" % (stop - start))
    # print('pairwise dense output:\n {}\n'.format(similarities))
    print('similarity matrix shape {}'.format(similarities.shape))
    return similarities


def export_similarity_matrix(simMatrix, export_path, export_file):
    start = timeit.default_timer()
    np.save(export_path + export_file, simMatrix)
    stop = timeit.default_timer()
    print("save dense matrix takes: %.1f seconds" % (stop - start))
    print('dense similarity is exported ')



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
    parser.add_argument("-d", "--dijkFile",
                        action="store",
                        dest="dijkFile",
                        help="dijk file name")
    parser.add_argument("-e", "--exportFile",
                        action="store",
                        dest="outputFile",
                        help="output file name")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        dest="verbose",
                        help="print debug info")

    args = parser.parse_args()

    #global verbose, label2Data, outDirPath, pmid2bag

    if(args.verbose):
        print("Loading dijkstra from {}".format(args.inPath + args.dijkFile))
    
    if(not args.outPath):
        args.outPath = args.inPath

    max_doc_number = check_input_file(args.inPath + args.dijkFile, True)
    m_clouds = create_csr_matrix(args.inPath + args.dijkFile, max_doc_number, True)
    simMatrix = calc_similarity(m_clouds)
    export_similarity_matrix(simMatrix, args.outPath, args.outputFile)


if __name__ == "__main__":
    main()

