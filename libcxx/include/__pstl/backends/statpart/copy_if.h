//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___PSTL_BACKENDS_STATPART_COPY_IF_H
#define _LIBCPP___PSTL_BACKENDS_STATPART_COPY_IF_H

#include <__algorithm/copy_if.h>
#include <__algorithm/fill.h>
#include <__assert>
#include <__config>
#include <__cstddef/size_t.h>
#include <__execution/is_execution_policy.h>
#include <__functional/operations.h>
#include <__iterator/concepts.h>
#include <__iterator/iterator_traits.h>
#include <__memory/unique_ptr.h>
#include <__new/global_new_delete.h>
#include <__new/interference_size.h>
#include <__new/nothrow_t.h>
#include <__optional/nullopt_t.h>
#include <__optional/optional.h>
#include <__pstl/backend_fwd.h>
#include <__pstl/backends/statpart/basis.h>
#include <__pstl/backends/statpart/decoupled_lookback.h>
#include <__utility/move.h>
#include <climits> // for CHAR_BIT

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_PUSH_MACROS
#include <__undef_macros>

#if _LIBCPP_STD_VER >= 17

_LIBCPP_BEGIN_NAMESPACE_STD
namespace __pstl {

namespace __statpart {

struct _LIBCPP_HIDE_FROM_ABI __bitset_view {
  size_t* __bits_;
  size_t __words_;

  _LIBCPP_HIDE_FROM_ABI bool __test(size_t __pos) const {
    size_t __word_index = __pos / (CHAR_BIT * sizeof(size_t));
    size_t __bit_index  = __pos % (CHAR_BIT * sizeof(size_t));
    return (__bits_[__word_index] & (size_t(1) << __bit_index)) != 0;
  }
  _LIBCPP_HIDE_FROM_ABI void __set(size_t __pos) {
    size_t __word_index = __pos / (CHAR_BIT * sizeof(size_t));
    size_t __bit_index  = __pos % (CHAR_BIT * sizeof(size_t));
    __bits_[__word_index] |= (size_t(1) << __bit_index);
  }
  _LIBCPP_HIDE_FROM_ABI void __reset() {
    for (size_t __i = 0; __i < __words_; ++__i) {
      __bits_[__i] = 0;
    }
  }
};

struct _LIBCPP_HIDE_FROM_ABI __sharded_dynamic_bitset {
  unique_ptr<size_t[]> __bits_;
  size_t __shards_count_;
  size_t __words_per_shard_;

  _LIBCPP_HIDE_FROM_ABI __sharded_dynamic_bitset(size_t __shards_count, size_t __max_partition_size)
      : __shards_count_(__shards_count) {
    static_assert(hardware_destructive_interference_size % sizeof(size_t) == 0,
                  "hardware_destructive_interference_size must be divisible by sizeof(size_t)");
    _LIBCPP_ASSERT_INTERNAL(__shards_count > 0, "number of shards must be positive");
    _LIBCPP_ASSERT_INTERNAL(__max_partition_size > 0, "partition size must be positive");
    constexpr size_t __bits_per_cacheline = hardware_destructive_interference_size * CHAR_BIT;
    size_t __cachelines_per_shard         = (__max_partition_size + __bits_per_cacheline - 1) / __bits_per_cacheline;
    __words_per_shard_ = __cachelines_per_shard * (hardware_destructive_interference_size / sizeof(size_t));
    __bits_.reset(new (std::nothrow) size_t[__shards_count * __words_per_shard_]);
    // TODO: use an overaligned operator new to ensure alignment by hardware_destructive_interference_size?
  }

  _LIBCPP_HIDE_FROM_ABI __sharded_dynamic_bitset(__sharded_dynamic_bitset&&) noexcept            = default;
  _LIBCPP_HIDE_FROM_ABI __sharded_dynamic_bitset& operator=(__sharded_dynamic_bitset&&) noexcept = default;

  _LIBCPP_HIDE_FROM_ABI __bitset_view __shard(size_t __shard_idx) const {
    return {__bits_.get() + __shard_idx * __words_per_shard_, __words_per_shard_};
  }
};

template <class _ForwardIterator, class _Pred>
_LIBCPP_HIDE_FROM_ABI size_t __count_and_cache_predicate_results(
    _ForwardIterator __first, _ForwardIterator __last, const _Pred& __pred, __bitset_view __bitset) {
  size_t __occupied_count = 0;
  size_t __local_idx      = 0;
  for (auto __it = __first; __it != __last; ++__it) {
    if (__pred(*__it)) {
      __bitset.__set(__local_idx);
      ++__occupied_count;
    }
    ++__local_idx;
  }
  return __occupied_count;
}

template <class _ForwardIterator1, class _ForwardIterator2>
_LIBCPP_HIDE_FROM_ABI _ForwardIterator2 __copy_if_flag_set(
    _ForwardIterator1 __first, _ForwardIterator1 __last, _ForwardIterator2 __result, __bitset_view __bitset) {
  size_t __local_idx = 0;
  for (auto __it = __first; __it != __last; ++__it) {
    if (__bitset.__test(__local_idx)) {
      *__result = *__it;
      ++__result;
    }
    ++__local_idx;
  }
  return __result;
}

} // namespace __statpart

template <class _ExecutionPolicy>
struct __copy_if<__statpart_backend_tag, _ExecutionPolicy> {
  template <class _Policy, class _ForwardIterator1, class _ForwardIterator2, class _Pred>
  _LIBCPP_HIDE_FROM_ABI optional<_ForwardIterator2>
  operator()(_Policy&&, _ForwardIterator1 __first, _ForwardIterator1 __last, _ForwardIterator2 __result, _Pred __pred)
      const noexcept {
    if constexpr (__is_parallel_execution_policy_v<_ExecutionPolicy> &&
                  __has_random_access_iterator_category_or_concept<_ForwardIterator1>::value &&
                  __has_random_access_iterator_category_or_concept<_ForwardIterator2>::value) {
      auto __setup = [](size_t __max_workers_count,
                        size_t __max_partition_size) -> optional<__statpart::__sharded_dynamic_bitset> {
        __statpart::__sharded_dynamic_bitset __bitset{__max_workers_count, __max_partition_size};
        if (!__bitset.__bits_)
          return {}; // Failed to allocate memory
        return __bitset;
      };

      _ForwardIterator2 __out_iter = __result;

      auto __scan_head =
          [&](__statpart::__sharded_dynamic_bitset& __ctx,
              size_t __worker_idx,
              _ForwardIterator1 __chunk_first,
              _ForwardIterator1 __chunk_last,
              __statpart::__decoupled_lookback_partition<size_t>* __optional_lookback_partition) {
            // Acquire the local scratch memory
            __statpart::__bitset_view __bitset = __ctx.__shard(__worker_idx);

            // Reset the local storage by setting all flags to false
            __bitset.__reset();

            // Populate the bitset for each of the elements in the chunk and get the occupancy
            size_t __occupied_count =
                __statpart::__count_and_cache_predicate_results(__chunk_first, __chunk_last, __pred, __bitset);

            // If we're not the only chunk - publish the final occupancy
            if (__optional_lookback_partition != nullptr) {
              __optional_lookback_partition->__construct_inclusive_prefix(__occupied_count);
            }

            // Copy the elements depending on the bitset flags
            _ForwardIterator2 __chunk_result =
                __statpart::__copy_if_flag_set(__chunk_first, __chunk_last, __result, __bitset);

            // If we're the last chunk - also the report the position of the last iterator in the result range
            if (__optional_lookback_partition == nullptr) {
              __out_iter = __chunk_result;
            }
          };

      auto __scan_middle = [&](__statpart::__sharded_dynamic_bitset& __ctx,
                               size_t __worker_idx,
                               _ForwardIterator1 __chunk_first,
                               _ForwardIterator1 __chunk_last,
                               __statpart::__decoupled_lookback_partition<size_t>* __lookback_partition) {
        // Acquire the local scratch memory
        __statpart::__bitset_view __bitset = __ctx.__shard(__worker_idx);

        // Reset the local storage by setting all flags to false
        __bitset.__reset();

        // Populate the bitset for each of the elements in the chunk and get the occupancy
        size_t __occupied_count =
            __statpart::__count_and_cache_predicate_results(__chunk_first, __chunk_last, __pred, __bitset);

        // Publish the occupancy as a local aggregate first
        __lookback_partition->__construct_aggregate(__occupied_count);

        // Obtain the exclusive prefix
        __statpart::__decoupled_lookback_partition<size_t>* __prev_partition = __lookback_partition - 1;
        size_t __exclusive_prefix =
            (__prev_partition->__acquire_available_status() & __statpart::__decoupled_lookback_status_prefix_available)
                ? __prev_partition->__inclusive_prefix()
                : __statpart::__calculate_inclusive_prefix_of_partition(__prev_partition, plus<>{});

        // Publish the final occupancy
        __lookback_partition->__construct_inclusive_prefix(__exclusive_prefix + __occupied_count);

        // Copy the elements depending on the bitset flags
        __statpart::__copy_if_flag_set(__chunk_first, __chunk_last, __result + __exclusive_prefix, __bitset);
      };

      auto __scan_tail = [&](__statpart::__sharded_dynamic_bitset& __ctx,
                             size_t __worker_idx,
                             _ForwardIterator1 __chunk_first,
                             _ForwardIterator1 __chunk_last,
                             __statpart::__decoupled_lookback_partition<size_t>* __nonexistent_lookback_partition) {
        // Acquire the local scratch memory
        __statpart::__bitset_view __bitset = __ctx.__shard(__worker_idx);

        // Reset the local storage by setting all flags to false
        __bitset.__reset();

        // Populate the bitset for each of the elements in the chunk and get the occupancy
        __statpart::__count_and_cache_predicate_results(__chunk_first, __chunk_last, __pred, __bitset);

        // Obtain the exclusive prefix
        __statpart::__decoupled_lookback_partition< size_t >* __prev_partition = __nonexistent_lookback_partition - 1;
        size_t __exclusive_prefix =
            (__prev_partition->__acquire_available_status() & __statpart::__decoupled_lookback_status_prefix_available)
                ? __prev_partition->__inclusive_prefix()
                : __statpart::__calculate_inclusive_prefix_of_partition(__prev_partition, plus<>{});

        // Copy the elements depending on the bitset flags
        _ForwardIterator2 __chunk_result =
            __statpart::__copy_if_flag_set(__chunk_first, __chunk_last, __result + __exclusive_prefix, __bitset);

        // Report the position of the last iterator in the result range
        __out_iter = __chunk_result;
      };

      auto __ret =
          __statpart::__lookback_scan<size_t>(__first, __last, __setup, __scan_head, __scan_middle, __scan_tail);
      if (!__ret)
        return nullopt;
      return __out_iter;
    } else {
      return std::copy_if(std::move(__first), std::move(__last), std::move(__result), std::move(__pred));
    }
  }
};

} // namespace __pstl
_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP_STD_VER >= 17

_LIBCPP_POP_MACROS

#endif // _LIBCPP___PSTL_BACKENDS_STATPART_COPY_IF_H
