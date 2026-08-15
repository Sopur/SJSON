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

    // Send a 2nd chunk to the stream
    stream.recv(R"(
            3,
            4
        ]
    )");

    // Result should be of value `[1,2,3,4]`
    std::cout << "Value after chunk 2 is: " << stream.to_string() << '\n';
    return 0;
}
