#include "splay.cpp"
#include "lazy-search-tree.cpp"
#include <iostream>

using namespace std;

int main() {
  splay_tree<int> tree;
  tree.insert(8);
  tree.insert(10);
  cout << tree.root->left->key << endl;
}
