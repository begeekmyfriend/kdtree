# kdtree
This is a (nearly absolute) balanced kdtree for fast kNN search with bad
performance for dynamic addition and removal. In fact we adopt quick sort to
rebuild the whole tree after changes of the nodes. We cache the added or the
deleted nodes which will not be actually mapped into the tree until the rebuild
method to be invoked. The good thing is we can always keep the tree balanced,
and the bad thing is we have to wait some time for the finish of tree rebuild.
Moreover duplicated samples are allowed to be added with the tree still kept
balanced.

The thought of the implementation is posted [here](https://www.joinquant.com/post/2843).

As for dynamical kdtree you may refer to this [project](https://www.github.com/jtsiomb/kdtree).

## Test
```shell
cd kdtree
gcc -O2 -o kdtree kdtree_test.c kdtree.c -lm
./kdtree
```
