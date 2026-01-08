#pragma once
#include "syntax.hpp"
#include "value.hpp"
#include <string>

namespace SJSON {
    enum class TokenType {
        Unresolved,
        Operator,
        Keyword,
        Number,
        String
    };

    class Token {
    public:
        TokenType type;
        std::string src;

        Token();
        Token(const Token& token);
        ~Token() = default;

        // Parsing shit
        bool is_operator() const noexcept;
        bool is_unresolved() const noexcept;
        bool is_value() const noexcept;
        void reset();
        void push(char c);
        bool is_terminating() const; // For end of file
        bool is_terminating(char c) const;
        Token copy() const;

        // Value shit
        Operators to_operator() const;
        Keywords to_keyword() const;
        JSNumber to_number() const;
        JSString to_string() const;
        JSValue to_value() const;

        // Debug shit
        const char* type_to_str() const;
        std::string to_debug() const;
    };
} // namespace SJSON