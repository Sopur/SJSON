#pragma once
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace SJSON {
    class sjson_parse_error : public std::runtime_error {
    public:
        explicit inline sjson_parse_error(const std::string& msg):
            std::runtime_error(msg) {}
        static inline sjson_parse_error unexpected_data() {
            return sjson_parse_error("Received more data in stream after finish");
        }
        static inline sjson_parse_error unexpected_character(char c) {
            return sjson_parse_error("Read unexpected character '" + std::string {c, '\''});
        }
        static inline sjson_parse_error invalid_token(const char* type, const std::string& src) {
            return sjson_parse_error("Invalid " + std::string(type) + " of value '" + src + "'");
        }
        static inline sjson_parse_error unexpected_token(const std::string& src) {
            return sjson_parse_error("Unexpected token of value '" + src + "'");
        }
        static inline sjson_parse_error unexpected_eof() {
            return sjson_parse_error("Unexpected end of input");
        }
        static inline sjson_parse_error schema_mismatch() {
            return sjson_parse_error("Input doesn't match the schema");
        }
    };
    class sjson_internal_parse_error : public std::runtime_error {
    public:
        explicit inline sjson_internal_parse_error(const std::string& msg):
            std::runtime_error(msg) {}
        static inline sjson_internal_parse_error invalid_continued_read() {
            return sjson_internal_parse_error("Continued reading for a token that's terminated");
        }
        static inline sjson_internal_parse_error read_end_of_chunk() {
            return sjson_internal_parse_error("Continued reading when there was no more to read");
        }
        static inline sjson_internal_parse_error invalid_token_eval() {
            return sjson_internal_parse_error("Attempted to convert a token without value into a value");
        }
        static inline sjson_internal_parse_error invalid_reference_state() {
            return sjson_internal_parse_error("Attempted to handle the next token for data type that can't handle one");
        }
        static inline sjson_internal_parse_error invalid_token_type(const std::string& context) {
            return sjson_internal_parse_error("Encountered an invalid type in " + context);
        }
        static inline sjson_internal_parse_error new_chunk_before_finish() {
            return sjson_internal_parse_error("Attempted to set a new chunk before the previous chunk was done reading");
        }
        static inline sjson_internal_parse_error vector_stack(const std::string& msg) {
            return sjson_internal_parse_error("Internal vector-stack encountered an error: " + msg);
        }
    };

    enum class Operators {
        Comma,
        Colon,
        ArrayStart,
        ArrayEnd,
        ObjectStart,
        ObjectEnd,
    };
    enum class Keywords {
        Null,
        True,
        False,
    };
    inline const std::unordered_map<char, Operators> operator_map {
        {',', Operators::Comma},
        {':', Operators::Colon},
        {'[', Operators::ArrayStart},
        {']', Operators::ArrayEnd},
        {'{', Operators::ObjectStart},
        {'}', Operators::ObjectEnd},
    };
    inline const std::unordered_map<std::string_view, Keywords> keyword_map {
        {"null", Keywords::Null},
        {"true", Keywords::True},
        {"false", Keywords::False},
    };
    // Rip performance
    inline const std::unordered_set<char> decimals_set {
        '-',
        '0',
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
    };
    inline const std::unordered_set<char> special_numbers_set {
        '.',
        'e',
        '+',
    };
    inline const std::unordered_set<char> letters_set {
        'q',
        'w',
        'e',
        'r',
        't',
        'y',
        'u',
        'i',
        'o',
        'p',
        'a',
        's',
        'd',
        'f',
        'g',
        'h',
        'j',
        'k',
        'l',
        'z',
        'x',
        'c',
        'v',
        'b',
        'n',
        'm',
        'Q',
        'W',
        'E',
        'R',
        'T',
        'Y',
        'U',
        'I',
        'O',
        'P',
        'A',
        'S',
        'D',
        'F',
        'G',
        'H',
        'J',
        'K',
        'L',
        'Z',
        'X',
        'C',
        'V',
        'B',
        'N',
        'M',
    };
    inline const std::unordered_set<char> whitespace_set {
        ' ',
        '\n',
        '\t',
    };
} // namespace SJSON
