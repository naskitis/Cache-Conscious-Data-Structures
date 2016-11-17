Refereed Cache-Conscious String Data Structures for Strings

This is the complete readme file for my memory-resident string
data structures, as detailed in my PhD Thesis. Depending on your download,
you may only have a selection of these data structures.  

Author:  Dr. Nikolas Askitis.
Email:   askitisn@gmail.com

PUBLICATIONS
--------------------------------------------------------------------------------------------------------------------
These data structures are available from the following refereed publications:

1. B-tries for Disk-based String Management, VLDB Journal, Volume 18, Issue 1, Pages 157 - 179, 2009.  ISSN:1066-8888 
2. Engineering scalable, cache and space efficient tries for strings, VLDB Journal, Published Online, 2010.  DOI: 10.1007/s00778-010-0183-9 ISSN: 1066-8888
3. HAT-Trie: A Cache-Conscious Trie-Based Data Structure For Strings, The 30th International Australasian Computer Science Conference (ACSC), Volume 62, pages 97 - 105, 2007.  ISSN: 1445-1336
4. Cache-conscious collision resolution in string hash tables, The 12th International String Processing and Information Retrieval Conference (SPIRE), LNCS 3772, pages 91 - 102, 2005.  ISSN: 0302-9743
5. Fast and compact hash tables for integer keys, The 32nd International Australasian Computer Science Conference (ACSC), Volume 91, pages 101 - 110, 2009.  ISSN: 1445-1336
6. Redesigning the string hash table, burst trie, and BST to exploit cache, ACM JEA Vol. 15, No. 1, Article 1.7 2011

GENERAL GUIDELINES:
--------------------------------------------------------------------------------------------------------------------

A file to insert or search is first buffered entirely in main-memory to factor-out the cost of fetching 
individual strings from disk.   Hence, it is important to keep the file size small, 
relative to the amount of memory your system has and the size of the data structure.   
Otherwise, the file buffer will occupy most of main memory, forcing the data structure 
to use virtual memory. 

If you want to use a large file, then simply break it up into smaller pieces and 
list the pieces on the command-line, as shown below.

*** You must provide the correct parameters (command-line format) to these data structures, 
    as shown below.  

For accurate results, be sure to run these data structures when your operating system is
under light load.  That is, don't run any other unnecessary software and drop down to
console mode if possible (temporarily stop your graphical user interface like KDE).

Also, please disable power-saving features such as Intel SpeedStep or AMD Cool 'n' Quiet technology. This is to
ensure that your processor(s) are operating at full speed before running the software.
You can find out what speed your processor (cores) are running at using the following command in Linux: less /proc/cpuinfo


DATA STRUCTURES
--------------------------------------------------------------------------------------------------------------------
1.  array BST
2.  standard BST
3.  array hash table        (3 variants)
4.  standard hash table
5.  array burst-trie        (2 variants)
6.  standard burst-trie
7.  standard redblack
8.  standard splay
9.  HAT-trie
10. B-trie
11. B-tree

SOFTWARE VARIANTS
--------------------------------------------------------------------------------------------------------------------
a) Hash tables:

array-hash-exact = array hash table where slots are resized by as many bytes as needed (i.e exact-fit)
array-hash-page  = array hash table where slots are resized in 64-byte blocks.
array-hash-exact-mtf = array hash table with exact-fit and move-to-front on access.
array-hash-32bit-int = array hash table designed for 32-bit integer (binary) keys (with 32-bit integer payload).

b) Burst tries:
The array burst trie also has the same variants, namely the:

array-burst-trie-exact
array-burst-trie-page


PC ARCHITECTURE (AMD 64-bit or Intel 64-bit Processor)
----------------------------------------------------------------------------------------------------------------------
Developed for x86_64 Linux Platforms on an AMD or Intel based processor.
We used Kubuntu 10.04 to compile and prepare the software. 

Currently, my data structures are designed for 64-bit architectures (hence,
they assume that pointers are 8-bytes long).  Please note, in my papers, I (mostly)
used the 32-bit versions, since back then, my primary machine was a 32-bit P4.  Check out
paper number 2, listed above, for reference of performance on a more modern 64-bit processor.


DATA SETS
---------------------------------------------------------------------------------------------------------------------
These (string dictionary) data structures support plain-text ASCII-7 datasets (printable-range). 
Strings can be of variable-length and are null-terminated, one string per line.

For example:

HTML
test
apple
automobile
aeroplane
space
b
a
genome sequence analysis
testing the source
ozsort
test
...


The 32-bit integer (dictionary) data structures support 4-byte integer datasets (in binary format, that is, 
non-human readable). They must appear all in one line. 



EXPERIMENTS
---------------------------------------------------------------------------------------------------------------------

./quick_compare.bsh [optional: user-specified file]

This script will run a "quick-and-dirty" benchmark experiment, comparing all these data structures using a user-specified file.

You will need a x86_64 Intel/AMD Linux PC with at least 4GB of RAM to run these experiments.  8GB of RAM is recommended.

Eg:

bunzip2 sampleData.bz2;
./quick_compare.bsh sampleData;


TO RUN:
----------------------------------------------------------------------------------------------------------------------

All data structures share the same command-line interface, as follows:


<data-structure> [Number of slots or the container size] [[ output file name ]]  <number of datasets to insert>  <datasets to insert (separated by white space)>  <number of datasets to search>  <datasets to search (separated by white space)>


Parameters within <> brackets are mandatory, whereas those in [] brackets are only required by the hash table(s) and burst trie(s). 
Parameters in [[ ]] brackets are only required by the B-tree and B-trie.  

1)  Container size:   the number of strings a container must store before it is burst. 
2)  Number of slots:  the number of slots allocated by the hash table (power of 2). 


**************************************************************************************
IMPORTANT: 
            Slot numbers of be of a power of 2 e.g:  65536, 8388608, etc.
            Slot numbers and container sizes must be set to no less than 16.
**************************************************************************************


EXAMPLES:
-----------------------------------------------------------------------------------------------------------------------
./nikolas_askitis_array_bst 1 /mnt/in/data/file1 2 /mnt/in/data/file1 /tmp/other
./standard-bst 1 /mnt/in/data/file1 2 /mnt/in/data/file1 /tmp/other
./nikolas_askitis_array_burst_trie_exact 256 3 file1 file2 file3 1 file2
./nikolas_askitis_hat_trie 32768 3500000 1 dataset 1 dataset
./nikolas_askitis_array_hash 65536 1 skew1.in 1 skew1.in
./nikolas_askitis_btrie output_file 1 file1 1 file2
./nikolas_askitis_btree output_file 1 file1 1 file2

OUTPUT:
------------------------------------------------------------------------------------------------------------------------

The output of all data structures are as follows (in order, from left-to-right):

1) The data structure used:    


2) Total memory (virtual memory): This is the total memory used by the running process, as reported by the operating system. 
                                  This is a true measure of memory consumption, capturing space lost due to fragmentation
                                  and other overheads.   This measure does not include the space taken to buffer the
                                  dataset.  If you use the 'top' Linux command, for example, this will capture the same
                                  information, but with the space occupied by the dataset buffer, which can make it
                                  difficult to see how must space is eaten by the program itself, and not its file buffer. 


3) Estimated memory usage:        The total memory, in megabytes, required by the data structure --- the measure
                                  includes the estimated 8-byte allocation overhead imposed by every call to malloc. It
                                  may be slightly less than or more than the virtual memory usage reported (in step 2).
                                  For the B-tree and B-trie, this represents the total disk space used.

4) Total insertion time:          The total or elapsed time, in seconds, required by the data structure
                                  to insert the files that were specified on the command line.

5) Total search time:             The total or elapsed time, in seconds, required by the data structure to
                                  search the files that were specified on the command line.

6) Number of strings inserted:    Returns the total number of strings stored by the data structure.
                                  IMPORTANT:  the data structure does not maintain duplicates, hence, only
                                  unique strings are inserted.

7) Number of strings searched:    Returns the total number of strings that were found during a search.


8) Number of slots allocated or 
   container threshold:           The number of slots allocated by the hash table or the container threshold (the
                                  number of strings a container can store prior to bursting) of a burst trie.  For
                                  the B-tree and B-trie, this is the bucket size used for leaf nodes, which is
                                  set to 8192 bytes in this release.

9) Additional (optional) info:    Extra information may be displayed, such as growth policy used and whether
                                  move-to-front is enabled.

An example (we map the output to the numbers above):

linux@: ./array-bst 1 28772169  1 28772169 
Array-BST 774.09 764.92 456.12 423.41 28772169 28772169
   (1)    (2)    (3)    (4)    (5)    (6)      (7)

