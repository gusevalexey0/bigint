#include "big_integer.h"

#include <cstring>
#include <stdexcept>
#include "utils/utils.h"
#include <numeric>
#include <limits>

big_integer::big_integer() : mag(0), sign(0) {}

big_integer::big_integer(big_integer const &other) = default;

big_integer::big_integer(int a) {
    if (!a) {
        sign = 0;
        return;
    }
    sign = a > 0 ? 1 : -1;
    mag.push_back(static_cast<digit_t>(std::abs(static_cast<int64_t>(a))));
}

big_integer::big_integer(std::string const &str) {
    big_integer tmp = 1;
    sign = 1;
    for (size_t i = str.size() - 1; i < str.size(); --i) {
        if ((i == 0) && str[i] == '-') {
            sign = -1;
            break;
        }
        big_integer t = tmp;
        mul_by_digit(t.mag, static_cast<big_integer::digit_t>(str[i] - '0'));

        *this += t;

        mul_by_digit(tmp.mag, 10);
    }
    strip_leading_zeroes(mag);
    if (is_zero())
        sign = 0;
}

big_integer::~big_integer()
= default;

big_integer &big_integer::operator=(big_integer const &other)
= default;

big_integer &big_integer::operator+=(big_integer const &rhs) {
    if (!rhs.sign)
        return *this;
    if (!sign)
        return *this = rhs;
    if (sign == rhs.sign)
        add_magnitude(mag, rhs.mag);
    else {
        negate();
        *this -= rhs;
        negate();
    }
    return *this;
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    if (!rhs.sign)
        return *this;
    if (!sign)
        return *this = -rhs;
    if (sign == rhs.sign) {
        int cmp = compare_magnitude(mag, rhs.mag);
        if (!cmp)
            return *this = 0;
        if (cmp == 1)
            sub_magnitude(mag, mag, rhs.mag);
        else {
            sub_magnitude(mag, rhs.mag, mag);
            negate();
        }
    } else {
        negate();
        *this += rhs;
        negate();
    }
    if (is_zero())
        sign = 0;
    return *this;
}

big_integer &big_integer::operator*=(big_integer const &rhs) {
    mul_magnitude(mag, rhs.mag);
    sign *= rhs.sign;
    return *this;
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    bool is_sign = (sign == rhs.sign);
    sign = 1;
    if (rhs.sign < 0) {
        big_integer tmp = rhs;
        tmp.sign = 1;
        *this = make_div(*this, tmp).first;
    } else {
        *this = make_div(*this, rhs).first;
    }
    if (is_sign)
        sign = 1;
    else
        sign = -1;
    strip_leading_zeroes(mag);
    if (is_zero())
        sign = 0;
    return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    bool is_sign = (sign == -1 && rhs.sign == 1);
    sign = 1;
    if (rhs.sign < 0) {
        big_integer tmp = rhs;
        tmp.sign = 1;
        *this = make_div(*this, tmp).second;
    } else {
        *this = make_div(*this, rhs).second;
    }
    if (is_sign)
        sign = -1;
    else
        sign = 1;
    strip_leading_zeroes(mag);
    if (is_zero())
        sign = 0;
    return *this;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
    return *this = big_integer::binary_operation(*this, rhs,
                                                 [&](big_integer::digit_t a, big_integer::digit_t b) { return a & b; });
}

big_integer &big_integer::operator|=(big_integer const &rhs) {

    return *this = big_integer::binary_operation(*this, rhs,
                                                 [&](big_integer::digit_t a, big_integer::digit_t b) { return a | b; });
}

big_integer &big_integer::operator^=(big_integer const &rhs) {

    return *this = big_integer::binary_operation(*this, rhs,
                                                 [&](big_integer::digit_t a, big_integer::digit_t b) { return a ^ b; });
}

big_integer &big_integer::operator<<=(int rhs) {
    shift_left_magnitude(mag, static_cast<digit_t>(rhs));
    return *this;
}

big_integer &big_integer::operator>>=(int rhs) {
    if (sign == -1) {
        *this -= 1;
    }
    shift_right_magnitude(mag, static_cast<digit_t>(rhs));
    if (sign == -1)
        *this -= 1;
    return *this;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    return big_integer(*this).negate();
}

big_integer big_integer::operator~() const {
    big_integer res = -*this - 1;
    return res;
}

big_integer &big_integer::operator++() {
    return *this = *this + 1;
}

big_integer big_integer::operator++(int) {
    big_integer r = *this;
    ++*this;
    return r;
}

big_integer &big_integer::operator--() {
    return *this = *this - 1;
}

big_integer big_integer::operator--(int) {
    big_integer r = *this;
    --*this;
    return r;
}

big_integer operator+(big_integer a, big_integer const &b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const &b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const &b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const &b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const &b) {
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const &b) {
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const &b) {
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const &b) {
    return a ^= b;
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
    return a >>= b;
}

bool operator==(big_integer const &a, big_integer const &b) {
    return a.sign == b.sign && a.mag == b.mag;
    //if (a.sign != b.sign) {
    //    return false;
    //}
    //if (a.mag.size() != b.mag.size()) {
    //    return false;
    //}
    //for (size_t i = a.mag.size() - 1; i < a.mag.size(); --i) {
    //    if (a.mag[i] != b.mag[i])
    //        return false;
    //}
    //return true;
}

bool operator!=(big_integer const &a, big_integer const &b) {
    return !(a == b);
}

bool operator<(big_integer const &a, big_integer const &b) {
    if (a.sign < b.sign)
        return true;
    if (a.sign > b.sign)
        return false;
    return compare_magnitude(a.mag, b.mag) * a.sign < 0;
}

bool operator>(big_integer const &a, big_integer const &b) {
    return !(a <= b);
}

bool operator<=(big_integer const &a, big_integer const &b) {
    return a < b || a == b;
}

bool operator>=(big_integer const &a, big_integer const &b) {
    return !(a < b);
}

std::string to_string(big_integer const &a) {
    std::string res;
    if (a.is_zero()) {
        return "0";
    }
    big_integer tmp = a;
    int sign = tmp.sign;
    tmp.sign = 1;
    while (tmp > 0) {
        big_integer::digit_t rem;
        div_by_digit(tmp.mag, 10, rem);
        res += std::to_string(rem);
        if (tmp.is_zero())
            break;
    }
    if (sign == -1)
        res += '-';
    std::reverse(res.begin(), res.end());
    return res;
}

bool big_integer::is_zero() const {
    return (sign == 0) || (mag.size() == 1 && mag[0] == 0) || mag.size() == 0;
}

big_integer &big_integer::negate() {
    sign *= -1;
    return *this;
}

template<typename F>
big_integer big_integer::binary_operation(big_integer a, big_integer b, F &&lambda) {
    if (a.mag.size() < b.mag.size()) {
        a.mag.resize(b.mag.size());
    } else {
        b.mag.resize(a.mag.size());
    }
    if (a.sign == -1) {
        twos_complement(a.mag);
        a.sign = 1;
    }
    if (b.sign == -1) {
        twos_complement(b.mag);
        b.sign = 1;
    }
    for (size_t i = 0; i < a.mag.size(); ++i) {
        a.mag[i] = lambda(a.mag[i], b.mag[i]);
    }
    if (a.mag[a.mag.size() - 1] > UINT32_MAX >> 1) {
        twos_complement(a.mag);
        a.sign = -1;
    }
    if (a.is_zero()) {
        a.sign = 0;
        a.mag.resize(0);
    }
    return a;
}

std::pair<big_integer, big_integer> big_integer::divmod(big_integer const &a, big_integer const &b) {
    big_integer r, q;
    q.sign = 1;
    size_t n = a.mag.size(), m = b.mag.size();
    uint64_t e = BASE;
    auto f = static_cast<digit_t>(e / (b.mag.back() + 1));
    if (f != 0) {
        r = a;
        mul_by_digit(r.mag, f);
    } else {
        r = a << (static_cast<int>(bits_in_base()));
    }
    big_integer d;
    if (f == 0)
        d = b << static_cast<int>(bits_in_base());
    else {
        d = b;
        mul_by_digit(d.mag, f);
    }
    q.mag.resize(n - m + 2);
    r.mag.push_back(0);
    big_integer dq;
    for (size_t k = n - m; k < n - m + 1; --k) {
        digit_t qt = trial(r, d, static_cast<digit_t>(k), static_cast<digit_t>(m));
        if (qt == 0)
            continue;
        dq = d;
        mul_by_digit(dq.mag, qt);
        dq.mag.push_back(0);
        while (cmp_pref(r, dq, k, m)) {
            qt -= 1;
            dq -= d;
        }
        q.mag[k] = qt;
        r -= dq << static_cast<int>(bits_in_base() * k);
    }
    if (f != 0) {
        digit_t tmp;
        div_by_digit(r.mag, f, tmp);
    } else {
        r >>= static_cast<int>(bits_in_base());
    }
    strip_leading_zeroes(q.mag);
    if (q.is_zero())
        q.sign = 0;
    return {q, r};
}

std::pair<big_integer, big_integer> big_integer::make_div(big_integer const &a, big_integer const &b) {
    if (a < b) {
        return {0, a};
    }
    if (a == b) {
        return {1, 0};
    }
    if (b.mag.size() == 1) {
        digit_t tmp;
        big_integer res = a;
        div_by_digit(res.mag, b.mag[0], tmp);
        return {res, tmp};
    }
    return divmod(a, b);
}

big_integer::digit_t
big_integer::trial(big_integer const &r, big_integer const &d, big_integer::digit_t k, big_integer::digit_t m) {
    digit_t km = k + m;
    if (r == 0)
        return 0;
    uint64_t rl =
            (static_cast<uint64_t>(r.mag[km]) << bits_in_base()) + (static_cast<uint64_t>(r.mag[km - 1]));
    uint64_t dl = static_cast<uint64_t>(d.mag[m - 1]);
    uint64_t left = rl / dl;
    return std::min(static_cast<digit_t>(left), UINT32_MAX);
}

bool big_integer::cmp_pref(big_integer const &r, big_integer const &d, digit_t k,
                           digit_t m) {
    size_t i = m, j = 0;
    while (i != j) {
        if (r.mag[i + k] != d.mag[i]) {
            j = i;
        } else {
            --i;
        }
    }
    return r.mag[i + k] < d.mag[i];
}

std::ostream &operator<<(std::ostream &s, big_integer const &a) {
    return s << to_string(a);
}
