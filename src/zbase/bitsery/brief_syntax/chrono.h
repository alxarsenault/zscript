// MIT License
//
// Copyright (c) 2019 Mindaugas Vinkelis
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

#ifndef BITSERY_BRIEF_SYNTAX_TYPE_STD_CHRONO_H
#define BITSERY_BRIEF_SYNTAX_TYPE_STD_CHRONO_H

#include "../ext/std_chrono.h"

namespace bitsery {
template <typename S, typename T, typename P>
void serialize(S& s, std::chrono::duration<T, P>& obj) {
  s.template ext<sizeof(T)>(obj, ext::StdDuration{});
}

template <typename S, typename C, typename T, typename P>
void serialize(S& s, std::chrono::time_point<C, std::chrono::duration<T, P>>& obj) {
  s.template ext<sizeof(T)>(obj, ext::StdTimePoint{});
}
} // namespace bitsery

#endif // BITSERY_BRIEF_SYNTAX_TYPE_STD_CHRONO_H
