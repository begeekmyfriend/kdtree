# kdtree
This is a (nearly absolute) balanced kdtree for fast kNN search with bad
performance for dynamic addition and removal. In fact we adopt quick sort to
rebuild the whole tree after changes of the nodes. We cache the added or the
deleted nodes which will not be actually mapped into the tree until the rebuild
method to be invoked. The good thing is we can always keep the tree balanced,
and the bad thing is we have to wait some time for the finish of tree rebuild.
Moreover duplicated samples are allowed to be added with the tree still kept
balanced.

The thought of the implementation is posted [here](https://www.joinquant.com/post/2843). Thanks 魏文睿 <weiwenrui@std.uestc.edu.cn> who
pointed out bugs in the previous version.
