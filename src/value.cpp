#include "value.hpp"
#include "util.hpp"
#include <sstream>
#include <string>
#include <string_view>
#include <variant>

namespace SJSON {
    JSValue::JSValue(JSNull v):
        SJSON::JSValue() {} // Call default constructor instead
    JSValue::JSValue(JSNumber v):
        src(v) {}
    // Fix constructor issues for integral constants
    JSValue::JSValue(int v):
        src(JSNumber(v)) {}
    JSValue::JSValue(long v):
        src(JSNumber(v)) {}
    JSValue::JSValue(unsigned long v):
        src(JSNumber(v)) {}
    JSValue::JSValue(JSBoolean v):
        src(v) {}
    JSValue::JSValue(JSString v):
        src(std::move(v)) {}
    JSValue::JSValue(std::string_view v):
        src(JSString(std::move(v))) {}
    JSValue::JSValue(const char* v):
        src(JSString(v)) {}
    JSValue::JSValue(JSObject v):
        src(std::move(v)) {}
    JSValue::JSValue(JSArray v):
        src(std::move(v)) {}

    JSValueType JSValue::type() const {
        return std::visit([](const auto& v) -> JSValueType {
            using V = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<V, JSNull>) return JSValueType::Null;
            if constexpr (std::is_same_v<V, JSNumber>) return JSValueType::Number;
            if constexpr (std::is_same_v<V, JSBoolean>) return JSValueType::Boolean;
            if constexpr (std::is_same_v<V, JSString>) return JSValueType::String;
            if constexpr (std::is_same_v<V, JSObject>) return JSValueType::Object;
            if constexpr (std::is_same_v<V, JSArray>) return JSValueType::Array;
        },
            src);
    }
    const char* JSValue::type_str() const noexcept {
        return type_to_string(type());
    }
    bool JSValue::is_null() const noexcept {
        return std::holds_alternative<JSNull>(src);
    }
    bool JSValue::is_number() const noexcept {
        return std::holds_alternative<JSNumber>(src);
    }
    bool JSValue::is_boolean() const noexcept {
        return std::holds_alternative<JSBoolean>(src);
    }
    bool JSValue::is_string() const noexcept {
        return std::holds_alternative<JSString>(src);
    }
    bool JSValue::is_object() const noexcept {
        return std::holds_alternative<JSObject>(src);
    }
    bool JSValue::is_array() const noexcept {
        return std::holds_alternative<JSArray>(src);
    }
    std::string JSValue::to_string(int index_length, int index) const {
        return std::visit([&](const auto& v) -> std::string {
            using V = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<V, JSNull>) return "null";
            if constexpr (std::is_same_v<V, JSNumber>) return num_to_string(v);
            if constexpr (std::is_same_v<V, JSBoolean>) return v ? "true" : "false";
            if constexpr (std::is_same_v<V, JSString>) return jsstring_escape(v);
            if constexpr (std::is_same_v<V, JSObject> || std::is_same_v<V, JSArray>) {
                // Most of this logic is the same for objects and arrays with slight differences
                constexpr bool is_obj = std::is_same_v<V, JSObject>;
                constexpr char start_char = is_obj ? '{' : '[';
                constexpr char end_char = is_obj ? '}' : ']';
                if (!v.size()) return std::string {start_char, end_char};
                const auto el_index = std::string(index * index_length, ' ');
                const auto base_index = std::string((index - 1) * index_length, ' ');
                const char* newline = index_length ? "\n" : "";
                const char* space = index_length ? " " : "";
                bool is_first = true;
                std::stringstream ss;
                for (const auto& el : v) {
                    ss << (is_first ? start_char : ',') << newline << el_index;
                    if constexpr (is_obj)
                        ss << jsstring_escape(el.first) << ":" << space << el.second.to_string(index_length, index + 1);
                    else
                        ss << el.to_string(index_length, index + 1);
                    is_first = false;
                }
                ss << newline << base_index << end_char;
                return ss.str();
            }
        },
            src);
    }

    JSNull& JSValue::null() {
        return std::get<JSNull>(src);
    }
    JSNumber& JSValue::number() {
        return std::get<JSNumber>(src);
    }
    JSBoolean& JSValue::boolean() {
        return std::get<JSBoolean>(src);
    }
    JSString& JSValue::string() {
        return std::get<JSString>(src);
    }
    JSObject& JSValue::object() {
        return std::get<JSObject>(src);
    }
    JSArray& JSValue::array() {
        return std::get<JSArray>(src);
    }
    const JSNull& JSValue::null() const {
        return std::get<JSNull>(src);
    }
    const JSNumber& JSValue::number() const {
        return std::get<JSNumber>(src);
    }
    const JSBoolean& JSValue::boolean() const {
        return std::get<JSBoolean>(src);
    }
    const JSString& JSValue::string() const {
        return std::get<JSString>(src);
    }
    const JSObject& JSValue::object() const {
        return std::get<JSObject>(src);
    }
    const JSArray& JSValue::array() const {
        return std::get<JSArray>(src);
    }
} // namespace SJSON
