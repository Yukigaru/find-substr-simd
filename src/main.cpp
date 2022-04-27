#include <immintrin.h>
#include <functional>
#include <cstring>
#include <gtest/gtest.h>
#include <benchmark/benchmark.h>

using compare_func_t = std::function<int (const char *, const char *)>;
std::string pattern = "xyzb";
std::vector<char> rand_chars;


int naive_find(const char *str, const char *sub) {
  auto *ptr = str;
  auto *psub = sub;
  const char *start = nullptr;
  while (*ptr != '\0') {
    if (*ptr == *psub) {
      if (!start) {
        start = ptr;
      }
      psub++;
      if (*psub == '\0') {
        return (int)(start - str);
      }
    } else {
      if (start) {
        ptr = start;
        start = nullptr;
        psub = sub;
      }
    }
    ptr++;
  }
  return -1;
}

int sse_find_4(const char *str, size_t len, const char *pattern) {
  __m128i cmp = _mm_set1_epi32(0);
  memcpy((void *)&cmp, (const void *)pattern, 4);

  // align `len` to 4 bytes
  // it works, because in a correct JSON the last four symbols definitely won't be what we're looking for
  // and it's done, because we're too lazy to handle the last iterations
  len &= ~0b11;

  const char *ptr = str;
  const char *end = str + len;

  auto zero = _mm_set1_epi8(0);

  while (ptr < end) {
    __m128i a;
    memcpy((void *)&a, (const void *)ptr, 16);

    // for 8 bytes of string and 4-byte pattern, calculate sum of absolute distances,
    // trying a 4-char pattern 8 times against the string
    // "xABCDxxx" vs "ABCD" => [9 0 8 7 9 6 5 8], each number is 16-bit in mm128 register
    auto tmp = _mm_mpsadbw_epu8(a, cmp, 0 /*offset for cmp*/);

    // filter zero distances: [9 0 8 7 9 6 5 8] => [0 1 0 0 0 0 0 0], each number is 16-bit in mm128
    auto tmp2 = _mm_cmpeq_epi16(tmp, zero);

    // [0 1 0 0 0 0 0 0] => an int with the corresponding bits
    uint32_t msk = _mm_movemask_epi8(tmp2);
    if (msk != 0) {
      // find an index of the most significant bit (that's the pattern position)
      auto idx = __builtin_ctz(msk);
      return (ptr - str) + idx / 2; // divide by two, because _mm_cmpeq_epi16 gives us 16-bit numbers
    }

    ptr += 8;
  }
  return -1;
}

/**********************************************/
void unit_test_find_4(compare_func_t func) {
  EXPECT_EQ(func("ooboo___", "oobo"), 0);
  EXPECT_EQ(func("oobooo__", "oobo"), 0);
  EXPECT_EQ(func("ooboobo_", "oobo"), 0);
  EXPECT_EQ(func("_oobo___", "oobo"), 1);
  EXPECT_EQ(func("ooobo___", "oobo"), 1);
  EXPECT_EQ(func("__oobo__", "oobo"), 2);
  EXPECT_EQ(func("_ooobo__", "oobo"), 2);
  EXPECT_EQ(func("___oobo_", "oobo"), 3);
  EXPECT_EQ(func("_oooobo_", "oobo"), 3);
  EXPECT_EQ(func("____oobo", "oobo"), 4);
  EXPECT_EQ(func("__oooobo", "oobo"), 4);
  EXPECT_EQ(func("__boocoocb_oobo_____", "oobo"), 11);
  EXPECT_EQ(func("__boocoocb__oobo____", "oobo"), 12);
  EXPECT_EQ(func("__boocoocb__ooobo___", "oobo"), 13);
  EXPECT_EQ(func("__boocoocb__oooobo__", "oobo"), 14);
}

/**********************************************/
std::string gen_benchmark_text(size_t len) {
  std::string text;
  std::generate_n(std::back_inserter(text), len, [i = 0]() mutable { return rand_chars[++i % rand_chars.size()]; });
  text += pattern + "____";
  return text;
}

/**********************************************/
void bm_naive(benchmark::State &state) {
  auto text = gen_benchmark_text(state.range(0));

  for (auto _: state) {
    benchmark::DoNotOptimize(naive_find(text.data(), pattern.data()));
  }
}
BENCHMARK(bm_naive)->Range(8, 2 << 16);

/**********************************************/
void bm_sse_find_4(benchmark::State &state) {
  auto text = gen_benchmark_text(state.range(0));

  for (auto _: state) {
    benchmark::DoNotOptimize(sse_find_4(text.data(), text.size(), pattern.c_str()));
  }
}
BENCHMARK(bm_sse_find_4)->Range(8, 2 << 16);

/**********************************************/
void bm_strstr(benchmark::State &state) {
  auto text = gen_benchmark_text(state.range(0));

  for (auto _: state) {
    benchmark::DoNotOptimize(std::strstr(text.data(), pattern.data()));
  }
}
BENCHMARK(bm_strstr)->Range(8, 2 << 16);

/**********************************************/
void bm_string_find(benchmark::State &state) {
  auto text = gen_benchmark_text(state.range(0));

  for (auto _: state) {
    benchmark::DoNotOptimize(text.find(pattern));
  }
}
BENCHMARK(bm_string_find)->Range(8, 2 << 16);

/**********************************************/
int main(int argc, char **argv) {
  // needed for benchmark data
  std::generate_n(std::back_inserter(rand_chars), 100, []() { return 'A' + rand() % 55; });

  // run unit tests for my implementation (on success, nothing is printed)
  unit_test_find_4([](const char *text, const char *pattern) { return sse_find_4((const char *)text, strlen(text), (const char *)pattern); });

  // then run benchmarks
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}
