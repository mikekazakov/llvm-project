//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___PSTL_CPU_ALGOS_FOR_EACH_ITERATOR_PARTITION_H
#define _LIBCPP___PSTL_CPU_ALGOS_FOR_EACH_ITERATOR_PARTITION_H

#include <__assert>
#include <__config>
#include <__iterator/concepts.h>
#include <__pstl/backend_fwd.h>
#include <__pstl/cpu_algos/cpu_traits.h>
#include <__pstl/iterator_partitions.h>
#include <__thread/thread.h>
#include <__type_traits/is_execution_policy.h>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#if _LIBCPP_STD_VER >= 17

_LIBCPP_BEGIN_NAMESPACE_STD
namespace __pstl {

template <class _Backend, class _RawExecutionPolicy>
struct __cpu_parallel_for_each_iterator_partition {
  template <class _Policy, class _ForwardIterator, class _Function>
  _LIBCPP_HIDE_FROM_ABI optional<__empty>
  operator()(_Policy&& /*__policy*/,
             const __iterator_partitions<_ForwardIterator>& __partitions,
             _Function __func) const noexcept {
    size_t __partitions_count = __partitions.__partitions_count();
    if constexpr (__is_parallel_execution_policy_v<_RawExecutionPolicy>) {
      return __cpu_traits<_Backend>::__for_each_chunk(__partitions_count, __func);
    } else {
      for (size_t __n = 0; __n != __partitions_count; ++__n)
        __func(__n);
      return __empty{};
    }
  }

  template <class _Policy, class _ForwardIterator>
  _LIBCPP_HIDE_FROM_ABI optional<__iterator_partitions<_ForwardIterator>>
  __partition(_Policy&& /*__policy*/,
              _ForwardIterator __first,
              _ForwardIterator __last,
              size_t __min_partition_size,
              size_t __suffix_to_skip) const noexcept {
    if constexpr (__is_parallel_execution_policy_v<_RawExecutionPolicy>) {
      size_t __count = static_cast<size_t>(std::distance(__first, __last));
      if (__count < __suffix_to_skip) {
        return {};
      }
      size_t __effective_count = __count - __suffix_to_skip;
      size_t __workers_count   = std::thread::hardware_concurrency(); // TODO: ask __cpu_traits<_Backend> for it?
      size_t __max_partitions_per_worker = 8;                         // TODO: should the backend decide this?
      size_t __max_partitions            = __workers_count * __max_partitions_per_worker;
      size_t __partition_count           = std::min(__max_partitions, __effective_count / __min_partition_size);
      if (__partition_count < 2)
        return {}; // not enough input data to process in parallel => no reason to build partitions
      return __pstl::__make_iterator_partitions(__first, __effective_count, __partition_count);
    } else {
      return {}; // non-parallel execution policy => no reason to build partitions
    }
  }
};

} // namespace __pstl
_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP_STD_VER >= 17

#endif // _LIBCPP___PSTL_CPU_ALGOS_FOR_EACH_ITERATOR_PARTITION_H
