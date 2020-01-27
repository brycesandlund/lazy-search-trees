#include "splay.cpp"
#include "lazy-search-tree.cpp"
#include <iostream>

using namespace std;

int main() {
  lazy_search_tree<int> lst;
  lst.insert(3);
  lst.insert(7);
  cout << lst.membership(3) << endl;
  cout << lst.membership(6) << endl;
  cout << lst.membership(7) << endl;
  cout << "done." << endl;
}
