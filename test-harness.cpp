#include "splay.cpp"
#include "lazy-search-tree.cpp"
#include <queue>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <random>
#include <chrono>

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
  shuffle(keys.begin(), keys.end(), default_random_engine(0));
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
  default_random_engine gen(0);
  shuffle(keys.begin(), keys.end(), gen);
/*  for (int i = 0; i < n; ++i) {
    cerr << keys[i] << " ";
  }
  cerr << endl;*/
  
  uniform_int_distribution<int> query(0, n-1);
  uniform_int_distribution<int> start(0, n-k);
  cout << "Begin test" << endl;
  auto t1 = chrono::high_resolution_clock::now();
  for (int i = 0; i < n; ++i) {
    c.insert(keys[i]);
    if (query(gen) < q/k) {
      int st = start(gen);
   //   cerr << st << endl;
      for (int j = 0; j < k; ++j) {
        c.count(st+j);
      }
    }
  }
  auto t2 = chrono::high_resolution_clock::now();
  cout << "Test Complete" << endl;
  auto duration = chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
  cout << "Time: " << duration << endl;
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
  srand(0);
/*  int n = 100000000;
  int q = 100;
  int k = 1;
  cout << "PQ insert first test n: " << n << " q:" << q << endl;
  if (argv[1][0] == 'B') {
    cout << "Time BST" << endl;
    pq_lib_speed(n, q);
  } else {
    cout << "Time LST" << endl;
    pq_speed(n, q);
  }*/
  int n = 10000000;
  int q = 25000;
  int k = 1;
  
  cout << "Clustered test n: " << n << " q:" << q << " k:" << k << endl;
  if (argc != 2) {
    cout << "Error, Usage: \"./test-harness L\", where L can be B, S, or L" << endl;
  }
  else if (argv[1][0] == 'B') {
    cout << "Time c++ set" << endl;
    set<int> bst;
    clustered_speed(n, q, k, bst);
  } else if (argv[1][0] == 'S') {
    cout << "Time splay tree" << endl;
    splay_tree<int> stree;
    clustered_speed(n, q, k, stree);
  } else if (argv[1][0] == 'L'){
    cout << "Time LST" << endl;
    lazy_search_tree<int> lst;
    clustered_speed(n, q, k, lst);
  } else {
    cout << "Argument not recognized" << endl;
  }
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
