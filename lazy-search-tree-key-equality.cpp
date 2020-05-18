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
      T min_e;
      unsigned long int_size;
      
      // intervals require a linked list data structure for O(1) merging, but by using a linked list of
      // vectors, we can take advantage of larger built intervals and inserted elements, reducing the
      // number of pointers in the entire data structure to O(min(n, q log n)).
      list<vector<T>> elements;
      
    public:
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
      
      // merges 'other' into this interval, destroying 'other'.
      void merge(shared_ptr<interval> other) {
        cerr << "merging" << endl;
        int_size += other->int_size;
        max_e = max(max_e, other->max_e);
        min_e = min(min_e, other->min_e);
        
        // may want to make this conditional so that the interval is loosely structured in order, that
        // is, if other is a left side interval, do as below, otherwise, add the elements to the
        // beginning, not end.
        elements.splice(elements.end(), other->elements);
        other->int_size = 0;  // shouldn't matter but doing it anyway.
      }
      
      // insert an element into this interval.
      void insert(const T &element) {
        elements.front().emplace_back(element); // doesn't actually matter which vector the element
                                                // is placed into.
        max_e = max(max_e, element);
        min_e = min(min_e, element);
        ++int_size;
      }
      
      // create an interval from a vector of type T.
      interval(vector<T> &starting_elements) {
        int_size = starting_elements.size();
        if (!starting_elements.empty()) {
          elements.emplace_back(starting_elements);
          max_e = *max_element(starting_elements.begin(), starting_elements.end());
          min_e = *min_element(starting_elements.begin(), starting_elements.end());
        }
      }
      
      // create an interval with a single element.
      interval(const T &element) {
        int_size = 1;
        elements.emplace_back(vector<T>({element}));
        max_e = element;
        min_e = element;
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
      
      // Pivot so that keys < p go left, > p go right. Equality is split 50-50.
      // ideally this could be replaced with in-place pivoting, but since intervals
      // get moved around and must be able to expand, I don't believe this is possible.
      pair<shared_ptr<interval>, shared_ptr<interval>> pivot(const T &p) {
        vector<T> lesser, greater;
        for (vector<T>& vec : elements) {
          for (T& e : vec) {
            if (e < p) {
              lesser.emplace_back(e);
            } else if (e > p) {
              greater.emplace_back(e);
            } else {
              if (rand()%2 == 0) {
                lesser.emplace_back(e);
              } else {
                greater.emplace_back(e);
              }
            }
          }
        }
        return make_pair(shared_ptr<interval>(new interval(lesser)),
                         shared_ptr<interval>(new interval(greater)));
      }
      
   /*   // compare gaps to one another via their maximum element.
      bool operator< (const interval& other) const {
        return max_e < other.max_e;
      }*/
      
      // return the number of elements in this interval.
      unsigned long size() const {
        return int_size;
      }
      
      // get max element. Undefined behavior if interval is empty.
      T get_max() {
        return max_e;
      }
      
      // get min element. Undefined behavior if interval is empty.
      T get_min() {
        return min_e;
      }
      
      // return if this interval is empty.
      bool empty( ) const { return size() == 0; }
    };  // end interval class
    
    unsigned long gap_size;
    int last_left_idx = 0;
    
    // the sorted set of intervals within this gap; all elements in intervals[i] <= intervals[i+1].
    vector<shared_ptr<interval>> intervals;
    
    // initialize a gap with a vector of intervals.
    gap(vector<shared_ptr<interval>> &intervals) {
      gap_size = 0;
      for (shared_ptr<interval> g_int : intervals) {
        if (!g_int->empty()) {
          this->intervals.emplace_back(g_int);
          gap_size += g_int->size();
        }
      }
      rebalance();
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
 /*   int get_n_left(int int_idx) {
      int left = 0;
      for (int i = 0; i < int_idx; ++i) {
        left += intervals[i]->size();
      }
      return left;
    }*/
    
    // returns the number of elements in intervals[start_idx : end_idx], inclusive on both ends.
 /*   int subrange_size(int start_idx, int end_idx) {
      int total = 0;
      for (int i = start_idx; i <= end_idx; ++i) {
        total += intervals[i]->size();
      }
      
      return total;
    }*/
    
    // pick the pivot element to split the interval. Currently unused.
  /*  T pick_pivot(int sample_size, shared_ptr<interval> g_int) {
      vector<T> pivots(sample_size);
      for (int i = 0; i < sample_size; ++i)
        pivots[i] = g_int->sample();
      sort(pivots.begin(), pivots.end());
      return pivots[sample_size/2];
    }*/
    
    // split interval g_int, recursing on either the left or right side of the split,
    // based on the value of "recurse_left". Return a vector of all resulting intervals.
    vector<shared_ptr<interval>> split(shared_ptr<interval> g_int, bool recurse_left) {
      // Base case.
      if (g_int->empty()) {
        return vector<shared_ptr<interval>>();
      } else if (g_int->size() == 1) {
        vector<shared_ptr<interval>> temp;
        temp.emplace_back(g_int);
        return temp;
      }
      
      T p = g_int->sample();
      auto result_split = g_int->pivot(p);
      shared_ptr<interval> lesser = result_split.first;
      shared_ptr<interval> greater = result_split.second;
      
      // Recurse.
      vector<shared_ptr<interval>> result;
      if (recurse_left) {
        result = split(lesser, true);
        result.emplace_back(greater);
      } else {
        result.emplace_back(lesser);
        vector<shared_ptr<interval>> temp = split(greater, false);
        result.insert(result.end(), temp.begin(), temp.end());
      }
      return result;
    }
    
  public:
    // create a gap with a single interval containing a single element.
    gap(const T &key) {
      gap_size = 1;
      intervals.emplace_back(new interval(key));
    }
    
    // compare gaps to one another via their maximum element.
    bool operator< (const gap& other) const {
      if (intervals.back()->get_max() < other.intervals.back()->get_max()) {
        return true;
      } else if (!(other.intervals.back()->get_max() < intervals.back()->get_max())) {  // back elements are equal
        return intervals.front()->get_min() < other.intervals.front()->get_min();
      } else {
        return false;
      }
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
    pair<gap, gap> restructure(const T &key) {
      int int_idx = getIntervalIdx(key);
      auto result = intervals[int_idx]->pivot(key);
      
      vector<shared_ptr<interval>> left_result = split(result.first, false);
      vector<shared_ptr<interval>> greater = split(result.second, true);
      vector<shared_ptr<interval>> lesser;
      for (int i = 0; i < int_idx; ++i) {
        lesser.emplace_back(intervals[i]);
      }
      lesser.insert(lesser.end(), left_result.begin(), left_result.end());
      for (int i = int_idx+1; i < intervals.size(); ++i) {
        greater.emplace_back(intervals[i]);
      }
      
      // for many reasons, the intervals of lesser or greater may be empty.
      // The gap constructor will keep only non-empty intervals.
      return make_pair(gap(lesser), gap(greater));
    }
    
    template <typename Iterator, typename f>
    int perform_merges(Iterator begin,
                        Iterator end,
                        f deletion) {
      int n_out = 0;
      int i = 0;
      auto it = begin;
      for (;;) {
        auto it2 = it;
        ++it2;
        if (it2 == end) break;
        int cur_size = (int)(*it)->size();
        int next_size = (int)(*it2)->size();
        int n_in = (int)size() - cur_size - n_out;
        if (n_out + cur_size >= n_in - next_size) {  // it2 is on the other side
          return i;
        }
        
        if (n_out >= cur_size + next_size) {
          (*it)->merge(*it2);
          deletion(it2);
        } else {
          n_out += cur_size;
          ++it;
          ++i;
        }
      }
      return i;
    }
    
    // rebalance according to (A) and (B). Precondition: no interval is empty.
    void rebalance() {
      list<shared_ptr<interval>> intervals_list(intervals.begin(), intervals.end());
      last_left_idx = perform_merges(intervals_list.begin(), intervals_list.end(),
                     [&intervals_list](typename list<shared_ptr<interval>>::iterator it){
        intervals_list.erase(it);
      });
      perform_merges(intervals_list.rbegin(), intervals_list.rend(),
                   [&intervals_list](typename list<shared_ptr<interval>>::reverse_iterator it){
        intervals_list.erase(next(it).base());
      });
      intervals = vector<shared_ptr<interval>>(intervals_list.begin(), intervals_list.end());
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
      pair<gap, gap> new_gaps = r_gap.restructure(key);
      gap_ds.erase(r_gap);  // note: this destroys r_gap.
      if (!new_gaps.first.empty()) {
        gap_ds.insert(new_gaps.first);
      }
      if (!new_gaps.second.empty()) {
        gap_ds.insert(new_gaps.second);
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
