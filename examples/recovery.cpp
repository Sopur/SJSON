// examples/recovery.cpp
#include "../src/sjson.hpp"
#include "util.hpp"

int main() {
    /*
        This example is for streams only because
        a string argument is parsed inside the constructor
    */
    // Define the stream without parsing
    SJSON::Parse stream(std::move(error_stream_example));
    try {
        // Try to parse the entire stream
        stream.all();
    } catch (SJSON::sjson_parse_error err) {
        // Catch the input error
        std::cout << "Error with the input stream during parse:\n"
                  << "\tsjson_parse_error.what(): " << err.what() << '\n';
    }

    // Log what was parsed before the error as pretty print with indents of 4 spaces
    std::cout << "Data parsed before error is: " << stream.to_string(4) << '\n';
    return 0;
}
