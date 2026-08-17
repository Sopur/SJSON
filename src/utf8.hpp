#pragma once
#include "syntax.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <format>
#include <string>

namespace SJSON {
    typedef uint32_t JSUnicode;
    typedef uint16_t JSUnicodeEscape;
    typedef uint8_t JSUTF8;
    typedef char JSChar;
    constexpr inline JSUnicode JSUnicodeInvalid = 0xFFFD;

    enum class EscapePolicy {
        All,      // Escape every character
        NonASCII, // Escape minimum + characters not in ASCII (0x7F < x)
        NonBMP,   // Escape minimum + characters not in the BMP (0xFFFF < x)
        Invalids, // Escape minimum + characters SJSON marked as invalid/corrupted
        Minimum,  // Escape only required characters (unicode C0, escape shortcuts)
        None,     // Escape nothing and sanitize (for `UTF8::to_valid` only)
    };

    class UTF8 {
    protected:
        const EscapePolicy policy;

        inline UTF8(EscapePolicy p) noexcept: policy(p) {}

        // Is code between inclusive
        inline static bool between(JSUnicode x, JSUnicode a, JSUnicode b) noexcept {
            return x >= a && x <= b;
        }

        inline bool requires_escape(JSUnicode code) const noexcept {
            switch (policy) {
                case EscapePolicy::All:
                    return true;
                case EscapePolicy::NonASCII: {
                    if (code > 0x7F) return true;
                    break;
                }
                case EscapePolicy::NonBMP: {
                    if (code > 0xFFFF) return true;
                    break;
                }
                case EscapePolicy::Invalids: {
                    if (code == JSUnicodeInvalid) return true;
                    break;
                }
                case EscapePolicy::Minimum:
                    break;
                case EscapePolicy::None:
                    return false;
            }
            // Minimum escapes required (unicode C0)
            return between(code, 0x00, 0x1F);
        }

        template <typename... T>
        inline static std::string stringify_bytes(T... bs) {
            std::string out;
            for (auto b : {static_cast<JSChar>(bs)...})
                out.push_back(b);
            return out;
        }
        inline static std::string stringify_escape(JSUnicodeEscape code) {
            return std::format("\\u{:04X}", code);
        }
        inline std::string stringify_unicode(JSUnicode U) const {
            // If escape shortcut or ASCII
            if (between(U, 0x00, 0x7F) && policy != EscapePolicy::None) {
                const auto c = static_cast<JSChar>(U);
                for (const auto& [k, v] : jsescape_map)
                    if (v == c) return std::string {'\\', k};
            }
            if (requires_escape(U)) {
                // If code point
                if (between(U, 0x0000, 0xFFFF) && !between(U, 0xD800, 0xDFFF)) {
                    return stringify_escape(U);
                }
                // If high+low surrogate pair
                else if (between(U, 0x10000, 0x10FFFF)) {
                    auto T = U - 0x10000;
                    auto high = 0xD800 + (T >> 10);
                    auto low = 0xDC00 + (T & 0x3FF);
                    return stringify_escape(high) + stringify_escape(low);
                } else {
                    return stringify_escape(JSUnicodeInvalid);
                }
            } else {
                // 1 byte
                if (between(U, 0x00, 0x7F)) {
                    auto b1 = U;
                    return stringify_bytes(b1);
                }
                // 2 bytes
                else if (between(U, 0x0080, 0x07FF)) {
                    auto b1 = 0xC0 | (U >> 6);
                    auto b2 = 0x80 | (U & 0x3F);
                    return stringify_bytes(b1, b2);

                }
                // 3 bytes
                else if (between(U, 0x0800, 0xFFFF) && !between(U, 0xD800, 0xDFFF)) {
                    auto b1 = 0xE0 | (U >> 12);
                    auto b2 = 0x80 | ((U >> 6) & 0x3F);
                    auto b3 = 0x80 | (U & 0x3F);
                    return stringify_bytes(b1, b2, b3);
                }
                // 4 bytes
                else if (between(U, 0x10000, 0x10FFFF)) {
                    auto b1 = 0xF0 | (U >> 18);
                    auto b2 = 0x80 | ((U >> 12) & 0x3F);
                    auto b3 = 0x80 | ((U >> 6) & 0x3F);
                    auto b4 = 0x80 | (U & 0x3F);
                    return stringify_bytes(b1, b2, b3, b4);
                } else {
                    assert(U != JSUnicodeInvalid);
                    return stringify_unicode(JSUnicodeInvalid); // Serialize invalid instead
                }
            }
        }

        inline static size_t utf8_width(JSUTF8 c0) noexcept {
            if (c0 <= 0x7F)
                return 1;
            else if ((c0 & 0xE0) == 0xC0)
                return 2;
            else if ((c0 & 0xF0) == 0xE0)
                return 3;
            else if ((c0 & 0xF8) == 0xF0)
                return 4;
            else
                return 1;
        }
        inline static bool utf8_isvalid(JSUTF8 c0) noexcept {
            return between(c0, 0x00, 0x7F);
        }
        inline static bool utf8_isvalid(JSUTF8 c0, JSUTF8 c1) noexcept {
            return between(c0, 0xC2, 0xDF) && between(c1, 0x80, 0xBF);
        }
        inline static bool utf8_isvalid(JSUTF8 c0, JSUTF8 c1, JSUTF8 c2) noexcept {
            return between(c0, 0xE0, 0xEF) && between(c1, 0x80, 0xBF) && between(c2, 0x80, 0xBF);
        }
        inline static bool utf8_isvalid(JSUTF8 c0, JSUTF8 c1, JSUTF8 c2, JSUTF8 c3) noexcept {
            return between(c0, 0xF0, 0xF4) && between(c1, 0x80, 0xBF) && between(c2, 0x80, 0xBF) && between(c3, 0x80, 0xBF);
        }

        inline std::string stringify_string(const std::string& src) const {
            std::string out;
            for (size_t i = 0; i < src.size();) {
                const auto c0 = static_cast<JSUTF8>(src[i]);
                const size_t cw_remaining = src.size() - i;
                const size_t cw = utf8_width(c0);
                JSUnicode U = JSUnicodeInvalid;

                if (cw > cw_remaining) {
                    out += stringify_unicode(JSUnicodeInvalid);
                    break;
                }
                switch (cw) {
                    case 1: {
                        if (!utf8_isvalid(c0)) break;
                        U = c0;
                        break;
                    }
                    case 2: {
                        const auto c1 = static_cast<JSUTF8>(src[i + 1]);
                        if (!utf8_isvalid(c0, c1)) break;
                        U = ((c0 & 0x1F) << 6) | (c1 & 0x3F);
                        break;
                    }
                    case 3: {
                        const auto c1 = static_cast<JSUTF8>(src[i + 1]);
                        const auto c2 = static_cast<JSUTF8>(src[i + 2]);
                        if (!utf8_isvalid(c0, c1, c2)) break;
                        U = ((c0 & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
                        break;
                    }
                    case 4: {
                        const auto c1 = static_cast<JSUTF8>(src[i + 1]);
                        const auto c2 = static_cast<JSUTF8>(src[i + 2]);
                        const auto c3 = static_cast<JSUTF8>(src[i + 3]);
                        if (!utf8_isvalid(c0, c1, c2, c3)) break;
                        U = ((c0 & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
                        break;
                    }
                }
                out += stringify_unicode(U);
                i += cw;
            }
            return out;
        };

    public:
        inline static bool is_high_surrogate(JSUnicodeEscape e) noexcept {
            return between(e, 0xD800, 0xDBFF);
        }
        inline static bool is_low_surrogate(JSUnicodeEscape e) noexcept {
            return between(e, 0xDC00, 0xDFFF);
        }

        inline static std::string from_unicode(JSUnicode U) {
            return UTF8(EscapePolicy::None).stringify_unicode(U);
        }
        inline static std::string from_escape(JSUnicodeEscape esc) {
            return from_unicode(esc);
        }
        inline static std::string from_escape(JSUnicodeEscape high, JSUnicodeEscape low) {
            if (is_high_surrogate(high) && is_low_surrogate(low)) {
                return from_unicode(0x10000 + ((high - 0xD800) << 10) + (low - 0xDC00));
            } else {
                return from_unicode(JSUnicodeInvalid);
            }
        }
        inline static std::string to_valid(const std::string& src, EscapePolicy policy = EscapePolicy::None) {
            return UTF8(policy).stringify_string(src);
        }
        inline static std::string stringify(const std::string& src, EscapePolicy policy = EscapePolicy::Minimum) {
            if (policy == EscapePolicy::None)
                throw sjson_parse_error("Escape policy 'none' for stringifying UTF8 produces malformed json");
            return "\"" + to_valid(src, policy) + "\"";
        }
    };
} // namespace SJSON
