// examples/error.cpp
#include "../src/sjson.hpp"
#include "util.hpp"

int main() {
    try {
        // Tries to parse the string and return a value
        auto value = SJSON::Parse::string(error_input_example);
    } catch (SJSON::sjson_parse_error err) {
        // Catch the input error
        std::cout << "Error with the input during parse:\n"
                  << "\tsjson_parse_error.what(): " << err.what() << '\n';
    }
    return 0;
}
