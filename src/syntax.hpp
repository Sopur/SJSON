#pragma once
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace SJSON {
    class sjson_parse_error : public std::runtime_error {
    public:
        inline sjson_parse_error(std::string msg):
            std::runtime_error(std::move(msg)) {}
        inline static sjson_parse_error unexpected_data() {
            return sjson_parse_error("Received more data in stream after finish");
        }
        inline static sjson_parse_error unexpected_character(char c) {
            return sjson_parse_error("Read unexpected character '" + std::string {c, '\''});
        }
        inline static sjson_parse_error invalid_token(const std::string& type, const std::string& src) {
            return sjson_parse_error("Invalid " + type + " of value '" + src + "'");
        }
        inline static sjson_parse_error invalid_escape(const std::string& seq) {
            return sjson_parse_error("Invalid escape sequence '" + seq + "' in string");
        }
        inline static sjson_parse_error unexpected_token(const std::string& src) {
            return sjson_parse_error("Unexpected token of value '" + src + "'");
        }
        inline static sjson_parse_error unexpected_eof() {
            return sjson_parse_error("Unexpected end of input");
        }
        inline static sjson_parse_error schema_mismatch() {
            return sjson_parse_error("Input doesn't match the schema");
        }
    };
    class sjson_internal_parse_error : public std::runtime_error {
    public:
        inline sjson_internal_parse_error(std::string msg):
            std::runtime_error(std::move(msg)) {}
        inline static sjson_internal_parse_error invalid_continued_read() {
            return sjson_internal_parse_error("Continued reading for a token that's terminated");
        }
        inline static sjson_internal_parse_error read_end_of_chunk() {
            return sjson_internal_parse_error("Continued reading when there was no more to read");
        }
        inline static sjson_internal_parse_error invalid_token_eval() {
            return sjson_internal_parse_error("Attempted to convert a token without value into a value");
        }
        inline static sjson_internal_parse_error invalid_reference_state() {
            return sjson_internal_parse_error("Attempted to handle the next token for data type that can't handle one");
        }
        inline static sjson_internal_parse_error invalid_token_type(const std::string& context) {
            return sjson_internal_parse_error("Encountered an invalid type in " + context);
        }
        inline static sjson_internal_parse_error invalid_escape_state(const std::string& context) {
            return sjson_internal_parse_error("Encountered an invalid escape state in " + context);
        }
        inline static sjson_internal_parse_error new_chunk_before_finish() {
            return sjson_internal_parse_error("Attempted to set a new chunk before the previous chunk was done reading");
        }
        inline static sjson_internal_parse_error vector_stack(const std::string& msg) {
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

    // All operators are single-character
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

    // Things numbers could start with
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
    // Things numbers could include
    inline const std::unordered_set<char> special_numbers_set {
        '.',
        'e',
        '+',
    };

    // Keywords contain only these letters
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

    // Only these are included in the spec
    inline const std::unordered_set<char> whitespace_set {
        ' ',
        '\t',
        '\n',
        '\r',
    };

    // Only these are explicitly listed as special characters in the spec (any others will be ignored)
    enum class EscapeState {
        None,
        End,
        Escaping,
        Sequence,
    };
    inline constexpr char string_char = '"';
    inline constexpr char escape_char = '\\';
    inline constexpr char sequence_escape_char = 'u'; // Follows the format uXXXX -> U+XXXX
    inline constexpr int sequence_escape_len = 4;
    inline const std::unordered_map<char, char> escape_map {
        {'"', '"'},
        {'\\', '\\'},
        {'/', '/'},
        {'b', '\b'},
        {'f', '\f'},
        {'n', '\n'},
        {'r', '\r'},
        {'t', '\t'},
    };
} // namespace SJSON
