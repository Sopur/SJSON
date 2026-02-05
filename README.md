# SJSON++

**SJSON** (**Streamed JSON**) is a modern **memory-optimized** C++23 JSON parsing library that supports **parsing JSON sent in a stream/chunks**, **forwarding & dropping values** sent to listeners in a JSON stream, malformed **syntax auto-correction**, **error recovery**, and regular **memory-optimized parsing** of a JSON string.

## Features

### 1. JSON Stream Parser

- Parses JSON sent in **individual chunks**.
- Use this to **avoid storing a massive string then spending lots of time parsing it** all at once.
- This **saves memory** because you will no longer have to **store the entire JSON string; only the individual chunks** need to be in memory at any given time.

### 2. Memory Optimized

- This JSON parser **prioritizes minimizing memory usage**.
- The parser is **non-recursive** and **discards unneeded tokens/syntax** to save memory.
- This parser is perfect for **dealing with massive amounts of data** at a single time, or for **low-spec machines**.

### 3. Stream Listeners & Data Discarding

- This JSON parser supports using **listeners** to run a callback **for specific values, properties, or indexes** after that specific value has been **fully parsed**.
- The parser supports **listening generically**, allowing a callback to run, for example, on every element of an array.
- The parser supports **deallocating/dropping values** sent to **generic listeners**, allowing you to have only **one value** of (for example an array) **stored at a time** during parse. This can **massively save memory and increase performance** on JSON that contains massive arrays.

### 4. Malformed Syntax & Error Recovery

- This parser can **parse JSON with extremely minimal** (and technically invalid) **syntax** due to the **discarding of unneeded tokens**. This allows you to **parse much smaller JSON strings/streams**.
- **For example**, the parser will parse `{"a"1"b"2}` **as** `{"a": 1, "b": 2}`.
- This parser supports **error recovery**, allowing you to **read data parsed before an unrecoverable syntax error** was encountered.
- You may use **error recovery** to, for example, **parse JSON with no closing brackets** and still **parsing it perfectly**.

### 5. Regular JSON Parsing

- This parser also supports **all of the above** with **normal strings**, so you may have **all the benefits without using a stream**.
- This parser is fully RFC4627 compliant, aside from syntax auto-correction (https://www.rfc-editor.org/rfc/rfc4627.html)

## Examples

To see all examples, go to the examples directory.

### Stream Parsing

```cpp
// examples/stream.cpp
#include "sjson.hpp"
#include "util.hpp"

int main() {
    // Parse until the stream is finished and return the value
    auto value = SJSON::Parse::stream(std::move(stream_example));

    // Log the value as pretty print and have indents be 4 spaces
    std::cout << "Finished parsed value: " << value.to_string(4) << '\n';
    // You may `.to_string()` the value with no pretty print by giving no arguments
    return 0;
}
```

### String Parsing

```cpp
// examples/string.cpp
#include "sjson.hpp"
#include "util.hpp"

int main() {
    // Parses and returns the value
    auto value = SJSON::Parse::string(input_example);

    // Log the value as pretty print and have indents be 4 spaces
    std::cout << "Value of the JSON is: " << value.to_string(4) << '\n';
    // You may `.to_string()` the value with no pretty print by giving no arguments
    return 0;
}
```

### JSON Listeners & Value Dropping

```cpp
// examples/listeners.cpp
#include "sjson.hpp"
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
```

### Error Recovery

```cpp
// examples/recovery.cpp
#include "sjson.hpp"
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
```

## Documentation

### Types

- `typedef std::move_only_function<void(const JSValue& value)> JSONCallback`
- `typedef std::move_only_function<std::string()> JSONStream`
- `typedef std::monostate JSNull`
- `typedef double JSNumber`
- `typedef bool JSBoolean`
- `typedef std::string JSString`
- `typedef std::map<std::string, JSValue> JSObject`
- `typedef std::vector<JSValue> JSArray`
- `using JSValueData = std::variant<JSNull, JSNumber, JSBoolean, JSString, JSObject, JSArray>`

### `SJSON::Parse`

- `Parse(JSONStream&& src, bool drop_generics = false)`
- `Parse(std::string src)`
- `static JSValue string(std::string src)`
- `static JSValue stream(JSONStream&& src)`
- `Parse& listen(std::string label, JSONCallback&& cb)`
- `bool next()`
- `void all()`
- `std::string to_string(int index_length = 0) const`

### `SJSON::JSValue`

- `JSValue() = default`
- `JSValue(JSNull v)`
- `JSValue(JSNumber v)`
- `JSValue(int v)`
- `JSValue(long v)`
- `JSValue(unsigned long v)`
- `JSValue(JSBoolean v)`
- `JSValue(JSString v)`
- `JSValue(std::string_view v)`
- `JSValue(const char* v)`
- `JSValue(JSObject v)`
- `JSValue(JSArray v)`
- `JSValueType type() const`
- `const char* type_str() const noexcept`
- `bool is_null() const noexcept`
- `bool is_number() const noexcept`
- `bool is_boolean() const noexcept`
- `bool is_string() const noexcept`
- `bool is_object() const noexcept`
- `bool is_array() const noexcept`
- `std::string to_string(int index_length = 0, int index = 1) const`
- `JSNull& null()`
- `JSNumber& number()`
- `JSBoolean& boolean()`
- `JSString& string()`
- `JSObject& object()`
- `JSArray& array()`
- `const JSNull& null() const`
- `const JSNumber& number() const`
- `const JSBoolean& boolean() const`
- `const JSString& string() const`
- `const JSObject& object() const`
- `const JSArray& array() const`
