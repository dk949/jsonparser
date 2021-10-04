#include "json_class.hpp"

#include <cstddef>     // for size_t
#include <type_traits>  // for decay_t

JsonValue::JsonValue(Data d): m_data(std::move(d)) {}

JsonValue JsonNull() {
    return JsonValue(JsonValue::Data(JsonValue::Null {}));
}
JsonValue JsonBool(JsonValue::Bool b) {
    return JsonValue(JsonValue::Bool {b});
}
JsonValue JsonNumber(JsonValue::Number n) {
    return JsonValue(JsonValue::Number {n});
}
JsonValue JsonString(const JsonValue::String &s) {
    return JsonValue(JsonValue::String {s});
}
JsonValue JsonArray(const JsonValue::Array &a) {
    return JsonValue(JsonValue::Array {a});
}
JsonValue JsonObject(const JsonValue::Object &o) {
    return JsonValue(JsonValue::Object {o});
}



std::ostream &operator<<(std::ostream &o, const JsonValue &v) {
    std::visit(
        [&o](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, JsonValue::Null>) {
                JsonValue::coutNull(o, arg);
            } else if constexpr (std::is_same_v<T, JsonValue::Bool>) {
                JsonValue::coutBool(o, arg);
            } else if constexpr (std::is_same_v<T, JsonValue::Number>) {
                JsonValue::coutNumber(o, arg);
            } else if constexpr (std::is_same_v<T, JsonValue::String>) {
                JsonValue::coutString(o, arg);
            } else if constexpr (std::is_same_v<T, JsonValue::Array>) {
                JsonValue::coutArray(o, arg);
            } else if constexpr (std::is_same_v<T, JsonValue::Object>) {
                JsonValue::coutObject(o, arg);
            }
        },
        v.m_data);
    return o;
}

void JsonValue::coutNull(std::ostream &o, Null) {
    o << "JsonNull(null)";
}
void JsonValue::coutBool(std::ostream &o, Bool arg) {
    o << "JsonBool(" << std::boolalpha << arg << ')';
}
void JsonValue::coutNumber(std::ostream &o, Number arg) {
    o << "JsonNumber(" << arg << ')';
}
void JsonValue::coutString(std::ostream &o, const String &arg) {
    o << "JsonString(" << arg << ')';
}
void JsonValue::coutArray(std::ostream &o, const Array &arg) {
    static size_t arrayDepth = 0;
    arrayDepth++;
    o << "JsonArray([";
    int i = 0;
    for (const auto &elem : arg) {
        o << (i++ == 0 ? "" : ",") << '\n' << std::string(arrayDepth, '\t') << elem;
    }
    o << '\n' << std::string(arrayDepth, '\t') << "])";
    arrayDepth--;
}
void JsonValue::coutObject(std::ostream &o, const Object &arg) {
    static size_t objectDepth = 0;
    objectDepth++;
    o << "JsonObject({";
    int i = 0;
    for (const auto &[key, value] : arg) {
        o << (i++ == 0 ? "" : ",") << '\n' << std::string(objectDepth, '\t') << key << " : " << value;
    }

    o << '\n' << std::string(objectDepth, '\t') << "])";
    objectDepth--;
}
