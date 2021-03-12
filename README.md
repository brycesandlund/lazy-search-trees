# lazy-search-trees
An implementation of the lazy search tree data structure: https://arxiv.org/abs/2010.08840.

This is a replacement for binary search trees that avoids sorting on insert and instead progressively sorts data as queries are answered. The current implementation is bare-bones, but can be extended and optimized for more generality and better performance. The data structure should outperform binary search trees when query volume is very small (say, less than the square root of the number of insertions) or the range of keys requested strongly non-uniform. The data structure can also be used as an efficient priority queue, but initial experiments show it non-competitive in that context.

When compared to the splay tree on which the implementation is based, as of September 2020, with n = 1,000,000 insertions, it is about 30% faster with no queries and remains faster with less than 2,500 uniformly-distributed queries. With n = 10,000,000 insertions, it is about 60% faster with no queries and remains faster with less than 20,000 uniformly-distributed queries.
