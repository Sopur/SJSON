#include "token.hpp"
#include "syntax.hpp"
#include "util.hpp"

namespace SJSON {
    Token::Token() {
        reset();
    }
    Token::Token(const Token& token):
        type(token.type),
        src(token.src) {}

    // Parsing shit
    bool Token::is_operator() const noexcept {
        return type == TokenType::Operator;
    }
    bool Token::is_unresolved() const noexcept {
        return type == TokenType::Unresolved;
    }
    bool Token::is_value() const noexcept {
        return !is_operator() && !is_unresolved();
    }
    void Token::reset() {
        type = TokenType::Unresolved;
        src = "";
    }
    void Token::push(char c) {
        if (is_terminating(c))
            throw sjson_internal_parse_error::invalid_continued_read();
        switch (type) {
            case TokenType::Unresolved: {
                if (whitespace_set.contains(c)) return; // Only ignore whitespace if it doesn't matter to the token
                src += c;
                if (operator_map.count(c)) {
                    type = TokenType::Operator;
                } else if (decimals_set.contains(c)) {
                    type = TokenType::Number;
                } else if (letters_set.contains(c)) {
                    type = TokenType::Keyword;
                } else if (c == '"') {
                    type = TokenType::String;
                } else {
                    throw sjson_parse_error::unexpected_character(c);
                }
                return;
            }
            case TokenType::Operator:
            case TokenType::Keyword:
            case TokenType::Number:
            case TokenType::String: {
                src += c;
                return;
            }
        }
        throw sjson_internal_parse_error::invalid_token_type("token.push(char)");
    }
    // For end of file
    bool Token::is_terminating() const {
        switch (type) {
            case TokenType::Unresolved: return false;
            case TokenType::Operator: return true;
            case TokenType::Keyword: return true;
            case TokenType::Number: return true;
            case TokenType::String: return true;
        }
        throw sjson_internal_parse_error::invalid_token_type("token.is_terminating()");
    }
    bool Token::is_terminating(char c) const {
        switch (type) {
            case TokenType::Unresolved: return false;
            case TokenType::Operator: return true;
            case TokenType::Keyword: return !letters_set.contains(c) || whitespace_set.contains(c);
            case TokenType::Number: return (!decimals_set.contains(c) && !special_numbers_set.contains(c)) || whitespace_set.contains(c);
            case TokenType::String: return src.size() > 1 && src.ends_with('"') && !src.ends_with("\\\""); // String termination doesn't depend on the proceeding character
        }
        throw sjson_internal_parse_error::invalid_token_type("token.is_terminating(char)");
    }
    Token Token::copy() const {
        return Token(*this);
    }

    // Value shit
    Operators Token::to_operator() const {
        if (src.size() != 1 || !operator_map.count(src[0])) // All operators are one character
            throw sjson_parse_error::invalid_token("operator", src);
        return operator_map.at(src[0]);
    }
    Keywords Token::to_keyword() const {
        if (!keyword_map.count(src))
            throw sjson_parse_error::invalid_token("keyword", src);
        return keyword_map.at(src);
    }
    JSNumber Token::to_number() const {
        if (!is_valid_number(src))
            throw sjson_parse_error::invalid_token("number", src);
        return std::stod(src);
    }
    JSString Token::to_string() const {
        if (src.size() <= 1)
            throw sjson_parse_error::unexpected_eof();
        if (!src.ends_with('"'))
            throw sjson_parse_error::unexpected_eof();
        return unescape(src);
    }
    JSValue Token::to_value() const {
        switch (type) {
            case TokenType::Unresolved: throw sjson_internal_parse_error::invalid_token_eval();
            case TokenType::Operator: throw sjson_internal_parse_error::invalid_token_eval();
            case TokenType::Keyword: {
                switch (to_keyword()) {
                    case Keywords::Null: return JSValue();
                    case Keywords::True: return JSValue(true);
                    case Keywords::False: return JSValue(false);
                }
                break;
            }
            case TokenType::Number: return JSValue(to_number());
            case TokenType::String: return JSValue(to_string());
        }
        throw sjson_internal_parse_error::invalid_token_type("token.to_value()");
    }

    // Debug shit
    const char* Token::type_to_str() const {
        switch (type) {
            case TokenType::Unresolved: return "Unresolved";
            case TokenType::Operator: return "Operator";
            case TokenType::Keyword: return "Keyword";
            case TokenType::Number: return "Number";
            case TokenType::String: return "String";
        }
        throw sjson_internal_parse_error::invalid_token_type("token.type_to_str()");
    }
    std::string Token::to_debug() const {
        return std::string(type_to_str()) + "<" + src + ">";
    }
} // namespace SJSON
