//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// REQUIRES: std-at-least-c++17

// UNSUPPORTED: libcpp-has-no-incomplete-pstl

// <__pstl/iterator_partitions.h>

// this test checks the validity of partitioning and support for different iterators

#include <__pstl/iterator_partitions.h>
#include <cassert>
#include <cstddef>
#include <vector>
#include <forward_list>
#include <list>
#include <numeric>

template <class Container>
static void test_make_iterator_partitions() {
  { // test with 1 element
    Container v(1);
    const auto partitions = std::__pstl::__make_iterator_partitions(v.begin(), 1, 1);
    assert(partitions != std::nullopt);
    assert(partitions->__partitions_count() == 1);
    assert(partitions->__end() == v.end());
    assert(partitions->__partition(0).__first == v.begin());
    assert(partitions->__partition(0).__last == v.end());
  }
  { // test with 10 elements
    size_t size = 10;
    Container v(size);
    { // 1 partition: [0..10)
      const auto partitions = std::__pstl::__make_iterator_partitions(v.begin(), size, 1);
      assert(partitions != std::nullopt);
      assert(partitions->__partitions_count() == 1);
      assert(partitions->__end() == v.end());
      assert(partitions->__partition(0).__first == v.begin());
      assert(partitions->__partition(0).__last == std::next(v.begin(), 10));
    }
    { // 2 partitions: [0..5), [5..10)
      const auto partitions = std::__pstl::__make_iterator_partitions(v.begin(), size, 2);
      assert(partitions != std::nullopt);
      assert(partitions->__partitions_count() == 2);
      assert(partitions->__end() == v.end());
      assert(partitions->__partition(0).__first == v.begin());
      assert(partitions->__partition(0).__last == std::next(v.begin(), 5));
      assert(partitions->__partition(1).__first == std::next(v.begin(), 5));
      assert(partitions->__partition(1).__last == std::next(v.begin(), 10));
    }
    { // 3 partitions: [0..4), [4..7), [7..10)
      const auto partitions = std::__pstl::__make_iterator_partitions(v.begin(), size, 3);
      assert(partitions != std::nullopt);
      assert(partitions->__partitions_count() == 3);
      assert(partitions->__end() == v.end());
      assert(partitions->__partition(0).__first == v.begin());
      assert(partitions->__partition(0).__last == std::next(v.begin(), 4));
      assert(partitions->__partition(1).__first == std::next(v.begin(), 4));
      assert(partitions->__partition(1).__last == std::next(v.begin(), 7));
      assert(partitions->__partition(2).__first == std::next(v.begin(), 7));
      assert(partitions->__partition(2).__last == std::next(v.begin(), 10));
    }
    { // 4 partitions: [0..3), [3..6), [6..8), [8..10)
      const auto partitions = std::__pstl::__make_iterator_partitions(v.begin(), size, 4);
      assert(partitions != std::nullopt);
      assert(partitions->__partitions_count() == 4);
      assert(partitions->__end() == v.end());
      assert(partitions->__partition(0).__first == v.begin());
      assert(partitions->__partition(0).__last == std::next(v.begin(), 3));
      assert(partitions->__partition(1).__first == std::next(v.begin(), 3));
      assert(partitions->__partition(1).__last == std::next(v.begin(), 6));
      assert(partitions->__partition(2).__first == std::next(v.begin(), 6));
      assert(partitions->__partition(2).__last == std::next(v.begin(), 8));
      assert(partitions->__partition(3).__first == std::next(v.begin(), 8));
      assert(partitions->__partition(3).__last == std::next(v.begin(), 10));
    }
    { // 5 partitions: [0..2), [2..4), [4..6), [6..8), [8..10)
      const auto partitions = std::__pstl::__make_iterator_partitions(v.begin(), size, 5);
      assert(partitions != std::nullopt);
      assert(partitions->__partitions_count() == 5);
      assert(partitions->__end() == v.end());
      assert(partitions->__partition(0).__first == v.begin());
      assert(partitions->__partition(0).__last == std::next(v.begin(), 2));
      assert(partitions->__partition(1).__first == std::next(v.begin(), 2));
      assert(partitions->__partition(1).__last == std::next(v.begin(), 4));
      assert(partitions->__partition(2).__first == std::next(v.begin(), 4));
      assert(partitions->__partition(2).__last == std::next(v.begin(), 6));
      assert(partitions->__partition(3).__first == std::next(v.begin(), 6));
      assert(partitions->__partition(3).__last == std::next(v.begin(), 8));
      assert(partitions->__partition(4).__first == std::next(v.begin(), 8));
      assert(partitions->__partition(4).__last == std::next(v.begin(), 10));
    }
    { // 6 partitions: [0..2), [2..4), [4..6), [6..8), [8..9), [9..10)
      const auto partitions = std::__pstl::__make_iterator_partitions(v.begin(), size, 6);
      assert(partitions != std::nullopt);
      assert(partitions->__partitions_count() == 6);
      assert(partitions->__end() == v.end());
      assert(partitions->__partition(0).__first == v.begin());
      assert(partitions->__partition(0).__last == std::next(v.begin(), 2));
      assert(partitions->__partition(1).__first == std::next(v.begin(), 2));
      assert(partitions->__partition(1).__last == std::next(v.begin(), 4));
      assert(partitions->__partition(2).__first == std::next(v.begin(), 4));
      assert(partitions->__partition(2).__last == std::next(v.begin(), 6));
      assert(partitions->__partition(3).__first == std::next(v.begin(), 6));
      assert(partitions->__partition(3).__last == std::next(v.begin(), 8));
      assert(partitions->__partition(4).__first == std::next(v.begin(), 8));
      assert(partitions->__partition(4).__last == std::next(v.begin(), 9));
      assert(partitions->__partition(5).__first == std::next(v.begin(), 9));
      assert(partitions->__partition(5).__last == std::next(v.begin(), 10));
    }
    { // 7 partitions: [0..2), [2..4), [4..6), [6..7), [7..8), [8..9), [9..10)
      const auto partitions = std::__pstl::__make_iterator_partitions(v.begin(), size, 7);
      assert(partitions != std::nullopt);
      assert(partitions->__partitions_count() == 7);
      assert(partitions->__end() == v.end());
      assert(partitions->__partition(0).__first == v.begin());
      assert(partitions->__partition(0).__last == std::next(v.begin(), 2));
      assert(partitions->__partition(1).__first == std::next(v.begin(), 2));
      assert(partitions->__partition(1).__last == std::next(v.begin(), 4));
      assert(partitions->__partition(2).__first == std::next(v.begin(), 4));
      assert(partitions->__partition(2).__last == std::next(v.begin(), 6));
      assert(partitions->__partition(3).__first == std::next(v.begin(), 6));
      assert(partitions->__partition(3).__last == std::next(v.begin(), 7));
      assert(partitions->__partition(4).__first == std::next(v.begin(), 7));
      assert(partitions->__partition(4).__last == std::next(v.begin(), 8));
      assert(partitions->__partition(5).__first == std::next(v.begin(), 8));
      assert(partitions->__partition(5).__last == std::next(v.begin(), 9));
      assert(partitions->__partition(6).__first == std::next(v.begin(), 9));
      assert(partitions->__partition(6).__last == std::next(v.begin(), 10));
    }
    { // 8 partitions: [0..2), [2..4), [4..5), [5..6), [6..7), [7..8), [8..9), [9..10)
      const auto partitions = std::__pstl::__make_iterator_partitions(v.begin(), size, 8);
      assert(partitions != std::nullopt);
      assert(partitions->__partitions_count() == 8);
      assert(partitions->__end() == v.end());
      assert(partitions->__partition(0).__first == v.begin());
      assert(partitions->__partition(0).__last == std::next(v.begin(), 2));
      assert(partitions->__partition(1).__first == std::next(v.begin(), 2));
      assert(partitions->__partition(1).__last == std::next(v.begin(), 4));
      assert(partitions->__partition(2).__first == std::next(v.begin(), 4));
      assert(partitions->__partition(2).__last == std::next(v.begin(), 5));
      assert(partitions->__partition(3).__first == std::next(v.begin(), 5));
      assert(partitions->__partition(3).__last == std::next(v.begin(), 6));
      assert(partitions->__partition(4).__first == std::next(v.begin(), 6));
      assert(partitions->__partition(4).__last == std::next(v.begin(), 7));
      assert(partitions->__partition(5).__first == std::next(v.begin(), 7));
      assert(partitions->__partition(5).__last == std::next(v.begin(), 8));
      assert(partitions->__partition(6).__first == std::next(v.begin(), 8));
      assert(partitions->__partition(6).__last == std::next(v.begin(), 9));
      assert(partitions->__partition(7).__first == std::next(v.begin(), 9));
      assert(partitions->__partition(7).__last == std::next(v.begin(), 10));
    }
    { // 9 partitions: [0..2), [2..3), [3..4), [4..5), [5..6), [6..7), [7..8), [8..9), [9..10)
      const auto partitions = std::__pstl::__make_iterator_partitions(v.begin(), size, 9);
      assert(partitions != std::nullopt);
      assert(partitions->__partitions_count() == 9);
      assert(partitions->__end() == v.end());
      assert(partitions->__partition(0).__first == v.begin());
      assert(partitions->__partition(0).__last == std::next(v.begin(), 2));
      assert(partitions->__partition(1).__first == std::next(v.begin(), 2));
      assert(partitions->__partition(1).__last == std::next(v.begin(), 3));
      assert(partitions->__partition(2).__first == std::next(v.begin(), 3));
      assert(partitions->__partition(2).__last == std::next(v.begin(), 4));
      assert(partitions->__partition(3).__first == std::next(v.begin(), 4));
      assert(partitions->__partition(3).__last == std::next(v.begin(), 5));
      assert(partitions->__partition(4).__first == std::next(v.begin(), 5));
      assert(partitions->__partition(4).__last == std::next(v.begin(), 6));
      assert(partitions->__partition(5).__first == std::next(v.begin(), 6));
      assert(partitions->__partition(5).__last == std::next(v.begin(), 7));
      assert(partitions->__partition(6).__first == std::next(v.begin(), 7));
      assert(partitions->__partition(6).__last == std::next(v.begin(), 8));
      assert(partitions->__partition(7).__first == std::next(v.begin(), 8));
      assert(partitions->__partition(7).__last == std::next(v.begin(), 9));
      assert(partitions->__partition(8).__first == std::next(v.begin(), 9));
      assert(partitions->__partition(8).__last == std::next(v.begin(), 10));
    }
    { // 10 partitions: [0..1), [1..2), [2..3), [3..4), [4..5), [5..6), [6..7), [7..8), [8..9), [9..10)
      const auto partitions = std::__pstl::__make_iterator_partitions(v.begin(), size, 10);
      assert(partitions != std::nullopt);
      assert(partitions->__partitions_count() == 10);
      assert(partitions->__end() == v.end());
      assert(partitions->__partition(0).__first == v.begin());
      assert(partitions->__partition(0).__last == std::next(v.begin(), 1));
      assert(partitions->__partition(1).__first == std::next(v.begin(), 1));
      assert(partitions->__partition(1).__last == std::next(v.begin(), 2));
      assert(partitions->__partition(2).__first == std::next(v.begin(), 2));
      assert(partitions->__partition(2).__last == std::next(v.begin(), 3));
      assert(partitions->__partition(3).__first == std::next(v.begin(), 3));
      assert(partitions->__partition(3).__last == std::next(v.begin(), 4));
      assert(partitions->__partition(4).__first == std::next(v.begin(), 4));
      assert(partitions->__partition(4).__last == std::next(v.begin(), 5));
      assert(partitions->__partition(5).__first == std::next(v.begin(), 5));
      assert(partitions->__partition(5).__last == std::next(v.begin(), 6));
      assert(partitions->__partition(6).__first == std::next(v.begin(), 6));
      assert(partitions->__partition(6).__last == std::next(v.begin(), 7));
      assert(partitions->__partition(7).__first == std::next(v.begin(), 7));
      assert(partitions->__partition(7).__last == std::next(v.begin(), 8));
      assert(partitions->__partition(8).__first == std::next(v.begin(), 8));
      assert(partitions->__partition(8).__last == std::next(v.begin(), 9));
      assert(partitions->__partition(9).__first == std::next(v.begin(), 9));
      assert(partitions->__partition(9).__last == std::next(v.begin(), 10));
    }
  }
}

int main() {
  test_make_iterator_partitions< std::vector<int> >();
  test_make_iterator_partitions< std::list<int> >();
  test_make_iterator_partitions< std::forward_list<int> >();
}
