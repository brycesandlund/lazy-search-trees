#include "splay.cpp"
#include <vector>
#include <list>

#ifndef LAZY_SEARCH_TREE
#define LAZY_SEARCH_TREE

using namespace std;

template<typename T, typename Comp = std::less<T>>
class lazy_search_tree {
private:
  Comp comp;
  unsigned long p_size;
  
  // data structure that contains a set of intervals within a gap.
  struct gap {
    // an interval within a gap.
    struct interval {
      T max_element;
      unsigned long int_size = 0;
      
      // intervals require a linked list data structure for O(1) merging, but by using a linked list of
      // vectors, we can take advantage of larger built intervals and inserted elements, reducing the
      // number of pointers in the entire data structure to O(min(n, q log n)).
      list<vector<T>> elements;
      
      void merge(interval other) {
        int_size += other.int_size;
        
        // may want to make this conditional so that the interval is loosely structured in order, that
        // is, if other other is a left side interval, do as below, otherwise, add the elements to the
        // beginning, not end.
        elements.splice(elements.end(), other);
      }
      
      void insert(T element) {
        ++int_size;
        if (elements.empty()) {   // could also initilize intervals to a single element list with
                                  // one empty vector. Can try both and choose better performer.
          elements.emplace_back(vector<T>());
        }
        elements.front().emplace_back(element);
      }
      
      interval(T element) {
        insert(element);
      }
    };
    
    // the sorted set of intervals within this gap; all elements in intervals[i] <= intervals[i+1].
    vector<interval> intervals;
    
    gap(vector<interval> intervals) : intervals(intervals) {}
    
    gap(T key) {
      intervals.emplace_back(interval(key));
    }
    
  /*  bool operator < (const gap& other) const {
      return intervals.back().max_element
    }*/
  };
  
  splay_tree<gap> gap_ds;
  
public:
  lazy_search_tree() : p_size(0) {}
  
  void insert(const T &key) {
  
  }
};

#endif // LAZY_SEARCH_TREE
