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

#ifndef GALOIS_WORKLIST_OBIM_H
#define GALOIS_WORKLIST_OBIM_H

#include "galois/FlatMap.h"
#include "galois/runtime/Substrate.h"
#include "galois/substrate/PerThreadStorage.h"
//#include "galois/substrate/Termination.h"
#include "galois/worklists/Chunk.h"
#include "galois/worklists/WorkListHelpers.h"

#include <deque>
#include <limits>
#include <type_traits>

namespace galois {
namespace worklists {

namespace internal {

template <typename T, typename Index, bool UseBarrier>
class OrderedByIntegerMetricData {
protected:
  struct ThreadData {};
  bool hasStored(ThreadData&, Index) { return false; }
  galois::optional<T> popStored(ThreadData&, Index) { return {}; }
};

template <typename T, typename Index>
class OrderedByIntegerMetricData<T, Index, true> {
protected:
  struct ThreadData {
    bool hasWork;
    std::deque<std::pair<Index, T>> stored;
  };

  substrate::Barrier& barrier;

  OrderedByIntegerMetricData()
      : barrier(runtime::getBarrier(runtime::activeThreads)) {}

  bool hasStored(ThreadData& p, Index idx) {
    for (auto& e : p.stored) {
      if (e.first == idx) {
        std::swap(e, p.stored.front());
        return true;
      }
    }
    return false;
  }

  galois::optional<T> popStored(ThreadData& p, Index idx) {
    galois::optional<T> item;
    for (auto ii = p.stored.begin(), ei = p.stored.end(); ii != ei; ++ii) {
      if (ii->first == idx) {
        item = ii->second;
        p.stored.erase(ii);
        break;
      }
    }
    return item;
  }
};

template <typename Index, bool UseDescending>
struct OrderedByIntegerMetricComparator {
  std::less<Index> compare;
  Index identity;

  template <typename C>
  struct with_local_map {
    typedef galois::flat_map<Index, C, std::less<Index>> type;
  };
  OrderedByIntegerMetricComparator()
      : identity(std::numeric_limits<Index>::min()) {}
};

template <typename Index>
struct OrderedByIntegerMetricComparator<Index, true> {
  std::greater<Index> compare;
  Index identity;

  template <typename C>
  struct with_local_map {
    typedef galois::flat_map<Index, C, std::greater<Index>> type;
  };
  OrderedByIntegerMetricComparator()
      : identity(std::numeric_limits<Index>::max()) {}
};

} // namespace internal

/**
 * Approximate priority scheduling. Indexer is a default-constructable class
 * whose instances conform to <code>R r = indexer(item)</code> where R is some
 * type with a total order defined by <code>operator&lt;</code> and
 * <code>operator==</code> and item is an element from the Galois set
 * iterator.
 *
 * An example:
 * \code
 * struct Item { int index; };
 *
 * struct Indexer {
 *   int operator()(Item i) const { return i.index; }
 * };
 *
 * typedef galois::worklists::OrderedByIntegerMetric<Indexer> WL;
 * galois::for_each<WL>(galois::iterate(items), Fn);
 * \endcode
 *
 * @tparam Indexer        Indexer class
 * @tparam Container      Scheduler for each bucket
 * @tparam BlockPeriod    Check for higher priority work every 2^BlockPeriod
 *                        iterations
 * @tparam BSP            Use back-scan prevention
 * @tparam UseBarrier     Eliminate priority inversions by placing a barrier
 * between priority levels
 * @tparam UseMonotonic   Assume that an activity at priority p will not
 * schedule work at priority p or any priority p1 where p1 < p.
 * @tparam UseDescending  Use descending order instead
 */
// TODO could move to general comparator but there are issues with atomic reads
// and initial values for arbitrary types
template <class Indexer      = DummyIndexer<int>,
          typename Container = PerSocketChunkFIFO<>, unsigned BlockPeriod = 0,
          bool BSP = true, typename T = int, typename Index = int,
          bool UseBarrier = false, bool UseMonotonic = false,
          bool UseDescending = false, bool Concurrent = true>
struct OrderedByIntegerMetric
    : private boost::noncopyable,
      public internal::OrderedByIntegerMetricData<T, Index, UseBarrier>,
      public internal::OrderedByIntegerMetricComparator<Index, UseDescending> {
  // static_assert(std::is_integral<Index>::value, "only integral index types
  // supported");

  template <typename _T>
  using retype = OrderedByIntegerMetric<
      Indexer, typename Container::template retype<_T>, BlockPeriod, BSP, _T,
      typename std::result_of<Indexer(_T)>::type, UseBarrier, UseMonotonic,
      UseDescending, Concurrent>;

  template <bool _b>
  using rethread =
      OrderedByIntegerMetric<Indexer, Container, BlockPeriod, BSP, T, Index,
                             UseBarrier, UseMonotonic, UseDescending, _b>;

  template <unsigned _period>
  struct with_block_period {
    typedef OrderedByIntegerMetric<Indexer, Container, _period, BSP, T, Index,
                                   UseBarrier, UseMonotonic, UseDescending,
                                   Concurrent>
        type;
  };

  template <typename _container>
  struct with_container {
    typedef OrderedByIntegerMetric<Indexer, _container, BlockPeriod, BSP, T,
                                   Index, UseBarrier, UseMonotonic,
                                   UseDescending, Concurrent>
        type;
  };

  template <typename _indexer>
  struct with_indexer {
    typedef OrderedByIntegerMetric<_indexer, Container, BlockPeriod, BSP, T,
                                   Index, UseBarrier, UseMonotonic,
                                   UseDescending, Concurrent>
        type;
  };

  template <bool _bsp>
  struct with_back_scan_prevention {
    typedef OrderedByIntegerMetric<Indexer, Container, BlockPeriod, _bsp, T,
                                   Index, UseBarrier, UseMonotonic,
                                   UseDescending, Concurrent>
        type;
  };

  template <bool _use_barrier>
  struct with_barrier {
    typedef OrderedByIntegerMetric<Indexer, Container, BlockPeriod, BSP, T,
                                   Index, _use_barrier, UseMonotonic,
                                   UseDescending, Concurrent>
        type;
  };

  template <bool _use_monotonic>
  struct with_monotonic {
    typedef OrderedByIntegerMetric<Indexer, Container, BlockPeriod, BSP, T,
                                   Index, UseBarrier, _use_monotonic,
                                   UseDescending, Concurrent>
        type;
  };

  template <bool _use_descending>
  struct with_descending {
    typedef OrderedByIntegerMetric<Indexer, Container, BlockPeriod, BSP, T,
                                   Index, UseBarrier, UseMonotonic,
                                   _use_descending, Concurrent>
        type;
  };

  typedef T value_type;
  typedef Index index_type;

private:
  typedef typename Container::template rethread<Concurrent> CTy;
  typedef internal::OrderedByIntegerMetricComparator<Index, UseDescending>
      Comparator;
  typedef typename Comparator::template with_local_map<CTy*>::type LMapTy;
  
  bool sync;
  int pd;

  struct ThreadData
      : public internal::OrderedByIntegerMetricData<T, Index,
                                                    UseBarrier>::ThreadData {
    int pd_counter = 0;
    unsigned int latest_index = 0;
    
    LMapTy local;
    Index curIndex;
    Index scanStart;
    CTy* current;
    unsigned int lastMasterVersion;
    unsigned int numPops;

    ThreadData(Index initial)
        : curIndex(initial), scanStart(initial), current(0),
          lastMasterVersion(0), numPops(0) {}
  };

  typedef std::deque<std::pair<Index, CTy*>> MasterLog;

  // NB: Place dynamically growing masterLog after fixed-size PerThreadStorage
  // members to give higher likelihood of reclaiming PerThreadStorage
  substrate::PerThreadStorage<ThreadData> data;
  substrate::PaddedLock<Concurrent> masterLock;
  MasterLog masterLog;

  std::atomic<unsigned int> masterVersion;
  Indexer indexer;

  bool updateLocal(ThreadData& p) {
    if (p.lastMasterVersion != masterVersion.load(std::memory_order_relaxed)) {
      for (;
           p.lastMasterVersion < masterVersion.load(std::memory_order_relaxed);
           ++p.lastMasterVersion) {
        // XXX(ddn): Somehow the second block is better than
        // the first for bipartite matching (GCC 4.7.2)
#if 0
        p.local.insert(masterLog[p.lastMasterVersion]);
#else
        std::pair<Index, CTy*> logEntry = masterLog[p.lastMasterVersion];
        p.local[logEntry.first]         = logEntry.second;
        assert(logEntry.second);
#endif
      }
      return true;
    }
    return false;
  }

  GALOIS_ATTRIBUTE_NOINLINE
  galois::optional<T> slowPop(ThreadData& p) {
    bool localLeader = substrate::ThreadPool::isLeader();
    Index msS        = this->identity;

    updateLocal(p);

    if (BSP && !UseMonotonic) {
      msS = p.scanStart;
      if (localLeader) {
        for (unsigned i = 0; i < runtime::activeThreads; ++i) {
          Index o = data.getRemote(i)->scanStart;
          if (this->compare(o, msS))
            msS = o;
        }
      } else {
        Index o = data.getRemote(substrate::ThreadPool::getLeader())->scanStart;
        if (this->compare(o, msS))
          msS = o;
      }
    }

    for (auto ii = p.local.lower_bound(msS), ei = p.local.end(); ii != ei;
         ++ii) {
      galois::optional<T> item;
      if ((item = ii->second->pop())) {
        p.current   = ii->second;
        p.curIndex  = ii->first;
        p.scanStart = ii->first;
        return item;
      }
    }

    return galois::optional<value_type>();
  }

  GALOIS_ATTRIBUTE_NOINLINE
  CTy* slowUpdateLocalOrCreate(ThreadData& p, Index i) {
    // update local until we find it or we get the write lock
    do {
      CTy* C;
      updateLocal(p);
      if ((C = p.local[i]))
        return C;
    } while (!masterLock.try_lock());
    // we have the write lock, update again then create
    updateLocal(p);
    CTy*& C2 = p.local[i];
    if (!C2) {
      C2                  = new CTy();
      p.lastMasterVersion = masterVersion.load(std::memory_order_relaxed) + 1;
      masterLog.push_back(std::make_pair(i, C2));
      masterVersion.fetch_add(1);
    }
    masterLock.unlock();
    return C2;
  }

  inline CTy* updateLocalOrCreate(ThreadData& p, Index i) {
    // Try local then try update then find again or else create and update the
    // master log
    CTy* C;
    if ((C = p.local[i]))
      return C;
    // slowpath
    return slowUpdateLocalOrCreate(p, i);
  }

public:
  OrderedByIntegerMetric(const Indexer& x = Indexer())
      : data(this->identity), masterVersion(0), indexer(x) {}

  ~OrderedByIntegerMetric() {
    // Deallocate in LIFO order to give opportunity for simple garbage
    // collection
    for (auto ii = masterLog.rbegin(), ei = masterLog.rend(); ii != ei; ++ii) {
      delete ii->second;
    }
  }

  void push(const value_type& val) {
    Index index   = indexer(val);
    //if (substrate::ThreadPool::getTID() == 0) {cout << index << endl;}

    ThreadData& p = *data.getLocal();

    assert(!UseMonotonic || this->compare(p.curIndex, index));

    // Fast path
    if (index == p.curIndex && p.current) {
      p.current->push(val);
      return;
    }

    // Slow path
    CTy* C = updateLocalOrCreate(p, index);
    if (BSP && this->compare(index, p.scanStart))
      p.scanStart = index;
    // Opportunistically move to higher priority work
    if (!UseBarrier && this->compare(index, p.curIndex)) {
      p.curIndex = index;
      p.current  = C;
    }
    C->push(val);
  }

  template <typename Iter>
  void push(Iter b, Iter e) {
    while (b != e)
      push(*b++);
  }

  template <typename RangeTy>
  void push_initial(const RangeTy& range) {
    //cout << "Push Initial" << endl;
    auto rp = range.local_pair();
    push(rp.first, rp.second);
  }

  galois::optional<value_type> pop() {
    ThreadData& p = *data.getLocal();
    galois::optional<value_type> retval;

    if (substrate::ThreadPool::getTID() == 0) {
      int pd_temp = 0;
    /* Priority drift logic */
      if (p.pd_counter == 2000) {
        sync = true;
      }
      if (p.pd_counter == 2100) {
        sync = false;
        p.pd_counter = 0;
      
        for (int i = 1; i < runtime::activeThreads; i++) {
          int pd_ = p.latest_index - data.getRemote(i)->latest_index;
          pd_temp += abs(pd_);
        } 
        pd = (pd + pd_temp) / 2;
        std::cout << "PD " << pd << std::endl;
        
      }
    }
    p.pd_counter++;

    if (sync == true) p.latest_index = indexer(retval.get());
    // Find a successful pop
    
    CTy* C        = p.current;

    if (this->hasStored(p, p.curIndex)) {
      retval = this->popStored(p, p.curIndex);
      if (sync == true) p.latest_index = indexer(retval.get());
      return retval;
    }
     

    if (!UseBarrier && BlockPeriod &&
        (p.numPops++ & ((1 << BlockPeriod) - 1)) == 0) {
          retval = slowPop(p);
          if (sync == true) p.latest_index = indexer(retval.get());
          return retval;          
        }
      
    galois::optional<value_type> item;
    if (C && (item = C->pop())) {
       if (sync == true) p.latest_index = indexer(item.get());
       return item;
    }
      

    if (UseBarrier) {
      if (sync == true) p.latest_index = indexer(item.get());
      return item;
    }
      

    // Slow path
    retval =  slowPop(p);
    if (sync == true) p.latest_index = indexer(retval.get());
    return retval;
  }

  template <bool Barrier = UseBarrier>
  auto empty() -> typename std::enable_if<Barrier, bool>::type {
    galois::optional<value_type> item;
    ThreadData& p = *data.getLocal();

    item = slowPop(p);
    if (item)
      p.stored.push_back(std::make_pair(p.curIndex, *item));
    p.hasWork = item;

    this->barrier.wait();

    bool hasWork   = p.hasWork;
    Index curIndex = p.curIndex;
    CTy* C         = p.current;

    for (unsigned i = 0; i < runtime::activeThreads; ++i) {
      ThreadData& o = *data.getRemote(i);
      if (o.hasWork && this->compare(o.curIndex, curIndex)) {
        curIndex = o.curIndex;
        C        = o.current;
      }
      hasWork |= o.hasWork;
    }

    this->barrier.wait();

    p.current  = C;
    p.curIndex = curIndex;

    if (UseMonotonic) {
      for (auto ii = p.local.begin(); ii != p.local.end();) {
        bool toBreak = ii->second == C;
        ii           = p.local.erase(ii);
        if (toBreak)
          break;
      }
    }

    return !hasWork;
  }
};
GALOIS_WLCOMPILECHECK(OrderedByIntegerMetric)

/* minn */
namespace internal {

template <typename T, typename Index, bool UseBarrier>
class OrderedByIntegerMetricMinnData {
protected:
  struct ThreadData {};
  bool hasStored(ThreadData&, Index) { return false; }
  galois::optional<T> popStored(ThreadData&, Index) { return {}; }
};

template <typename T, typename Index>
class OrderedByIntegerMetricMinnData<T, Index, true> {
protected:
  struct ThreadData {
    bool hasWork;
    std::deque<std::pair<Index, T>> stored;
  };

  substrate::Barrier& barrier;

  OrderedByIntegerMetricMinnData()
      : barrier(runtime::getBarrier(runtime::activeThreads)) {}

  bool hasStored(ThreadData& p, Index idx) {
    for (auto& e : p.stored) {
      if (e.first == idx) {
        std::swap(e, p.stored.front());
        return true;
      }
    }
    return false;
  }

  galois::optional<T> popStored(ThreadData& p, Index idx) {
    galois::optional<T> item;
    for (auto ii = p.stored.begin(), ei = p.stored.end(); ii != ei; ++ii) {
      if (ii->first == idx) {
        item = ii->second;
        p.stored.erase(ii);
        break;
      }
    }
    return item;
  }
};

template <typename Index, bool UseDescending>
struct OrderedByIntegerMetricMinnComparator {
  std::less<Index> compare;
  Index identity;

  template <typename C>
  struct with_local_map {
    typedef galois::flat_map<Index, C, std::less<Index>> type;
  };
  OrderedByIntegerMetricMinnComparator()
      : identity(std::numeric_limits<Index>::min()) {}
};

template <typename Index>
struct OrderedByIntegerMetricMinnComparator<Index, true> {
  std::greater<Index> compare;
  Index identity;

  template <typename C>
  struct with_local_map {
    typedef galois::flat_map<Index, C, std::greater<Index>> type;
  };
  OrderedByIntegerMetricMinnComparator()
      : identity(std::numeric_limits<Index>::max()) {}
};

} // namespace internal

/**
 * Approximate priority scheduling. Indexer is a default-constructable class
 * whose instances conform to <code>R r = indexer(item)</code> where R is some
 * type with a total order defined by <code>operator&lt;</code> and
 * <code>operator==</code> and item is an element from the Galois set
 * iterator.
 *
 * An example:
 * \code
 * struct Item { int index; };
 *
 * struct Indexer {
 *   int operator()(Item i) const { return i.index; }
 * };
 *
 * typedef galois::worklists::OrderedByIntegerMetricMinn<Indexer> WL;
 * galois::for_each<WL>(galois::iterate(items), Fn);
 * \endcode
 *
 * @tparam Indexer        Indexer class
 * @tparam Container      Scheduler for each bucket
 * @tparam BlockPeriod    Check for higher priority work every 2^BlockPeriod
 *                        iterations
 * @tparam BSP            Use back-scan prevention
 * @tparam UseBarrier     Eliminate priority inversions by placing a barrier
 * between priority levels
 * @tparam UseMonotonic   Assume that an activity at priority p will not
 * schedule work at priority p or any priority p1 where p1 < p.
 * @tparam UseDescending  Use descending order instead
 */
// TODO could move to general comparator but there are issues with atomic reads
// and initial values for arbitrary types
template <class Indexer      = DummyIndexer<int>,
          typename Container = PerSocketChunkFIFO<>, unsigned BlockPeriod = 0,
          bool BSP = true, typename T = int, typename Index = int,
          bool UseBarrier = false, bool UseMonotonic = false,
          bool UseDescending = false, bool Concurrent = true>
struct OrderedByIntegerMetricMinn
    : private boost::noncopyable,
      public internal::OrderedByIntegerMetricMinnData<T, Index, UseBarrier>,
      public internal::OrderedByIntegerMetricMinnComparator<Index, UseDescending> {
  // static_assert(std::is_integral<Index>::value, "only integral index types
  // supported");

  template <typename _T>
  using retype = OrderedByIntegerMetricMinn<
      Indexer, typename Container::template retype<_T>, BlockPeriod, BSP, _T,
      typename std::result_of<Indexer(_T)>::type, UseBarrier, UseMonotonic,
      UseDescending, Concurrent>;

  template <bool _b>
  using rethread =
      OrderedByIntegerMetricMinn<Indexer, Container, BlockPeriod, BSP, T, Index,
                             UseBarrier, UseMonotonic, UseDescending, _b>;

  template <unsigned _period>
  struct with_block_period {
    typedef OrderedByIntegerMetricMinn<Indexer, Container, _period, BSP, T, Index,
                                   UseBarrier, UseMonotonic, UseDescending,
                                   Concurrent>
        type;
  };

  template <typename _container>
  struct with_container {
    typedef OrderedByIntegerMetricMinn<Indexer, _container, BlockPeriod, BSP, T,
                                   Index, UseBarrier, UseMonotonic,
                                   UseDescending, Concurrent>
        type;
  };

  template <typename _indexer>
  struct with_indexer {
    typedef OrderedByIntegerMetricMinn<_indexer, Container, BlockPeriod, BSP, T,
                                   Index, UseBarrier, UseMonotonic,
                                   UseDescending, Concurrent>
        type;
  };

  template <bool _bsp>
  struct with_back_scan_prevention {
    typedef OrderedByIntegerMetricMinn<Indexer, Container, BlockPeriod, _bsp, T,
                                   Index, UseBarrier, UseMonotonic,
                                   UseDescending, Concurrent>
        type;
  };

  template <bool _use_barrier>
  struct with_barrier {
    typedef OrderedByIntegerMetricMinn<Indexer, Container, BlockPeriod, BSP, T,
                                   Index, _use_barrier, UseMonotonic,
                                   UseDescending, Concurrent>
        type;
  };

  template <bool _use_monotonic>
  struct with_monotonic {
    typedef OrderedByIntegerMetricMinn<Indexer, Container, BlockPeriod, BSP, T,
                                   Index, UseBarrier, _use_monotonic,
                                   UseDescending, Concurrent>
        type;
  };

  template <bool _use_descending>
  struct with_descending {
    typedef OrderedByIntegerMetricMinn<Indexer, Container, BlockPeriod, BSP, T,
                                   Index, UseBarrier, UseMonotonic,
                                   _use_descending, Concurrent>
        type;
  };

  typedef T value_type;
  typedef Index index_type;

private:
  bool sync = false;
  unsigned int pd = 0;

  typedef typename Container::template rethread<Concurrent> CTy;
  typedef internal::OrderedByIntegerMetricMinnComparator<Index, UseDescending>
      Comparator;
  typedef typename Comparator::template with_local_map<CTy*>::type LMapTy;


  struct ThreadData
      : public internal::OrderedByIntegerMetricMinnData<T, Index,
                                                    UseBarrier>::ThreadData {
    queue<T> minnow_enqueue;
    queue<galois::optional<T>> minnow_dequeue;
    substrate::PaddedLock<Concurrent> enqueue_lock;
    substrate::PaddedLock<Concurrent> dequeue_lock;

    bool minnow_thread;
    bool done = false;
    
    LMapTy local;
    Index curIndex;
    Index scanStart;
    CTy* current;
    unsigned int lastMasterVersion;
    unsigned int numPops;

    int pd_counter = 0;
    unsigned int latest_index = 0;
    
    ThreadData(Index initial)
        : curIndex(initial), scanStart(initial), current(0),
          lastMasterVersion(0), numPops(0) {
          }
  };


  int minnow_threads;
  typedef std::deque<std::pair<Index, CTy*>> MasterLog;

  // NB: Place dynamically growing masterLog after fixed-size PerThreadStorage
  // members to give higher likelihood of reclaiming PerThreadStorage
  substrate::PerThreadStorage<ThreadData> data;
  substrate::PaddedLock<Concurrent> masterLock;
  MasterLog masterLog;

  std::atomic<unsigned int> masterVersion;
  Indexer indexer;

  bool updateLocal(ThreadData& p) {
    if (p.lastMasterVersion != masterVersion.load(std::memory_order_relaxed)) {
      for (;
           p.lastMasterVersion < masterVersion.load(std::memory_order_relaxed);
           ++p.lastMasterVersion) {
        // XXX(ddn): Somehow the second block is better than
        // the first for bipartite matching (GCC 4.7.2)
#if 0
        p.local.insert(masterLog[p.lastMasterVersion]);
#else
        std::pair<Index, CTy*> logEntry = masterLog[p.lastMasterVersion];
        p.local[logEntry.first]         = logEntry.second;
        assert(logEntry.second);
#endif
      }
      return true;
    }
    return false;
  }

  GALOIS_ATTRIBUTE_NOINLINE
  galois::optional<T> slowPop(ThreadData& p) {
    bool localLeader = substrate::ThreadPool::isLeader();
    Index msS        = this->identity;

    updateLocal(p);

    if (BSP && !UseMonotonic) {
      msS = p.scanStart;
      if (localLeader) {
        for (unsigned i = 0; i < runtime::activeThreads; ++i) {
          Index o = data.getRemote(i)->scanStart;
          if (this->compare(o, msS))
            msS = o;
        }
      } else {
        Index o = data.getRemote(substrate::ThreadPool::getLeader())->scanStart;
        if (this->compare(o, msS))
          msS = o;
      }
    }

    for (auto ii = p.local.lower_bound(msS), ei = p.local.end(); ii != ei;
         ++ii) {
      galois::optional<T> item;
      if ((item = ii->second->pop())) {
        p.current   = ii->second;
        p.curIndex  = ii->first;
        p.scanStart = ii->first;
        return item;
      }
    }

    return galois::optional<value_type>();
  }

  GALOIS_ATTRIBUTE_NOINLINE
  CTy* slowUpdateLocalOrCreate(ThreadData& p, Index i) {
    // update local until we find it or we get the write lock
    do {
      CTy* C;
      updateLocal(p);
      if ((C = p.local[i]))
        return C;
    } while (!masterLock.try_lock());
    // we have the write lock, update again then create
    updateLocal(p);
    CTy*& C2 = p.local[i];
    if (!C2) {
      C2                  = new CTy();
      p.lastMasterVersion = masterVersion.load(std::memory_order_relaxed) + 1;
      masterLog.push_back(std::make_pair(i, C2));
      masterVersion.fetch_add(1);
    }
    masterLock.unlock();
    return C2;
  }

  inline CTy* updateLocalOrCreate(ThreadData& p, Index i) {
    // Try local then try update then find again or else create and update the
    // master log
    CTy* C;
    if ((C = p.local[i]))
      return C;
    // slowpath
    return slowUpdateLocalOrCreate(p, i);
  }

public:
  OrderedByIntegerMetricMinn(const Indexer& x = Indexer(), int minnow_threads = 0)
      : data(this->identity), masterVersion(0), indexer(x), minnow_threads(minnow_threads) {}

  ~OrderedByIntegerMetricMinn() {
    // Deallocate in LIFO order to give opportunity for simple garbage
    // collection
    for (auto ii = masterLog.rbegin(), ei = masterLog.rend(); ii != ei; ++ii) {
      delete ii->second;
    }
  }

  void push(const value_type& val) {
    ThreadData& p = *data.getLocal();
    p.enqueue_lock.lock();
    p.minnow_enqueue.push(val);
    p.enqueue_lock.unlock();
    return;
  }
  
  galois::optional<value_type> pop() {
    
    ThreadData& p = *data.getLocal();
    int minnow_thread_id = substrate::ThreadPool::getTID();  
    int minnow_threads_start = runtime::activeThreads - minnow_threads;
    int minnow_workers_per_core = minnow_threads_start / minnow_threads;
    if (minnow_thread_id >= minnow_threads_start) {
      int start = 0; int end = 0;
      int thread_index = minnow_thread_id - minnow_threads_start;
      start = minnow_workers_per_core * thread_index;
      end = start + (minnow_workers_per_core - 1);

      bool work_done = false;
      int work_done_counter = 0;
      while (!p.done) {
        
        for (int i = start; i < end; i++) {
        T val;
        /* POP */
        work_done = false;
        while(true) {
          data.getRemote(i)->enqueue_lock.lock();
          if (data.getRemote(i)->minnow_enqueue.empty()) {data.getRemote(i)->enqueue_lock.unlock(); break;}
          
          val = data.getRemote(i)->minnow_enqueue.front(); data.getRemote(i)->minnow_enqueue.pop();
          data.getRemote(i)->enqueue_lock.unlock();

          Index index   = indexer(val);
          assert(!UseMonotonic ||this->compare(data.getRemote(i)->curIndex, index));
          work_done = true;
          // Fast path
          if (index == data.getRemote(i)->curIndex && data.getRemote(i)->current) {
            data.getRemote(i)->current->push(val);
            continue;
          }

          // Slow path
          CTy* C = updateLocalOrCreate(*data.getRemote(i), index);
          if (BSP && this->compare(index, data.getRemote(i)->scanStart))
            data.getRemote(i)->scanStart = index;
          // Opportunistically move to higher priority work
          if (!UseBarrier && this->compare(index, data.getRemote(i)->curIndex)) {
            data.getRemote(i)->curIndex = index;
            data.getRemote(i)->current  = C;
          }
          C->push(val);
        }
      }


      for (int i = start; i < end; i++) {
        CTy* C  = data.getRemote(i)->current;
        
        
        if (this->hasStored(*data.getRemote(i), data.getRemote(i)->curIndex)) {
            galois::optional<value_type> item;
            item = this->popStored(*data.getRemote(i), data.getRemote(i)->curIndex);
            
            if (item.is_initialized()) {
              data.getRemote(i)->dequeue_lock.lock();
              data.getRemote(i)->minnow_dequeue.push(item);
              data.getRemote(i)->dequeue_lock.unlock();
              work_done = true;
            }
            continue;

          }
        
        if (!UseBarrier && BlockPeriod &&
            (data.getRemote(i)->numPops++ & ((1 << BlockPeriod) - 1)) == 0) {
            galois::optional<value_type> item;
            item = slowPop(*data.getRemote(i));
 
            if (item.is_initialized()) {
              data.getRemote(i)->dequeue_lock.lock();
              data.getRemote(i)->minnow_dequeue.push(item); 
              data.getRemote(i)->dequeue_lock.unlock();
              work_done = true;
            }
            continue;
          }

        galois::optional<value_type> item;
        if (C && (item = C->pop())) {
          if (item.is_initialized()) {
            data.getRemote(i)->dequeue_lock.lock();
            data.getRemote(i)->minnow_dequeue.push(item);
            data.getRemote(i)->dequeue_lock.unlock();
            work_done = true;
          }
          continue;
        }
    
        if (UseBarrier) {
          if (item.is_initialized()) {
            data.getRemote(i)->dequeue_lock.lock();
            data.getRemote(i)->minnow_dequeue.push(item);
            data.getRemote(i)->dequeue_lock.unlock();
            work_done = true;
          }
          continue;
        }
          

        // Slow path
        item = slowPop(*data.getRemote(i));
        if (item.is_initialized()) {
          data.getRemote(i)->dequeue_lock.lock();
          data.getRemote(i)->minnow_dequeue.push(item);
          data.getRemote(i)->dequeue_lock.unlock();
          work_done = true;
        }
        continue;
      }
      /* Check termination condition */
      if (work_done == false) {
        for (int i = start; i < end; i++) {
          if (data.getRemote(i)->minnow_enqueue.empty() && data.getRemote(i)->minnow_dequeue.empty()) {
            
          }
        }
      }
        work_done_counter++;
        if (work_done_counter == 65536*28) {
          break;
        }
    }
     
    }
    else {
      galois::optional<value_type> retval;
      p.dequeue_lock.lock();
      if (p.minnow_dequeue.empty()) { p.dequeue_lock.unlock(); return retval;}
      else {
        retval = p.minnow_dequeue.front(); p.minnow_dequeue.pop();
        p.dequeue_lock.unlock();

        if (substrate::ThreadPool::getTID() == 0) {
          int pd_temp = 0;
        /* Priority drift logic */
          if (p.pd_counter == 2000) {
            sync = true;
          }
          if (p.pd_counter == 2100) {
            sync = false;
            p.pd_counter = 0;
          
            for (int i = 1; i < 6; i++) {
              int pd_ = p.latest_index - data.getRemote(i)->latest_index;
              pd_temp += abs(pd_ * 512);
            } 
            pd = (pd + pd_temp) / 2;
            std::cout << "PD " << pd << std::endl;
            
          }
        }
        p.pd_counter++;
        if (sync == true) p.latest_index = indexer(retval.get());
        return retval;
      }
    }
    p.done = true;
    galois::optional<value_type> retval;
    return retval;

  }

  template <typename Iter>
  void push(Iter b, Iter e) {
    while (b != e)
      push(*b++);
  }

  template <typename RangeTy>
  void push_initial(const RangeTy& range) {
    
    if (substrate::ThreadPool::getTID() == 0) {
      auto rp = range.local_pair();
      push(rp.first, rp.second);
    }
  }


  template <bool Barrier = UseBarrier>
  auto empty() -> typename std::enable_if<Barrier, bool>::type {
    galois::optional<value_type> item;
    ThreadData& p = *data.getLocal();

    item = slowPop(p);
    if (item)
      p.stored.push_back(std::make_pair(p.curIndex, *item));
    p.hasWork = item;

    this->barrier.wait();

    bool hasWork   = p.hasWork;
    Index curIndex = p.curIndex;
    CTy* C         = p.current;

    for (unsigned i = 0; i < runtime::activeThreads; ++i) {
      ThreadData& o = *data.getRemote(i);
      if (o.hasWork && this->compare(o.curIndex, curIndex)) {
        curIndex = o.curIndex;
        C        = o.current;
      }
      hasWork |= o.hasWork;
    }

    this->barrier.wait();

    p.current  = C;
    p.curIndex = curIndex;

    if (UseMonotonic) {
      for (auto ii = p.local.begin(); ii != p.local.end();) {
        bool toBreak = ii->second == C;
        ii           = p.local.erase(ii);
        if (toBreak)
          break;
      }
    }

    return !hasWork;
  }
};
GALOIS_WLCOMPILECHECK(OrderedByIntegerMetricMinn)


} // end namespace worklists
} // end namespace galois

#endif
