//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___PSTL_ITERATOR_PARTITIONS_H
#define _LIBCPP___PSTL_ITERATOR_PARTITIONS_H

/*
iterator_partitions.h synopsis

// a pair of iterators representing one partition
template <class _Iterator>
struct __iterator_range {
  _Iterator __first;
  _Iterator __last;
};

// generic abstraction that provides constant-time access to iterator ranges of each partition
template <class _Iterator, bool _IsRandomAccess = ...>
struct __iterator_partitions {
  // returns the number of partitions
  size_t __partitions_count() const;

  // returns an iterator range of a specific partition
  __iterator_range<_Iterator> __partition(size_t __n) const;

  // return the global end iterator
  _Iterator __end() const;
};

// partitions the input depending on the input size and the number of partitions.
// for random-access iterators effectively stores the input iterator and a minimal bookkeeping info.
// for forward iterators traverses the entire range and stores first-last pairs of each partition, O(N) time complexity.
template <class _Iterator>
std::optional<__iterator_partitions<_Iterator>>
__make_iterator_partitions(_Iterator __first, size_t __count, size_t __partitions);
*/

#include <__config>
#include <__iterator/concepts.h>
#include <__iterator/iterator_traits.h>
#include <__iterator/next.h>
#include <__memory/allocator.h>
#include <__memory/construct_at.h>
#include <__memory/destroy.h>
#include <__memory/unique_ptr.h>
#include <optional>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_PUSH_MACROS
#include <__undef_macros>

#if _LIBCPP_STD_VER >= 17

_LIBCPP_BEGIN_NAMESPACE_STD
namespace __pstl {

template <class _Iterator>
struct __iterator_range {
  _Iterator __first;
  _Iterator __last;
};

template <class _Iterator,
          bool _IsRandomAccessOrContiguous = __has_random_access_iterator_category_or_concept<_Iterator>::value>
struct __iterator_partitions;

template <class _Iterator>
struct __iterator_partitions<_Iterator, true> {
  size_t __partitions_count() const { return __partitions_count_; }

  __iterator_range<_Iterator> __partition(size_t __n) const {
    _Iterator __first = __base_ + __n * __partition_size_ + (__n < __remainder_ ? __n : __remainder_);
    _Iterator __last  = __first + __partition_size_ + (__n < __remainder_ ? 1 : 0);
    return __iterator_range<_Iterator>{__first, __last};
  }

  _Iterator __end() const { return __base_ + __count_; }

  _Iterator __base_;
  size_t __count_;
  size_t __partition_size_;
  size_t __remainder_;
  size_t __partitions_count_;
};

template <class _Iterator>
struct __iterator_partitions<_Iterator, false> {
  size_t __partitions_count() const { return __partitions_count_; }

  __iterator_range<_Iterator> __partition(size_t __n) const { return *(__partitions_.get() + __n); }

  _Iterator __end() const { return (__partitions_.get() + __partitions_count_ - 1)->__last; }

  struct __destroy_partitions {
    size_t __allocated;
    size_t __constructed;
    void operator()(__iterator_range<_Iterator>* __ptr) {
      std::destroy_n(__ptr, __constructed);
      std::allocator<__iterator_range<_Iterator>>().deallocate(__ptr, __allocated);
    }
  };

  std::unique_ptr<__iterator_range<_Iterator>[], __destroy_partitions> __partitions_;
  size_t __partitions_count_;
};

template <class _Iterator>
std::optional<__iterator_partitions<_Iterator>>
__make_iterator_partitions(_Iterator __first, size_t __count, size_t __partitions) {
  // TODO: assert that __partitions > 0 && __partitions <= __count
  __iterator_partitions<_Iterator> __p;
  if constexpr (__has_random_access_iterator_category_or_concept<_Iterator>::value) {
    __p.__base_             = std::move(__first);
    __p.__count_            = __count;
    __p.__partition_size_   = __count / __partitions;
    __p.__remainder_        = __count % __partitions;
    __p.__partitions_count_ = __partitions;
  } else {
    using __destroy_partitions = typename __iterator_partitions<_Iterator>::__destroy_partitions;
    __p.__partitions_          = std::unique_ptr<__iterator_range<_Iterator>[], __destroy_partitions> {
      [&]() -> __iterator_range<_Iterator>* {
#  if _LIBCPP_HAS_EXCEPTIONS
        try {
#  endif
          return std::allocator<__iterator_range<_Iterator>>().allocate(__partitions);
#  if _LIBCPP_HAS_EXCEPTIONS
        } catch (const std::bad_alloc&) {
          return nullptr;
        }
#  endif
      }(),
          __destroy_partitions{__partitions, 0}
    };
    if (!__p.__partitions_) {
      return {};
    }
    __p.__partitions_count_      = __partitions;
    size_t __base_partition_size = __count / __partitions;
    size_t __remainder           = __count % __partitions;
    for (size_t __i = 0; __i != __partitions; ++__i) {
      size_t __partition_size = __base_partition_size;
      if (__i < __remainder) {
        __partition_size++;
      }
      _Iterator __last = std::next(__first, __partition_size);
      std::__construct_at(__p.__partitions_.get() + __i, __iterator_range<_Iterator>{__first, __last});
      __p.__partitions_.get_deleter().__constructed++;
      __first = __last;
    }
  }
  return __p;
}

} // namespace __pstl
_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP_STD_VER >= 17

_LIBCPP_POP_MACROS

#endif // _LIBCPP___PSTL_ITERATOR_PARTITIONS_H
