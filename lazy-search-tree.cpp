// An implementation of the lazy search tree data structure, from the paper "Lazy Search Trees"
// by Bryce Sandlund and Sebastian Wild. A splay tree is used as the data structure for the gaps
// and a linked list of vectors is the data structure for the intervals, which allows O(1) time
// merge, insert, and delete, while using far fewer pointers than a standard linked list.

#ifndef LAZY_SEARCH_TREE
#define LAZY_SEARCH_TREE

#include "splay.cpp"
#include <vector>
#include <list>
#include <algorithm>
#include <cstdlib>

using namespace std;

//TODO: make sure Comp is being used on elements of type T, don't believe it's correct atm.
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
      T max_e;
      unsigned long int_size;
      
      // intervals require a linked list data structure for O(1) merging, but by using a linked list of
      // vectors, we can take advantage of larger built intervals and inserted elements, reducing the
      // number of pointers in the entire data structure to O(min(n, q log n)).
      list<vector<T>> elements;
      
      // merges 'other' into this interval, destroying 'other'.
      void merge(interval &other) {
        int_size += other.int_size;
        max_e = max(max_e, other.max_e);
        
        // may want to make this conditional so that the interval is loosely structured in order, that
        // is, if other is a left side interval, do as below, otherwise, add the elements to the
        // beginning, not end.
        elements.splice(elements.end(), other.elements);
      }
      
      // insert an element into this interval.
      void insert(T element) {
        elements.front().emplace_back(element); // doesn't actually matter which vector the element
                                                // is placed into.
        max_e = max(max_e, element);
        ++int_size;
      }
      
      // create an interval from a vector of type T.
      interval(vector<T> &starting_elements) {
        int_size = starting_elements.size();
        elements.emplace_back(starting_elements);
        max_e = *max_element(starting_elements.begin(), starting_elements.end());
      }
      
      // create an interval with a single element.
      interval(T element) {
        int_size = 1;
        elements.emplace_back(vector<T>({element}));
        max_e = element;
      }
      
      // linearly scan the interval to determine if the key is present.
      bool membership(T key) {
        for (vector<T> vec : elements) {
          for (T e : vec) {
            if (key == e) return true;
          }
        }
        return false;
      }
      
      // returns an element uniformly at random from the interval. Time complexity is no worse
      // than linear in the size of the interval, but typically more like logarithmic. (Can we
      // derive a more rigorous bound here?)
      T sample() {
        int idx = rand()%size();
        for (vector<T> vec : elements) {
          if (idx < vec.size()) {
            return vec[idx];
          }
          idx -= vec.size();
        }
        return T();
      }
      
      // pick the pivot element to split the interval. Currently, this picks the median of
      // three elements sampled uniformly at random.
      T pick_pivot(int sample_size) {
        vector<T> pivots(sample_size);
        for (int i = 0; i < sample_size; ++i)
          pivots[i] = sample();
        sort(pivots.begin(), pivots.end());
        return pivots[sample_size/2];
      }
      
      // ideally this could be replaced with in-place pivoting, but since intervals
      // get moved around and must be able to expand, I don't believe this is possible.
      pair<vector<T>, vector<T>> pivot(T p) {
        vector<T> lesser, greater;
        for (vector<T> vec : elements) {
          for (T e : vec) {
            if (e <= p) {
              lesser.push_back(e);
            } else {
              greater.push_back(e);
            }
          }
        }
        return make_pair(lesser, greater);
      }
      
      // split this interval, recursing on either the left or right side of the split,
      // based on the value of "go_left". Return a vector of all resulting intervals,
      // including this interval.
      vector<interval> split(bool go_left) {
        // Base case.
        if (size() == 0) {
          return vector<interval>();
        } else if (size() == 1) {
          vector<interval> temp;
          temp.emplace_back(*this);
          return temp;
        }
        
        // pick a pivot that splits the interval into a constant fraction:
        // between 1/3 and 2/3 of elements in each resulting split.
        // TODO: this may not be possible if there are many elements that compare equally; fix.
        // TODO: also, I guarantee this can be made faster; investigate.
        vector<T> lesser, greater;
        while (true) {
          T p = pick_pivot(min(5UL, size())); // expected 1.72 iterations of loop
          pair<vector<T>, vector<T>> result_split = pivot(p);
          lesser = result_split.first;
          greater = result_split.second;
          if (lesser.size() <= greater.size()*2 && greater.size() <= lesser.size()*2) {
            break;
          }
        }
        
        // Recurse.
        vector<interval> result;
        if (go_left) {
          result = interval(lesser).split(go_left);
          result.push_back(interval(greater));
          return result;
        } else {
          result = interval(greater).split(go_left);
          // O(log^2 |I|), but doesn't matter since we pay |I| to split.
          result.insert(result.begin(), interval(lesser));
          return result;
        }
      }
      
      // compare gaps to one another via their maximum element.
      bool operator < (const interval& other) const {
        return max_e < other.max_e;
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
    int getIntervalIdx(T key) {
      // can't get vector<interval>::iterator lb to compile.
      // could switch to more intelligent method to remove O(log log n) factor to
      // O(1) in average case.
      auto lb = lower_bound(intervals.begin(), intervals.end(), interval(key));
      if (lb == intervals.end()) {
        --lb;
      }
      return lb-intervals.begin();
    }
    
    // insert key into this gap.
    void insert(T key) {
      intervals[getIntervalIdx(key)].insert(key);
      ++gap_size;
    }
    
    // query for membership of key in this gap. Note: this only answers the query. Restructuring
    // is done in the restructure method.
    bool membership(T key) {
      return intervals[getIntervalIdx(key)].membership(key);
    }
    
    // returns the number of elements left of this interval. Note: runs in O(number of intervals)
    // time, not O(log(number of intervals)), thus should only be used on query.
    int get_n_left(int int_idx) {
      int left = 0;
      for (int i = 0; i < int_idx; ++i) {
        left += intervals[i].size();
      }
      return left;
    }
    
    int total_size(vector<interval> &vint) {
      int total = 0;
      for (interval I : vint) {
        total += I.size();
      }
      return total;
    }
    
    // restructure the gap so that all elements in gap <= key remain in the gap and a new gap
    // is created with elements > key. TODO: replace with more general function.
    gap restructure(T key) {
      int int_idx = getIntervalIdx(key);
      pair<vector<T>, vector<T>> result = intervals[int_idx].pivot(key);
      vector<interval> left_result = interval(result.first).split(false);
      vector<interval> greater = interval(result.second).split(true);
      vector<interval> lesser;
      for (int i = 0; i < int_idx; ++i) {
        lesser.push_back(intervals[i]);
      }
      lesser.insert(lesser.end(), left_result.begin(), left_result.end());
      for (int i = int_idx+1; i < intervals.size(); ++i) {
        greater.push_back(intervals[i]);
      }
      intervals = lesser;
      gap_size = total_size(lesser);
      return gap(greater);
      
      // for the moment, let's ignore the special case. I'm not positive it's necessary for runtime
      // analysis and it certainly doesn't impact correctness.
      
      /*
      int size_lesser = total_size(lesser);
      int size_greater = total_size(greater);
      int larger_size = size_lesser > size_greater ? size_lesser : size_greater;
      vector<interval>& larger = size_lesser > size_greater ? lesser : greater;
      */
      
      /*
      int n_left = get_n_left(int_idx);
      int n_right = size()-n_left-intervals[int_idx].size();
      
      // special median case.
      if (n_left < size()/2 && n_right < size()/2) {
        
      }*/
    }
    
    
    /*struct index_removal {
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
    };*/
    
    // rebalance according to (A) and (B).
    void rebalance() {
      vector<interval> left_intervals;
      // rebalance left side intervals
      int n_left = 0;
      for (int i = 0; i < intervals.size()-1; ++i) {
        n_left += intervals[i].size();
        left_intervals.emplace_back(intervals[i]);
        // break if interval i+1 is on the right side. An interval i is on the left side if it has less
        // or equal number of elements to its left than to its right, within the gap, and is on the right
        // side otherwise.
        if (gap_size - intervals[i+1].size() - n_left > n_left) {
          break;
        }
        if (n_left >= intervals[i].size() + intervals[i+1].size()) {
          intervals[i].merge(intervals[i+1]);
          ++i;  // skip over i+1
        }
      }
      
      vector<interval> right_intervals;
      // rebalance right side intervals
      int n_right = 0;
      for (int i = (int)intervals.size()-1; i >= 0; --i) {
        n_right += intervals[i].size();
        right_intervals.emplace_back(intervals[i]);
        // break if interval i-1 is on the left side.
        if (gap_size - intervals[i-1].size() - n_right >= n_right) {
          break;
        }
        if (n_right >= intervals[i].size() + intervals[i-1].size()) {
          intervals[i].merge(intervals[i-1]);
          --i;  // skip over i-1
        }
      }
      left_intervals.insert(left_intervals.end(), right_intervals.rbegin(), right_intervals.rend());
      intervals = left_intervals;
      
      /*vector<int> indices_to_remove = left_indices;
      indices_to_remove.insert(indices_to_remove.end(), right_indices.rbegin(), right_indices.rend());
      auto new_end = remove_if(intervals.begin(), intervals.end(), index_removal(indices_to_remove));
      intervals.resize(new_end - intervals.begin());*/
    }
    
    // return the number of elements in this gap.
    unsigned long size() const {
      return gap_size;
    }
    
    // return if this gap is empty.
    bool empty( ) const { return size() == 0; }
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
      gap new_gap = r_gap.restructure(key);
      r_gap.rebalance();
      if (!new_gap.empty()) {
        new_gap.rebalance();
        gap_ds.insert(new_gap);
      }
      
      return result;
    }
  }
  
  unsigned long size( ) const { return lst_size; }
  bool empty( ) const { return size() == 0; }
};

#endif // LAZY_SEARCH_TREE
