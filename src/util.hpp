#pragma once
#include "syntax.hpp"
#include <charconv>
#include <cstddef>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace SJSON {
    inline std::string escape(const std::string& str) {
        std::ostringstream ss;
        ss << std::quoted(str);
        return ss.str();
    }
    inline std::string unescape(const std::string& str) {
        std::string output;
        std::stringstream ss;
        ss << str;
        ss >> std::quoted(output);
        return output;
    }
    // This way unnecessary 0's aren't added
    inline std::string num_to_string(double x) {
        std::ostringstream oss;
        oss << x;
        return oss.str();
    }
    template <typename T = double>
    inline bool is_valid_number(const std::string& src) {
        T value;
        auto [ptr, ec] = std::from_chars(src.data(), src.data() + src.size(), value);
        return ec == std::errc() && ptr == src.data() + src.size();
    }
    inline bool is_valid_integer(const std::string& src) {
        return is_valid_number<int>(src);
    }

    /*
        Basically std::stack but can peak into the previous value from the top
        Also handles errors and internal errors to potentially stop some retarded attack
    */
    template <typename T>
    class VectorStack {
    private:
        std::vector<T> vec;

    public:
        inline constexpr VectorStack() = default;
        inline constexpr VectorStack(const std::vector<T>& vec):
            vec(vec) {}
        inline constexpr ~VectorStack() = default;

        inline constexpr void push(const T& v) {
            vec.push_back(v);
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
