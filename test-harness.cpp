#include "splay.cpp"
#include "lazy-search-tree.cpp"
#include <iostream>
#include <set>

using namespace std;

int main() {
  
  lazy_search_tree<int> lst;
  set<int> bst;
  
  for (int i = 0; i < 10000; ++i) {
    int item = rand() % 20000;
    cout << "Insert " << item << endl;  // Note: this may insert duplicates.
    lst.insert(item);
    bst.insert(item);
  }
  
  for (int i = 0; i < 10000; ++i) {
    int item = rand() % 20000;
    cout << "Check " << item << endl;
    if (lst.membership(item) != bst.count(item)) {
      cerr << "Error!: " << item << endl;
    }
  }
}
