#include "json_class.hpp"

JsonValue::JsonValue(Data d):m_data(d){}

JsonValue JsonNull() {
    return JsonValue(JsonValue::Data(JsonValue::Null {}));
}
JsonValue JsonBool(JsonValue::Bool b) {
    return JsonValue(JsonValue::Bool {b});
}
JsonValue JsonNumber(JsonValue::Number n) {
    return JsonValue(JsonValue::Number {n});
}
JsonValue JsonString(JsonValue::String s) {
    return JsonValue(JsonValue::String {s});
}
JsonValue JsonArray(JsonValue::Array a) {
    return JsonValue(JsonValue::Array {a});
}
JsonValue JsonObject(JsonValue::Object o) {
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
            } else {
                static_assert(sizeof(T) < 0, "Too many types in variant");
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
void JsonValue::coutString(std::ostream &o, String arg) {
    o << "JsonString(" << arg << ')';
}
void JsonValue::coutArray(std::ostream &o, Array arg) {
    o << "JsonArray([";
    int i = 0;
    for (const auto &elem : arg) {
        o << (i++ == 0 ? "" : ",") << elem;
    }
    o << "])";
}
void JsonValue::coutObject(std::ostream &o, Object arg) {
    o << "JsonObject({";
    int i = 0;
    for (const auto &[key, value] : arg) {
        o << (i++ == 0 ? "" : ",") << key << " : " << value;
    }
    o << "})";
}
