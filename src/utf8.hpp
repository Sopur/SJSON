#pragma once
#include "syntax.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format>
#include <string>

namespace SJSON {
    inline std::string to_codepoint(uint16_t code) {
        return std::format("\\u{:04X}", code);
    }
    inline std::string unicode_escape(char32_t code) {
        // If ASCII
        if (code <= 0x7F) {
            const char c = static_cast<char>(code);
            // If has escape shortcut
            for (const auto& [k, v] : jsescape_map)
                if (v == c) return std::string {'\\', k};
            if (
                (code >= 0x00 && code <= 0x1F) || // If ASCII control
                code == 0x7F                      // If ASCII del
                ) return to_codepoint(static_cast<uint16_t>(code));
            return "";
        }
        // If unicode control code (unneeded, but in my opinion this looks more human readable)
        if (code >= 0x80 && code <= 0x9F)
            return to_codepoint(static_cast<uint16_t>(code));
        return "";
    }
    inline std::string jsstring_escape(const std::string& src) {
        std::string out = "\"";
        for (size_t i = 0; i < src.size();) {
            const auto c0 = static_cast<uint8_t>(src[i]);
            const size_t cw_remaining = src.size() - i;
            size_t cw = 0;
            std::string escape;
            if (c0 <= 0x7F) { // 1 byte
                escape = unicode_escape(static_cast<char32_t>(c0));
                cw = 1;
            } else if ((c0 & 0xE0) == 0xC0 && cw_remaining > 1) { // 2 bytes
                const auto c1 = static_cast<uint8_t>(src[i + 1]);
                char32_t cp = c0 & 0x1F;
                cp = (cp << 6) | (c1 & 0x3F);
                escape = unicode_escape(cp);
                cw = 2;
            } else if ((c0 & 0xF0) == 0xE0) { // 3 bytes
                // Escape is unneeded
                cw = 3;
            } else if ((c0 & 0xF8) == 0xF0) { // 4 bytes
                // Escape is unneeded
                cw = 4;
            } else { // Invalid byte
                cw = 1;
            }
            if (escape.size()) {
                out += escape;
            } else {
                for (size_t j = 0; j < std::min(cw, cw_remaining); j++)
                    out.push_back(src[i + j]);
            }
            i += cw;
        }
        out += "\"";
        return out;
    }
} // namespace SJSON
