#include "vector.h"

uint32_t *copy(uint32_t *data, size_t cap, size_t size) {
    auto *copy = new uint32_t[cap];
    std::memcpy(copy, data, size * sizeof(uint32_t));
    return copy;
}

void my_vector::ensure_capacity(size_t capacity) {
    if (capacity > get_capacity() && capacity > SMALL_CAP) {
        capacity = std::max(capacity, get_capacity() * MULTIPLIER);
        if (!is_big) {
            new(&data.big) big_data(copy(real_data, capacity, size_), capacity);
            is_big = true;
        } else {
            data.big.ptr.reset(copy(real_data, capacity, size_));
            data.big.capacity = capacity;
        }
        real_data = data.big.ptr.get();
    }
}


my_vector::my_vector() : size_(0), is_big(false), real_data(data.small) {}

my_vector::my_vector(size_t size) : my_vector() {
    ensure_capacity(size);
    std::memset(real_data, 0, size * sizeof(uint32_t));
    size_ = size;
}

my_vector::my_vector(my_vector const &other) : my_vector() {
    if (other.is_big) {
        is_big = true;
        new(&data.big) big_data(other.data.big);
        real_data = data.big.ptr.get();
    } else {
        is_big = false;
        std::memcpy(data.small, other.data.small, SMALL_CAP * sizeof(uint32_t));
        real_data = data.small;
    }
    size_ = other.size_;
}

my_vector::~my_vector() {
    if (is_big) {
        data.big.~big_data();
    }
}

void my_vector::resize(size_t new_size) {
    ensure_capacity(new_size);
    if (new_size > size_)
        std::memset(real_data + size_, 0, (new_size - size_) * sizeof(uint32_t));
    size_ = new_size;
}

size_t my_vector::size() const { return size_; }


uint32_t &my_vector::operator[](size_t index) {
    if (is_big && !data.big.ptr.unique()) {
        data.big.ptr.reset(copy(real_data, data.big.capacity, size_));
        real_data = data.big.ptr.get();
    }
    return real_data[index];
}

uint32_t const &my_vector::operator[](size_t index) const {
    return real_data[index];
}

void my_vector::pop_back() { size_--; }

void my_vector::push_back(uint32_t x) {
    if (get_capacity() < size_ + 1) {
        ensure_capacity(get_capacity() * MULTIPLIER);
    }
    real_data[size_++] = x;
}

uint32_t my_vector::back() const { return real_data[size_ - 1]; }

bool operator==(const my_vector &a, const my_vector &b) {
    return (a.size_ == b.size_) && (std::memcmp(a.real_data, b.real_data, a.size_ * sizeof(uint32_t)) == 0);
}

void my_vector::swap_different(my_vector::any_data &big,
                               my_vector::any_data &small) noexcept {
    uint32_t temp[SMALL_CAP];
    std::memcpy(temp, small.small, SMALL_CAP * sizeof(uint32_t));
    new(&small.big) big_data(big.big);
    big.big.~big_data();
    std::memcpy(big.small, temp, SMALL_CAP * sizeof(uint32_t));
}

void my_vector::swap(my_vector &other) noexcept {
    if (is_big && other.is_big) {
        std::swap(data.big, other.data.big);
        real_data = data.big.ptr.get();
        other.real_data = other.data.big.ptr.get();
    } else if (!is_big && !other.is_big) {
        std::swap(data.small, other.data.small);
    } else if (is_big) {
        swap_different(data, other.data);
        real_data = data.small;
        other.real_data = other.data.big.ptr.get();
    } else {
        swap_different(other.data, data);
        real_data = data.big.ptr.get();
        other.real_data = other.data.small;
    }
    std::swap(size_, other.size_);
    std::swap(is_big, other.is_big);
}

my_vector &my_vector::operator=(my_vector const &other) {
    my_vector temp(other);
    swap(temp);
    return *this;
}

size_t my_vector::get_capacity() {
    if (is_big) {
        return data.big.capacity;
    } else {
        return SMALL_CAP;
    }
}

