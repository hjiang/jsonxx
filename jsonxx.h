// Author: Hong Jiang <hong@hjiang.net>

#include <cassert>
#include <iostream>

namespace jsonxx {

// A JSON Object
class Object {
  public:
    bool parse(std::istream& input);
};

class Array {
  public:
    bool parse(std::istream& input);
};

// A value could be a number, an array, a string, an object, a
// boolean, or null
class Value {
  public:
    class Null {};

    Value();
    ~Value();
    bool parse(std::istream& input);
    template<typename T>
    bool is();
    template<typename T>
    T get();
  private:
    Value(const Value&);
    Value& operator=(const Value&);
    enum {
        INTEGER_,
        STRING_,
        BOOL_,
        NULL_,
        INVALID_
    } type_;
    union {
        long integer_value_;
        std::string* string_value_;
        bool bool_value_;
    };
};

template<>
inline bool Value::is<Value::Null>() {
    return type_ == NULL_;
}

template<>
inline bool Value::is<bool>() {
    return type_ == BOOL_;
}

template<>
inline bool Value::is<std::string>() {
    return type_ == STRING_;
}

template<>
inline bool Value::get<bool>() {
    assert(is<bool>());
    return bool_value_;
}

template<>
inline std::string Value::get<std::string>() {
    assert(is<std::string>());
    return *string_value_;
}

}  // namespace jsonxx
