echo "L2"
./evalHybrid -m /zfs/safrolab/users/jsybran/moliere/results/validation/2010/VIEW_REAL/C0043241---C1611640 -n /zfs/safrolab/users/jsybran/moliere/data/yearlySubsets/2010/fastText/canon.vec -c /zfs/safrolab/users/jsybran/moliere/data/yearlySubsets/2010/fastText/umls.data -s C0043241 -t C1611640 -B -e -v

echo "Cos"
./evalHybrid -m /zfs/safrolab/users/jsybran/moliere/results/validation/2010/VIEW_REAL/C0043241---C1611640 -n /zfs/safrolab/users/jsybran/moliere/data/yearlySubsets/2010/fastText/canon.vec -c /zfs/safrolab/users/jsybran/moliere/data/yearlySubsets/2010/fastText/umls.data -s C0043241 -t C1611640 -B
