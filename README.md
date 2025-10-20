# kdtree
This is a (nearly absolute) balanced kdtree for fast kNN search. It does not
support dynamic insertion and removal. Actually we adopt quick sort to rebuild
the whole tree after changes of nodes. We cache the added or the deleted nodes
which will not be actually mapped into the tree until the rebuild method to be
invoked. The good thing is we can always keep the tree balanced, and the bad
thing is we have to wait some time for the finish of tree rebuild. Moreover,
duplicated samples are allowed to be added.

The thought of the implementation is posted [here](https://www.joinquant.com/post/2843).

## how to debug

$ cmake .
$ make
$ ./kdtree 1234
you can try different seed number for random sample generations