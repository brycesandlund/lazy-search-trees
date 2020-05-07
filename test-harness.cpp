#include "splay.cpp"
#include "lazy-search-tree.cpp"
#include <iostream>
#include <set>

using namespace std;

int main() {
  
  lazy_search_tree<int> lst;
  set<int> bst;
  
  for (int i = 0; i < 10000; ++i) {
    if (rand()%2 == 0) {
      cout << "Insert " << i << endl;
      lst.insert(i);
      bst.insert(i);
    }
  }
  
  for (int i = 0; i < 10000; ++i) {
    int item = rand() % 10000;
    cout << "Check " << item << endl;
    if (lst.membership(item) != bst.count(item)) {
      cerr << "Error!: " << item << endl;
    }
  }
}
