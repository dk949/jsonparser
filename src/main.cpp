#include "json_class.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <source_location>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>


using std::operator""s;

template<typename F>
using FunctionReturnT = typename decltype(std::function {std::declval<F>()})::result_type;

template<typename T>
using CollectionOf = std::conditional_t<std::is_same_v<T, char>, std::string, std::vector<T>>;


template<typename T>
using ParserRetT = std::optional<std::pair<std::string, T>>;

using ParserArgT = std::string;

template<typename T>
using ParserT = ParserRetT<T>(ParserArgT);

template<typename T>
struct Parser {
    std::function<ParserT<T>> runParser;
};

template<typename T, typename U>
concept ParserFactory = requires(T t) {
    { t() } -> std::same_as<Parser<U>>;
};


Parser<JsonValue> jsonValue();
Parser<JsonValue> jsonNull();
Parser<JsonValue> jsonBool();
Parser<JsonValue> jsonNumber();
Parser<JsonValue> jsonString();
Parser<JsonValue> jsonArray();

template<typename F, typename T, typename Ret = FunctionReturnT<F>>
Parser<Ret> fmap(F f, const Parser<T> &p) {
    return Parser<Ret> {[p, f](const ParserArgT &input) -> ParserRetT<Ret> {
        if (const auto par = p.runParser(input)) {
            const auto [nextInput, x] = par.value();
            return std::make_pair(nextInput, f(x));
        }
        return std::nullopt;
    }};
}


template<typename T>
Parser<T> pure(T x) {
    return Parser<T> {[x](const ParserArgT &input) -> ParserRetT<T> {  //
        return std::make_pair(input, x);
    }};
}


template<typename F, typename T, typename Ret = FunctionReturnT<F>>
std::vector<Ret> map(F f, T input) {
    std::vector<Ret> out;
    std::transform(std::begin(input), std::end(input), std::back_inserter(out), f);
    return out;
}

template<typename Collection>
std::pair<Collection, Collection> span(const auto &f, const Collection &l) {
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

template<typename T>
Parser<T> notNull(const Parser<T> &p) {
    return Parser<T> {[p](const ParserArgT &input) -> ParserRetT<T> {
        if (const auto par = p.runParser(input)) {
            const auto [nextInput, x] = par.value();
            if (x != "") {
                return par.value();
            }
        }
        return std::nullopt;
    }};
}


template<typename T, typename Ret = CollectionOf<T>>
Parser<Ret> sequenceA(const std::vector<Parser<T>> &a) {
    return Parser<Ret> {[a](ParserArgT input) -> ParserRetT<Ret> {
        Ret out;
        for (const auto &p : a) {
            if (const auto success = p.runParser(input)) {
                const auto [rest, ret] = success.value();
                out.push_back(ret);
                input = rest;
            } else {
                return std::nullopt;
            }
        }
        return std::make_pair(input, out);
    }};
}

template<typename T, ParserFactory<T> F>
Parser<T> operator|(const Parser<T> &p1, const F &p2) {  // alternative
    return Parser<T> {[p1, p2](const ParserArgT &input) -> ParserRetT<T> {
        if (const auto par1 = p1.runParser(input)) {
            return par1;
        }
        if (const auto par2 = p2().runParser(input)) {
            return par2;
        }
        return std::nullopt;
    }};
}

template<typename T, typename U>
Parser<U> operator>(const Parser<T> &p1, const Parser<U> &p2) {
    return Parser<U> {[p1, p2](const ParserArgT &input) -> ParserRetT<U> {
        if (const auto par1 = p1.runParser(input)) {
            const auto rest = par1.value().first;
            if (const auto par2 = p2.runParser(rest)) {
                return par2;
            }
        }
        return std::nullopt;
    }};
}

template<typename T, typename U>
Parser<T> operator<(const Parser<T> &p1, const Parser<U> &p2) {
    return Parser<T> {[p1, p2](const ParserArgT &input) -> ParserRetT<T> {
        if (const auto par1 = p1.runParser(input)) {
            const auto [rest, ret] = par1.value();
            if (const auto par2 = p2.runParser(rest)) {
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


template<typename F>
Parser<ParserArgT> spanP(const F &f) {
    return Parser<ParserArgT> {[f](const ParserArgT &input) -> ParserRetT<ParserArgT> {
        auto ret = span(f, input);
        std::swap(ret.first, ret.second);
        return ret;
    }};
}

template<typename T>
Parser<std::vector<T>> many(const Parser<T> &p) {
    return Parser<std::vector<T>> {[p](ParserArgT input) {
        std::vector<T> out;
        while (!input.empty()) {
            if (const auto par = p.runParser(input)) {
                const auto [rest, ret] = par.value();
                out.push_back(ret);
                input = rest;
                continue;
            }
            return std::make_pair(input, out);
        }
        return std::make_pair(input, out);
    }};
}


Parser<ParserArgT> stringLiteral() {
    return charP('"') > spanP([](char c) { return c != '"'; }) < charP('"');
}

Parser<ParserArgT> ws() {
    return spanP([](char c) { return std::isspace(c); });
}

template<typename T, typename U, typename Ret = CollectionOf<U>>
Parser<Ret> sepBy(const Parser<T> &sep, const Parser<U> &element) {
    const auto l = [sep, element](std::string input) -> ParserRetT<Ret> {
        Ret out;
        if (const auto elem = element.runParser(input)) {
            const auto [rest, ret] = elem.value();
            const auto m = many(sep > element);
            if (const auto manyRes = m.runParser(rest)) {
                const auto [rest1, ret1] = manyRes.value();
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
    const auto elements = []() -> Parser<std::vector<JsonValue>> {
        const auto sep = ws() > charP(',') < ws();
        return sepBy(sep, jsonValue());
    };
    return fmap(JsonArray, charP('[') > ws() > elements() < ws() < charP(']'));
}

Parser<JsonValue> jsonObject() {
    Parser<std::pair<std::string, JsonValue>> pair {[](std::string input) -> ParserRetT<std::pair<std::string, JsonValue>> {
        const auto l = stringLiteral();
        if (const auto liter = l.runParser(std::move(input))) {
            const auto [rest, literal] = liter.value();
            const auto o = ws() > charP(':') > ws() > jsonValue();
            if (const auto obj = o.runParser(rest)) {
                const auto [rest1, object] = obj.value();
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
    const auto valueParser = jsonValue();
    [[maybe_unused]] const auto printParsed = [](const auto &v) {
        if (v.has_value()) {
            const auto [rest, ret] = v.value();
            std::cout << "Just(" << rest << ", " << ret << ")\n";
        } else {
            std::cout << "Nothing\n";
        }
    };
    std::array files {
        std::ifstream("tests/example1.json"),  // https://json.org/example.html
        // std::ifstream("tests/example2.json"),  // https://json.org/example.html
        // std::ifstream("tests/example3.json"),  // https://json.org/example.html
        // std::ifstream("tests/example4.json"),  // https://json.org/example.html
        // std::ifstream("tests/example5.json"),  // https://json.org/example.html
        // std::ifstream("tests/example6.json"),  // https://raw.githubusercontent.com/json-iterator/test-data/master/large-file.json
    };

    std::stringstream contents;
    for (const auto &file : files) {
        contents << file.rdbuf();
        const auto tmp = contents.str();
        const auto start = std::chrono::high_resolution_clock::now();
        const auto v = valueParser.runParser(tmp);
        const auto end = std::chrono::high_resolution_clock::now();
        std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";
        if (!v) {
            std::cout << "oh no\n";
            return -1;
        }
    }
}
