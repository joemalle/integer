#pragma once

#include <algorithm> // std::copy_n
#include <cassert> // assert
#include <cstdint> // std::uint ... 
#include <cstring> // memset
#include <string>
#include <type_traits> // is_integral_v
#include <utility> // std::move

#ifndef DNDEBUG
#include <iostream>
#endif

#define THROW_NEW noexcept

struct integer {
  integer() noexcept
    : ptr(nullptr)
    , size(0)
    , is_negative(0)
  {}

  integer(integer&& other) noexcept
    : ptr(std::move(other.ptr))
    , size(std::move(other.size))
    , is_negative(std::move(other.is_negative))
  {
    other.ptr = nullptr;
    other.size = 0;
  }
      
  integer(integer const& other) THROW_NEW
    : integer()
  {
    *this = other;
  }
  
  template<class T> integer(T const& other) THROW_NEW
    : integer()
  {
    if constexpr (std::is_integral_v<T>) {
      *this = other;
    } else {
      static_assert(std::is_integral_v<T>, "can only construct from an integral type");
    }
  }

  integer& operator=(integer&& other) & noexcept {
    std::swap(ptr, other.ptr);
    std::swap(size, other.size);
    std::swap(is_negative, other.is_negative);
    return *this;
  }

  integer& operator=(integer const& other) THROW_NEW {
    auto const pother = other.ptr;
    auto const sz = other.size;
    make_size_at_least(sz);
    std::copy_n(pother, sz, ptr);
    is_negative = other.is_negative;
    return *this;
  }

  template<class T> integer& operator=(T const& other) THROW_NEW {
    if constexpr (std::is_integral_v<T>) {
      static_assert(sizeof(T) <= sizeof(std::uintmax_t), "assume std:uintmax_t can hold other");
      
      make_size_at_least(1);
      ptr[0] = abs(other);
      is_negative = other < 0;
      
    } else {
      static_assert(std::is_integral_v<T>, "can only assign from an integral type");
    }
    return *this;
  }
  
  integer& operator+=(integer&& other) & THROW_NEW {
    auto const add_ignore_sign = [](integer& lhs, integer& rhs) THROW_NEW {
      bool const lhs_is_larger = rhs.size < lhs.size;
      auto& larger = lhs_is_larger ? lhs : rhs;
      auto& smaller = !lhs_is_larger ? lhs : rhs;
      
      std::uintmax_t carry = 0;
      for (std::uintmax_t i = 0; i < larger.size; ++i) {
        larger.ptr[i] = __builtin_addcl(
          larger.ptr[i],
          i < smaller.size ? smaller.ptr[i] : 0,
          carry,
          &carry
        );
        if (0 == carry && !(i < smaller.size)) {
          return larger;
        }
      }
      
      if (0 < carry) {
        larger.make_size_at_least(larger.size + 1);
        larger.ptr[larger.size - 1] = carry;
      }
      return larger;
    };
    
    auto const subtract_ignore_sign = [&add_ignore_sign](integer& lhs, integer& rhs) THROW_NEW {
      std::uintmax_t carry = 0;
      for (std::uintmax_t i = 0; i < std::max(lhs.size, rhs.size); ++i) {
        if (!(i < lhs.size)) {
          lhs.make_size_at_least(i + 1);
          lhs.ptr[i] = 0;
        }
        lhs.ptr[i] = __builtin_subcl(
          lhs.ptr[i],
          i < rhs.size ? rhs.ptr[i] : std::uintmax_t{0},
          carry,
          &carry
        );
        if (0 == carry && !(i < rhs.size)) {
          return lhs;
        }
      }
      
      if (0 < carry) {
        lhs = ~lhs;
        integer ci = carry;
        lhs = add_ignore_sign(lhs, ci);
        lhs.is_negative = true;
      }
      
      return lhs;
    };
    
    if (!is_negative && !other.is_negative) {
      return *this = add_ignore_sign(*this, other);
    } else if (is_negative && other.is_negative) {
      return *this = -add_ignore_sign(*this, other);
    } else if (!is_negative && other.is_negative) {
      return *this = subtract_ignore_sign(*this, other);
    } else {
      assert(is_negative && !other.is_negative);
      return *this = subtract_ignore_sign(other, *this);
    }
  }
  
  integer& operator-=(integer&& other) & THROW_NEW {
    return *this += -other;
  }
  
  integer& operator++() & THROW_NEW {
    return *this += integer(1); 
  }
  
  integer& operator--() & THROW_NEW {
    return *this -= integer(1);
  }
  
  integer operator++(int) & THROW_NEW {
    auto tmp = *this;
    operator++();
    return tmp;
  }
  
  integer operator--(int) & THROW_NEW {
    auto tmp = *this;
    operator--();
    return tmp;
  }
  
  integer& operator*=(integer&& other) & THROW_NEW {
    integer res = 0;
    while (other) {
      res += integer(*this);
      --other;
    }
    return *this = res;
  }
  
  integer& operator/=(integer&& other) & THROW_NEW {
    assert(integer(0) != other);
    integer res = 0;
    integer copy = *this;
    while (1) {
      integer copy2 = copy;
      copy2 -= integer(other);
      if (copy2 < integer(0)) {
        return *this = res;
      }
      ++res;
      copy = copy2;
    }
  }
  
  integer& operator<<=(integer&& other) & THROW_NEW {
    integer res = *this;
    while (other) {
      res *= 2;
      --other;
    }
    return *this = res;
  }
  
  integer& operator>>=(integer&& other) & THROW_NEW {
    integer res = *this;
    while (other) {
      res /= 2;
      --other;
    }
    return *this = res;
  }
  
  integer& operator%=(integer&& other) & THROW_NEW {
    assert(integer(0) != other);
    integer copy = *this;
    while (1) {
      integer next = copy;
      next -= integer(other);
      if (next < integer(0)) {
        return *this = copy;
      } 
      copy = next;
    }
  }
  
  integer operator~() const noexcept {
    auto copy = *this;
    for (std::uintmax_t i = 0; i < copy.size; ++i) {
      copy.ptr[i] = ~copy.ptr[i];
    }
    return copy;
  }
  
  integer operator-() const noexcept {
    auto copy = *this;
    copy.is_negative = !copy.is_negative;
    return copy;
  }
  
  integer operator+() const noexcept {
    return integer(*this);
  }
  
  bool operator<(integer const& other) const noexcept {
    if (is_negative && !other.is_negative) {
      return true;
    } else if (!is_negative && other.is_negative) {
      return false;
    }
    
    auto const [this_is_smaller, this_is_bigger] = compare_magnitude(other);
    
    if (!is_negative && !other.is_negative) {
      return this_is_smaller;
    } else {
      assert(is_negative && other.is_negative);
      return this_is_bigger;
    }
  }
  
  bool operator>(integer const& other) const noexcept {
    if (is_negative && !other.is_negative) {
      return false;
    } else if (!is_negative && other.is_negative) {
      return true;
    }
    
    auto const [this_is_smaller, this_is_bigger] = compare_magnitude(other);
    
    if (!is_negative && !other.is_negative) {
      return this_is_bigger;
    } else {
      assert(is_negative && other.is_negative);
      return this_is_smaller;
    }
  }
  
  bool operator==(integer const& other) const noexcept {
    return !(*this < other) && !(*this > other);
  }
  
  bool operator!=(integer const& other) const noexcept {
    return !(*this == other);
  }
  
  bool operator<=(integer const& other) const noexcept {
    return *this < other || *this == other;
  }
  
  bool operator>=(integer const& other) const noexcept {
    return *this > other || *this == other;
  }
  
  ~integer() noexcept {
    assert(!(nullptr == ptr) || 0 == size);
    delete [] ptr;
  }
  
  explicit operator bool() const noexcept {
    return !(integer(0) == *this);
  }
  
  explicit operator std::uintmax_t() const noexcept {
    assert(nullptr != ptr);
    return ptr[0];
  }
  
  std::string string() const noexcept {
    auto copy = *this;
    std::string res;
    while (integer(0) < copy) {
      integer copy2 = copy;
      copy2 %= integer(10);
      auto digit = static_cast<std::uintmax_t>(copy2);
      assert(0 <= digit && digit < 10);
      res += std::to_string(digit);
      copy /= integer(10);
    }
    std::reverse(res.begin(), res.end());
    return res;
  }

#ifndef DNDEBUG
  void print_internals() const noexcept {
    std::cout << 
    "--- printing ---" << std::endl <<
    "--- ptr:   " << ptr << std::endl <<
    "--- size:  " << size << std::endl <<
    "--- neg:   " << is_negative << std::endl;
    for (std::uintmax_t i = 0; i < size; ++i) {
      std::cout << "--- --- ptr[" << i << "] = " << ptr[i] << std::endl;
    }
    std::cout << "----------------" << std::endl;
  }
#endif

private:
  std::uintmax_t* ptr;
  std::uintmax_t size;
  bool is_negative;
  
  void make_size_at_least(std::uintmax_t const sz) THROW_NEW {
    assert(!(nullptr == ptr) || 0 == size);
    if (size < sz) {
      auto tmp = new std::uintmax_t[sz];
      std::copy_n(ptr, size, tmp);
      delete [] ptr;
      ptr = tmp;
      size = sz;
    } else if (sz < size) {
      std::memset(ptr + sz, 0, size - sz);
    }
  }
  
  template <class T> static T abs(T const t) noexcept {
    if constexpr(std::is_signed<T>::value)  {
      return std::abs(t);
    } else {
      return t;
    }
  }
  
  std::pair<bool, bool> compare_magnitude(integer const& other) const& noexcept {
    for (auto i = std::max(size, other.size); ; --i) {
      auto this_now = i < size ? ptr[i] : 0;
      auto other_now = i < other.size ? other.ptr[i] : 0;
      
      if (this_now < other_now) {
        return {true, false};
      }
      
      if (other_now < this_now) {
        return {false, true};
      }
      
      if (0 == i) {
        return {false, false};
      }
    }
  }
};


#define ARITH_HELPER(OPERATOR, OP, NAME) \
template<class T> integer OPERATOR([[maybe_unused]] T const& lhs, integer rhs) THROW_NEW { \
  if constexpr (std::is_integral_v<T>) { \
    integer n = lhs; \
    n OP std::move(rhs); \
    return n; \
  } else { \
    static_assert(std::is_integral_v<T>, "can only " NAME " integeral types"); \
    return rhs; \
  } \
} \
template<class T> integer OPERATOR(integer lhs, [[maybe_unused]] T const& rhs) THROW_NEW { \
  if constexpr (std::is_integral_v<T>) { \
    lhs OP integer(rhs); \
  } else { \
    static_assert(std::is_integral_v<T>, "can only " NAME " integeral types"); \
  } \
  return lhs; \
} \
integer OPERATOR(integer rhs, integer lhs) THROW_NEW { \
  return rhs OP std::move(lhs); \
}

ARITH_HELPER(operator+, +=, "add");
ARITH_HELPER(operator-, -=, "subtract");
ARITH_HELPER(operator*, *=, "multiply");
ARITH_HELPER(operator/, /=, "divide");
ARITH_HELPER(operator<<, <<=, "shift-left");
ARITH_HELPER(operator>>, >>=, "shift-right");
ARITH_HELPER(operator%, %=, "calculate modulus with");

#undef ARITH_HELPER

#define COMP_HELPER(OPERATOR, OP) \
template <class T> bool OPERATOR([[maybe_unused]] T const& lhs, integer const& rhs) noexcept { \
  if constexpr (std::is_integral_v<T>) { \
    return integer(lhs) OP rhs; \
  } else { \
    static_assert(std::is_integral_v<T>, "can only compare integeral types"); \
    return true; \
  } \
} \
template <class T> bool OPERATOR(integer const& lhs, [[maybe_unused]] T const& rhs) noexcept { \
  if constexpr (std::is_integral_v<T>) { \
    return lhs OP integer(rhs); \
  } else { \
    static_assert(std::is_integral_v<T>, "can only compare integeral types"); \
    return true; \
  } \
}

COMP_HELPER(operator<, <);
COMP_HELPER(operator>, >);
COMP_HELPER(operator<=, <=);
COMP_HELPER(operator>=, >=);
COMP_HELPER(operator==, ==);
COMP_HELPER(operator!=, !=);