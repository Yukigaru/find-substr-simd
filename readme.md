# What it is
SIMD implementation of find substring algorithm. Works with help of `mpsadbw` instruction that does several comparisons within four cycles. See `sse_find_4` function.
It's not a generic function though, it works only with patterns of 4 characters.

# Performance
On AMD Ryzen 7 5800U I got following results:
```
----------------------------------------------------------------
Benchmark                      Time             CPU   Iterations
----------------------------------------------------------------
bm_naive/8                  6.52 ns         6.52 ns    109516026
bm_naive/64                 36.4 ns         36.4 ns     19245312
bm_naive/512                 238 ns          238 ns      2937559
bm_naive/131072            59085 ns        59085 ns        11807

bm_sse_find_4/8             1.58 ns         1.58 ns    440216480
bm_sse_find_4/64            5.81 ns         5.81 ns    119765141
bm_sse_find_4/512           47.1 ns         47.1 ns     14871945
bm_sse_find_4/131072        9849 ns         9849 ns        71077

bm_strstr/8                 8.16 ns         8.16 ns     79124900
bm_strstr/64                8.80 ns         8.80 ns     78513864
bm_strstr/512               11.3 ns         11.3 ns     61008402
bm_strstr/131072            1095 ns         1095 ns       637363

bm_string_find/8            5.90 ns         5.90 ns    118766824
bm_string_find/64           6.56 ns         6.56 ns    106311533
bm_string_find/512          7.69 ns         7.69 ns     89604716
bm_string_find/131072        934 ns          934 ns       741651
```
Results: SSE variant is faster than both naive and std implementation (for small strings), but due to effective algorithms, standard implementation beats my version a lot for long strings.

# How to build and run
In the repo directory:
```
git submodules update --init --recursive
cmake -Bbuild -DCMAKE_BUILD_TYPE=Release .
cmake --build build
./build/find-substr-simd
```


