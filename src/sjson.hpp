#pragma once
#include "listener.hpp"
#include "token.hpp"
#include "util.hpp"
#include "value.hpp"
#include <cstddef>
#include <functional>
#include <string>

namespace SJSON {
    typedef std::move_only_function<std::string()> JSONStream;

    class Parse {
    private:
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
        void use_chunk(const std::string& src);
        Token mk_token();
        Token read_token();
        void parse_chunk(const std::string& src);

    public:
        JSValue value;

        Parse(JSONStream&& src, bool drop_generics = false);
        Parse(const std::string& src);
        Parse(const Parse&) = delete;
        Parse(Parse&&) noexcept = default;
        Parse& operator=(const Parse&) = delete;
        Parse& operator=(Parse&&) noexcept = default;
        ~Parse() = default;

        // Data parsing
        static JSValue string(const std::string& src);
        static JSValue stream(JSONStream&& src);
        Parse& listen(const std::string& label, JSONCallback&& cb);
        bool next();
        void all();

        // Data access
        std::string to_string(int index_length = 0) const;
    };
} // namespace SJSON
