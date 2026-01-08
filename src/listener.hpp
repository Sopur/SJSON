#pragma once
#include "syntax.hpp"
#include "util.hpp"
#include "value.hpp"
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>

namespace SJSON {
    typedef std::move_only_function<void(const JSValue& value)> JSONCallback;

    class JSPath {
    private:
        bool drop_generics = false;
        VectorStack<std::string> parts;
        std::unordered_map<std::string, JSONCallback> listeners;

        inline static bool needs_escape(const std::string& part) {
            for (char c : part) {
                if (!letters_set.contains(c)) return true;
            }
            return false;
        }
        inline static bool is_index(const std::string& part) {
            return is_valid_integer(part);
        }
        inline bool call_if(const std::string& key, const JSValue& value) {
            if (listeners.contains(key)) {
                listeners.at(key)(value);
                return true;
            }
            return false;
        }

    public:
        inline JSPath(bool drop_generics):
            drop_generics(drop_generics),
            parts({"JSON"}) {} // This is only here to match with the references stack
        inline ~JSPath() = default;

        inline constexpr void push(const std::string& part) {
            parts.push(part);
        }
        inline void push(size_t part) {
            parts.push(std::to_string(part));
        }
        // Not constexpr cuz it has an overload that isn't constexpr
        inline bool pop() {
            parts.pop();
            return false;
        }
        // Shorthand for pop and call
        inline bool pop(const JSValue& value) {
            auto drop = call(value);
            parts.pop();
            return drop;
        }
        inline constexpr size_t length() const noexcept {
            return parts.size();
        }
        inline std::string to_string(bool is_generic = false) const {
            std::string out;
            for (size_t i = 1; i < length(); i++) {
                auto& part = parts[i];
                if (needs_escape(part)) {
                    if (is_index(part)) {
                        if (is_generic)
                            out += "[]";
                        else
                            out += "[" + part + "]";
                    } else {
                        out += "[" + escape(part) + "]";
                    }
                } else {
                    if (i != 1) out += ".";
                    out += part;
                }
            }
            return out;
        }
        inline void listen(const std::string& path, JSONCallback&& cb) {
            if (listeners.contains(path)) return; // Disallow multiple listeners per path
            listeners[path] = std::move(cb);
        }
        inline bool call(const JSValue& value) {
            if (!listeners.size()) return false;
            if (call_if(to_string(false), value)) return false;
            return call_if(to_string(true), value) && drop_generics; // Only drop generics to stop drop loops
        }
        inline constexpr std::string& operator[](size_t i) {
            return parts[i];
        }
        inline constexpr const std::string& operator[](size_t i) const {
            return parts[i];
        }
        // Not used but useful
        inline constexpr bool operator==(const JSPath& a) const {
            if (length() != a.length()) return false;
            for (size_t i = 0; i < length(); i++) {
                if (parts[i] != a.parts[i]) return false;
            }
            return true;
        }
        inline constexpr bool operator!=(const JSPath& a) const {
            return !((*this) == a);
        }

        // Debug
        inline friend std::ostream& operator<<(std::ostream& out, const JSPath& path) {
            return out << path.to_string();
        }
    };
} // namespace SJSON
