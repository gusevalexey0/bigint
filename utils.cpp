//
// Created by qw on 10.09.18.
//

#include "utils.h"

void add_magnitude(big_integer::magnitude_t &a, const big_integer::magnitude_t &b) {
    uint64_t carry = 0;
    for (size_t i = 0; i < std::min(a.size(), b.size()); ++i) {
        uint64_t sum = static_cast<uint64_t>(a[i]) + static_cast<uint64_t>(b[i]) + carry;
        a[i] += b[i] + carry;
        if (sum > UINT32_MAX) {
            carry = 1;
        } else {
            carry = 0;
        }
    }
    for (size_t i = std::min(a.size(), b.size()); i < std::max(a.size(), b.size()); ++i) {
        if (a.size() < b.size())
            a.push_back(b[i]);
        uint64_t sum = static_cast<uint64_t>(a[i]) + static_cast<uint64_t>(carry);
        a[i] += carry;
        if (sum > UINT32_MAX) {
            carry = 1;
        } else {
            carry = 0;
        }
    }
    if (carry)
        a.push_back(static_cast<big_integer::digit_t>(carry));
    strip_leading_zeroes(a);
}

void strip_leading_zeroes(big_integer::magnitude_t &mag) {
    while (mag.size() > 1 && mag.back() == 0)
        mag.pop_back();
}

void sub_magnitude(big_integer::magnitude_t &res, const big_integer::magnitude_t &big, const big_integer::magnitude_t &little) {
    big_integer::digit_t carry = 0;
    big_integer::digit_t diff = 0;
    big_integer::digit_t tmp = 0;
    for (size_t i = 0; i < little.size(); ++i) {
        tmp = big[i];
        diff = tmp - little[i] - carry;
        if (diff > tmp) {
            carry = 1;
        } else {
            carry = 0;
        }
        res[i] = diff;
    }
    if (carry) {
         res[little.size()] -= carry;
    }
    strip_leading_zeroes(res);
}

int compare_magnitude(const big_integer::magnitude_t &a, const big_integer::magnitude_t &b) {
    if (a.size() != b.size())
        return a.size() > b.size() ? 1 : -1;
    for (int i = static_cast<int>(a.size()) - 1; i >= 0; --i) {
        if (a[i] > b[i])
            return 1;
        if (a[i] < b[i])
            return -1;
    }
    return 0;
}

void mul_magnitude(big_integer::magnitude_t &a, const big_integer::magnitude_t &b) {
    big_integer::magnitude_t res(a.size() + b.size() + 2, 0);
    for (size_t i = 0; i < a.size(); ++i) {
        uint64_t product = 0, sum = 0, tmp = 0;
        for (size_t j = 0; j < b.size(); ++j) {
            product = static_cast<uint64_t>(a[i]) * static_cast<uint64_t>(b[j]);
            tmp = (product & UINT32_MAX) + res[i + j] + sum;
            res[i + j] = static_cast<big_integer::digit_t>(tmp & UINT32_MAX);
            sum = (product >> bits_in_base()) + (tmp >> bits_in_base());
        }
        res[i + b.size()] += static_cast<big_integer::digit_t>(sum);
    }
    strip_leading_zeroes(res);
    a = res;
}

void mul_by_digit(big_integer::magnitude_t &a, const big_integer::digit_t b) {
    uint64_t tmp = 0;
    size_t i;
    for (i = 0; i < a.size(); ++i) {
        uint64_t product = a[i] * static_cast<uint64_t>(b) + tmp;
        a[i] = static_cast<big_integer::digit_t>(product & UINT32_MAX);
        tmp = product >> bits_in_base();
    }
    if (tmp)
        a.push_back(static_cast<big_integer::digit_t>(tmp));
}

void div_by_digit(big_integer::magnitude_t &a, const big_integer::digit_t b, big_integer::digit_t &rem) {
    uint64_t tmp = 0;
    big_integer::magnitude_t res(a.size());
    size_t j = 0;
    for (size_t i = a.size() - 1; i < a.size(); --i) {
        tmp = (tmp << bits_in_base()) + a[i];
        res[j++] = static_cast<big_integer::digit_t>(tmp / b);
        tmp %= b;
    }
    std::reverse(res.begin(), res.end());
    strip_leading_zeroes(res);
    rem = static_cast<big_integer::digit_t>(tmp);
    a = res;
}

void shift_right_magnitude(big_integer::magnitude_t &a, big_integer::digit_t rhs) {
    big_integer::digit_t d = 0;
    big_integer::magnitude_t res;
    size_t st = 0;
    while (rhs >= sizeof(big_integer::digit_t) * 8) {
        st = rhs / bits_in_base();
        rhs %= bits_in_base();
    }
    for (size_t i = a.size() - 1; (i >= st) && (i < a.size()); --i) {
        big_integer::digit_t digit = (a[i] >> rhs) & ((1u << (bits_in_base() - rhs)) - 1u);
        digit |= d;
        d = (a[i] & ((1u << rhs) -1u)) << (bits_in_base() - rhs);
        res.push_back(digit);
    }
    std::reverse(res.begin(), res.end());
    strip_leading_zeroes(res);
    a = res;
}
size_t bits_in_base() {
    return sizeof(big_integer::digit_t) * 8;
}

void shift_left_magnitude(big_integer::magnitude_t &a, big_integer::digit_t rhs) {
    big_integer::digit_t d = 0;
    big_integer::magnitude_t res(a.size() + static_cast<big_integer::digit_t>((rhs + 31) / 32));
    size_t j = 0;
    while (rhs >= int(bits_in_base())) {
        rhs -= bits_in_base();
        res[j++] = 0;
    }
    if (!rhs) {
        for (auto i : a) {
            res[j++] = i;
        }
        strip_leading_zeroes(res);
        a = res;
        return;
    }
    for (auto i : a) {
        big_integer::digit_t digit = d | (i << rhs);
        res[j++] = digit;
        d = ((i >> (bits_in_base() - static_cast<big_integer::digit_t>(rhs))));
    }
    if (d)
        res.push_back(d);
    strip_leading_zeroes(res);
    a = res;
}

void twos_complement(big_integer::magnitude_t &a) {
    for (auto &i : a) {
        i = ~i;
    }
    add_magnitude(a, {1});
}
