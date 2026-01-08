// examples/util.hpp
#include "../src/sjson.hpp"

// Examples
inline const char* input_example =
    R"(
        {
            "test": [
                1,
                2,
                {
                    "a": 5
                }
            ]
        }
    )";
inline const char* error_input_example =
    R"(
        {
            "test": [
                1,
                2,
                {
                    "a": 5
                }
    )"; // Unexpected end of input error
inline SJSON::JSONStream stream_example([i = 0]() mutable -> std::string {
    // Example stream of a worse case scenario (one character at a time)
    if (input_example[i] == '\0') return ""; // A string of length 0 represents end of input
    return std::string {input_example[i++]}; // Return one character at a time
});
inline SJSON::JSONStream error_stream_example([i = 0]() mutable -> std::string {
    if (error_input_example[i] == '\0') return "";
    return std::string {error_input_example[i++]};
});
