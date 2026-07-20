// examples/recv.cpp
#include "../src/sjson.hpp"
#include "util.hpp"

int main() {
    // Define parser with no inital input
    SJSON::Parse stream;

    // Send 1 chunk worth of data; enough for the parser to create an array with 2 numbers
    stream.recv(R"(
        [
            1,
            2,
    )");

    // Result should be of value `[1,2]` because that is all that has been received
    std::cout << "Value after chunk 1 is: " << stream.to_string() << '\n';

    // You may optionally interact with the parsed data during parsing
    if (stream.value.is_array())
        stream.value.array().push_back(3);

    // Send a 2nd chunk to the stream after manually modifying the JSON
    stream.recv(R"(
            4,
            5
        ]
    )");

    // Result should be of value `[1,2,3,4,5]` because a `3` was pushed before the 2nd chunk was received
    std::cout << "Value after chunk 2 is: " << stream.to_string() << '\n';
    return 0;
}
