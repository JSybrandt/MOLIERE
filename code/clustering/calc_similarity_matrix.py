'''
First step for clustering is converting the Moliere IDs for each hypothesis to a binary feature
    Then, calculate the distaces to produce the similarity matrix

The output should save to file.

Next, we use XXX script to cluster them (hierarchical)    
'''
# coding: utf-8

import numpy as np
import scipy as sp
from scipy.sparse import csr_matrix 
from sklearn.metrics.pairwise import cosine_similarity
import timeit


input_file = "/scratch2/esadrfa/hiv_associate_dementia.dijkstra"
#input_file = "/home/esadrfa/test"
out_file = input_file + ".out"



start = timeit.default_timer()        
numbers = []
max_line_num = 0
# new_list = []
with open(input_file) as f:
    for line in f:
        if ':' in line:   #the path summary
            continue
        else:
            max_line_num += 1
            new_list = (line.split(' '))
            for i in new_list:
                try:
                    numbers.append(int(i))
                except:
                    pass 

largest_num = max(numbers)
print('number of items: {}'.format(len(numbers)))

stop = timeit.default_timer()
print("count the number of lines and documents: %.1f seconds" % (stop - start))

print('largest_num:{}'.format(largest_num))
print('max_line_num:{}'.format(max_line_num))


print("Count of all doc id (with duplicates):{}".format(len(numbers)))      

start = timeit.default_timer()
uniq_id = set(numbers)
print("number of uniq documents ID:{}".format(len(uniq_id)))
stop = timeit.default_timer()
print("create a set takes: %.1f seconds" % (stop - start))

# create the csr_matrix

start = timeit.default_timer()
row = []
col = []
data = []
with open(input_file) as f_in:
    line_idx = 0
    for line in f_in:
#         if(line_idx > 10000000):
#             break
        if ':' not in line: 
            cloud = line.rstrip().split(' ')
            for doc_idx in cloud:
                row.append(int(line_idx))
                col.append(int(doc_idx))
                data.append(1)
            #line_idx is updated for each line of the file, not documents
            line_idx += 1

stop = timeit.default_timer()
print("load the arrays takes: %.1f seconds" % (stop - start))
print('number of lines in file (only clouds) {}'.format(line_idx))


items=['row','col','data']
items_name=[row,col,data]
for i in range(len(items_name)):
    print("num_{} in row:{}".format(items[i],len(items_name[i])))



## In[78]:

#'\n' in col


# In[64]:

#'' in col
# col.index('')

start = timeit.default_timer()
m_clouds = csr_matrix((data, (row, col)), shape=(max_line_num, largest_num+1))
stop = timeit.default_timer()
print("create a set takes: %.1f seconds" % (stop - start))
print('m_clouds shape {}'.format(m_clouds.shape))


start = timeit.default_timer()
similarities = cosine_similarity(m_clouds)
stop = timeit.default_timer()
print("calculate the similarities takes: %.1f seconds" % (stop - start))
# print('pairwise dense output:\n {}\n'.format(similarities))
print('similarity matrix shape {}'.format(similarities.shape))

##also can output sparse matrices
#similarities_sparse = cosine_similarity(A_sparse,dense_output=False)
#print('pairwise sparse output:\n {}\n'.format(similarities_sparse))

#np.save('/scratch2/esadrfa/sim_sparse_matrix',similarities_sparse)

#print('sparse similarity is exported ')
start = timeit.default_timer()
np.save('/scratch2/esadrfa/sim_dense_matrix_091417_1400',similarities)
stop = timeit.default_timer()
print("save dense matrix takes: %.1f seconds" % (stop - start))
print('dense similarity is exported ')


