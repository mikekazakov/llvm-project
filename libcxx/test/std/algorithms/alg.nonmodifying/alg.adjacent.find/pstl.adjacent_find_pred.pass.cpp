//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// REQUIRES: std-at-least-c++17

// UNSUPPORTED: libcpp-has-no-incomplete-pstl

// template<class ExecutionPolicy, class ForwardIterator, class BinaryPredicate>
// ForwardIterator adjacent_find(ExecutionPolicy&& exec,
//                               ForwardIterator first, ForwardIterator last,
//                               BinaryPredicate pred);

#include <algorithm>
#include <cassert>
#include <numeric>

#include "test_execution_policies.h"
#include "test_macros.h"
#include "test_iterators.h"

auto pred = [](int a, int b) -> bool { return static_cast<long long>(b) - static_cast<long long>(a) == 42; };

template <class Iter>
struct Test {
  template <class ExecutionPolicy>
  void operator()(ExecutionPolicy&& policy) {
    {
      int a[]           = {42};
      const unsigned sa = sizeof(a) / sizeof(a[0]);
      assert(std::adjacent_find(policy, Iter(a), Iter(a), pred) == Iter(a));
      assert(std::adjacent_find(policy, Iter(a), Iter(a + sa), pred) == Iter(a + sa));
    }
    {
      int a[]           = {42, 84};
      const unsigned sa = sizeof(a) / sizeof(a[0]);
      assert(std::adjacent_find(policy, Iter(a), Iter(a + sa), pred) == Iter(a));
      assert(std::adjacent_find(policy, Iter(a), Iter(a + 1), pred) == Iter(a + 1));
      assert(std::adjacent_find(policy, Iter(a + 1), Iter(a + sa), pred) == Iter(a + sa));
    }
    {
      int a[]           = {0, 42, 84};
      const unsigned sa = sizeof(a) / sizeof(a[0]);
      assert(std::adjacent_find(policy, Iter(a), Iter(a + sa), pred) == Iter(a));
    }
    {
      int a[]           = {0, 41, 0};
      const unsigned sa = sizeof(a) / sizeof(a[0]);
      assert(std::adjacent_find(policy, Iter(a), Iter(a + sa), pred) == Iter(a + sa));
    }
    {
      int a[]           = {0, 1, 0, 42};
      const unsigned sa = sizeof(a) / sizeof(a[0]);
      assert(std::adjacent_find(policy, Iter(a), Iter(a + sa), pred) == Iter(a + 2));
    }
    {
      int a[]           = {0, 41, 0, 41};
      const unsigned sa = sizeof(a) / sizeof(a[0]);
      assert(std::adjacent_find(policy, Iter(a), Iter(a + sa), pred) == Iter(a + sa));
    }
    {
      int a[]           = {0, 1, 43, 85};
      const unsigned sa = sizeof(a) / sizeof(a[0]);
      assert(std::adjacent_find(policy, Iter(a), Iter(a + sa), pred) == Iter(a + 1));
    }
    {
      int a[]           = {0, 1, 2, 44, 0, 1, 2, 3};
      const unsigned sa = sizeof(a) / sizeof(a[0]);
      assert(std::adjacent_find(policy, Iter(a), Iter(a + sa), pred) == Iter(a + 2));
      assert(std::adjacent_find(policy, Iter(a + 2), Iter(a + sa), pred) == Iter(a + 2));
      assert(std::adjacent_find(policy, Iter(a), Iter(a), pred) == Iter(a));
      assert(std::adjacent_find(policy, Iter(a + 3), Iter(a + sa), pred) == Iter(a + sa));
    }
    {
      int a[]           = {0, 1, 2, 45, 0, 1, 2, 3};
      const unsigned sa = sizeof(a) / sizeof(a[0]);
      assert(std::adjacent_find(policy, Iter(a), Iter(a + sa), pred) == Iter(a + sa));
    }
    {
      int a[]     = {std::numeric_limits<int>::min(),
                     std::numeric_limits<int>::min() + 42,
                     std::numeric_limits<int>::max() - 42,
                     std::numeric_limits<int>::max()};
      unsigned sa = sizeof(a) / sizeof(a[0]);
      assert(std::adjacent_find(policy, Iter(a), Iter(a + sa), pred) == Iter(a));
    }
    {
      int a[]     = {std::numeric_limits<int>::max(),
                     std::numeric_limits<int>::min(),
                     std::numeric_limits<int>::max() - 42,
                     std::numeric_limits<int>::max()};
      unsigned sa = sizeof(a) / sizeof(a[0]);
      assert(std::adjacent_find(policy, Iter(a), Iter(a + sa), pred) == Iter(a + 2));
    }
    {
      int a[]     = {std::numeric_limits<int>::max(),
                     std::numeric_limits<int>::min(),
                     std::numeric_limits<int>::max(),
                     std::numeric_limits<int>::min()};
      unsigned sa = sizeof(a) / sizeof(a[0]);
      assert(std::adjacent_find(policy, Iter(a), Iter(a + sa), pred) == Iter(a + sa));
    }
    {
      int a[]     = {std::numeric_limits<int>::max(),
                     std::numeric_limits<int>::min(),
                     std::numeric_limits<int>::min() + 42,
                     std::numeric_limits<int>::max()};
      unsigned sa = sizeof(a) / sizeof(a[0]);
      assert(std::adjacent_find(policy, Iter(a), Iter(a + sa), pred) == Iter(a + 1));
    }
    {
      int a[1073];
      unsigned sa = sizeof(a) / sizeof(a[0]);
      std::iota(a, a + sa, 0);
      for (unsigned i = 1; i < sa; i = i <= 16 ? i + 1 : unsigned(3.1415 * i)) {
        a[i] = a[i - 1] + 42;
        assert(std::adjacent_find(policy, Iter(a), Iter(a + sa), pred) == Iter(a + i - 1));
        a[i] = i;
      }
    }
  }
};

int main(int, char**) {
  types::for_each(types::concatenate_t<types::forward_iterator_list<int*>,
                                       types::bidirectional_iterator_list<int*>,
                                       types::random_access_iterator_list<int*>>{},
                  TestIteratorWithPolicies<Test>{});

  return 0;
}
