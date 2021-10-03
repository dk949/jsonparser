#include "json_class.hpp"

#include <cassert>
#include <fmt/format.h>
#include <functional>
#include <iomanip>
#include <numeric>
#include <optional>
#include <string>
#include <utility>

using std::operator""s;

#define DEV(X) [[maybe_unused]] X

template<typename A, typename B>
struct Query {
    static_assert(std::is_same_v<A, B>);
};

#define QUERY(A, B, C) [[maybe_unused]] Query<A, B> z##C

void printParsed(auto v) {
    if (v.has_value()) {
        auto [rest, ret] = v.value();
        std::cout << "Just(" << rest << ", " << ret << ")\n";
    } else {
        std::cout << "Nothing\n";
    }
}

template<typename a>
using ParserRetT = std::optional<std::pair<std::string, a>>;

using ParserArgT = std::string;

template<typename a>
using ParserT = ParserRetT<a>(ParserArgT);

template<typename a>
struct Parser {
    std::function<ParserT<a>> runParser;
    using type = a;
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


auto fmap(auto f, auto p) {
    using Ret = typename decltype(std::function {f})::result_type;
    return Parser<Ret> {[p, f](ParserArgT input) -> ParserRetT<Ret> {
        if (auto par = p.runParser(input)) {
            auto [nextInput, x] = par.value();
            QUERY(Ret, decltype(f(x)), 1);
            return std::make_pair(nextInput, f(x));
        }
        return std::nullopt;
    }};
}


template<typename a>
Parser<a> pure(a x) {
    return Parser<a> {[x](ParserArgT input) -> ParserRetT<a> {  //
        return std::make_pair(input, x);
    }};
}


auto map(auto f, auto input) {
    using Ret = typename decltype(std::function {f})::result_type;
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
Parser<a> notNull(Parser<a> p) {
    return Parser<a> {[p](ParserArgT input) -> ParserRetT<a> {
        if (auto par = p.runParser(input)) {
            auto [nextInput, x] = par.value();
            if (x != "") {
                return par.value();
            }
        }
        return std::nullopt;
    }};
}

auto id(auto a) {
    return a;
}



template<typename t, typename Ret = std::conditional_t<std::is_same_v<t, char>, std::string, std::vector<t>>>
Parser<Ret> sequenceA(std::vector<Parser<t>> a) {
    return Parser<Ret> {[a](ParserArgT input) -> ParserRetT<Ret> {
        Ret out;
        auto copyInput = input;
        for (auto p : a) {
            if (auto success = p.runParser(input)) {
                auto [rest, res] = success.value();
                out.push_back(res);
                input = rest;
            } else {
                return std::nullopt;
            }
        }
        return std::make_pair(input, out);
    }};
}

template<typename a, ParserFactory<a> f>
auto operator|(Parser<a> p1, f p2) {  // alternative
    return Parser<a> {[p1, p2](ParserArgT input) -> ParserRetT<a> {
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
auto operator>(Parser<a> p1, Parser<b> p2) {
    return Parser<b> {[p1, p2](ParserArgT input) -> ParserRetT<b> {
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
auto operator<(Parser<a> p1, Parser<b> p2) {
    return Parser<a> {[p1, p2](ParserArgT input) -> ParserRetT<a> {
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
    return Parser<char> {[x](ParserArgT input) -> ParserRetT<char> {
        if (!input.empty()) {
            if (char y = input.front(); y == x) {
                return std::make_pair(input.substr(1), y);
            }
        }
        return std::nullopt;
    }};
}

Parser<ParserArgT> stringP(ParserArgT x) {
    return sequenceA(map(charP, x));
}


Parser<ParserArgT> spanP(auto f) {
    return Parser<ParserArgT> {[f](ParserArgT input) -> ParserRetT<ParserArgT> {
        auto ret = span(f, input);

        std::swap(ret.first, ret.second);
        return ret;
    }};
}

template<typename a>
Parser<std::vector<a>> many(Parser<a> p) {
    return Parser<std::vector<a>> {[p](ParserArgT input) {
        std::vector<a> out;
        while (!input.empty()) {
            if (auto par = p.runParser(input)) {
                auto [rest, ret] = par.value();
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

template<typename a, typename b, typename Ret = std::conditional_t<std::is_same_v<b, char>, std::string, std::vector<b>>>
Parser<Ret> sepBy(Parser<a> sep, Parser<b> element) {
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
    return fmap([](std::string) { return JsonNull(); }, stringP("null"));
}

Parser<JsonValue> jsonBool() {
    return fmap([](std::string input) { return JsonBool(input == "true"s); },
        (stringP("true") | []() { return stringP("false"); }));
}

Parser<JsonValue> jsonNumber() {
    return fmap([](std::string input) { return JsonNumber(std::stoi(input)); },
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
        if (auto liter = l.runParser(input)) {
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


std::optional<std::pair<std::string, int>> innerL(std::string) {
    return std::optional<std::pair<std::string, int>> {std::make_pair("hello"s, 42)};
};

int main() {
    auto valueParser = jsonValue();
    auto v0 = valueParser.runParser("null");
    auto v1 = valueParser.runParser("true");
    auto v2 = valueParser.runParser("false");
    auto v3 = valueParser.runParser("1234");
    auto v4 = valueParser.runParser(R"("hello")");
    auto v5 = valueParser.runParser(R"(["hello", 1, 2, "hi", [1,2,3 ], []])");
    auto v6 = valueParser.runParser(R"({"hello":"hi", "one": 1, "two": 2})");

    printParsed(v0);
    printParsed(v1);
    printParsed(v2);
    printParsed(v3);
    printParsed(v4);
    printParsed(v5);
    printParsed(v6);
}
