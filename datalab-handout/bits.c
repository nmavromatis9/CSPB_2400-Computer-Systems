/* 
 * CS:APP Data Lab 
 * 
 * <Nicolas Mavromatis, nima6629>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 10.0.0.  Version 10.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2017, fifth edition, plus
   the following additions from Amendment 1 to the fifth edition:
   - 56 emoji characters
   - 285 hentaigana
   - 3 additional Zanabazar Square characters */
/* We do not support C11 <threads.h>.  */
/* 
 * bitOr - x|y using only ~ and & 
 *   Example: bitOr(6, 5) = 7
 *   Legal ops: ~ &
 *   Max ops: 8
 *   Rating: 1
 */
int bitOr(int x, int y) 
{
    /* Demorgan's Law: ~(x&y)==~x | ~y */
  return ~(~x & ~y);
}
/* 
 * evenBits - return word with all even-numbered bits set to 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 1
 */
/*NOTE: right most bit is 0th, which is even, moving left, add 1*/
int evenBits(void) 
{
    /* want to return 01010101=0x55, but in 32 bit:0x55555555 */
    int val1=0x55;
    /*use bit shifts*/
    int val2=val1<<8; /*0x5500*/
    int val3=val1<<16; /*0x550000*/
    int val4=val1<<24; /*0x55000000*/
    /*using | creates all 0x5's*/
    int ans=val1|val2|val3|val4;
  return (ans);
}
/* 
 * minusOne - return a value of -1 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 2
 *   Rating: 1
 */
int minusOne(void) 
{
  /* The inverse of 0000 = 1111, (4 bits) which is -1 in Two's complement */
    /*0 is automatically represented in 32 bits. */
  return ~(0);
}
/* 
 * allEvenBits - return 1 if all even-numbered bits in word set to 1
 *   Examples allEvenBits(0xFFFFFFFE) = 0, allEvenBits(0x55555555) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allEvenBits(int x) 
{
    /*0x55 is mask again. Has 1 in all even bit positions*/
    int mask=0x55;
    mask=mask<<8|mask; /*0x5555*/
    mask=mask<<16|mask;/*0x55555555*/
    /*first, use x&mask to get all 1's in even positions, all 0's in odd for result*/
    x=x&mask;
    /*if x now matches the mask exactly, it will return all 0's*/
    /*use ! to get it to return 1(True)*/
    return!(x^mask);
}
/* 
 * anyOddBit - return 1 if any odd-numbered bit in word set to 1
 *   Examples anyOddBit(0x5) = 0, anyOddBit(0x7) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int anyOddBit(int x) 
{
    /* filter will be 0xAA: 10101010 in 32 bits. Has 1's in all odd pos.*/
    int mask=0xAA;
    mask=mask<<8|mask; /*0xAAAA*/
    mask=mask<<16|mask; /*0xAAAAAAAA*/
    /*Use x&mask. If odd pos has 1, it will be pos value*/
    /*must use ! logical op to convert any nonzero value to 0, then 
    ! to convert it back to 1*/
    return !!(x&mask);
}
/* 
 * byteSwap - swaps the nth byte and the mth byte
 *  Examples: byteSwap(0x12345678, 1, 3) = 0x56341278
 *            byteSwap(0xDEADBEEF, 0, 2) = 0xDEEFBEAD
 *  You may assume that 0 <= n <= 3, 0 <= m <= 3
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 25
 *  Rating: 2
 */
int byteSwap(int x, int n, int m) 
{/*bytes numbered from rightmost (0), consist of two hex digits.*/
    /*vars to hold nth and mth bytes*/
    int nthbyte=0;
    int mthbyte=0;
    
    int maskn=0;
    int maskm=0;
    int finalmask=0;
    int swap=0;
    /*shifting left by 3 converts num bytes to bits (mult by 8)*/
    n=n<<3;
    m=m<<3;
    /*shift right by n or m, placing desired bits in rightmost 8 positions.*/
    /*then use &0xFF to extract those specific bits only*/
    nthbyte=x>>n;
    nthbyte=nthbyte&0xFF;
    mthbyte=x>>m;
    mthbyte=mthbyte&0xFF;
    /*now create masks with 1's in m or n bit postions only*/
    maskm=0xFF<<m;
    maskn=0xFF<<n;
    /*Create final mask with 0's in m and n spots, all else 1's by inversion*/
    finalmask=~(maskm|maskn);
    /*shift nth byte to mth position, mth byte to nth position*/
    nthbyte=nthbyte<<m;
    mthbyte=mthbyte<<n;
    /*mask x&final mask to place 0's in m and n byte positions.*/
    x=x&finalmask;
    /*swap original positions of m and n*/
    swap= x|nthbyte|mthbyte;
    return swap;
}
/* 
 * addOK - Determine if can compute x+y without overflow
 *   Example: addOK(0x80000000,0x80000000) = 0,
 *            addOK(0x80000000,0x70000000) = 1, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int addOK(int x, int y) 
{
    /*extract the sign of y and x, and s=x+y to find type of overflow*/
    /* if both x and y are pos(>0), but s is neg(<=0), pos overflow.
       if both x and y<=0, but s>0, neg overflow. */
    
    /*>>31 extracts just the sign bit. 
    Makes 0xFFFFFF if neg, or 0x0000000 if not. */
    int xsign=x>>31;
    int ysign=y>>31;
    int addsign=(x+y)>>31;
    /*overflow is 0xffffffff if true, 0x0 if false.*/
    /*if xsign and ysign are the same, xsign^ysign=0x0000, and ~(xsign ^ ysign)=0xffff. */
    /*if xsign and ysign are diff, xsign^ysign=0xffff, and ~(xsign ^ ysign)=0x0000*/
    /*if xsign and addsign are diff, (xsign ^ addsign )=0xffff. */
    /*if xsign and addsign  are same, (xsign ^ addsign )=0x0000
    /*if overflow, gives ~(xsign ^ ysign) & (xsign ^ addsign )= 0xffff. Else, gives 0x0000*/
    /*use ! to get opposite bool result, for addok(overflow)=false.*/
    int overflow=!(~(xsign ^ ysign) & (xsign ^ addsign));
    return overflow;
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) 
{
    /*Mask:if x is positive: we want all 1's. if x is 0: we want all 0's in 32 bits*/
    /*first convert positive x to 0x0000 then 0x0001, or x==0 to 0x0001 then 0x0000 by !! op.*/
    /*shifting <<31 makes 1 sign bit if positive, 0 the sign bit if x==0.*/
    /*shift >>31 uses arithmetic right shift, making all 1's if x is pos,
    all 0's if x is 0.*/
    int mask=((!!x)<<31)>>31;
    /*if x=0, we want to mask y, and return z. Mask=0x0000*/
    /*if x=pos, we want to mask z, and return y, Mask=0xFFFF*/
    return((mask&y)|(~mask&z));
}
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) 
{
    /*
    Adding the ~complement is the same as subtracting one greater than that |value| due to 32 bit truncation.
    AKA ~ is the same as the Two's complement-1.
   if x>0x39, is added to ~0x3a(=-0x3b), result is neg.
   if x<0x30, is added to ~0x2f(=-0x30), result is neg.*/
    /* Ex: 
    0x3a=0000 0000 0000 0000 0000 0000 0011 1010 +
   ~0x3a=1111 1111 1111 1111 1111 1111 1100 0101 =
         1111 1111 1111 1111 1111 1111 1111 1111(-1).*/
    int high=0x3a;
    int low=0x2f;
    int toolow=x+~low;
    int toohigh=high+~x;
    /*if either is negative, that means it is out of ascii range. sign bit=1.*/
    /*extract sign bit, use ! to make binary true/false*/
    return !((toolow|toohigh)>>31);
    
}
/* 
 * replaceByte(x,n,c) - Replace byte n in x with c
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: replaceByte(0x12345678,1,0xab) = 0x1234ab78
 *   You can assume 0 <= n <= 3 and 0 <= c <= 255
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 3
 */
int replaceByte(int x, int n, int c) 
{
    int mask=0;
    int mask2=0;
  /*first convert n byte into bits by mult by 8, or <<3*/
    n=n<<3;
    /*next, create mask by shifting 0xFF n positions, taking inverse. 
    so that in nth position are zeros, and 1's in all other pos.*/
    mask=~(0xFF<<n);
    /*mask off bits in nth position*/
    x=x&mask;
    /*create mask with desired hex digits in nth pos*/
    mask2=c<<n;
    /*finally, switch on bits in nth position to c*/
    return (x|mask2);
    
}
/* reverseBits - reverse the bits in a 32-bit integer,
              i.e. b0 swaps with b31, b1 with b30, etc
 *  Examples: reverseBits(0x11111111) = 0x88888888
 *            reverseBits(0xdeadbeef) = 0xf77db57b
 *            reverseBits(0x88888888) = 0x11111111
 *            reverseBits(0)  = 0
 *            reverseBits(-1) = -1
 *            reverseBits(0x9) = 0x90000000
 *  Legal ops: ! ~ & ^ | + << >> and unsigned int type
 *  Max ops: 90
 *  Rating: 4
 */
int reverseBits(int x) 
{
    /*
    0x55555555 = 01010101010101010101010101010101
    0x33333333 = 00110011001100110011001100110011
    0x0F0F0F0F = 00001111000011110000111100001111
    0xFF00FF00 = 11111111000000001111111100000000
    0xFFFF0000 = 11111111111111110000000000000000
    Strategy is to reverse bits by swapping adjacent 16 bits,
    then adjacent 8 bits, then 4 bits, then 2 bits, then 1 bits.
    */
    
    /*EX:
    x=         1010 0011 ...
    x>>1=      1101 0001  which offsets x by 1 position right
    0x55=      0101 0101  Bit filter for all even pos=1
    x>>1&0x55= 0101 0001  which flips bit position on if x (shifted by 1) right is on
    x&0x55=    0000 0001  which flips bit position on if x is on
    <<1=       0000 0010  then offsets result by 1 position left
               
      |        0000 0010 
               0101 0001 
               0101 0011  Which exchanged all adjacent 1 bits.
    */
    x = ((x >> 16) & 0xffff) | ((x & 0xffff) << 16);
    x = ((x >> 8) & 0x00ff00ff) | ((x & 0x00ff00ff) << 8);
    x = ((x >> 4) & 0x0f0f0f0f) | ((x & 0x0f0f0f0f) << 4);
    x = ((x >> 2) & 0x33333333) | ((x & 0x33333333) << 2);
    x = ((x >> 1) & 0x55555555) | ((x & 0x55555555) << 1);
    return x;
}
/*
 * satAdd - adds two numbers but when positive overflow occurs, returns
 *          maximum possible value (Tmax), and when negative overflow occurs,
 *          it returns minimum negative value (Tmin)
 *   Examples: satAdd(0x40000000,0x40000000) = 0x7fffffff
 *             satAdd(0x80000000,0xffffffff) = 0x80000000
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 30
 *   Rating: 4
 */
int satAdd(int x, int y) 
{
    /*
    Overflow:Tmin-1=Tmax, and Tmax+1=Tmin
    Tmin: 1000 0000 0000 0000 0000 0000 0000 0000
    Tmax: 0111 1111 1111 1111 1111 1111 1111 1111
    SO: note that they are inverses. 
    */
    int sum=x+y;
    int xsign=x>>31;
    int ysign=y>>31;
    //int finalshift=0;
    /*if sign is positive, shifting >>31 gives 0x00000000.
    if sign is negative, shifting >>31 gives 0xFFFFFFFF.*/
    int signsum=sum>>31;
    int shift=0;

    /*overflow is 0xffffffff if true, 0x0 if false.*/
    /*if xsign and ysign are the same, xsign^ysign=0x0000, and ~(xsign ^ ysign)=0xffff. */
    /*if xsign and signsum are diff, (xsign ^ signsum)=0xffff. */
    /*if xsign and signsum are same, (xsign ^ signsum)=0x0000 */
    /*if overflow, gives ~(xsign ^ ysign) & (xsign ^ signsum)= 0xffff.*/
    int overflow=((~(xsign ^ ysign)) & (xsign ^ signsum)); /*using !! creates some weird memory loop...*/
    /*0xFFFFFFFF for overflow true, 0x0000000 for overflow false.*/
    //overflow=overflow>>31;

    /*Neg overflow: 0xFFFFFFFF 00000000
     pos overflow   0x00000000 FFFFFFFE/*
     
    /*(overflow << 31) is tmin if overflow occured:*/
    /*10000000000000000000000000000000* (TMIN)*/
    /*shifting (sum >> 31) if overflow occurred gives: 
      00000000000000000000000000000000 for neg overflow,
      11111111111111111111111111111111 for pos overflow.
      xor either of these two with Tmin gives tmin(neg overflow),
      Tmax(pos overflow). 
      
      Overflow is 0 if did not occur, returning sum b/c:
      Xor Original Digit and 0=Original Digit always.*/
    
    /* This makes a max shift of 31 if overflow happened, or 0 if not*/
     shift=(overflow&31);
     return ((sum >> shift) ^ (overflow << 31));
}
/*
 * Extra credit
 */
/* 
 * float_abs - Return bit-level equivalent of absolute value of f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument..
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_abs(unsigned uf) 
{
    unsigned mask = 0x7FFFFFFF;
    unsigned minimumNaN = 0x7F800001;
    /*minimum naN==
    /*01111111100000000000000000000001*/
    /*B/c all exponent 1 bits and frac bits!=00...00 means NaN. So setting LSB to 1 makes minimum NaN.
    /*First, Set the sign bit to 0, because it is absolute value:*/
    unsigned answer = mask & uf; 
    
    /*all naN are >= to this minimum. If so, return original argument.*/
    if (answer >= minimumNaN)
    {
        return uf;
    }
    else
    {
        return answer;
    }
}
/* 
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) 
{
    return 2;
}
  
/* 
 * float_half - Return bit-level equivalent of expression 0.5*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_half(unsigned uf) {
  return 2;
}
