/*
 * This file belongs to the Galois project, a C++ library for exploiting parallelism.
 * The code is being released under the terms of the 3-Clause BSD License (a
 * copy is located in LICENSE.txt at the top-level directory).
 *
 * Copyright (C) 2018, The University of Texas at Austin. All rights reserved.
 * UNIVERSITY EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES CONCERNING THIS
 * SOFTWARE AND DOCUMENTATION, INCLUDING ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR ANY PARTICULAR PURPOSE, NON-INFRINGEMENT AND WARRANTIES OF
 * PERFORMANCE, AND ANY WARRANTY THAT MIGHT OTHERWISE ARISE FROM COURSE OF
 * DEALING OR USAGE OF TRADE.  NO WARRANTY IS EITHER EXPRESS OR IMPLIED WITH
 * RESPECT TO THE USE OF THE SOFTWARE OR DOCUMENTATION. Under no circumstances
 * shall University be liable for incidental, special, indirect, direct or
 * consequential damages or loss of profits, interruption of business, or
 * related expenses which may arise from use of Software or Documentation,
 * including but not limited to those resulting from defects in Software and/or
 * Documentation, or loss or inaccuracy of data of any kind.
 */

#ifndef GALOIS_WORKLIST_WORKLISTHELPERS_H
#define GALOIS_WORKLIST_WORKLISTHELPERS_H

#include "WLCompileCheck.h"
#include "../PriorityQueue.h"
#include "galois/substrate/PtrLock.h"
#include "galois/FlatMap.h"
#include <boost/iterator/iterator_facade.hpp>
#include <iostream>
#include <queue>
#include <cmath>

using namespace std;
namespace galois {
namespace worklists {

template <typename T>
class ConExtListNode {
  T* next;

public:
  ConExtListNode() : next(0) {}
  T*& getNext() { return next; }
  T* const& getNext() const { return next; }
};

template <typename T>
class ConExtIterator
    : public boost::iterator_facade<ConExtIterator<T>, T,
                                    boost::forward_traversal_tag> {
  friend class boost::iterator_core_access;
  T* at;

  template <typename OtherTy>
  bool equal(const ConExtIterator<OtherTy>& o) const {
    return at == o.at;
  }

  T& dereference() const { return *at; }
  void increment() { at = at->getNext(); }

public:
  ConExtIterator() : at(0) {}

  template <typename OtherTy>
  ConExtIterator(const ConExtIterator<OtherTy>& o) : at(o.at) {}

  explicit ConExtIterator(T* x) : at(x) {}
};

template <typename T, bool concurrent>
class ConExtLinkedStack {
  // fixme: deal with concurrent
  substrate::PtrLock<T> head;

public:
  typedef ConExtListNode<T> ListNode;

  bool empty() const { return !head.getValue(); }

  void push(T* C) {
    T* oldhead(0);
    do {
      oldhead      = head.getValue();
      C->getNext() = oldhead;
    } while (!head.CAS(oldhead, C));
  }

  T* pop() {
    // lock free Fast path (empty)
    if (empty())
      return 0;

    // Disable CAS
    head.lock();
    T* C = head.getValue();
    if (!C) {
      head.unlock();
      return 0;
    }
    head.unlock_and_set(C->getNext());
    C->getNext() = 0;
    return C;
  }

  //! iterators not safe with concurrent modifications
  typedef T value_type;
  typedef T& reference;
  typedef ConExtIterator<T> iterator;
  typedef ConExtIterator<const T> const_iterator;

  iterator begin() { return iterator(head.getValue()); }
  iterator end() { return iterator(); }

  const_iterator begin() const { return const_iterator(head.getValue()); }
  const_iterator end() const { return const_iterator(); }
};

template <typename T, bool concurrent>
class ConExtLinkedQueue {
  // Fixme: deal with concurrent
  substrate::PtrLock<T> head;
  T* tail;

public:
  typedef ConExtListNode<T> ListNode;

  ConExtLinkedQueue() : tail(0) {}

  bool empty() const { return !tail; }

  void push(T* C) {
    head.lock();
    // std::cerr << "in(" << C << ") ";
    C->getNext() = 0;
    if (tail) {
      tail->getNext() = C;
      tail            = C;
      head.unlock();
    } else {
      assert(!head.getValue());
      tail = C;
      head.unlock_and_set(C);
    }
  }

  T* pop() {
    // lock free Fast path empty case
    if (empty())
      return 0;

    head.lock();
    T* C = head.getValue();
    if (!C) {
      head.unlock();
      return 0;
    }
    if (tail == C) {
      tail = 0;
      assert(!C->getNext());
      head.unlock_and_clear();
    } else {
      head.unlock_and_set(C->getNext());
      C->getNext() = 0;
    }
    return C;
  }

  //! iterators not safe with concurrent modifications
  typedef T value_type;
  typedef T& reference;
  typedef ConExtIterator<T> iterator;
  typedef ConExtIterator<const T> const_iterator;

  iterator begin() { return iterator(head.getValue()); }
  iterator end() { return iterator(); }

  const_iterator begin() const { return const_iterator(head.getValue()); }
  const_iterator end() const { return const_iterator(); }
};

template <typename T>
struct DummyIndexer : public std::unary_function<const T&, unsigned> {
  unsigned operator()(const T& x) { return 0; }
};

/* Mohsin */

//! Simple Container Wrapper worklist (not scalable).
template <typename T, typename container = std::deque<T>, bool popBack = true>
class SwarmPQ : private boost::noncopyable {
  substrate::PaddedLock<true> lock;
  container wl;

public:
  template <typename _T>
  using retype = SwarmPQ<_T>;

  template <bool b>
  using rethread = SwarmPQ;

  typedef T value_type;

  void push(const value_type& val) {
    std::lock_guard<substrate::PaddedLock<true>> lg(lock);
    wl.push_back(val);
  }

  template <typename Iter>
  void push(Iter b, Iter e) {
    std::lock_guard<substrate::PaddedLock<true>> lg(lock);
    wl.insert(wl.end(), b, e);
  }

  template <typename RangeTy>
  void push_initial(const RangeTy& range) {
    if (substrate::ThreadPool::getTID() == 0)
      push(range.begin(), range.end());
  }

  galois::optional<value_type> pop() {
    galois::optional<value_type> retval;
    std::lock_guard<substrate::PaddedLock<true>> lg(lock);
    if (!wl.empty()) {
      if (popBack) {
        retval = wl.back();
        wl.pop_back();
      } else {
        retval = wl.front();
        wl.pop_front();
      }
    }
    return retval;
  }
};
GALOIS_WLCOMPILECHECK(SwarmPQ)

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <vector>
using namespace std;

template <typename T>
class RELD : private boost::noncopyable {
/* Lock */
using Lock_ty = galois::substrate::SimpleLock;

bool sync = false;
unsigned int pd = 0;

public:
  struct ThreadData {
    priority_queue<T> PQ;
    Lock_ty m_mutex;
    int remote_thread;

    int pd_counter = 0;
    unsigned int latest_index = 0;
  };

  RELD() {
    srand( (unsigned)time(NULL) );
    
  }

  ~RELD() {

  }

  substrate::PerThreadStorage<ThreadData> data;

  template <typename _T>
  using retype = RELD<_T>;

  template <bool b>
  using rethread = RELD;

  typedef T value_type;

  void push(const value_type& val) {

    ThreadData& p = *data.getLocal();
    p.remote_thread = std::rand() % runtime::activeThreads;
    if (p.remote_thread == substrate::ThreadPool::getTID()) {
      p.m_mutex.lock();
      p.PQ.push(val);
      p.m_mutex.unlock();
    }
    else {
      ThreadData& r = *data.getRemote(p.remote_thread);
      r.m_mutex.lock();
      r.PQ.push(val);
      r.m_mutex.unlock();
    }

  }

  template <typename Iter>
  void push(Iter b, Iter e) {
    
    for (; b!=e; ++b) {
      push(*b);
    }

  }

  template <typename RangeTy>
  void push_initial(const RangeTy& range) {
    if (substrate::ThreadPool::getTID() == 0) {push(range.begin(), range.end());}  
  }
  
  galois::optional<value_type> pop() { 
    
    ThreadData& p = *data.getLocal();
    p.m_mutex.lock();

    if (p.PQ.empty()) {
        p.m_mutex.unlock(); 
        return galois::optional<value_type>();
    }
    
    galois::optional<value_type> retval;
    
    /* PD */
    if (sync == true) {
      p.latest_index = p.PQ.top().dist;
    }

    retval = p.PQ.top(); p.PQ.pop();
    p.m_mutex.unlock();
    
    if (substrate::ThreadPool::getTID() == 0) {
      /* Priority drift logic */
      if (p.pd_counter == 2000) {
        sync = true;
      }
      if (p.pd_counter == 2100) {
        sync = false;
        p.pd_counter = 0;
        
        for (int i = 1; i <  runtime::activeThreads; i++) {
          int pd_ = p.latest_index - data.getRemote(i)->latest_index;
          pd += abs(pd_);
        } 
        pd = 0;
      }
    }
    p.pd_counter++;


    return retval;
  }
};
GALOIS_WLCOMPILECHECK(RELD)

#define MSG_QUEUE_SIZE 512

template <typename T, class Indexer = DummyIndexer<int>>
class HDCPS : private boost::noncopyable {

/* PD */
bool sync = false;
unsigned int pd = 0;
unsigned int pd_prev = 0;
bool prev_decision = false; // false decrease
bool first_iter = true;
int dist_factor = 8;
int dist_factor_prev = 8;
int dist_factor_den = 10;

public:
  struct ThreadData {
    ThreadData() {
      msg_queue =  new T[MSG_QUEUE_SIZE];
    }
    priority_queue<T> PQ;
    int ctr = 0;
    T *msg_queue;
    unsigned long msg_loc_curr; 
    unsigned long msg_loc = 0;
    int rr = substrate::ThreadPool::getTID();
    
    /* PD */
    int pd_counter = 0;
    unsigned int latest_index = 0;
  };

  HDCPS(const Indexer& x) : indexer(x) {

  }

  ~HDCPS() {
    
  }
  substrate::PerThreadStorage<ThreadData> data;
  Indexer indexer;

  template <typename _T>
  using retype = HDCPS<_T, Indexer>;

  template <bool b>
  using rethread = HDCPS;

  typedef T value_type;

  void push(const value_type& val) {
    
    ThreadData& p = *data.getLocal();
    if (p.msg_loc != p.msg_loc_curr) {
        p.PQ.push(p.msg_queue[p.msg_loc]);
        p.msg_loc = (p.msg_loc + 1) % MSG_QUEUE_SIZE;
    }
    
    if (p.ctr <= dist_factor) {
      p.PQ.push(val);
    }
    else {
      p.rr = (p.rr + 1) % runtime::activeThreads;
      if (p.rr == substrate::ThreadPool::getTID()) {p.PQ.push(val);}
      ThreadData& r = *data.getRemote(p.rr); 
     
      int loc = r.msg_loc_curr; r.msg_loc_curr = (r.msg_loc_curr + 1) % MSG_QUEUE_SIZE;
      r.msg_queue[loc] = val; 
      
    }
    
    p.ctr = (p.ctr + 1) % dist_factor_den;
    
  }

  template <typename Iter>
  void push(Iter b, Iter e) {
    for (; b!=e; ++b) {
      push(*b);
    }
 
  }

  template <typename RangeTy>
  void push_initial(const RangeTy& range) {
    if (substrate::ThreadPool::getTID() == 0){push(range.begin(), range.end());}  
  }
  
  galois::optional<value_type> pop() { 
    
    ThreadData& p = *data.getLocal();
   
    unsigned long loc = p.msg_loc_curr;

    if (p.msg_loc != loc) {
        p.PQ.push(p.msg_queue[p.msg_loc]);
        p.msg_loc = (p.msg_loc + 1) % MSG_QUEUE_SIZE;
    }

    if (p.PQ.empty()) {
        return galois::optional<value_type>();
    }
    galois::optional<value_type> retval;
   
    {
      if (substrate::ThreadPool::getTID() == 0) {
        /* Priority drift logic */
        if (p.pd_counter == 2000) {
          sync = true;
        }
        if (p.pd_counter == 2200) {
          sync = false;
          p.pd_counter = 0;
          
          for (int i = 1; i <  runtime::activeThreads; i++) {
            int pd_ = p.latest_index - data.getRemote(i)->latest_index;
            pd += abs(pd_);
          }

          if (first_iter) {
            pd_prev = pd;
            first_iter = false;
          }
          else {
            if (pd >= (pd_prev) && prev_decision == true) {
              dist_factor = min(dist_factor + 1, 8); // decrease TDF
              prev_decision = false;
            }
            else if (pd >= (pd_prev) && prev_decision == false) {
              dist_factor = max(dist_factor - 1, 3); // increase tdf
              prev_decision = true;
            }
            else {
              dist_factor = min(dist_factor + 1, 8); // decrease TDF
              prev_decision = false;
            }
          }
          pd_prev = pd;
          pd = 0;
          
        }
      }
    }
    p.pd_counter++;
    
    /* PD */
    if (sync == true) {
      p.latest_index = p.PQ.top().dist;
    }
    
    retval = p.PQ.top(); p.PQ.pop();
   
    return retval;
  }
};
GALOIS_WLCOMPILECHECK(HDCPS)

} // namespace worklists
} // end namespace galois

#endif
