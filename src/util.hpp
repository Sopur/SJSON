#pragma once
#include "syntax.hpp"
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <format>
#include <sstream>
#include <string>
#include <vector>

namespace SJSON {
    inline std::string jschar_escape(char c) {
        // If a escape sequence is needed for ts char
        for (const auto& [k, v] : escape_map)
            if (v == c) return std::string {'\\', k};
        // If this char is displayable
        if (c >= 32 && c <= 126) return std::string {c};
        return ""; // Can't escape cuz this char needs a multiescape
    }
    inline std::string jschar_multiescape(char a, char b) {
        return std::format("\\u{:02x}{:02x}", static_cast<uint8_t>(a), static_cast<uint8_t>(b));
    }
    inline std::string jschar_multiescape(char b) {
        return jschar_multiescape(0, b);
    }
    inline std::string jsstring_escape(const std::string& src) {
        std::string out = "\"";
        for (size_t i = 0; i < src.size();) {
            if (auto esc = jschar_escape(src[i]); esc.size()) {
                // If single char escape
                out += esc;
                i++;
            } else if (i + 1 == src.size()) {
                // If single char multiescape
                out += jschar_multiescape(src[i]);
                i++;
            } else {
                // if normal multiescape
                out += jschar_multiescape(src[i], src[i + 1]);
                i += 2;
            }
        }
        out += "\"";
        return out;
    }

    // This way unnecessary 0's aren't added
    inline std::string num_to_string(double x) {
        std::ostringstream oss;
        oss << x;
        return oss.str();
    }
    inline bool is_valid_number(const std::string& src) {
        double value;
        auto [ptr, ec] = std::from_chars(src.data(), src.data() + src.size(), value);
        return ec == std::errc() && ptr == src.data() + src.size();
    }
    inline bool is_valid_integer(const std::string& src, int base = 10) {
        uint64_t value;
        auto [ptr, ec] = std::from_chars(src.data(), src.data() + src.size(), value, base);
        return ec == std::errc() && ptr == src.data() + src.size();
    }

    inline std::string hexToUTF8(const std::string_view& hex) {
        uint16_t value;
        std::from_chars(hex.data(), hex.data() + hex.size(), value, 16);
        return std::string(reinterpret_cast<char*>(&value), 2);
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
