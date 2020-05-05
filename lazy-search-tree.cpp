// An implementation of the lazy search tree data structure, from the paper "Lazy Search Trees"
// by Bryce Sandlund and Sebastian Wild. A splay tree is used as the data structure for the gaps
// and a linked list of vectors is the data structure for the intervals, which allows O(1) time
// merge, insert, and delete, while maintaining the O(min(n, q log n)) pointer bound.

// Currently seems to assume elements inserted are unique.

#ifndef LAZY_SEARCH_TREE
#define LAZY_SEARCH_TREE

#include "splay.cpp"
#include <vector>
#include <list>
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <iostream>

using namespace std;

//TODO: make sure Comp is being used on elements of type T, don't believe it's correct atm.
template<typename T, typename Comp = std::less<T>>
class lazy_search_tree {
private:
  Comp comp;
  unsigned long lst_size;
  
  // data structure that contains a set of intervals within a gap.
  class gap {
  private:
    // an interval within a gap.
    class interval {
    private:
      T max_e;
      unsigned long int_size;
      
      // intervals require a linked list data structure for O(1) merging, but by using a linked list of
      // vectors, we can take advantage of larger built intervals and inserted elements, reducing the
      // number of pointers in the entire data structure to O(min(n, q log n)).
      list<vector<T>> elements;
      
      // returns an element uniformly at random from the interval. Time complexity is no worse
      // than linear in the size of the interval, but typically more like logarithmic. (Can we
      // derive a more rigorous bound here?)
      T sample() {
        int idx = rand()%size();
        for (vector<T>& vec : elements) {
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
      
    public:
      // merges 'other' into this interval, destroying 'other'.
      void merge(shared_ptr<interval> other) {
        int_size += other->int_size;
        max_e = max(max_e, other->max_e);
        
        // may want to make this conditional so that the interval is loosely structured in order, that
        // is, if other is a left side interval, do as below, otherwise, add the elements to the
        // beginning, not end.
        elements.splice(elements.end(), other->elements);
      }
      
      // insert an element into this interval.
      void insert(const T &element) {
        elements.front().emplace_back(element); // doesn't actually matter which vector the element
                                                // is placed into.
        max_e = max(max_e, element);
        ++int_size;
      }
      
      // create an interval from a vector of type T.
      interval(vector<T> &starting_elements) {
        int_size = starting_elements.size();
        if (!starting_elements.empty()) {
          elements.emplace_back(starting_elements);
          max_e = *max_element(starting_elements.begin(), starting_elements.end());
        }
      }
      
      // create an interval with a single element.
      interval(const T &element) {
        int_size = 1;
        elements.emplace_back(vector<T>({element}));
        max_e = element;
      }
      
      // linearly scan the interval to determine if the key is present.
      bool membership(const T &key) {
        for (vector<T>& vec : elements) {
          for (T e : vec) {
            if (key == e) return true;
          }
        }
        return false;
      }
      
      // Pivot so that keys < p go left, >= p go right.
      // ideally this could be replaced with in-place pivoting, but since intervals
      // get moved around and must be able to expand, I don't believe this is possible.
      pair<shared_ptr<interval>, shared_ptr<interval>> pivot(const T &p) {
        vector<T> lesser, greater;
        for (vector<T>& vec : elements) {
          for (T e : vec) {
            if (e < p) {
              lesser.emplace_back(e);
            } else {
              greater.emplace_back(e);
            }
          }
        }
        return make_pair(shared_ptr<interval>(new interval(lesser)),
                         shared_ptr<interval>(new interval(greater)));
      }
      
      // split this interval, recursing on either the left or right side of the split,
      // based on the value of "go_left". Return a vector of all resulting intervals,
      // including this interval.
      vector<shared_ptr<interval>> split(bool go_left, shared_ptr<interval> this_interval) {
        // Base case.
        if (empty()) {
          return vector<shared_ptr<interval>>();
        } else if (size() == 1) {
          vector<shared_ptr<interval>> temp;
          temp.emplace_back(this_interval);
          return temp;
        }
        
        // pick a pivot that splits the interval into a constant fraction:
        // between 1/3 and 2/3 of elements in each resulting split.
        // TODO: this may not be possible if there are many elements that compare equally; fix.
        // TODO: also, I guarantee this can be made faster; investigate.
        shared_ptr<interval> lesser, greater;
        while (true) {
          T p = pick_pivot(min(5UL, size())); // expected 1.72 iterations of loop
          auto result_split = pivot(p);
          lesser = result_split.first;
          greater = result_split.second;
          if (lesser->size() <= greater->size()*2 && greater->size() <= lesser->size()*2) {
            break;
          }
        }
        
        // Recurse.
        vector<shared_ptr<interval>> result;
        if (go_left) {
          result = lesser->split(go_left, lesser);
          result.emplace_back(greater);
          return result;
        } else {
          result = greater->split(go_left, greater);
          // O(log^2 |I|), but doesn't matter since we pay |I| to split.
          result.emplace(result.begin(), lesser);
          return result;
        }
      }
      
      // compare gaps to one another via their maximum element.
      bool operator < (const interval& other) const {
        return max_e < other.max_e;
      }
      
      // return the number of elements in this interval.
      unsigned long size() const {
        return int_size;
      }
      
      // get max element. Undefined behavior if interval is empty.
      T get_max() {
        return max_e;
      }
      
      // return if this interval is empty.
      bool empty( ) const { return size() == 0; }
    };  // end interval class
    
    unsigned long gap_size;
    int last_left_idx = 0;
    
    // the sorted set of intervals within this gap; all elements in intervals[i] <= intervals[i+1].
    // consider switching to a vector of pointers to intervals.
    vector<shared_ptr<interval>> intervals;
    
    // initialize a gap with a vector of intervals.
    gap(vector<shared_ptr<interval>> &intervals) : intervals(intervals) {
      gap_size = 0;
      for (shared_ptr<interval> c_int : intervals) {
        gap_size += c_int->size();
      }
    }
    
    // returns smallest interval with maximum element larger than or equal to key,
    // or the last interval if key is the maximum amongst all elements.
    int getIntervalIdx(const T &key) {
      int lo = last_left_idx, hi, mult;
      bool init = key <= intervals[last_left_idx]->get_max();
      if (init) {
        mult = -1;
      } else {
        mult = 1;
      }
      
      // exponential search
      for (int i = 0; ; ++i) {
        hi = lo + mult * (1 << i);
        if (hi < 0) {
          hi = -1;
          break;
        }
        if (hi >= intervals.size()) {
          hi = (int)intervals.size();
          break;
        }
        if (init != (key <= intervals[hi]->get_max())) {
          break;
        }
      }
      
      // binary search
      for (;;) {
        if (abs(hi - lo) <= 1) {
          if (init || hi == intervals.size()) return lo;
          else return hi;
        }
        
        int mid = (lo+hi)/2;
        if (init == (key <= intervals[mid]->get_max())) {
          lo = mid;
        } else {
          hi = mid;
        }
      }
    }
    
    // returns the number of elements left of this interval. Note: runs in O(number of intervals)
    // time, not O(log(number of intervals)), thus should only be used on query.
    int get_n_left(int int_idx) {
      int left = 0;
      for (int i = 0; i < int_idx; ++i) {
        left += intervals[i]->size();
      }
      return left;
    }
    
    int total_size(vector<shared_ptr<interval>> &vint) {
      int total = 0;
      for (shared_ptr<interval> I : vint) {
        total += I->size();
      }
      return total;
    }
    
  public:
    // create a gap with a single interval containing a single element.
    gap(const T &key) {
      gap_size = 1;
      intervals.emplace_back(new interval(key));
    }
    
    // compare gaps to one another via their maximum element.
    bool operator < (const gap& other) const {
      return intervals.back() < other.intervals.back();
    }
    
    // insert key into this gap.
    void insert(const T &key) {
      intervals[getIntervalIdx(key)]->insert(key);
      ++gap_size;
    }
    
    // query for membership of key in this gap. Note: this only answers the query. Restructuring
    // is done in the restructure method.
    bool membership(const T &key) {
      return intervals[getIntervalIdx(key)]->membership(key);
    }
    
    // restructure the gap so that all elements in gap > key remain in the gap and a new gap
    // is created and returned with elements <= key. TODO: replace with more general function.
    gap restructure(const T &key) {
      int int_idx = getIntervalIdx(key);
      auto result = intervals[int_idx]->pivot(key);
      
      vector<shared_ptr<interval>> left_result = result.first->split(false, result.first);
      vector<shared_ptr<interval>> greater = result.second->split(true, result.second);
      vector<shared_ptr<interval>> lesser;
      for (int i = 0; i < int_idx; ++i) {
        lesser.emplace_back(intervals[i]);
      }
      lesser.insert(lesser.end(), left_result.begin(), left_result.end());
      for (int i = int_idx+1; i < intervals.size(); ++i) {
        greater.emplace_back(intervals[i]);
      }
      
      if (greater.empty()) {
        intervals = lesser;
        gap_size = total_size(lesser);
        return gap(greater);  // return empty gap
      }
      
      intervals = greater;
      gap_size = total_size(greater);
      return gap(lesser);
      
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
    
    // rebalance according to (A) and (B).
    void rebalance() {
      vector<shared_ptr<interval>> left_intervals;
      // rebalance left side intervals
      int n_left = 0;
      for (int i = 0; i < intervals.size(); ++i) {
        int n_right = gap_size - (int)intervals[i]->size() - n_left;
        if (n_left > n_right) {
          last_left_idx = i-1;
          break;
        }
        
        left_intervals.emplace_back(intervals[i]);
        if (i+1 < intervals.size() && n_left >= intervals[i]->size() + intervals[i+1]->size()) {
          intervals[i]->merge(intervals[i+1]);
          n_left += intervals[i]->size();
          ++i;  // skip over i+1
        } else {
          n_left += intervals[i]->size();
        }
      }
      
      vector<shared_ptr<interval>> right_intervals;
      int n_right = 0;
      for (int i = (int)intervals.size()-1; i > last_left_idx; --i) {
        right_intervals.emplace_back(intervals[i]);
        if (i > last_left_idx && n_right >= intervals[i]->size() + intervals[i-1]->size()) {
          intervals[i]->merge(intervals[i-1]);
          n_right += intervals[i]->size();
          --i;  // skip over i-1
        } else {
          n_right += intervals[i]->size();
        }
      }
      
      left_intervals.insert(left_intervals.end(), right_intervals.rbegin(), right_intervals.rend());
      intervals = left_intervals; // will effectively destroy the intervals that were merged (which now hold no elements)
    }
    
    // return the number of elements in this gap.
    unsigned long size() const {
      return gap_size;
    }
    
    // return if this gap is empty.
    bool empty( ) const { return size() == 0; }
    
    T get_max() {
      return intervals.back()->get_max();
    }
    
    void print() {
      cout << get_max() << endl;
    }
  };  // end gap class
  
  splay_tree<gap> gap_ds;
  
public:
  lazy_search_tree() : lst_size(0) {}
  
  // insert key into the lazy search tree.
  void insert(const T &key) {
    if (empty()) {
      gap r_gap = gap(key);
      gap_ds.insert(r_gap);
    } else {
      gap& r_gap = gap_ds.lower_bound_or_last(gap(key));
      r_gap.insert(key);
    }
    ++lst_size;
  }
  
  // return if key is present in the lazy search tree and restructure
  // according to the new query.
  bool membership(const T &key) {
    if (empty()) {
      return false;
    } else {
      gap &r_gap = gap_ds.lower_bound_or_last(gap(key));
    //  r_gap.rebalance();  // I believe I can show the first rebalance is unnecessary.
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
  
  void print() {
    gap_ds.print();
  }
  
  unsigned long size( ) const { return lst_size; }
  bool empty( ) const { return size() == 0; }
};

#endif // LAZY_SEARCH_TREE
