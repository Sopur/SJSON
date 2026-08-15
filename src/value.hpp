#pragma once
#include <concepts>
#include <map>
#include <ostream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace SJSON {
    class JSValue;
    typedef std::monostate JSNull;
#ifdef SJSON_LONG_DOUBLE
    typedef long double JSNumber;
#else
    typedef double JSNumber; // Spec notes that double/float64 is a common default for numbers in JSON parsers
#endif
    typedef bool JSBoolean;
    typedef std::string JSString;
    typedef std::map<std::string, JSValue> JSObject;
    typedef std::vector<JSValue> JSArray;
    using JSValueData = std::variant<
        JSNull,
        JSNumber,
        JSBoolean,
        JSString,
        JSObject,
        JSArray>;

    enum class JSValueType {
        Null,
        Number,
        Boolean,
        String,
        Object,
        Array,
    };

    class JSValue {
    private:
        JSValueData src;

    public:
        JSValue() = default;
        /* Auto conversion bullshit */
        JSValue(JSNull);
        // For all diff types of numbers (int, float, size_t)
        template <typename T>
            requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
        inline JSValue(T v):
            src(static_cast<JSNumber>(v)) {}
        JSValue(JSBoolean v);
        // For all diff types of strings (string, const char*, string_view)
        template <typename T>
            requires(std::constructible_from<JSString, T &&>)
        inline JSValue(T v):
            src(JSString(std::move(v))) {}
        JSValue(JSObject v);
        JSValue(JSArray v);
        ~JSValue() = default;

        // Non-type specific
        JSValueType type() const;
        const char* type_str() const noexcept;
        bool is_null() const noexcept;
        bool is_number() const noexcept;
        bool is_boolean() const noexcept;
        bool is_string() const noexcept;
        bool is_object() const noexcept;
        bool is_array() const noexcept;
        std::string to_string(int index_length = 0, int index = 1) const;

        // Type specific
        JSNull& null();
        JSNumber& number();
        JSBoolean& boolean();
        JSString& string();
        JSObject& object();
        JSArray& array();
        const JSNull& null() const;
        const JSNumber& number() const;
        const JSBoolean& boolean() const;
        const JSString& string() const;
        const JSObject& object() const;
        const JSArray& array() const;

        // Debug shit
        inline friend std::ostream& operator<<(std::ostream& out, const JSValue& v) {
            return out << v.to_string(4);
        }
    };

    // Static debug shit
    inline constexpr const char* type_to_string(JSValueType type) noexcept {
        switch (type) {
            case JSValueType::Null: return "Null";
            case JSValueType::Number: return "Number";
            case JSValueType::Boolean: return "Boolean";
            case JSValueType::String: return "String";
            case JSValueType::Object: return "Object";
            case JSValueType::Array: return "Array";
        }
        return "Error";
    }
} // namespace SJSON
