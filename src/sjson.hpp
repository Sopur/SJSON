#pragma once
#include "listener.hpp"
#include "token.hpp"
#include "utf8.hpp"
#include "util.hpp"
#include "value.hpp"
#include <cstddef>
#include <functional>
#include <string>

namespace SJSON {
    typedef std::move_only_function<std::string()> JSONStream;

    struct ParseOpts {
        bool drop_generics = false;      // Drop values when called in generic listeners?
        size_t nested_object_limit = 80; // Max level for nested objects (default=80 like Google Cloud Spanner)
    };
    class Parse {
    protected:
        ParseOpts opts;
        JSONStream istream;
        VectorStack<JSValue*> references;
        JSPath path;
        Token current_token;
        size_t i;
        std::string chunk;

        bool is_eof() const noexcept;
        bool is_finished() const noexcept;
        bool readable() const noexcept;
        bool prev_is_type(JSValueType type) const;
        void use_chunk(std::string src);
        Token mk_token();
        Token read_token();
        void parse_chunk(std::string src);

    public:
        JSValue value;

        Parse(ParseOpts opts = {});                   // Receive streams
        Parse(JSONStream&& src, ParseOpts opts = {}); // Input streams
        Parse(std::string src, ParseOpts opts = {});  // No stream
        Parse(const Parse&) = delete;
        Parse& operator=(const Parse&) = delete;
        Parse(Parse&&) noexcept = default;
        Parse& operator=(Parse&&) noexcept = default;
        ~Parse() = default;

        // Data parsing
        static JSValue string(std::string src, ParseOpts opts = {});
        static JSValue stream(JSONStream&& src, ParseOpts opts = {});
        Parse& listen(std::string label, JSONCallback&& cb);
        bool recv(std::string chunk);
        bool next();
        void all();

        // Data access
        std::string to_string(int index_length = 0, EscapePolicy ep = EscapePolicy::Minimum) const;
    };
} // namespace SJSON
