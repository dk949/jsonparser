#ifndef JSON_CLASS_HPP
#define JSON_CLASS_HPP

#include <cstdint>   // for int64_t
#include <iostream>  // for ostream, nullptr_t
#include <string>    // for string
#include <utility>   // for pair
#include <variant>   // for variant
#include <vector>    // for vector


class JsonValue {
public:
    using Null = std::nullptr_t;
    using Bool = bool;
    using Number = int64_t;  // No float yet
    using String = std::string;
    using Array = std::vector<JsonValue>;
    using Object = std::vector<std::pair<std::string, JsonValue>>;
    using Data = std::variant<Null, Bool, Number, String, Array, Object>;

private:
    Data m_data;

public:
    explicit JsonValue(Data d);
    friend std::ostream &operator<<(std::ostream &o, const JsonValue &v);

    static void coutNull(std::ostream &o, Null arg);
    static void coutBool(std::ostream &o, Bool arg);
    static void coutNumber(std::ostream &o, Number arg);
    static void coutString(std::ostream &o, const String &arg);
    static void coutArray(std::ostream &o, const Array &arg);
    static void coutObject(std::ostream &o, const Object &arg);

    bool operator==(const JsonValue &other) const = default;
    bool operator!=(const JsonValue &other) const = default;
};


JsonValue JsonNull();
JsonValue JsonBool(JsonValue::Bool b);
JsonValue JsonNumber(JsonValue::Number n);
JsonValue JsonString(const JsonValue::String &s);
JsonValue JsonArray(const JsonValue::Array &a);
JsonValue JsonObject(const JsonValue::Object &o);


#endif  // JSON_CLASS_HPP
