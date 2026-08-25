#pragma once
#include "syntax.hpp"
#include <charconv>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace SJSON {
    // Needed cuz default behavior is locale sensitive and low precision
    template <typename T>
    inline std::string num_to_string(T x) {
        if constexpr (std::is_floating_point_v<T>) {
            if (!std::isfinite(x)) {
                x = 0;
            }
        }
        std::ostringstream oss;
        oss.imbue(std::locale::classic());
        oss << std::setprecision(std::numeric_limits<T>::digits10) << x;
        return oss.str();
    }
    template <typename T, int Base = 10>
    inline T string_to_num(const std::string& src) {
        T value;
        if constexpr (std::is_floating_point_v<T>) {
            static_assert(Base == 10, "float not base 10");
            std::from_chars(src.data(), src.data() + src.size(), value);
        } else {
            std::from_chars(src.data(), src.data() + src.size(), value, Base);
        }
        return value;
    }

    inline bool is_valid_number(const std::string& src, const std::from_chars_result& res) {
        return res.ec == std::errc() && res.ptr == src.data() + src.size();
    }
    template <typename T, int Base = 10>
    inline bool is_valid_number(const std::string& src) {
        T value = 0;
        if constexpr (std::is_floating_point_v<T>) {
            static_assert(Base == 10, "float not base 10");
            return is_valid_number(src, std::from_chars(src.data(), src.data() + src.size(), value));
        } else {
            return is_valid_number(src, std::from_chars(src.data(), src.data() + src.size(), value, Base));
        }
    }

    /*
        Basically std::stack but can peak into the previous value from the top
        Also handles errors and internal errors to potentially stop some retarded attack
    */
    template <typename T>
    class VectorStack {
    protected:
        std::vector<T> vec;

    public:
        VectorStack() = default;
        inline constexpr VectorStack(std::vector<T> vec):
            vec(std::move(vec)) {}
        ~VectorStack() = default;

        inline constexpr void push(T v) {
            vec.push_back(std::move(v));
        }
        inline constexpr void pop() {
            if (empty()) throw sjson_internal_parse_error::vector_stack("Call to pop() while empty");
            vec.pop_back();
        }
        inline constexpr size_t size() const noexcept {
            return vec.size();
        }
        inline constexpr bool empty() const noexcept {
            return vec.empty();
        }
        inline constexpr bool has(size_t i) const noexcept {
            return i < size();
        }
        inline constexpr bool has_top() const noexcept {
            return size() >= 1;
        }
        inline constexpr bool has_prev() const noexcept {
            return size() >= 2;
        }
        inline constexpr T& top() {
            if (!has_top()) throw sjson_internal_parse_error::vector_stack("Call to top() while empty");
            return vec[size() - 1];
        }
        inline constexpr const T& top() const {
            if (!has_top()) throw sjson_internal_parse_error::vector_stack("Call to top() while empty");
            return vec[size() - 1];
        }
        inline constexpr T& prev() {
            if (!has_prev()) throw sjson_internal_parse_error::vector_stack("Call to prev() while having no previous element");
            return vec[size() - 2];
        }
        inline constexpr const T& prev() const {
            if (!has_prev()) throw sjson_internal_parse_error::vector_stack("Call to prev() while having no previous element");
            return vec[size() - 2];
        }
        // std::vector has .at() which throws errors but I want to use my custom errors
        inline constexpr T& at(size_t i) {
            if (!has(i)) throw sjson_internal_parse_error::vector_stack("Call to at() with an out of bounds index");
            return vec[i];
        }
        inline constexpr const T& at(size_t i) const {
            if (!has(i)) throw sjson_internal_parse_error::vector_stack("Call to at() with an out of bounds index");
            return vec[i];
        }
        inline constexpr T& operator[](size_t i) {
            return at(i);
        }
        inline constexpr const T& operator[](size_t i) const {
            return at(i);
        }
    };
} // namespace SJSON
