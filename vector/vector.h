#ifndef MY_VECTOR
#define MY_VECTOR

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <variant>

struct my_vector {
public:

    my_vector();

    explicit my_vector(size_t size);
    my_vector(my_vector const& other);

    ~my_vector();

    size_t size() const;
    void resize(size_t new_size);

    uint32_t& operator[](size_t index);
    uint32_t const& operator[](size_t index) const;

    my_vector& operator=(my_vector const& other);
    friend bool operator==(const my_vector& a, const my_vector& b);

    void push_back(uint32_t x);
    uint32_t back() const;
    void pop_back();

    void swap(my_vector& other) noexcept;

private:
    const size_t MULTIPLIER = 2;
    static const size_t SMALL_CAP = 4;

    size_t size_;
    bool is_big;

    struct big_data {
        std::shared_ptr<uint32_t[]> ptr;
        size_t capacity;

        big_data(uint32_t* p, size_t capacity) : ptr(p), capacity(capacity) {}

        big_data(const big_data& other) = default;
    };

    union any_data {
        uint32_t small[SMALL_CAP]{};
        big_data big;
        any_data() {}
        ~any_data() {}
    } data;

    uint32_t* real_data;

    size_t get_capacity();
    void swap_different(any_data &big, any_data &small) noexcept;
    void ensure_capacity(size_t capacity);

};

#endif