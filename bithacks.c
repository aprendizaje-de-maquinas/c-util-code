
#include "bits.h"
#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/*
  This function takes two integers (a and b) and returns 1 if a has move bits set, -1 if b has more
  bits set, and 0 if a and b have the same number of bits set.
  Goes through the loop as many times as a has set bits.
 */
int cmp_bits(int a, int b)
{
  int cnt ;
  for(cnt = 0 ; a ; cnt++){
    a &= a-1 ; // clear the last set bit.
    if(!b) return 1 ; // b is already equal to 0 but a was non 0 so return positive. 
    b &= b-1 ;
  }
  if(b) return -1 ; // if b is non zero, it has more set bits.
  return 0 ; // they are equal.
}

/*
  Takes an array of integers (values) and its number of elements.
  Itterates through the array and for every i in values, sets the ith bit of result.
  Returns the result.
 */
unsigned short make_set(int values[], int nvalues)
{
  unsigned short result = 0 ;
  for(int x = 0 ; x < nvalues ; x++){
    result |= (0x1<<values[x])  ;
  }
  return result ;
}

/*
  Takes in three shorts representing the numbers  in a row, column, and block of a sudoku state
  Returns if there is only one number that can be in the remaining space (ie the one in the given row and column)

  Creates a superset containing all the numbers used in the row, column, and block
  Then XORs it with 0xfe to get the bits that are unset in the range [8 , 1] (ie the ones representing sudoku numbers)
  Returns true if the resulting number is a power of 2 (ie has one bit set).
 */
bool is_single(unsigned short used_in_row, unsigned short used_in_col, unsigned short used_in_block)
{
  unsigned short temp = 0xfe^(used_in_row | used_in_col | used_in_block) ; // create superset
  return (temp & (temp-1)) ==0 ; // return if power of two.
}

/*
  Takes in two unsigned utypes (a and b) and adds them using saturating arithmatic.
  ie if a+b overflows, return the max for the utype else return the sum as usual
 */
utype sat_add_unsigned(utype a, utype b)
{
  utype temp = a+b ;
  // overflow
  if(temp < a) return (utype)(-1) ; // this will only happen in the case of overflow. NOTE -1 as a utype has all bits set.
  
  return temp ; // normal.
}

/*
  Takes in two signed stypes (a and b) and returns their sum using saturating arithmatic
  ie if the sum is greater than stype_max, we return stype_max.
      else if sum is less than stype_min, return stype_min
      else return the sum as usual.
 */
stype sat_add_signed(stype a, stype b)
{
  stype temp = a+b ;

  // this is the case of overflow. Return all the bits set except for the first one (hence the shift after thec cast to utype).
  if((temp < a && b >0) ||( temp < b && a >0)) return ((utype)(-1))>>1 ;

  // this is the case of underflow. Return the first bit set and all the others as 0.
  if(a < 0 && b<0 && temp > 0) return (stype)0x1<<(stype)(sizeof(stype)*8-1) ;

  return temp ; // normal.
}

#define MAX_BYTES 5

/* Function: print_hex_bytes
 * --------------------------
 * Given the pointer to a sequence of bytes and a count, and this
 * function will print count number of raw bytes in hex starting at
 * the bytes address, filling in with spaces to align at the width
 * of the maximum number of bytes printed (5 for push).
 */
void print_hex_bytes(const unsigned char *bytes, int nbytes)
{
    for (int i = 0; i < MAX_BYTES; i++)
        if (i < nbytes)
            printf("%02x ", bytes[i]);
        else
            printf("  ");
    printf("\t");
}




