#include "splay.cpp"
#include "lazy-search-tree.cpp"
#include <iostream>

using namespace std;

int main() {
  lazy_search_tree<int> lst;
  for (int i = 0; i < 19; ++i) {
    lst.insert(19-i);
  }
  cout << lst.membership(6) << endl;
  lst.print();
  cout << lst.membership(10) << endl;
  lst.print();
  cout << "done." << endl;
}
