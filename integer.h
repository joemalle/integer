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
#include <bitset>
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
    size = 0; // redundant, but helps cppcheck
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
      assert(nullptr != other.ptr);
      if (other.ptr[0] & 1) {
        res += integer(*this);
      }
      *this <<= integer(1);
      other >>= integer(1);
    }
    return *this = res;
  }
  
  integer& operator/=(integer&& divisor) & THROW_NEW {
    // This is a bad algorithm but division feels hard and boring
    // The link below has a lot of helpful material.  Maybe another day
    // http://bioinfo.ict.ac.cn/~dbu/AlgorithmCourses/Lectures/Lec5-Fast-Division-Hasselstrom2003.pdf
    
    // For now, binary search! O(n log(n)) ... optimal would be O(n)
    assert(integer(0) != divisor);
    if (*this < divisor) {
      return *this = integer(0);
    }
    
    integer high_q = integer(1);
    integer low_q = integer(1);
    integer prod = integer(1);
    
    do {
      low_q = high_q;
      high_q <<= integer(1);
      prod = divisor;
      prod *= integer(high_q);
    } while (prod <= *this);
    
    while (1) {
      integer copy = low_q;
      copy += integer(1);
      if (!(copy < high_q)) {
        break;
      }
      integer mid_q = high_q;
      mid_q -= integer(low_q);
      mid_q >>= integer(1);
      mid_q += integer(low_q);
      prod = mid_q;
      prod *= integer(divisor);
      if (prod < *this) {
        low_q = mid_q;
      } else if (*this < prod) {
        high_q = mid_q;
      } else {
        return *this = mid_q;
      }
    }
    
    return *this = low_q;
  }
  
  integer& operator%=(integer&& other) & THROW_NEW {
    assert(integer(0) != other);
    integer copy = *this;
    copy /= integer(other);
    copy *= integer(other);
    return *this -= integer(copy);
  }
  
  integer& operator<<=(integer&& other) & THROW_NEW {
    auto constexpr nBits = 8 * sizeof(std::uintmax_t);
    while (!(other < integer(nBits))) {
      assert(false); // for now
    }
    assert(!(integer(nBits) < other));
    auto nother = static_cast<std::uintmax_t>(other);
    std::uintmax_t carry = 0;
    for (std::uintmax_t i = 0; i < size; ++i) {
      auto tmp = ptr[i];
      ptr[i] <<= nother;
      ptr[i] += carry;
      carry = tmp >> (nBits - nother);
      /*
      std::cout << "----" << std::endl;
      std::cout << "nBits - nother: " << (nBits - nother) << std::endl;
      std::cout << "ptr[i] init: " << std::bitset<64>(tmp) << std::endl;
      std::cout << "ptr[i]:      " << std::bitset<64>(ptr[i]) << std::endl;
      std::cout << "carry:       " << std::bitset<64>(carry) << std::endl;
      */
    }
    if (0 < carry) {
      make_size_at_least(size + 1);
      ptr[size - 1] = carry;
    }
    return *this;
  }
  
  integer& operator>>=(integer&& other) & THROW_NEW {
    auto constexpr nBits = 8 * sizeof(std::uintmax_t);
    while (!(other < integer(nBits))) {
      assert(false);
      /*for (std::uintmax_t i = 0; i + 1 < size; ++i) {
        assert(nullptr != ptr);
        ptr[i] = ptr[i + 1];
      }
      ptr[size - 1] = 0;
      other -= integer(nBits);*/
    }
    assert(!(integer(nBits) < other));
    auto nother = static_cast<std::uintmax_t>(other);
    for (std::uintmax_t i = 0; i + 1 < size; ++i) {
      ptr[i] >>= nother;
      ptr[i] += (ptr[i + 1] & ((1 << nother) - 1)) << (nBits - nother);
    }
    ptr[size - 1] >>= nother;
    return *this;
  }
  
  integer& operator&=(integer&& other) & THROW_NEW {
    make_size_at_least(other.size);
    assert(nullptr != ptr);
    assert(nullptr != other.ptr);
    for (std::uintmax_t i = 0; i < other.size; ++i) {
      ptr[i] &= other.ptr[i];
    }
    return *this;
  }
  
  integer& operator|=(integer&& other) & THROW_NEW {
    make_size_at_least(other.size);
    assert(nullptr != ptr);
    assert(nullptr != other.ptr);
    for (std::uintmax_t i = 0; i < other.size; ++i) {
      ptr[i] |= other.ptr[i];
    }
    return *this;
  }
  
  integer& operator^=(integer&& other) & THROW_NEW {
    make_size_at_least(other.size);
    assert(nullptr != ptr);
    assert(nullptr != other.ptr);
    for (std::uintmax_t i = 0; i < other.size; ++i) {
      ptr[i] ^= other.ptr[i];
    }
    return *this;
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
      //std::cout << "--- --- ptr[" << i << "] = " << std::bitset<64>(ptr[i]) << std::endl;
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
ARITH_HELPER(operator&, &=, "bitwise and");
ARITH_HELPER(operator|, |=, "bitwise or");
ARITH_HELPER(operator^, ^=, "bitwise xor");

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

#undef COMP_HELPER