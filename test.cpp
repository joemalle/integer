#include "integer.h" // This file intentionally relies on integer.h for all includes


int main() {
  integer i0 = 0;
  i0.print_internals();
  assert(i0 == 0);
  assert(0 == i0);
  assert(!(0 < i0));
  assert(!(i0 < 0));
  assert(!(0 > i0));
  assert(!(i0 > 0));
  assert(0 <= i0);
  assert(i0 <= 0);
  assert(0 >= i0);
  assert(i0 >= 0);
  assert(!(0 != i0));
  assert(!(i0 != 0));
  
  integer i1 = 1;
  //i1.print_internals();
  assert(!(i1 == 0));
  assert(!(0 == i1));
  assert(0 < i1);
  assert(!(i1 < 0));
  assert(!(0 > i1));
  assert(i1 > 0);
  assert(0 <= i1);
  assert(!(i1 <= 0));
  assert(!(0 >= i1));
  assert(i1 >= 0);
  assert(0 != i1);
  assert(i1 != 0);
  
  integer im1 = -1;
  //im1.print_internals();
  assert(!(im1 == 0));
  assert(!(0 == im1));
  assert(!(0 < im1));
  assert(im1 < 0);
  assert(0 > im1);
  assert(!(im1 > 0));
  assert(!(0 <= im1));
  assert(im1 <= 0);
  assert(0 >= im1);
  assert(!(im1 >= 0));
  assert(0 != im1);
  assert(im1 != 0);
  
  integer i2 = integer(1) + integer(1);
  //i2.print_internals();
  assert(i2 == 2);
  assert(i2 > -12);
  
  integer i4 = integer(2) * integer(2);
  //i4.print_internals();
  assert(i4 == 4);
  
  integer is4 = integer(7) - integer(3);
  //is4.print_internals();
  assert(is4 == 4);
  
  integer is0 = integer(7) - integer(7);
  //is0.print_internals();
  assert(is0 == 0);
  
  integer ism4 = integer(3) - integer(7);
  //ism4.print_internals();
  assert(ism4 == -4);
  assert(!(0 < ism4));
  
  
  integer id2 = integer(4) / integer(2);
  //id2.print_internals();
  assert(id2 == 2);
  
  integer id3 = integer(10) / integer(3);
  //id3.print_internals();
  assert(id3 == 3);
  
  integer imod4 = integer(14) % integer(10);
  //imod4.print_internals();
  assert(imod4 == 4);
  
  integer imod0 = integer(10) % integer(10);
  //imod0.print_internals();
  assert(imod0 == 0);
  
  integer ishl4 = integer(2) << integer(1);
  //ishl4.print_internals();
  assert(ishl4 == 4);
  
  integer ishr4 = integer(8) >> integer(1);
  //ishr4.print_internals();
  assert(ishr4 == 4);
  
  integer i54321 = 54321;
  [[maybe_unused]] auto n54321 = static_cast<std::uintmax_t>(54321);
  assert(n54321 == 54321);
  
  assert(i54321 % 10 == 1);
  
  std::string res = "54321";
  assert(res == i54321.string());
  
  integer nBig = 18446744073709551615ull;
  integer nBigger = nBig + nBig + nBig + nBig;
  //nBigger.print_internals();
  
  integer nSmaller = nBigger - 10000;
  //nSmaller.print_internals();
  
  integer nVeryNegative = 10000 - nBigger;
  //nVeryNegative.print_internals();
  
  integer nVeryPositive = -nVeryNegative;
  //nSmaller.print_internals();
  //nVeryPositive.print_internals();
  assert(nVeryPositive == nSmaller);
  
  auto nVeryBig250 = nBigger + integer(250);
  auto nVeryBig250250 = nVeryBig250 + integer(250);
  auto nVeryBig250250m25 = nVeryBig250250 - integer(25);
  auto nVeryBig475 = nBigger + integer(475);
  //nVeryBig475.print_internals();
  //nVeryBig250250m25.print_internals();
  assert(nVeryBig475 == nVeryBig250250m25);
  
  assert(static_cast<bool>(nVeryBig250250m25));
  
  assert(!static_cast<bool>(i0));
  
  //std::cout << nVeryBig250250m25.string() << std::endl;
  integer orig = nVeryBig475;
  integer intermediate = orig << integer(1);
  integer notorig = intermediate >> integer(1);
  //orig.print_internals();
  //notorig.print_internals();
  assert(orig == notorig);
  
  integer n8 = 0b1000;
  assert(0b0 == (n8 & 0b1));
  assert(0b1000 == (n8 & 0b11111));
  assert(0b1001 == (n8 | 0b1));
  assert(0b11111 == (n8 | 0b11111));
  assert(0b1001 == (n8 ^ 0b1));
  assert(0b10111 == (n8 ^ 0b11111));
  
  /*
  integer nHarderShift = 14769796029758971152u;
  nHarderShift.print_internals();
  
  integer nHarderShiftSL1 = nHarderShift << 1;
  nHarderShiftSL1.print_internals();
  */
  
  integer nKindaBig1 = 12345678900;
  integer nKindaBig2 = 56789123400;
  assert(5678912340 == nKindaBig2 / 10);
  assert(0 == nKindaBig2 % 10);
  //nKindaBig2.print_internals();
  std::cout << nKindaBig2.string() << std::endl;
  integer nKindaBigProd = nKindaBig1 * nKindaBig2;
  //nKindaBigProd.print_internals();
  std::cout << nKindaBigProd.string() << std::endl;
  
}
