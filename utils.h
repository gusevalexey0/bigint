//
// Created by qw on 10.09.18.
//

#ifndef BIGINT_UTILS_H
#define BIGINT_UTILS_H

#include "big_integer.h"
#include <algorithm>

const uint64_t BASE = 1 << 32;
void add_magnitude(big_integer::magnitude_t &, const big_integer::magnitude_t &);
void sub_magnitude(big_integer::magnitude_t &, const big_integer::magnitude_t &, const big_integer::magnitude_t &);
void strip_leading_zeroes(big_integer::magnitude_t &);
int compare_magnitude(const big_integer::magnitude_t &, const big_integer::magnitude_t &);
void mul_magnitude(big_integer::magnitude_t &, const big_integer::magnitude_t &);
void mul_by_digit(big_integer::magnitude_t &, big_integer::digit_t);
void div_by_digit(big_integer::magnitude_t &, const big_integer::digit_t, big_integer::digit_t &);
void shift_right_magnitude(big_integer::magnitude_t &, big_integer::digit_t);
void shift_left_magnitude(big_integer::magnitude_t &, big_integer::digit_t);
void twos_complement(big_integer::magnitude_t &);

size_t bits_in_base();
#endif //BIGINT_UTILS_H
