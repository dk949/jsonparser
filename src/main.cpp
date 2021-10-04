#include "json_class.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using std::operator""s;

void printParsed(const auto &v) {
    if (v.has_value()) {
        auto [rest, ret] = v.value();
        std::cout << "Just(" << rest << ", " << ret << ")\n";
    } else {
        std::cout << "Nothing\n";
    }
}

template<typename Func>
struct function_return {
    using type = typename decltype(std::function {std::declval<Func>()})::result_type;
};


template<typename Func>
using function_return_t = typename function_return<Func>::type;


template<typename a>
using ParserRetT = std::optional<std::pair<std::string, a>>;

using ParserArgT = std::string;

template<typename a>
using ParserT = ParserRetT<a>(ParserArgT);

template<typename a>
struct Parser {
    std::function<ParserT<a>> runParser;
};

template<typename T, typename a>
concept ParserFactory = requires(T t) {
    { t() } -> std::same_as<Parser<a>>;
};


Parser<JsonValue> jsonValue();
Parser<JsonValue> jsonNull();
Parser<JsonValue> jsonBool();
Parser<JsonValue> jsonNumber();
Parser<JsonValue> jsonString();
Parser<JsonValue> jsonArray();


template<typename Func, typename a, typename Ret = function_return_t<Func>>
Parser<Ret> fmap(Func f, const Parser<a> &p) {
    return Parser<Ret> {[p, f](const ParserArgT &input) -> ParserRetT<Ret> {
        if (auto par = p.runParser(input)) {
            auto [nextInput, x] = par.value();
            return std::make_pair(nextInput, f(x));
        }
        return std::nullopt;
    }};
}


template<typename a>
Parser<a> pure(a x) {
    return Parser<a> {[x](const ParserArgT &input) -> ParserRetT<a> {  //
        return std::make_pair(input, x);
    }};
}


template<typename Func, typename Input, typename Ret = function_return_t<Func>>
std::vector<Ret> map(Func f, Input input) {
    std::vector<Ret> out;
    std::transform(std::begin(input), std::end(input), std::back_inserter(out), f);
    return out;
}

template<typename Collection>
std::pair<Collection, Collection> span(auto f, Collection l) {
    std::pair<Collection, Collection> out;
    auto it = std::begin(l);

    for (; it != std::end(l); it++) {
        if (!f(*it)) {
            break;
        }
        out.first.push_back(*it);
    }
    for (; it != std::end(l); it++) {
        out.second.push_back(*it);
    }
    return out;
}

template<typename a>
Parser<a> notNull(const Parser<a> &p) {
    return Parser<a> {[p](const ParserArgT &input) -> ParserRetT<a> {
        if (auto par = p.runParser(input)) {
            auto [nextInput, x] = par.value();
            if (x != "") {
                return par.value();
            }
        }
        return std::nullopt;
    }};
}


template<typename t, typename Ret = std::conditional_t<std::is_same_v<t, char>, std::string, std::vector<t>>>
Parser<Ret> sequenceA(const std::vector<Parser<t>> &a) {
    return Parser<Ret> {[a](const ParserArgT &input) -> ParserRetT<Ret> {
        Ret out;
        auto copyInput = input;
        for (const auto &p : a) {
            if (auto success = p.runParser(copyInput)) {
                auto [rest, res] = success.value();
                out.push_back(res);
                copyInput = rest;
            } else {
                return std::nullopt;
            }
        }
        return std::make_pair(copyInput, out);
    }};
}

template<typename a, ParserFactory<a> f>
Parser<a> operator|(const Parser<a> &p1, const f &p2) {  // alternative
    return Parser<a> {[p1, p2](const ParserArgT &input) -> ParserRetT<a> {
        if (auto par1 = p1.runParser(input)) {
            return par1;
        }
        if (auto par2 = p2().runParser(input)) {
            return par2;
        }
        return std::nullopt;
    }};
}

template<typename a, typename b>
Parser<b> operator>(const Parser<a> &p1, const Parser<b> &p2) {
    return Parser<b> {[p1, p2](const ParserArgT &input) -> ParserRetT<b> {
        if (auto par1 = p1.runParser(input)) {
            auto rest = par1.value().first;
            if (auto par2 = p2.runParser(rest)) {
                return par2;
            }
        }
        return std::nullopt;
    }};
}

template<typename a, typename b>
Parser<a> operator<(const Parser<a> &p1, const Parser<b> &p2) {
    return Parser<a> {[p1, p2](const ParserArgT &input) -> ParserRetT<a> {
        if (auto par1 = p1.runParser(input)) {
            auto [rest, ret] = par1.value();
            if (auto par2 = p2.runParser(rest)) {
                return std::make_pair(par2.value().first, ret);
            }
        }
        return std::nullopt;
    }};
}



Parser<char> charP(char x) {
    return Parser<char> {[x](const ParserArgT &input) -> ParserRetT<char> {
        if (!input.empty()) {
            if (char y = input.front(); y == x) {
                return std::make_pair(input.substr(1), y);
            }
        }
        return std::nullopt;
    }};
}

Parser<ParserArgT> stringP(const ParserArgT &x) {
    return sequenceA(map(charP, x));
}


Parser<ParserArgT> spanP(auto f) {
    return Parser<ParserArgT> {[f](const ParserArgT &input) -> ParserRetT<ParserArgT> {
        auto ret = span(f, input);

        std::swap(ret.first, ret.second);
        return ret;
    }};
}

template<typename a>
Parser<std::vector<a>> many(const Parser<a> &p) {
    return Parser<std::vector<a>> {[p](const ParserArgT &input) {
        std::vector<a> out;
        auto copyInput = input;
        while (!copyInput.empty()) {
            if (auto par = p.runParser(copyInput)) {
                auto [rest, ret] = par.value();
                out.push_back(ret);
                copyInput = rest;
                continue;
            }
            return std::make_pair(copyInput, out);
        }
        return std::make_pair(copyInput, out);
    }};
}


Parser<ParserArgT> stringLiteral() {
    return charP('"') > spanP([](char c) { return c != '"'; }) < charP('"');
}

Parser<ParserArgT> ws() {
    return spanP([](char c) { return std::isspace(c); });
}

template<typename a, typename b, typename Ret = std::conditional_t<std::is_same_v<b, char>, std::string, std::vector<b>>>
Parser<Ret> sepBy(const Parser<a> &sep, const Parser<b> &element) {
    auto l = [sep, element](std::string input) -> ParserRetT<Ret> {
        Ret out;
        if (auto elem = element.runParser(input)) {
            auto [rest, ret] = elem.value();
            auto m = many(sep > element);
            if (auto manyRes = m.runParser(rest)) {
                auto [rest1, ret1] = manyRes.value();
                out.push_back(ret);
                out.insert(std::end(out), std::begin(ret1), std::end(ret1));
                return std::make_pair(rest1, out);
            }
        }
        return std::nullopt;
    };
    return Parser<Ret> {l} | []() { return pure(Ret {}); };
}



Parser<JsonValue> jsonNull() {
    return fmap([]([[maybe_unused]] const std::string &_) { return JsonNull(); }, stringP("null"));
}

Parser<JsonValue> jsonBool() {
    return fmap([](const std::string &input) { return JsonBool(input == "true"s); },
        (stringP("true") | []() { return stringP("false"); }));
}

Parser<JsonValue> jsonNumber() {
    return fmap([](const std::string &input) { return JsonNumber(std::stoi(input)); },
        notNull(spanP([](char c) { return std::isdigit(c); })));
}


Parser<JsonValue> jsonString() {
    return fmap(JsonString, stringLiteral());
}

Parser<JsonValue> jsonArray() {
    auto elements = []() -> Parser<std::vector<JsonValue>> {
        auto sep = ws() > charP(',') < ws();
        return sepBy(sep, jsonValue());
    };
    return fmap(JsonArray, charP('[') > ws() > elements() < ws() < charP(']'));
}

Parser<JsonValue> jsonObject() {
    Parser<std::pair<std::string, JsonValue>> pair {[](std::string input) -> ParserRetT<std::pair<std::string, JsonValue>> {
        auto l = stringLiteral();
        if (auto liter = l.runParser(std::move(input))) {
            auto [rest, literal] = liter.value();
            auto o = ws() > charP(':') > ws() > jsonValue();
            if (auto obj = o.runParser(rest)) {
                auto [rest1, object] = obj.value();
                return std::make_pair(rest1, std::make_pair(literal, object));
            }
        }
        return std::nullopt;
    }};

    return fmap(JsonObject, charP('{') > ws() > sepBy(charP(',') > ws(), pair) < ws() < charP('}'));
}



Parser<JsonValue> jsonValue() {
    return jsonNull() | jsonBool | jsonNumber | jsonString | jsonArray | jsonObject;
}

int main() {
    [[maybe_unused]] auto valueParser = jsonValue();
    std::cout << std::filesystem::current_path();
    std::array files {
        std::ifstream("tests/example1.json"),  // https://json.org/example.html
        std::ifstream("tests/example2.json"),  // https://json.org/example.html
        std::ifstream("tests/example3.json"),  // https://json.org/example.html
        std::ifstream("tests/example4.json"),  // https://json.org/example.html
        std::ifstream("tests/example5.json"),  // https://json.org/example.html
        std::ifstream("tests/example6.json"),  // https://raw.githubusercontent.com/json-iterator/test-data/master/large-file.json
    };
    std::stringstream contents;
    for (const auto &file : files) {
        contents << file.rdbuf();
        auto tmp = contents.str();
        auto start = std::chrono::high_resolution_clock::now();
        auto v = valueParser.runParser(tmp);
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";
        if (!v) {
            std::cout << "oh no!\n";
        }
    }
}
