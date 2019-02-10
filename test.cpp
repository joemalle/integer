#include "integer.h"

// This file intentionally relies on integer.h for all includes

int main() {    
    integer i0 = 0;
    //i0.print_internals();
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
    
    /*
    integer i5 = 5;
    assert(i5 == 5);
    integer i6 = 6;
    integer i11 = i5 + i6;
    assert(i11 == 11);
    assert(i11 == i11);
    assert(11 == i11);
    assert(0 < i11);
    assert(static_cast<bool>(i11));
    */
}
