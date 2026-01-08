// examples/listeners.cpp
#include "../src/sjson.hpp"
#include "util.hpp"

int main() {
    // Define the stream; no parsing is done until told to
    SJSON::Parse json(std::move(stream_example), true); // Boolean here tells the parser if it should drop values sent to generic listeners
    json
        .listen("test[]", [](const SJSON::JSValue& value) {
            std::cout << "Generic value in 'test': " << value.to_string(4) << '\n';
        })
        .listen("test", [](const SJSON::JSValue& value) {
            // Array will be empty due to the generic callback dropping the elements
            std::cout << "Value of 'test': " << value.to_string(4) << '\n';
        });

    // Start parsing one iteration a time; you could also use the `.all()` method to parse until the stream is finished
    for (int i = 0; json.next(); i++) {
        std::cout << "JSON Stream iteration #" << i << '\n';
    }

    // Result should be of value `{"test":[]}` because dropped values aren't stored
    std::cout << "Finished parsed value: " << json.to_string(4) << '\n';
    return 0;
}
