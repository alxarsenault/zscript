//
// MIT License
//
// Copyright (c) 2024 Alexandre Arsenault
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#pragma once

#include <zbase/zbase.h>
#include <zbase/memory/memory.h>
#include <stdint.h>
#include <string.h>

#if defined(_MSC_VER)
#include <intrin.h>
#if defined(_M_X64) && !defined(_M_ARM64EC)
#pragma intrinsic(_umul128)
#endif
#endif

/*
 *  Protection macro, alters behaviour of rapid_mum multiplication function.
 *
 *  ZBASE_HASH_FAST: Normal behavior, max speed.
 *  ZBASE_HASH_PROTECTED: Extra protection against entropy loss.
 */
#ifndef ZBASE_HASH_PROTECTED
#define ZBASE_HASH_FAST
#elif defined(ZBASE_HASH_FAST)
#error "cannot define ZBASE_HASH_PROTECTED and ZBASE_HASH_FAST simultaneously."
#endif

ZBASE_BEGIN_NAMESPACE

/// Default seed.
#define ZBASE_HASH_DEFAULT_SEED (0xbdd89aa982704029ull)

namespace hash_detail {
/// Default secret parameters.
inline constexpr uint64_t k_default_hash_secret[3]
    = { 0x2d358dccaa6c78a5ull, 0x8bb84b93962eacc9ull, 0x4b33a62ed433d4a3ull };

///
///  64*64 -> 128bit multiply function.
///
///  @param A  Address of 64-bit number.
///  @param B  Address of 64-bit number.
///
///  Calculates 128-bit C = *A * *B.
///
///  When ZBASE_HASH_FAST is defined:
///  Overwrites A contents with C's low 64 bits.
///  Overwrites B contents with C's high 64 bits.
///
///  When ZBASE_HASH_PROTECTED is defined:
///  Xors and overwrites A contents with C's low 64 bits.
///  Xors and overwrites B contents with C's high 64 bits.
///
#if defined(__SIZEOF_INT128__)
inline void rapid_mum(uint64_t* A, uint64_t* B) noexcept {
  __uint128_t r = *A;
  r *= *B;

#ifdef ZBASE_HASH_PROTECTED
  *A ^= (uint64_t)r;
  *B ^= (uint64_t)(r >> 64);
#else
  *A = (uint64_t)r;
  *B = (uint64_t)(r >> 64);
#endif // ZBASE_HASH_PROTECTED.
}

#elif defined(_MSC_VER) && (defined(_WIN64) || defined(_M_HYBRID_CHPE_ARM64)) && defined(_M_X64)
inline void rapid_mum(uint64_t* A, uint64_t* B) noexcept {
#ifdef ZBASE_HASH_PROTECTED
  uint64_t a, b;
  a = _umul128(*A, *B, &b);
  *A ^= a;
  *B ^= b;
#else
  *A = _umul128(*A, *B, B);
#endif // ZBASE_HASH_PROTECTED
}

#elif defined(_MSC_VER) && (defined(_WIN64) || defined(_M_HYBRID_CHPE_ARM64)) && !defined(_M_X64)
inline void rapid_mum(uint64_t* A, uint64_t* B) noexcept {
#ifdef ZBASE_HASH_PROTECTED
  uint64_t a, b;
  b = __umulh(*A, *B);
  a = *A * *B;
  *A ^= a;
  *B ^= b;
#else
  uint64_t c = __umulh(*A, *B);
  *A = *A * *B;
  *B = c;
#endif // ZBASE_HASH_PROTECTED
}

#else
inline void rapid_mum(uint64_t* A, uint64_t* B) noexcept {
  uint64_t ha = *A >> 32, hb = *B >> 32, la = (uint32_t)*A, lb = (uint32_t)*B, hi, lo;
  uint64_t rh = ha * hb, rm0 = ha * lb, rm1 = hb * la, rl = la * lb, t = rl + (rm0 << 32), c = t < rl;
  lo = t + (rm1 << 32);
  c += lo < t;
  hi = rh + (rm0 >> 32) + (rm1 >> 32) + c;
#ifdef ZBASE_HASH_PROTECTED
  *A ^= lo;
  *B ^= hi;
#else
  *A = lo;
  *B = hi;
#endif // ZBASE_HASH_PROTECTED
}
#endif //

///
///  Multiply and xor mix function.
///
///  @param A  64-bit number.
///  @param B  64-bit number.
///
///  Calculates 128-bit C = A * B.
///  Returns 64-bit xor between high and low 64 bits of C.
///
inline uint64_t rapid_mix(uint64_t A, uint64_t B) noexcept {
  rapid_mum(&A, &B);
  return A ^ B;
}

/// Read functions.

inline uint64_t rapid_read64(const uint8_t* p) noexcept {
  uint64_t v;
  memcpy(&v, p, sizeof(uint64_t));
  return v;
}

inline uint64_t rapid_read32(const uint8_t* p) noexcept {
  uint32_t v;
  memcpy(&v, p, sizeof(uint32_t));
  return v;
}

///  Reads and combines 3 bytes of input.
///
///  @param p  Buffer to read from.
///  @param k  Length of @p, in bytes.
///
///  Always reads and combines 3 bytes from memory.
///  Guarantees to read each buffer position at least once.
///
///  Returns a 64-bit value containing all three bytes read.
inline uint64_t rapid_readSmall(const uint8_t* p, size_t k) noexcept {
  return (((uint64_t)p[0]) << 56) | (((uint64_t)p[k >> 1]) << 32) | p[k - 1];
}

///  rapidhash main function.
///
///  @param key     Buffer to be hashed.
///  @param len     @key length, in bytes.
///  @param seed    64-bit seed used to alter the hash result predictably.
///  @param secret  Triplet of 64-bit secrets used to alter hash result predictably.
///
///  Returns a 64-bit hash.
inline uint64_t hash_internal(const void* key, size_t len, uint64_t seed, const uint64_t* secret) noexcept {
  const uint8_t* p = (const uint8_t*)key;
  seed ^= rapid_mix(seed ^ secret[0], secret[1]) ^ len;
  uint64_t a, b;
  if (ZBASE_LIKELY(len <= 16)) {
    if (ZBASE_LIKELY(len >= 4)) {
      const uint8_t* plast = p + len - 4;
      a = (rapid_read32(p) << 32) | rapid_read32(plast);
      const uint64_t delta = ((len & 24) >> (len >> 3));
      b = ((rapid_read32(p + delta) << 32) | rapid_read32(plast - delta));
    }
    else if (ZBASE_LIKELY(len > 0)) {
      a = rapid_readSmall(p, len);
      b = 0;
    }
    else {
      a = b = 0;
    }
  }
  else {
    size_t i = len;
    if (ZBASE_UNLIKELY(i > 48)) {
      uint64_t see1 = seed, see2 = seed;

      while (ZBASE_LIKELY(i >= 96)) {
        seed = rapid_mix(rapid_read64(p) ^ secret[0], rapid_read64(p + 8) ^ seed);
        see1 = rapid_mix(rapid_read64(p + 16) ^ secret[1], rapid_read64(p + 24) ^ see1);
        see2 = rapid_mix(rapid_read64(p + 32) ^ secret[2], rapid_read64(p + 40) ^ see2);
        seed = rapid_mix(rapid_read64(p + 48) ^ secret[0], rapid_read64(p + 56) ^ seed);
        see1 = rapid_mix(rapid_read64(p + 64) ^ secret[1], rapid_read64(p + 72) ^ see1);
        see2 = rapid_mix(rapid_read64(p + 80) ^ secret[2], rapid_read64(p + 88) ^ see2);
        p += 96;
        i -= 96;
      }
      if (ZBASE_UNLIKELY(i >= 48)) {
        seed = rapid_mix(rapid_read64(p) ^ secret[0], rapid_read64(p + 8) ^ seed);
        see1 = rapid_mix(rapid_read64(p + 16) ^ secret[1], rapid_read64(p + 24) ^ see1);
        see2 = rapid_mix(rapid_read64(p + 32) ^ secret[2], rapid_read64(p + 40) ^ see2);
        p += 48;
        i -= 48;
      }

      seed ^= see1 ^ see2;
    }
    if (i > 16) {
      seed = rapid_mix(rapid_read64(p) ^ secret[2], rapid_read64(p + 8) ^ seed ^ secret[1]);
      if (i > 32) {
        seed = rapid_mix(rapid_read64(p + 16) ^ secret[2], rapid_read64(p + 24) ^ seed);
      }
    }
    a = rapid_read64(p + i - 16);
    b = rapid_read64(p + i - 8);
  }

  a ^= secret[1];
  b ^= seed;
  rapid_mum(&a, &b);
  return rapid_mix(a ^ secret[0] ^ len, b ^ secret[1]);
}

} // namespace hash_detail.

///  rapidhash default seeded hash function.
///
///  @param key     Buffer to be hashed.
///  @param len     @key length, in bytes.
///  @param seed    64-bit seed used to alter the hash result predictably.
///
///  Calls hash_internal using provided parameters and default secrets.
///
///  Returns a 64-bit hash.
inline uint64_t rapid_hash(const void* key, size_t len, uint64_t seed) noexcept {
  return hash_detail::hash_internal(key, len, seed, hash_detail::k_default_hash_secret);
}

///  rapidhash default hash function.
///
///  @param key     Buffer to be hashed.
///  @param len     @key length, in bytes.
///
///  Calls hash using provided parameters and the default seed.
///
///  Returns a 64-bit hash.
inline uint64_t rapid_hash(const void* key, size_t len) noexcept {
  return rapid_hash(key, len, ZBASE_HASH_DEFAULT_SEED);
}

template <class T>
ZB_CHECK ZB_INLINE uint64_t rapid_hash(const T& t) noexcept {
  return __zb::rapid_hash((const void*)&t, sizeof(T));
}

// TODO: Add more string types.
ZB_CHECK ZB_INLINE uint64_t rapid_hash(std::string_view t) noexcept {
  return __zb::rapid_hash((const void*)t.data(), t.size());
}

template <class T>
struct rapid_hasher {
  inline size_t operator()(const T& v) noexcept { return __zb::rapid_hash(v); }
};

namespace hash_detail {
// This is free and unencumbered software released into the public domain under The Unlicense
// (http://unlicense.org/) main repo: https://github.com/wangyi-fudan/wyhash author: 王一 Wang Yi
// <godspeed_china@yeah.net> contributors: Reini Urban, Dietrich Epp, Joshua Haberman, Tommy Ettinger, Daniel
// Lemire, Otmar Ertl, cocowalla, leo-yuriev, Diego Barrios Romero, paulie-g, dumblob, Yann Collet, ivte-ms,
// hyb, James Z.M. Gao, easyaspi314 (Devin), TheOneric

// The wyrand PRNG that pass BigCrush and PractRand
static inline uint64_t wyrand(uint64_t* seed) {
  *seed += 0x2d358dccaa6c78a5ull;
  return rapid_mix(*seed, *seed ^ 0x8bb84b93962eacc9ull);
}

// modified from https://github.com/going-digital/Prime64
static inline unsigned long long mul_mod(unsigned long long a, unsigned long long b, unsigned long long m) {
  unsigned long long r = 0;
  while (b) {
    if (b & 1) {
      unsigned long long r2 = r + a;
      if (r2 < r)
        r2 -= m;
      r = r2 % m;
    }
    b >>= 1;
    if (b) {
      unsigned long long a2 = a + a;
      if (a2 < a)
        a2 -= m;
      a = a2 % m;
    }
  }
  return r;
}

static inline unsigned long long pow_mod(unsigned long long a, unsigned long long b, unsigned long long m) {
  unsigned long long r = 1;
  while (b) {
    if (b & 1)
      r = mul_mod(r, a, m);
    b >>= 1;
    if (b)
      a = mul_mod(a, a, m);
  }
  return r;
}

inline unsigned sprp(unsigned long long n, unsigned long long a) noexcept {
  unsigned long long d = n - 1;
  unsigned char s = 0;
  while (!(d & 0xff)) {
    d >>= 8;
    s += 8;
  }
  if (!(d & 0xf)) {
    d >>= 4;
    s += 4;
  }
  if (!(d & 0x3)) {
    d >>= 2;
    s += 2;
  }
  if (!(d & 0x1)) {
    d >>= 1;
    s += 1;
  }
  unsigned long long b = pow_mod(a, d, n);
  if ((b == 1) || (b == (n - 1)))
    return 1;
  unsigned char r;
  for (r = 1; r < s; r++) {
    b = mul_mod(b, b, n);
    if (b <= 1)
      return 0;
    if (b == (n - 1))
      return 1;
  }
  return 0;
}

inline unsigned is_prime(unsigned long long n) noexcept {
  if (n < 2 || !(n & 1))
    return 0;
  if (n < 4)
    return 1;
  if (!sprp(n, 2))
    return 0;
  if (n < 2047)
    return 1;
  if (!sprp(n, 3))
    return 0;
  if (!sprp(n, 5))
    return 0;
  if (!sprp(n, 7))
    return 0;
  if (!sprp(n, 11))
    return 0;
  if (!sprp(n, 13))
    return 0;
  if (!sprp(n, 17))
    return 0;
  if (!sprp(n, 19))
    return 0;
  if (!sprp(n, 23))
    return 0;
  if (!sprp(n, 29))
    return 0;
  if (!sprp(n, 31))
    return 0;
  if (!sprp(n, 37))
    return 0;
  return 1;
}

// make your own secret
static inline void make_secret(uint64_t seed, uint64_t* secret) {
  uint8_t c[] = { 15, 23, 27, 29, 30, 39, 43, 45, 46, 51, 53, 54, 57, 58, 60, 71, 75, 77, 78, 83, 85, 86, 89,
    90, 92, 99, 101, 102, 105, 106, 108, 113, 114, 116, 120, 135, 139, 141, 142, 147, 149, 150, 153, 154, 156,
    163, 165, 166, 169, 170, 172, 177, 178, 180, 184, 195, 197, 198, 201, 202, 204, 209, 210, 212, 216, 225,
    226, 228, 232, 240 };

  for (size_t i = 0; i < 4; i++) {
    uint8_t ok;
    do {
      ok = 1;
      secret[i] = 0;

      for (size_t j = 0; j < 64; j += 8) {
        secret[i] |= ((uint64_t)c[wyrand(&seed) % sizeof(c)]) << j;
      }

      if (secret[i] % 2 == 0) {
        ok = 0;
        continue;
      }

      for (size_t j = 0; j < i; j++) {
#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
        if (__builtin_popcountll(secret[j] ^ secret[i]) != 32) {
          ok = 0;
          break;
        }
#elif defined(_MSC_VER) && defined(_WIN64)
#if defined(_M_X64)
        if (_mm_popcnt_u64(secret[j] ^ secret[i]) != 32) {
          ok = 0;
          break;
        }
#else
        if (_CountOneBits64(secret[j] ^ secret[i]) != 32) {
          ok = 0;
          break;
        }
#endif
#else
        // manual popcount
        uint64_t x = secret[j] ^ secret[i];
        x -= (x >> 1) & 0x5555555555555555;
        x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
        x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;
        x = (x * 0x0101010101010101) >> 56;
        if (x != 32) {
          ok = 0;
          break;
        }
#endif
      }

      if (ok && !is_prime(secret[i])) {
        ok = 0;
      }

    } while (!ok);
  }
}

} // namespace hash_detail.

/* The Unlicense
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

ZBASE_END_NAMESPACE

/*
 * rapidhash - Very fast, high quality, platform-independent hashing algorithm.
 * Copyright (C) 2024 Nicolas De Carli
 *
 * Based on 'wyhash', by Wang Yi <godspeed_china@yeah.net>
 *
 * BSD 2-Clause License (https://www.opensource.org/licenses/bsd-license.php)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You can contact the author at:
 *   - rapidhash source repository: https://github.com/Nicoshev/rapidhash
 */
