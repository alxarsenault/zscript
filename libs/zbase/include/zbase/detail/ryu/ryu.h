// Copyright 2018 Ulf Adams
//
// The contents of this file may be used under the terms of the Apache License,
// Version 2.0.
//
//    (See accompanying file LICENSE-Apache or copy at
//     http://www.apache.org/licenses/LICENSE-2.0)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.
#pragma once

#include <zbase/zbase.h>

ZBASE_BEGIN_SUB_NAMESPACE(ryu)
int d2s_buffered_n(double f, char* result, size_t sz);

int f2s_buffered_n(float f, char* result, size_t sz);

int d2fixed_buffered_n(double d, uint32_t precision, char* result, size_t sz);

int d2exp_buffered_n(double d, uint32_t precision, char* result, size_t sz);

// This is an experimental implementation of parsing strings to 64-bit floats
// using a Ryu-like algorithm. At this time, it only support up to 17 non-zero
// digits in the input, and also does not support all formats. Use at your own
// risk.

enum Status { SUCCESS, INPUT_TOO_SHORT, INPUT_TOO_LONG, MALFORMED_INPUT };

enum Status s2d_n(const char* buffer, size_t len, double* result);

enum Status s2f_n(const char* buffer, size_t len, float* result);

ZBASE_END_SUB_NAMESPACE(ryu)
