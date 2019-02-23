#pragma once

#include <cstdint> // std::uint ... 
#include <string>
#include <type_traits> // is_integral_v
#include <utility> // std::move

#ifndef DNDEBUG
#include <iostream>
#endif

// If you don't want std::terminate on out of memory,
// then #define INTEGER_THROW_NEW to nothing.
#ifndef INTEGER_THROW_NEW
#define INTEGER_THROW_NEW noexcept
#endif

// By default, implicit construction is allowed
// This can break otherwise OK code, but it's convenient
// #define INTEGER_EXPLICITNESS explicit to disable conversions
#ifndef INTEGER_EXPLICITNESS
#define INTEGER_EXPLICITNESS
#endif


struct integer {
  INTEGER_EXPLICITNESS integer() noexcept;

  integer(integer&& other) noexcept;
      
  integer(integer const& other) INTEGER_THROW_NEW;
  
  template<class T> INTEGER_EXPLICITNESS integer(T const& other) INTEGER_THROW_NEW
    : integer()
  {
    if constexpr (std::is_integral_v<T>) {
      *this = other;
    } else {
      static_assert(std::is_integral_v<T>, "can only construct from an integral type");
    }
  }

  integer& operator=(integer&& other) & noexcept;

  integer& operator=(integer const& other) INTEGER_THROW_NEW;
  
  template<class T> integer& operator=(T const& other) INTEGER_THROW_NEW {
    if constexpr (std::is_integral_v<T>) {
      make_size_at_least(1);
      ptr[0] = integer_abs(other);
      is_negative = other < 0;
      return *this;
    } else {
      static_assert(std::is_integral_v<T>, "can only assign from an integral type");
    }
    return *this;
  }
  
  integer& operator+=(integer&& other) & INTEGER_THROW_NEW;
  
  integer& operator-=(integer&& other) & INTEGER_THROW_NEW;
  
  integer& operator++() & INTEGER_THROW_NEW;
  
  integer& operator--() & INTEGER_THROW_NEW;
  
  integer operator++(int) & INTEGER_THROW_NEW;
  
  integer operator--(int) & INTEGER_THROW_NEW;
  
  integer& operator*=(integer&& other) & INTEGER_THROW_NEW;
  
  integer& operator/=(integer&& divisor) & INTEGER_THROW_NEW;
  
  integer& operator%=(integer&& other) & INTEGER_THROW_NEW;
  
  integer& operator<<=(integer&& other) & INTEGER_THROW_NEW;
  
  integer& operator>>=(integer&& other) & INTEGER_THROW_NEW;
  
  integer& operator&=(integer&& other) & INTEGER_THROW_NEW;
  
  integer& operator|=(integer&& other) & INTEGER_THROW_NEW;
  
  integer& operator^=(integer&& other) & INTEGER_THROW_NEW;
  
  integer operator~() const noexcept;
  
  integer operator-() const noexcept;
  
  integer operator+() const noexcept;
  
  bool operator<(integer const& other) const noexcept;
  
  bool operator>(integer const& other) const noexcept;
  
  bool operator==(integer const& other) const noexcept;
  
  bool operator!=(integer const& other) const noexcept;
  
  bool operator<=(integer const& other) const noexcept;
  
  bool operator>=(integer const& other) const noexcept;
  
  ~integer() noexcept;
  
  explicit operator bool() const noexcept;
  
  explicit operator std::uintmax_t() const noexcept;
  
  std::string string() const noexcept;

#ifndef DNDEBUG
  void print_internals() const noexcept;
#endif

private:
  std::uintmax_t* ptr;
  std::uintmax_t size;
  bool is_negative;
  
  template <class T> static T integer_abs(T const t) noexcept {
    if constexpr(std::is_signed<T>::value)  {
      return std::abs(t);
    } else {
      return t;
    }
  }
  
  void make_size_at_least(std::uintmax_t const sz) INTEGER_THROW_NEW;
  
  std::pair<bool, bool> compare_magnitude(integer const& other) const& noexcept;
};

#define ARITH_HELPER(OPERATOR, OP, NAME) \
template<class T> integer OPERATOR([[maybe_unused]] T const& lhs, integer rhs) INTEGER_THROW_NEW { \
  if constexpr (std::is_integral_v<T>) { \
    integer n = lhs; \
    n OP std::move(rhs); \
    return integer(n); \
  } else { \
    static_assert(std::is_integral_v<T>, "can only " NAME " integeral types"); \
    return integer(rhs); \
  } \
} \
template<class T> integer OPERATOR(integer lhs, [[maybe_unused]] T const& rhs) INTEGER_THROW_NEW { \
  if constexpr (std::is_integral_v<T>) { \
    lhs OP integer(rhs); \
  } else { \
    static_assert(std::is_integral_v<T>, "can only " NAME " integeral types"); \
  } \
  return integer(lhs); \
} \
integer OPERATOR(integer rhs, integer lhs) INTEGER_THROW_NEW;

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