// Author: Hong Jiang <hong@hjiang.net>

#include <cassert>
#include <iostream>
#include <map>
#include <vector>

namespace jsonxx {

// TODO: *::parse() should be static functions.

class Value;

// A JSON Object
class Object {
  public:
    Object();
    ~Object();
    bool parse(std::istream& input);

    template <typename T>
    bool has(const std::string& key);

    // Always call has<>() first. If the key doesn't exist, consider
    // the behavior undefined.
    template <typename T>
    T& get(const std::string& key);
  private:
    Object(const Object&);
    Object& operator=(const Object&);

    std::map<std::string, Value*> value_map_;
};

class Value;

class Array {
  public:
    Array();
    ~Array();
    bool parse(std::istream& input);

    unsigned int size() { return values_.size(); }

    template <typename T>
    bool has(unsigned int i);

    template <typename T>
    T& get(unsigned int i);

  private:
    Array(const Array&);
    Array& operator=(const Array&);
    std::vector<Value*> values_;
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
    T& get();
  private:
    Value(const Value&);
    Value& operator=(const Value&);
    enum {
        INTEGER_,
        STRING_,
        BOOL_,
        NULL_,
        ARRAY_,
        OBJECT_,
        INVALID_
    } type_;
    union {
        long integer_value_;
        std::string* string_value_;
        bool bool_value_;
        Array* array_value_;
        Object* object_value_;
    };
};

template <typename T>
bool Array::has(unsigned int i) {
    if (i >= size()) {
        return false;
    } else {
        return values_[i]->is<T>();
    }
}

template <typename T>
T& Array::get(unsigned int i) {
    assert(i < size());
    return values_[i]->get<T>();
}

template <typename T>
bool Object::has(const std::string& key) {
    return value_map_.count(key) > 0 && value_map_[key]->is<T>();
}

template <typename T>
T& Object::get(const std::string& key) {
    assert(has<T>(key));
    return value_map_[key]->get<T>();
}

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
inline bool Value::is<long>() {
    return type_ == INTEGER_;
}

template<>
inline bool Value::is<Array>() {
    return type_ == ARRAY_;
}

template<>
inline bool Value::is<Object>() {
    return type_ == OBJECT_;
}

template<>
inline bool& Value::get<bool>() {
    assert(is<bool>());
    return bool_value_;
}

template<>
inline std::string& Value::get<std::string>() {
    assert(is<std::string>());
    return *string_value_;
}

template<>
inline long& Value::get<long>() {
    assert(is<long>());
    return integer_value_;
}

template<>
inline Array& Value::get<Array>() {
    assert(is<Array>());
    return *array_value_;
}

template<>
inline Object& Value::get<Object>() {
    assert(is<Object>());
    return *object_value_;
}

}  // namespace jsonxx
