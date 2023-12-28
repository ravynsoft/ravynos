#source: sort-file2.s
#source: sort-file1.s
#ld: -T sort-file.t
#nm: -n

# Check that SORT_BY_NAME on filenames works
# the text sections should come in sorted order, the data
# sections in input order.  Note how we specifically pass
# the object filenames in non-alphabetical order
#...
0[0-9a-f]* t infile1
#...
0[0-9a-f]* t infile2
#...
0[0-9a-f]* d data2
#...
0[0-9a-f]* d data1
#pass
