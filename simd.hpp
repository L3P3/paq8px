#ifndef PAQ8PX_SIMD_HPP
#define PAQ8PX_SIMD_HPP

///////////////////////// SIMD Vectorization detection //////////////////////////////////

// Uncomment one or more of the following includes if you plan adding more SIMD dispatching
//#include <mmintrin.h>  //MMX
//#include <xmmintrin.h> //SSE
#include <emmintrin.h> //SSE2
//#include <pmmintrin.h> //SSE3
//#include <tmmintrin.h> //SSSE3
//#include <smmintrin.h> //SSE4.1
//#include <nmmintrin.h> //SSE4.2
//#include <ammintrin.h> //SSE4A
#include <immintrin.h> //AVX, AVX2
//#include <zmmintrin.h> //AVX512

//define CPUID
#ifdef _MSC_VER
#include <intrin.h>
#define cpuid(info, x) __cpuidex(info, x, 0)
#elif defined(__GNUC__)

#include <cpuid.h>

#define cpuid(info, x) __cpuid_count(x, 0, info[0], info[1], info[2], info[3])
#else
#error Unknown compiler
#endif

// Define interface to xgetbv instruction
static inline unsigned long long xgetbv(unsigned long ctr) {
#if (defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 160040000) || (defined(__INTEL_COMPILER) && __INTEL_COMPILER >= 1200)
  return _xgetbv(ctr);
#elif defined(__GNUC__)
  uint32_t a, d;
  __asm("xgetbv"
  : "=a"(a), "=d"(d)
  : "c"(ctr)
  :);
  return a | (((uint64_t) d) << U
  32UU);
#else
#error Unknown compiler
#endif
}

/* Returns system's highest supported SIMD instruction set as
0: None
1: MMX
2: SSE
3: SSE2
4: SSE3
5: SSSE3
6: SSE4.1
7: SSE4.2
 : SSE4A //SSE4A is not supported on Intel, so we will exclude it
8: AVX
9: AVX2
 : AVX512 //TODO
*/
int simdDetect() {
  int cpuidResult[4] = {0, 0, 0, 0};
  cpuid(cpuidResult, 0); // call cpuid function 0 ("Get vendor ID and highest basic calling parameter")
  if( cpuidResult[0] == 0 )
    return 0; //cpuid is not supported
  cpuid(cpuidResult, 1); // call cpuid function 1 ("Processor Info and Feature Bits")
  if((cpuidResult[3] & (1U << 23U)) == 0 )
    return 0; //no MMX
  if((cpuidResult[3] & (1U << 25U)) == 0 )
    return 1; //no SSE
  //SSE: OK
  if((cpuidResult[3] & (1U << 26U)) == 0 )
    return 2; //no SSE2
  //SSE2: OK
  if((cpuidResult[2] & (1U << 0U)) == 0 )
    return 3; //no SSE3
  //SSE3: OK
  if((cpuidResult[2] & (1U << 9U)) == 0 )
    return 4; //no SSSE3
  //SSSE3: OK
  if((cpuidResult[2] & (1U << 19U)) == 0 )
    return 5; //no SSE4.1
  //SSE4.1: OK
  if((cpuidResult[2] & (1U << 20U)) == 0 )
    return 6; //no SSE4.2
  //SSE4.2: OK
  if((cpuidResult[2] & (1U << 27U)) == 0 )
    return 7; //no OSXSAVE (no XGETBV)
  if((xgetbv(0) & 6) != 6 )
    return 7; //AVX is not enabled in OS
  if((cpuidResult[2] & (1U << 28U)) == 0 )
    return 7; //no AVX
  //AVX: OK
  cpuid(cpuidResult, 7); // call cpuid function 7 ("Extended Feature Bits")
  if((cpuidResult[1] & (1U << 5U)) == 0 )
    return 8; //no AVX2
  //AVX2: OK
  return 9;
}

#endif //PAQ8PX_SIMD_HPP
