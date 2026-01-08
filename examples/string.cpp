// examples/string.cpp
#include "../src/sjson.hpp"
#include "util.hpp"

int main() {
    // Parses and returns the value
    auto value = SJSON::Parse::string(input_example);

    // Log the value as pretty print and have indents be 4 spaces
    std::cout << "Value of the JSON is: " << value.to_string(4) << '\n';
    // You may `.to_string()` the value with no pretty print by giving no arguments
    return 0;
}
