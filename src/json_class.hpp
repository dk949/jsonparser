#ifndef JSON_CLASS_HPP
#define JSON_CLASS_HPP

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>


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
    JsonValue(Data d);
    friend std::ostream &operator<<(std::ostream &o, const JsonValue &v);

    static void coutNull(std::ostream &o, Null arg);
    static void coutBool(std::ostream &o, Bool arg);
    static void coutNumber(std::ostream &o, Number arg);
    static void coutString(std::ostream &o, String arg);
    static void coutArray(std::ostream &o, Array arg);
    static void coutObject(std::ostream &o, Object arg);

    bool operator==(const JsonValue &other) const = default;
    bool operator!=(const JsonValue &other) const = default;
};


JsonValue JsonNull();
JsonValue JsonBool(JsonValue::Bool b);
JsonValue JsonNumber(JsonValue::Number n);
JsonValue JsonString(JsonValue::String s);
JsonValue JsonArray(JsonValue::Array a);
JsonValue JsonObject(JsonValue::Object o);


#endif  // JSON_CLASS_HPP
