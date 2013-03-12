// -*- mode: c++; c-basic-offset: 4; -*-

// Author: Hong Jiang <hong@hjiang.net>
// Contributors:
//   Sean Middleditch <sean@middleditch.us>
//   rlyeh <https://github.com/r-lyeh>

#include <cassert>
#include <iostream>
#include <map>
#include <vector>
#include <string>

namespace jsonxx {

enum settings {
  STRICT = false    // true = strict parsing, false = permissive parsing
};

typedef double Number;
typedef bool Boolean;
typedef std::string String;
struct Null {};

class Value;

// A JSON Object
class Object {
 public:
  Object();
  ~Object();

  static bool parse(std::istream& input, Object& object);

  template <typename T>
  bool has(const std::string& key) const;

  // Always call has<>() first. If the key doesn't exist, consider
  // the behavior undefined.
  template <typename T>
  T& get(const std::string& key);
  template <typename T>
  const T& get(const std::string& key) const;

  const std::map<std::string, Value*>& kv_map() const { return value_map_; }
  std::string jsonx( const std::string &header = std::string(), const std::string &attrib = std::string() ) const;
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

  static bool parse(std::istream& input, Array& array);

  size_t size() const { return values_.size(); }

  template <typename T>
  bool has(unsigned int i) const;

  template <typename T>
  T& get(unsigned int i);
  template <typename T>
  const T& get(unsigned int i) const;

  const std::vector<Value*>& values() const {
    return values_;
  }
  std::string jsonx( const std::string &header = std::string(), const std::string &attrib = std::string() ) const;
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
  ~Value() { reset(); }
  void reset();

  static bool parse(std::istream& input, Value& value);

  template<typename T>
  bool is() const;
  template<typename T>
  T& get();
  template<typename T>
  const T& get() const;
 private:
  Value(const Value&);
  Value& operator=(const Value&);
 public:
  enum {
    NUMBER_,
    STRING_,
    BOOL_,
    NULL_,
    ARRAY_,
    OBJECT_,
    INVALID_
  } type_;
  union {
    Number number_value_;
    String* string_value_;
    Boolean bool_value_;
    Array* array_value_;
    Object* object_value_;
  };
};

template <typename T>
bool Array::has(unsigned int i) const {
  if (i >= size()) {
    return false;
  } else {
    Value* v = values_.at(i);
    return v->is<T>();
  }
}

template <typename T>
T& Array::get(unsigned int i) {
  assert(i < size());
  Value* v = values_.at(i);
  return v->get<T>();
}

template <typename T>
const T& Array::get(unsigned int i) const {
  assert(i < size());
  const Value* v = values_.at(i);
  return v->get<T>();
}

template <typename T>
bool Object::has(const std::string& key) const {
  std::map<std::string, Value*>::const_iterator it(value_map_.find(key));
  return it != value_map_.end() && it->second->is<T>();
}

template <typename T>
T& Object::get(const std::string& key) {
  assert(has<T>(key));
  return value_map_.find(key)->second->get<T>();
}

template <typename T>
const T& Object::get(const std::string& key) const {
  assert(has<T>(key));
  return value_map_.find(key)->second->get<T>();
}

template<>
inline bool Value::is<Null>() const {
  return type_ == NULL_;
}

template<>
inline bool Value::is<Boolean>() const {
  return type_ == BOOL_;
}

template<>
inline bool Value::is<String>() const {
  return type_ == STRING_;
}

template<>
inline bool Value::is<Number>() const {
  return type_ == NUMBER_;
}

template<>
inline bool Value::is<Array>() const {
  return type_ == ARRAY_;
}

template<>
inline bool Value::is<Object>() const {
  return type_ == OBJECT_;
}

template<>
inline bool& Value::get<Boolean>() {
  assert(is<Boolean>());
  return bool_value_;
}

template<>
inline std::string& Value::get<String>() {
  assert(is<String>());
  return *string_value_;
}

template<>
inline Number& Value::get<Number>() {
  assert(is<Number>());
  return number_value_;
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

template<>
inline const Boolean& Value::get<Boolean>() const {
  assert(is<Boolean>());
  return bool_value_;
}

template<>
inline const String& Value::get<String>() const {
  assert(is<String>());
  return *string_value_;
}

template<>
inline const Number& Value::get<Number>() const {
  assert(is<Number>());
  return number_value_;
}

template<>
inline const Array& Value::get<Array>() const {
  assert(is<Array>());
  return *array_value_;
}

template<>
inline const Object& Value::get<Object>() const {
  assert(is<Object>());
  return *object_value_;
}

}  // namespace jsonxx

std::ostream& operator<<(std::ostream& stream, const jsonxx::Value& v);
std::ostream& operator<<(std::ostream& stream, const jsonxx::Object& v);
std::ostream& operator<<(std::ostream& stream, const jsonxx::Array& v);

