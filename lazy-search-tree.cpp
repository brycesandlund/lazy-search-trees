#include "splay.cpp"
#include <vector>
#include <list>
#include <algorithm>

#ifndef LAZY_SEARCH_TREE
#define LAZY_SEARCH_TREE

using namespace std;

template<typename T, typename Comp = std::less<T>>
class lazy_search_tree {
private:
  Comp comp;
  unsigned long p_size;
  
  // data structure that contains a set of intervals within a gap.
  // a gap should never be empty.
  struct gap {
    // an interval within a gap.
    struct interval {
      T max_element;
      unsigned long int_size = 0;
      
      // intervals require a linked list data structure for O(1) merging, but by using a linked list of
      // vectors, we can take advantage of larger built intervals and inserted elements, reducing the
      // number of pointers in the entire data structure to O(min(n, q log n)).
      list<vector<T>> elements;
      
      // merges 'other' into this interval, destroying 'other'.
      void merge(interval &other) {
        int_size += other.int_size;
        max_element = max(max_element, other.max_element);
        
        // may want to make this conditional so that the interval is loosely structured in order, that
        // is, if other other is a left side interval, do as below, otherwise, add the elements to the
        // beginning, not end.
        elements.splice(elements.end(), other);
      }
      
      // insert an element into this interval.
      void insert(T element) {
        ++int_size;
        if (elements.empty()) {   // could also initilize intervals to a single element list with
                                  // one empty vector. Can try both and choose better performer.
          elements.emplace_back(vector<T>());
          max_element = element;
        }
        elements.front().emplace_back(element); // doesn't actually matter which vector the element
                                                // is placed into.
        max_element = max(max_element, element);
      }
      
      interval(T element) {
        insert(element);
      }
      
      // compare gaps to one another via their maximum element.
      bool operator < (const interval& other) const {
        return max_element < other.max_element;
      }
    };
    
    // the sorted set of intervals within this gap; all elements in intervals[i] <= intervals[i+1].
    vector<interval> intervals;
    
    // initialize a gap with a vector of intervals.
    gap(vector<interval> intervals) : intervals(intervals) {}
    
    // create a gap with a single interval containing a single element.
    gap(T key) {
      intervals.emplace_back(interval(key));
    }
    
    // compare gaps to one another via their maximum element.
    bool operator < (const gap& other) const {
      return intervals.back() < other.intervals.back();
    }
    
    // insert the element into this gap.
    void insert(T key) {
      interval test_int(key);
      // can't get vector<interval>::iterator lb to compile.
      // could switch to more intelligent method to remove O(log log n) factor to
      // O(1) in average case.
      auto lb = lower_bound(intervals.begin(), intervals.end(), test_int);
      if (lb == intervals.end()) {
        --lb;
      }
      lb->insert(key);
    }
  };
  
  splay_tree<gap> gap_ds;
  
public:
  lazy_search_tree() : p_size(0) {}
  
  void insert(const T &key) {
    if (empty()) {
      gap r_gap = gap(key);
      gap_ds.insert(r_gap);
    } else {
      gap r_gap = gap_ds.successor_or_equal(gap(key));
    }
  }
  
  unsigned long size( ) const { return p_size; }
  bool empty( ) const { return size() == 0; }
};

#endif // LAZY_SEARCH_TREE
