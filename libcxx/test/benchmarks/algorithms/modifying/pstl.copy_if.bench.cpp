//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// REQUIRES: std-at-least-c++20

// UNSUPPORTED: libcpp-has-no-incomplete-pstl

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>
#include <execution>
#include <utility>

#include <benchmark/benchmark.h>
#include "test_macros.h"
#include "../../GenerateInput.h"

int main(int argc, char** argv) {
  // A predicate that does just a bit more than a no-op
  struct minimal {
    bool operator()(double x) const { return x > std::numeric_limits<double>::max() / 2.; }
  };

  // A predicate that does miniscule work per element
  struct cheap {
    bool operator()(double x) const { return x / 2. + 10. > std::numeric_limits<double>::max() / 4.; }
  };

  // A predicate that does significant work per element
  struct expensive {
    bool operator()(double x) const {
      // (compilers aren't allowed to optimize away the FP divisions and multiplications without fast-math)
      return x / 2. * 2. / 3. * 3. / 4. * 4. / 5. * 5. > std::numeric_limits<double>::max() / 2.;
    }
  };

  auto bm = [](std::string name, auto&& policy, auto&& pred) {
    benchmark::RegisterBenchmark(
        name,
        [&policy, pred](auto& st) {
          std::size_t size = st.range(0);
          std::vector<double> c(size), out(size);
          std::generate_n(std::back_inserter(c), size, [] { return Generate<double>::random(); });
          for (auto _ : st) {
            benchmark::ClobberMemory();
            auto result = std::copy_if(policy, c.begin(), c.end(), out.begin(), pred);
            benchmark::DoNotOptimize(result);
          }
        })
        ->Arg(1 << 6)  // 64
        ->Arg(1 << 16) // 65'536
        ->Arg(1 << 20) // 1,048,576
        ->Arg(1 << 26) // 67'108'864
        ->UseRealTime();
  };
#if defined(TEST_PSTL_ENABLE_SEQ_BASELINES)
  bm("std::copy_if(std::execution::seq, vector<double>) (minimal)", std::execution::seq, minimal{});
  bm("std::copy_if(std::execution::seq, vector<double>) (cheap)", std::execution::seq, cheap{});
  bm("std::copy_if(std::execution::seq, vector<double>) (expensive)", std::execution::seq, expensive{});
#endif
  bm("std::copy_if(std::execution::par, vector<double>) (minimal)", std::execution::par, minimal{});
  bm("std::copy_if(std::execution::par, vector<double>) (cheap)", std::execution::par, cheap{});
  bm("std::copy_if(std::execution::par, vector<double>) (expensive)", std::execution::par, expensive{});

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}
