#include "splay.cpp"
#include "lazy-search-tree.cpp"
#include <iostream>
#include <set>

using namespace std;

void correctness() {
  lazy_search_tree<int> lst;
  set<int> bst;
  
  for (int i = 0; i < 10000; ++i) {
    int item = rand() % 20000;
    if (rand()%2) {
      if (bst.count(item) == 0) {
        cout << "Insert " << item << endl;
        lst.insert(item);
        bst.insert(item);
      }
    } else {
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
  }
  
/*  for (int i = 0; i < 10000; ++i) {
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
  }*/
}

template <typename container>
void insert_time_test(container c, long long bound) {
  cout << "Begin insert" << endl;
  for (int i = 0; i < bound; ++i) {
    int item = rand() % (1000*bound);
    c.insert(item);
  }
  cout << "Insert complete" << endl;
}

int main(int argc, char *argv[]) {
/*  lazy_search_tree<int> lst;
  set<int> bst;
  if (argv[1][0] == 'B') {
    cout << "Time BST" << endl;
    insert_time_test(bst, 10000000);
  } else {
    cout << "Time LST" << endl;
    insert_time_test(lst, 10000000);
  }*/
  correctness();
  
  cout << "done" << endl;
}
