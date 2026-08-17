#pragma once
#include "sjson.hpp"
#include "utf8.hpp"
#include <iostream>
#include <string>

namespace SJSON {
    class Tester {
    protected:
        inline static void log_fail(const std::string& src, const std::string& output, const char* details = " ") {
            std::cout << "[FAILED]" << details << "'" << src << "' -> '" << output << "'\n";
        }
        inline static void log_internal_fail(const std::string& src, const std::string& output) {
            return log_fail(src, output, " (Internal Error) ");
        }
        inline static void log_pass(const std::string& src, const std::string& output) {
            std::cout << "[PASSED] '" << src << "' -> " << output << "\n";
        }
        inline static void log(bool passed, const std::string& src, const std::string& output) {
            if (passed)
                return log_pass(src, output);
            else
                return log_fail(src, output);
        }

    public:
        struct {
            int parsing_total = 0;
            int parsing_passed = 0;
            int errors_total = 0;
            int errors_passed = 0;
            int internal_errors = 0;
        } tests;

        inline Tester() { run(); };
        ~Tester() = default;

        inline std::string string(const std::string& src) const {
            // Simulate one character streams cuz it's the worse case scenario
            Parse json([&src, i = 0ul]() mutable -> std::string {
                if (i >= src.size()) return "";
                return std::string {src[i++]};
            });
            json.all();
            return json.to_string(0, EscapePolicy::Invalids);
        }
        inline void section(const char* name) const {
            std::cout << "[SECTION] Now testing " << name << '\n';
        }
        inline void error(const std::string& src) {
            tests.errors_total++;
            try {
                auto output = string(src);
                log_fail(src, output); // Error if success
            } catch (const sjson_parse_error& err) {
                log_pass(src, err.what()); // Success if error
                tests.errors_passed++;
            } catch (const sjson_internal_parse_error& err) {
                log_internal_fail(src, err.what());
                tests.internal_errors++;
            }
        }
        inline void test(const std::string& src, const std::string& expected) {
            tests.parsing_total++;
            try {
                auto output = string(src);
                bool passed = output == expected;
                log(passed, src, output);
                tests.parsing_passed += passed;
            } catch (const sjson_parse_error& err) {
                log_fail(src, err.what());
            } catch (const sjson_internal_parse_error& err) {
                log_internal_fail(src, err.what());
                tests.internal_errors++;
            }
        }
        inline void test(const std::string& src) {
            return test(src, src);
        }

        inline void run() {
            section("unstrict json");
            // I hate UTF8
            test(R"("")");                              // Empty String
            test(R"("string")");                        // String
            test(R"("`string's \"quotes\"`")");         // String quotes
            test(R"("\t\n\r")");                        // String escape shortcuts
            test(R"("héllo")");                         // UTF8-2
            test(R"("h€llo")");                         // UTF8-3
            test(R"("h😍llo")");                        // UTF8-4
            test("\"\x80hjk\"", R"("\uFFFDhjk")");      // UTF8-1 invalid
            test("\"\xC2\x41hjk\"", R"("\uFFFDhjk")");  // UTF8-2 invalid
            test("\"\u0001hjk\"", R"("\u0001hjk")");    // UTF8 C0
            test(R"("\u0000hjk")");                     // Unicode C0
            test(R"("\u1E9Ehjk")", R"("ẞhjk")");        // Unicode point
            test(R"("\uD83D\uDE0Dhjk")", R"("😍hjk")"); // Unicode surrogate
            test(R"("\uD83Dhjk")", R"("\uFFFDhjk")");   // Unicode invalid high-surrogate
            test(R"("\uDE0Dhjk")", R"("\uFFFDhjk")");   // Unicode invalid low-surrogate
            // Num no frac
            test("1");
            test("-1");
            // Num w frac
            test("1.23");
            test("-1.23");
            test("3.1415926535897931");
            test("-3.1415926535897931");
            // Scientific w +
            test("1e+10", "10000000000");
            test("-1E+10", "-10000000000");
            // Scientific no +
            test("1e10", "10000000000");
            test("-1E10", "-10000000000");
            // Keywords
            test("null");
            test("true");
            test("false");

            section("array of literals");
            test("[]");
            test(R"(["string\r"])");
            test(R"(["string","string \"quotes\"",""])");
            test("[1,2,3]");
            test("[1.23,4,5.67]");
            test("[null]");
            test("[null,null]");
            test("[true]");
            test("[false]");
            test("[true,false]");

            section("nested arrays");
            test("[[]]");
            test("[[],[]]");
            test(R"([1,[],""])");
            test(R"([null,[],"string \"quotes\""])");
            test("[[],[1,2]]");
            test(R"([[],[1,[true,"string \"quotes\"",[]]],[]])");
            test("[[[[[[]]],[]]]]");
            test(R"([[[1,[[[],[["string\n"]]]],[]],["string \"quotes\""]],null])");

            section("object of literals");
            test(R"({"a":"string\n"})");
            test(R"({"\n":"string\r"})");
            test(R"({"a":"string \"quotes\""})");
            test(R"({"a":1})");
            test(R"({"a":1.23})");
            test(R"({"a":1,"b":1.23})");
            test(R"({"a":1,"b":1.23,"c":"string","d":"string \"quotes\""})");
            test(R"({"a":1,"b":null,"c":true,"d":false})");

            section("nested objects");
            test("{}");
            test(R"({"a":[]})");
            test(R"({"a":{}})");
            test(R"({"a":[],"b":{}})");
            test(R"({"a":{"a":{}}})");
            test(R"({"a":{"a":{},"b":[]}})");
            test(R"({"a":{"a":{"a":1,"b":1.23,"c":"string","d":"string \"quotes\""},"b":[null,true,false]}})");
            test(R"({"a":{"a":[[[1,[[[],[["string"]]]],[]],["string \"quotes\""]],null],"b":[{"a":null},{"a":true}]}})");

            section("white space");
            test("  1", "1");
            test("1  ", "1");
            test("\n\n1", "1");
            test("1\n\n", "1");
            test("  \n  1 \n \n", "1");
            test("[   1, \n 2.34,\n\n \ntrue]", "[1,2.34,true]");
            test("  {\n\n \"a\"\n:  \n2   }", "{\"a\":2}");

            section("invalid token errors");
            error("[test]");
            error(R"({"a":truey})");
            error(R"({"a":2.2.2})");
            error("[-0.3-2]");
            error("-");
            error("--1");
            error("1e");
            error("e10");

            section("end of input syntax errors");
            error("");
            error(R"(")");
            error(R"("string)");
            error("[");
            error("[[]");
            error("[[1],[null],true");
            error("[[[1],[null],true]");
            error("[[[[[]]]]");
            error(R"(["string])");
            error(R"(["string]")");
            error("{");
            error(R"({"a":[])");
            error(R"({"a":[[])");

            section("more data after finish errors");
            error(R"("string""string")");
            error(R"(""""")");
            error("1[]");
            error("1.23:");
            error(R"(null"string")");
            error("true1");
            error("1.23false");
            error("[][]");
            error("[]true");
            error("[1,2],3");
            error("{}[]");
            error("{}null");
            error(R"({"a":1.23}4)");

            section("unexpected token errors");
            error(":1");
            error("/true");
            error("/false");
            error(",null");
            error(R"(,"string")");
            error("[.32]");
            error("[,,,]");
            error("[1,]");
            error("[1 1 1]");
            error("[,1]");
            error("[true,,,]");
            error("[,,,null]");
            error(R"(["string":2])");
            error(R"([{])");
            error(R"([}])");
            error(R"({1})");
            error(R"({"a"})");
            error(R"({1:2})");
            error(R"({"a"1})");
            error(R"({"a""b":1})");
            error(R"({{"a":2}})");
            error(R"({,"a":1})");
            error(R"({"a":1,})");
            error(R"({"a"::1})");
            error(R"({"a":1"b":2})");
            error(R"({"a":1,,"b":2})");
            error(R"({"a":1,  ,"b":2})");

            section("misc errors");
            error(std::string((ParseOpts {}).nested_object_limit + 1, '['));

            // Not passing all error tests is not a bad thing
            // Passing all parsing tests is required
            std::cout << "[RESULT] Passed " << (tests.parsing_passed + tests.errors_passed) << '/' << (tests.parsing_total + tests.errors_total) << " tests\n"
                      << "[RESULT] Passed " << tests.parsing_passed << '/' << tests.parsing_total << " parsing tests\n"
                      << "[RESULT] Passed " << tests.errors_passed << '/' << tests.errors_total << " errors tests\n"
                      << "[RESULT] Encountered " << tests.internal_errors << " internal errors\n";
        }
    };
} // namespace SJSON
