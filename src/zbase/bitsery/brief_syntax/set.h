// MIT License
//
// Copyright (c) 2017 Mindaugas Vinkelis
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

#ifndef BITSERY_BRIEF_SYNTAX_TYPE_STD_SET_H
#define BITSERY_BRIEF_SYNTAX_TYPE_STD_SET_H

#include "../ext/std_set.h"
#include <limits>
#include <set>

namespace bitsery {
template <typename S, typename Key, typename Compare, typename Allocator>
void serialize(
    S& s, std::set<Key, Compare, Allocator>& obj, size_t maxSize = std::numeric_limits<size_t>::max()) {
  s.ext(obj, ext::StdSet{ maxSize });
}

template <typename S, typename Key, typename Compare, typename Allocator>
void serialize(
    S& s, std::multiset<Key, Compare, Allocator>& obj, size_t maxSize = std::numeric_limits<size_t>::max()) {
  s.ext(obj, ext::StdSet{ maxSize });
}

} // namespace bitsery

#endif // BITSERY_BRIEF_SYNTAX_TYPE_STD_SET_H
