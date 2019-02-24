#include "integer.h"

#include <algorithm> // std::copy_n
#include <cassert> // assert
#include <cstdint> // std::uint ... 
#include <cstring> // memset
#include <malloc/malloc.h>
#include <string>
#include <type_traits> // is_integral_v
#include <utility> // std::move

#ifndef DNDEBUG
#include <iostream>
#include <bitset>
#endif

/*INTEGER_EXPLICITNESS*/ integer::integer() noexcept {}

integer::integer(integer&& other) noexcept
  : ptr(std::move(other.ptr))
{
  other.ptr.set(nullptr);
}
    
integer::integer(integer const& other) INTEGER_THROW_NEW {
  *this = other;
}

integer& integer::operator=(integer&& other) & noexcept {
  std::swap(ptr, other.ptr);
  return *this;
}

integer& integer::operator=(integer const& other) INTEGER_THROW_NEW {
  auto const pother = other.ptr.get();
  auto const sz = other.size();
  make_size_at_least(sz);
  std::copy_n(pother, sz, ptr.get());
  make_negative(other.is_negative());
  return *this;
}

integer& integer::operator+=(integer&& other) & INTEGER_THROW_NEW {
  auto const add_ignore_sign = [](integer& lhs, integer& rhs) INTEGER_THROW_NEW {
    bool const lhs_is_larger = rhs.size() < lhs.size();
    auto& larger = lhs_is_larger ? lhs : rhs;
    auto& smaller = !lhs_is_larger ? lhs : rhs;
    
    std::uintmax_t carry = 0;
    for (std::uintmax_t i = 0; i < larger.size(); ++i) {
      larger.ptr.get()[i] = __builtin_addcl(
        larger.ptr.get()[i],
        i < smaller.size() ? smaller.ptr.get()[i] : 0,
        carry,
        &carry
      );
      if (0 == carry && !(i < smaller.size())) {
        return larger;
      }
    }
    
    if (0 < carry) {
      larger.make_size_at_least(larger.size() + 1);
      larger.ptr.get()[larger.size() - 1] = carry;
    }
    return larger;
  };
  
  auto const subtract_ignore_sign = [&add_ignore_sign](integer& lhs, integer& rhs) INTEGER_THROW_NEW {
    std::uintmax_t carry = 0;
    for (std::uintmax_t i = 0; i < std::max(lhs.size(), rhs.size()); ++i) {
      if (!(i < lhs.size())) {
        lhs.make_size_at_least(i + 1);
        lhs.ptr.get()[i] = 0;
      }
      lhs.ptr.get()[i] = __builtin_subcl(
        lhs.ptr.get()[i],
        i < rhs.size() ? rhs.ptr.get()[i] : std::uintmax_t{0},
        carry,
        &carry
      );
      if (0 == carry && !(i < rhs.size())) {
        return lhs;
      }
    }
    
    if (0 < carry) {
      lhs = ~lhs;
      integer ci{carry};
      lhs = add_ignore_sign(lhs, ci);
      lhs.make_negative(true);
    }
    
    return lhs;
  };
  
  if (!is_negative() && !other.is_negative()) {
    return *this = add_ignore_sign(*this, other);
  } else if (is_negative() && other.is_negative()) {
    return *this = -add_ignore_sign(*this, other);
  } else if (!is_negative() && other.is_negative()) {
    return *this = subtract_ignore_sign(*this, other);
  } else {
    assert(is_negative() && !other.is_negative());
    return *this = subtract_ignore_sign(other, *this);
  }
}

integer& integer::operator-=(integer&& other) & INTEGER_THROW_NEW {
  return *this += -other;
}

integer& integer::operator++() & INTEGER_THROW_NEW {
  return *this += integer(1); 
}

integer& integer::operator--() & INTEGER_THROW_NEW {
  return *this -= integer(1);
}

integer integer::operator++(int) & INTEGER_THROW_NEW {
  auto tmp = *this;
  operator++();
  return tmp;
}

integer integer::operator--(int) & INTEGER_THROW_NEW {
  auto tmp = *this;
  operator--();
  return tmp;
}

integer& integer::operator*=(integer&& other) & INTEGER_THROW_NEW {
  integer res{0};
  while (other) {
    assert(nullptr != other.ptr.get());
    if (other.ptr.get()[0] & 1) {
      res += integer(*this);
    }
    *this <<= integer(1);
    other >>= integer(1);
  }
  return *this = res;
}

integer& integer::operator/=(integer&& divisor) & INTEGER_THROW_NEW {
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

integer& integer::operator%=(integer&& other) & INTEGER_THROW_NEW {
  assert(integer(0) != other);
  integer copy = *this;
  copy /= integer(other);
  copy *= integer(other);
  return *this -= integer(copy);
}

integer& integer::operator<<=(integer&& other) & INTEGER_THROW_NEW {
  auto constexpr nBits = 8 * sizeof(std::uintmax_t);
  while (!(other < integer(nBits))) {
    assert(false); // for now
  }
  assert(!(integer(nBits) < other));
  auto nother = static_cast<std::uintmax_t>(other);
  std::uintmax_t carry = 0;
  for (std::uintmax_t i = 0; i < size(); ++i) {
    auto tmp = ptr.get()[i];
    ptr.get()[i] <<= nother;
    ptr.get()[i] += carry;
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
    make_size_at_least(size() + 1);
    ptr.get()[size() - 1] = carry;
  }
  return *this;
}

integer& integer::operator>>=(integer&& other) & INTEGER_THROW_NEW {
  auto constexpr nBits = 8 * sizeof(std::uintmax_t);
  while (!(other < integer(nBits))) {
    assert(false);
  }
  assert(!(integer(nBits) < other));
  auto nother = static_cast<std::uintmax_t>(other);
  for (std::uintmax_t i = 0; i + 1 < size(); ++i) {
    ptr.get()[i] >>= nother;
    ptr.get()[i] += (ptr.get()[i + 1] & ((1 << nother) - 1)) << (nBits - nother);
  }
  ptr.get()[size() - 1] >>= nother;
  return *this;
}

integer& integer::operator&=(integer&& other) & INTEGER_THROW_NEW {
  make_size_at_least(other.size());
  assert(nullptr != ptr.get());
  assert(nullptr != other.ptr.get());
  for (std::uintmax_t i = 0; i < other.size(); ++i) {
    ptr.get()[i] &= other.ptr.get()[i];
  }
  return *this;
}

integer& integer::operator|=(integer&& other) & INTEGER_THROW_NEW {
  make_size_at_least(other.size());
  assert(nullptr != ptr.get());
  assert(nullptr != other.ptr.get());
  for (std::uintmax_t i = 0; i < other.size(); ++i) {
    ptr.get()[i] |= other.ptr.get()[i];
  }
  return *this;
}

integer& integer::operator^=(integer&& other) & INTEGER_THROW_NEW {
  make_size_at_least(other.size());
  assert(nullptr != ptr.get());
  assert(nullptr != other.ptr.get());
  for (std::uintmax_t i = 0; i < other.size(); ++i) {
    ptr.get()[i] ^= other.ptr.get()[i];
  }
  return *this;
}

integer integer::operator~() const noexcept {
  auto copy = *this;
  for (std::uintmax_t i = 0; i < copy.size(); ++i) {
    copy.ptr.get()[i] = ~copy.ptr.get()[i];
  }
  return copy;
}

integer integer::operator-() const noexcept {
  auto copy = *this;
  copy.make_negative(!copy.is_negative());
  return copy;
}

integer integer::operator+() const noexcept {
  return integer(*this);
}

bool integer::operator<(integer const& other) const noexcept {
  if (is_negative() && !other.is_negative()) {
    return true;
  } else if (!is_negative() && other.is_negative()) {
    return false;
  }
  
  auto const [this_is_smaller, this_is_bigger] = compare_magnitude(other);
  
  if (!is_negative() && !other.is_negative()) {
    return this_is_smaller;
  } else {
    assert(is_negative() && other.is_negative());
    return this_is_bigger;
  }
}

bool integer::operator>(integer const& other) const noexcept {
  if (is_negative() && !other.is_negative()) {
    return false;
  } else if (!is_negative() && other.is_negative()) {
    return true;
  }
  
  auto const [this_is_smaller, this_is_bigger] = compare_magnitude(other);
  
  if (!is_negative() && !other.is_negative()) {
    return this_is_bigger;
  } else {
    assert(is_negative() && other.is_negative());
    return this_is_smaller;
  }
}

bool integer::operator==(integer const& other) const noexcept {
  return !(*this < other) && !(*this > other);
}

bool integer::operator!=(integer const& other) const noexcept {
  return !(*this == other);
}

bool integer::operator<=(integer const& other) const noexcept {
  return *this < other || *this == other;
}

bool integer::operator>=(integer const& other) const noexcept {
  return *this > other || *this == other;
}

integer::~integer() noexcept {
  assert(!(nullptr == ptr.get()) || 0 == size());
  delete [] ptr.get();
}

/*explicit*/ integer::operator bool() const noexcept {
  return !(integer(0) == *this);
}

/*explicit*/ integer::operator std::uintmax_t() const noexcept {
  assert(nullptr != ptr.get());
  return ptr.get()[0];
}

std::string integer::string() const noexcept {
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
void integer::print_internals() const noexcept {
  printf(
    "--- printing ---\n"
    "--- ptr:   %p\n"
    "--- size:  %lu\n"
    "--- neg:   %d\n",
    ptr.get(), size(), is_negative()
  );
  for (std::uintmax_t i = 0; i < size(); ++i) {
    //std::cout << "--- --- ptr[" << i << "] = " << std::bitset<64>(ptr[i]) << std::endl;
    printf("--- --- ptr[%lu] = %lu\n", i, ptr.get()[i]);
  }
  printf("----------------\n");
}
#endif

integer::tagged_ptr::tagged_ptr() noexcept
  : ptr(nullptr)
{}

std::uintmax_t* integer::tagged_ptr::get() const noexcept {
  auto p = reinterpret_cast<std::uintptr_t>(ptr); 
  p &= ~1;
  return reinterpret_cast<std::uintmax_t*>(p);
}

void integer::tagged_ptr::set(std::uintmax_t* p) noexcept {
  ptr = p;
}

#define TAGVAL(WHICH, BIT) \
bool integer::is_##WHICH() const noexcept { \
  return reinterpret_cast<std::uintptr_t>(ptr.ptr) & (1 << BIT); \
} \
void integer::make_##WHICH(bool const b) noexcept { \
  auto p = reinterpret_cast<std::uintptr_t>(ptr.ptr); \
  if (b) { \
    p |= (1 << BIT); \
  } else { \
    p &= ~(1 << BIT); \
  } \
  ptr.ptr = reinterpret_cast<uintmax_t*>(p); \
}
TAGVAL(negative, 0);
TAGVAL(large, 1);

std::uintmax_t integer::size() const noexcept {
  if (nullptr == ptr.get()) {
    return 0;
  }
  return malloc_size(ptr.get()) / sizeof(uintmax_t);
}

void integer::make_size_at_least(std::uintmax_t const sz) INTEGER_THROW_NEW {
  if (size() < sz) {
    auto tmp = reinterpret_cast<uintmax_t*>(malloc(sizeof(std::uintmax_t) * sz));
    std::copy_n(ptr.get(), size(), tmp);
    delete [] ptr.get();
    ptr.set(tmp);
  }
  if (sz < size()) {
    std::memset(
      ptr.get() + sz,
      0,
      sizeof(std::uintmax_t) * (size() - sz)
    );
  }
}

std::pair<bool, bool> integer::compare_magnitude(integer const& other) const& noexcept {
  for (auto i = std::max(size(), other.size()); ; --i) {
    auto this_now = i < size() ? ptr.get()[i] : 0;
    auto other_now = i < other.size() ? other.ptr.get()[i] : 0;
    
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

#define ARITH_HELPER(OPERATOR, OP, NAME) \
integer OPERATOR(integer rhs, integer lhs) INTEGER_THROW_NEW { \
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