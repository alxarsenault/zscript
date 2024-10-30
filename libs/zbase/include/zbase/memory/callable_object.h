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
#include <array>
#include <memory>
#include <utility>

ZBASE_BEGIN_NAMESPACE

template <class _Fct>
class callable_object;

template <class _Ret, class... _Args>
class callable_object<_Ret(_Args...)> {
public:
  virtual ~callable_object() = default;

  virtual _Ret operator()(_Args... args) = 0;

  inline _Ret call(_Args... args) { return this->operator()(std::forward<_Args>(args)...); }

  ///
  template <class Fct, std::enable_if_t<std::is_invocable_r_v<_Ret, Fct, _Args...>, int> = 0>
  class callable : public callable_object {
  public:
    inline callable(Fct&& fct)
        : _fct(std::forward<Fct>(fct)) {}

    virtual ~callable() override = default;

    virtual _Ret operator()(_Args... args) override { return _fct(std::forward<_Args>(args)...); }

  private:
    Fct _fct;
  };

  using unique_ptr = std::unique_ptr<callable_object>;
  using shared_ptr = std::shared_ptr<callable_object>;

  ///
  template <class Fct, std::enable_if_t<std::is_invocable_r_v<_Ret, Fct, _Args...>, int> = 0>
  inline static unique_ptr create_unique(Fct&& fct) {
    return unique_ptr(new callable<Fct>(std::forward<Fct>(fct)));
  }

  ///
  template <class Fct, std::enable_if_t<std::is_invocable_r_v<_Ret, Fct, _Args...>, int> = 0>
  inline static shared_ptr create_shared(Fct&& fct) {
    return shared_ptr(new callable<Fct>(std::forward<Fct>(fct)));
  }

  template <size_t _StackSize>
  class stack_callable_object : public __zb::nocopy_nomove {
  public:
    static constexpr size_t stack_size = _StackSize;
    using stack_buffer = std::array<uint8_t, stack_size>;
    using callable_object = callable_object;

    stack_callable_object() noexcept = default;

    inline ~stack_callable_object() { reset(); }

    inline _Ret operator()(_Args... args) {
      if (_pointer) {
        return (*_pointer)(std::forward<_Args>(args)...);
      }

      if constexpr (!std::is_same_v<_Ret, void>) {
        return _Ret{};
      }
    }

    inline _Ret call(_Args... args) {
      if (_pointer) {
        return (*_pointer)(std::forward<_Args>(args)...);
      }

      if constexpr (!std::is_same_v<_Ret, void>) {
        return _Ret{};
      }
    }

    template <class _R = _Ret, std::enable_if_t<!std::is_same_v<_R, void>, int> = 0>
    inline _Ret call_or(_Args... args, const _R& r) {
      if (_pointer) {
        return (*_pointer)(std::forward<_Args>(args)...);
      }

      if constexpr (!std::is_same_v<_Ret, void>) {
        return r;
      }
    }

    template <class Fct, std::enable_if_t<std::is_invocable_r_v<_Ret, Fct, _Args...>, int> = 0>
    inline void set(Fct&& fct) {
      static_assert(sizeof(Fct) <= stack_size, "wrong size");
      reset();
      _pointer = zb_placement_new(_data.data()) callable_object::callable<Fct>(std::forward<Fct>(fct));
    }

    inline void set(std::unique_ptr<callable_object> obj) {
      reset();
      _pointer = obj.release();
    }

    inline void set(std::nullptr_t) { reset(); }

    inline void reset() {
      if (_pointer) {

        if ((const uint8_t*)_pointer == _data.data()) {
          _pointer->~callable_object();
        }
        else {
          delete _pointer;
        }
      }

      _pointer = nullptr;
    }

    inline bool is_valid() const noexcept { return (bool)_pointer; }

    inline explicit operator bool() const noexcept { return is_valid(); }

  private:
    stack_buffer _data;
    callable_object* _pointer = nullptr;
  };
};

template <class _Ret, class... _Args>
callable_object(_Ret (*)(_Args...)) -> callable_object<_Ret(_Args...)>;

using task = callable_object<void()>;

ZBASE_END_NAMESPACE
