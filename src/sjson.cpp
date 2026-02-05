#include "sjson.hpp"
#include "listener.hpp"
#include "syntax.hpp"
#include "value.hpp"
#include <initializer_list>
#include <string>

namespace SJSON {
    // End of file if stream returns an empty string
    bool Parse::is_eof() const noexcept {
        return chunk.size() == 0;
    }
    // If there are no more references, no more values are expected
    bool Parse::is_finished() const noexcept {
        return references.empty();
    }
    bool Parse::readable() const noexcept {
        return i < chunk.size();
    }
    bool Parse::prev_is_type(JSValueType type) const {
        if (!references.has_prev()) return false;
        return references.prev()->type() == type;
    }
    // Reset read state
    void Parse::use_chunk(std::string src) {
        if (readable()) throw sjson_internal_parse_error::new_chunk_before_finish();
        i = 0;
        chunk = std::move(src);
    }
    // Copy and reset for a new streamed token
    Token Parse::mk_token() {
        Token token = current_token.copy();
        current_token.reset();
        return token;
    }
    Token Parse::read_token() {
        if (readable()) {
            while (readable()) {
                auto c = chunk[i];
                if (current_token.is_terminating(c))
                    return mk_token();
                current_token.push(c);
                i++;
            }
        } else if (is_eof()) {
            // No argument represents eof (errors still work cuz tokens check validity when their value is accessed)
            if (current_token.is_terminating())
                return mk_token();
        }
        // If the current token is unfinished
        return Token();
    }
    void Parse::parse_chunk(std::string src) {
        use_chunk(std::move(src));
        while (true) {
            auto token = read_token();
            if (token.is_unresolved()) {
                if (is_eof() && !is_finished())
                    throw sjson_parse_error::unexpected_eof();
                break;
            };
            if (is_finished())
                throw sjson_parse_error::unexpected_data();
            switch (references.top()->type()) {
                // This shouldn't happen because literals are automatically escaped
                case JSValueType::Number:
                case JSValueType::Boolean:
                case JSValueType::String:
                    throw sjson_internal_parse_error::invalid_reference_state();
                // If value needs to be determined
                case JSValueType::Null: {
                    switch (token.type) {
                        case TokenType::Unresolved: break;
                        case TokenType::Operator: {
                            switch (token.to_operator()) {
                                case Operators::Colon:
                                    // Colons are only valid in object contexts
                                    if (!prev_is_type(JSValueType::Object))
                                        throw sjson_parse_error::unexpected_token(token.src);
                                    break;
                                case Operators::Comma:
                                case Operators::ArrayEnd:
                                case Operators::ObjectEnd:
                                    throw sjson_parse_error::unexpected_token(token.src);
                                case Operators::ArrayStart:
                                    *references.top() = JSValue(JSArray());
                                    break;
                                case Operators::ObjectStart:
                                    *references.top() = JSValue(JSObject());
                                    break;
                            }
                            break;
                        }
                        case TokenType::Keyword:
                        case TokenType::Number:
                        case TokenType::String: {
                            *references.top() = token.to_value();
                            path.pop(*references.top());
                            references.pop();
                            break;
                        }
                    }
                    break;
                }
                case JSValueType::Object: {
                    switch (token.type) {
                        case TokenType::Unresolved: break;
                        case TokenType::Operator: {
                            switch (token.to_operator()) {
                                case Operators::Colon:
                                case Operators::ArrayEnd:
                                case Operators::ArrayStart:
                                case Operators::ObjectStart:
                                    throw sjson_parse_error::unexpected_token(token.src);
                                case Operators::Comma:
                                    break; // Commas are ignored cuz objects follow a specific pattern anyways
                                case Operators::ObjectEnd: {
                                    // Handle a generic drop for arrays
                                    if (path.pop(*references.top()) && prev_is_type(JSValueType::Array))
                                        references.prev()->array().pop_back();
                                    references.pop();
                                    break;
                                }
                            }
                            break;
                        }
                        case TokenType::Keyword:
                        case TokenType::Number:
                            throw sjson_parse_error::unexpected_token(token.src);
                        case TokenType::String: {
                            const auto key = token.to_string();
                            auto& root = references.top()->object();
                            root[key] = JSValue();
                            references.push(&root[key]);
                            path.push(key);
                            break;
                        }
                    }
                    break;
                }
                case JSValueType::Array: {
                    switch (token.type) {
                        case TokenType::Unresolved: break;
                        case TokenType::Operator: {
                            const auto op = token.to_operator();
                            switch (op) {
                                case Operators::Colon:
                                case Operators::ObjectEnd:
                                    throw sjson_parse_error::unexpected_token(token.src);
                                case Operators::Comma:
                                    break; // Commas are ignored cuz the parser handles values individually
                                case Operators::ArrayEnd: {
                                    // Handle a generic drop for arrays
                                    if (path.pop(*references.top()) && prev_is_type(JSValueType::Array))
                                        references.prev()->array().pop_back();
                                    references.pop();
                                    break;
                                }
                                case Operators::ArrayStart:
                                case Operators::ObjectStart: {
                                    auto& root = references.top()->array();
                                    if (op == Operators::ArrayStart)
                                        root.push_back(JSArray());
                                    else
                                        root.push_back(JSObject());
                                    references.push(&root.back());
                                    path.push(root.size() - 1);
                                    break;
                                }
                            }
                            break;
                        }
                        case TokenType::Keyword:
                        case TokenType::Number:
                        case TokenType::String: {
                            const auto value = token.to_value();
                            auto& root = references.top()->array();
                            path.push(root.size());
                            if (!path.pop(value)) root.push_back(value); // Only push if needed
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    Parse::Parse(JSONStream&& src, bool drop_generics):
        istream(std::move(src)),
        references({&value}),
        path(drop_generics) {}
    Parse::Parse(std::string src):
        istream([]() -> std::string {
            return ""; // Predefined parse, no stream needed
        }),
        references({&value}),
        path(false) {
        parse_chunk(std::move(src));
        parse_chunk(""); // Simulate end of stream
    }

    // Data parsing
    JSValue Parse::string(std::string src) {
        return Parse(std::move(src)).value;
    }
    JSValue Parse::stream(JSONStream&& src) {
        Parse json(std::move(src));
        json.all();
        return json.value;
    }
    Parse& Parse::listen(std::string label, JSONCallback&& cb) {
        path.listen(std::move(label), std::move(cb));
        return *this;
    }
    bool Parse::next() {
        parse_chunk(istream()); // Parse stream even if eof
        return !is_eof();
    }
    void Parse::all() {
        while (next());
    }

    // Data access
    std::string Parse::to_string(int index_length) const {
        return value.to_string(index_length);
    }
} // namespace SJSON
