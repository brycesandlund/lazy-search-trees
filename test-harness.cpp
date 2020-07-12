#include "splay.cpp"
#include "lazy-search-tree.cpp"
#include <queue>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;

// tests for correctness of LST
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
      bool LST_in = lst.count(item);
      bool BST_in = bst.count(item);
      
      cerr << "LST: " << LST_in << ", BST: " << BST_in << endl;
      if (LST_in != BST_in) {
        lst.print();
        cerr << lst.count(item);
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

// tests for q uniformly distributed queries on n elements,
// queries and insertions are interspersed.
template <typename container>
void uniform_speed(int n, int q, container c) {
  vector<int> keys(n);
  for (int i = 0; i < n; ++i) {
    keys[i] = i;
  }
  random_shuffle(keys.begin(), keys.end());
  cout << "Begin test" << endl;
  for (int i = 0; i < n; ++i) {
    c.insert(keys[i]);
    if (rand()%n < q) {
      c.count(rand()%n);
    }
  }
  cout << "Test Complete" << endl;
}

// tests for q total queries on n elements, queries are batched into queries for k
// consecutive elements. Queries and insertions are interspersed, each batch is
// uniformly distributed.
template <typename container>
void clustered_speed(int n, int q, int k, container c) {
  vector<int> keys(n);
  for (int i = 0; i < n; ++i) {
    keys[i] = i;
  }
  random_shuffle(keys.begin(), keys.end());
  cout << "Begin test" << endl;
  for (int i = 0; i < n; ++i) {
    c.insert(keys[i]);
    if (rand()%n < q/k) {
      int start = rand()%(n-k+1);
      for (int j = 0; j < k; ++j) {
        c.count(start+j);
      }
    }
  }
  cout << "Test Complete" << endl;
}

// priority queue test of library PQ, insertions uniformly distributed, insertions
// preceding queries.
void pq_lib_speed(int n, int q) {
  priority_queue<int> pq;
  vector<int> keys(n);
  cout << "Begin test" << endl;
  for (int i = 0; i < n; ++i) {
    pq.push(i);
  }
  for (int i = 0; i < q; ++i) {
    pq.pop();
  }
  cout << "Test Complete" << endl;
}

// priority queue test of lazy search tree, insertions uniformly distributed, insertions
// preceding queries. Doesn't really extract data, just queries the appropriate ranks, which
// is roughly the same, but slightly faster. Most favorable scenario for LST.
void pq_speed(int n, int q) {
  lazy_search_tree<int> pq;
  cout << "Begin test" << endl;
  for (int i = 0; i < n; ++i) {
    pq.insert(i);
  }
  for (int i = 0; i < q; ++i) {
    pq.count(i);
  }
  cout << "Test Complete" << endl;
}

// tests for insertions and no queries.
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
  int n = 100000000;
  int q = 100;
  int k = 1;
  cout << "PQ insert first test n: " << n << " q:" << q << endl;
  if (argv[1][0] == 'B') {
    cout << "Time BST" << endl;
    pq_lib_speed(n, q);
  } else {
    cout << "Time LST" << endl;
    pq_speed(n, q);
  }
/*  lazy_search_tree<int> lst;
  set<int> bst;
  int n = 10000000;
  int q = 1000;
  int k = 1;
  cout << "Clustered test n: " << n << " q:" << q << " k:" << k << endl;
  if (argv[1][0] == 'B') {
    cout << "Time BST" << endl;
    clustered_speed(n, q, k, bst);
  } else {
    cout << "Time LST" << endl;
    clustered_speed(n, q, k, lst);
  }*/
/*  cout << "Uniform test n: " << n << " q:" << q << endl;
  if (argv[1][0] == 'B') {
    cout << "Time BST" << endl;
    uniform_speed(n, q, bst);
  } else {
    cout << "Time LST" << endl;
    uniform_speed(n, q, lst);
  }*/
  
  cout << "done" << endl;
}
