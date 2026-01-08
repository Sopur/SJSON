// examples/stream.cpp
#include "../src/sjson.hpp"
#include "util.hpp"

int main() {
    // Parse until the stream is finished and return the value
    auto value = SJSON::Parse::stream(std::move(stream_example));

    // Log the value as pretty print and have indents be 4 spaces
    std::cout << "Finished parsed value: " << value.to_string(4) << '\n';
    // You may `.to_string()` the value with no pretty print by giving no arguments
    return 0;
}
