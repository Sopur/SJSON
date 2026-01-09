#pragma once
#include <map>
#include <ostream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace SJSON {
    class JSValue;
    typedef std::monostate JSNull;
    typedef double JSNumber;
    typedef bool JSBoolean;
    typedef std::string JSString;
    typedef std::map<std::string, JSValue> JSObject; // Ordering is important for JavaScript for some reason
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
        // Auto conversion bullshit
        JSValue(JSNull v);
        JSValue(JSNumber v);
        JSValue(int v);
        JSValue(long v);
        JSValue(unsigned long v);
        JSValue(JSBoolean v);
        JSValue(const JSString& v);
        JSValue(const std::string_view& v);
        JSValue(const char* v);
        JSValue(const JSObject& v);
        JSValue(const JSArray& v);
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
