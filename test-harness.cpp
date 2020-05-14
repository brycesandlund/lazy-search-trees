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
    if (bst.count(item) == 0) {
      cout << "Insert " << item << endl;
      lst.insert(item);
      bst.insert(item);
    }
  }
  
  for (int i = 0; i < 10000; ++i) {
    int item = rand() % 20000;
    cout << "Check " << item << endl;
    bool LST_in = lst.membership(item);
    bool BST_in = bst.count(item);
    
    cerr << "LST: " << LST_in << ", BST: " << BST_in << endl;
    if (LST_in != BST_in) {
      lst.print();
      cerr << lst.membership(item);
      cerr << "Error!: " << item << endl;
    }
  }
  cout << "done" << endl;
}
