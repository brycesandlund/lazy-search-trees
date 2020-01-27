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
  unsigned long lst_size;
  
  // data structure that contains a set of intervals within a gap.
  // a gap should never be empty.
  struct gap {
  
    // an interval within a gap. An interval should never be empty.
    struct interval {
      T max_element;
      unsigned long int_size;
      
      // intervals require a linked list data structure for O(1) merging, but by using a linked list of
      // vectors, we can take advantage of larger built intervals and inserted elements, reducing the
      // number of pointers in the entire data structure to O(min(n, q log n)).
      list<vector<T>> elements;
      
      // merges 'other' into this interval, destroying 'other'.
      void merge(interval &other) {
        int_size += other.int_size;
        max_element = max(max_element, other.max_element);
        
        // may want to make this conditional so that the interval is loosely structured in order, that
        // is, if other is a left side interval, do as below, otherwise, add the elements to the
        // beginning, not end.
        elements.splice(elements.end(), other);
      }
      
      // insert an element into this interval.
      void insert(T element) {
        elements.front().emplace_back(element); // doesn't actually matter which vector the element
                                                // is placed into.
        max_element = max(max_element, element);
        ++int_size;
      }
      
      // create an interval with a single element.
      interval(T element) {
        int_size = 1;
        elements.emplace_back(vector<T>({element}));
        max_element = element;
      }
      
      bool membership(T key) {
        for (auto vec : elements) {
          for (auto e : vec) {
            if (key == e) return true;
          }
        }
        return false;
      }
      
      // compare gaps to one another via their maximum element.
      bool operator < (const interval& other) const {
        return max_element < other.max_element;
      }
      
      // return the number of elements in this interval.
      unsigned long size() {
        return int_size;
      }
    };
    
    unsigned long gap_size;
    
    // the sorted set of intervals within this gap; all elements in intervals[i] <= intervals[i+1].
    // consider switching to a vector of pointers to intervals.
    vector<interval> intervals;
    
    // initialize a gap with a vector of intervals.
    gap(vector<interval> intervals) : intervals(intervals) {
      for (interval c_int : intervals) {
        gap_size += c_int.size();
      }
    }
    
    // create a gap with a single interval containing a single element.
    gap(T key) {
      gap_size = 1;
      intervals.emplace_back(interval(key));
    }
    
    // compare gaps to one another via their maximum element.
    bool operator < (const gap& other) const {
      return intervals.back() < other.intervals.back();
    }
    
    // returns smallest interval with maximum element larger than or equal to key,
    // or the last interval if key is the maximum amongst all elements.
    interval& getInterval(T key) {
      // can't get vector<interval>::iterator lb to compile.
      // could switch to more intelligent method to remove O(log log n) factor to
      // O(1) in average case.
      auto lb = lower_bound(intervals.begin(), intervals.end(), interval(key));
      if (lb == intervals.end()) {
        --lb;
      }
      return *lb;
    }
    
    // insert key into this gap.
    void insert(T key) {
      getInterval(key).insert(key);
      ++gap_size;
    }
    
    // query for membership of key in this gap. Note: this only answers the query. Restructuring
    // is done in the restructure method.
    bool membership(T key) {
      return getInterval(key).membership(key);
    }
    
    // restructure the gap so that all elements in gap <= key remain in the gap and a new gap
    // is created with elements > key. TODO: replace with more general function.
    gap& restructure(T key) {
      
    }
    
    
    struct index_removal {
      vector<int> indices_to_remove;
      int idx;
      
      index_removal(vector<int> &indices_to_remove) : indices_to_remove(indices_to_remove), idx(0) {}
      
      bool operator() (const int& value) {
        if (value == indices_to_remove[idx]) {
          ++idx;
          return true;
        }
        return false;
      }
    };
    
    // rebalance according to (A) and (B).
    void rebalance() {
      vector<int> left_indices;
      // rebalance left side intervals
      int n_left = 0;
      for (int i = 0; i < intervals.size()-1; ++i) {
        n_left += intervals[i].size();
        // break if interval i+1 is on the right side. An interval i is on the left side if it has less
        // or equal number of elements to its left than to its right, within the gap, and is on the right
        // side otherwise.
        if (gap_size - intervals[i+1].size() - n_left > n_left) {
          break;
        }
        if (n_left >= intervals[i].size() + intervals[i+1].size()) {
          intervals[i].merge(intervals[i+1]);
          left_indices.emplace_back(i+1);
          ++i;
        }
      }
      
      vector<int> right_indices;
      // rebalance right side intervals
      int n_right = 0;
      for (int i = intervals.size()-1; i >= 0; --i) {
        n_right += intervals[i].size();
        // break if interval i-1 is on the left side.
        if (gap_size - intervals[i-1].size() - n_right >= n_right) {
          break;
        }
        if (n_right >= intervals[i].size() + intervals[i-1].size()) {
          intervals[i].merge(intervals[i-1]);
          right_indices.emplace_back(i-1);
          --i;
        }
      }
      
      vector<int> indices_to_remove = left_indices;
      indices_to_remove.insert(indices_to_remove.end(), right_indices.rbegin(), right_indices.rend());
      auto new_end = remove_if(intervals.begin(), intervals.end(), index_removal(indices_to_remove));
      intervals.resize(new_end - intervals.begin());
    }
    
    // return the number of elements in this gap.
    unsigned long size() {
      return gap_size;
    }
  };
  
  splay_tree<gap> gap_ds;
  
public:
  lazy_search_tree() : lst_size(0) {}
  
  void insert(const T &key) {
    if (empty()) {
      gap r_gap = gap(key);
      gap_ds.insert(r_gap);
    } else {
      gap& r_gap = gap_ds.successor_or_equal(gap(key));
      r_gap.insert(key);
    }
    ++lst_size;
  }
  
  bool membership(const T &key) {
    if (empty()) {
      return false;
    } else {
      gap& r_gap = gap_ds.successor_or_equal(gap(key));
      r_gap.rebalance();
      bool result = r_gap.membership(key);
      
      
      return result;
    }
  }
  
  unsigned long size( ) const { return lst_size; }
  bool empty( ) const { return size() == 0; }
};

#endif // LAZY_SEARCH_TREE
