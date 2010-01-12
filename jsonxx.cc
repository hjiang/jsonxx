// Author: Hong Jiang <hong@hjiang.net>

#include "jsonxx.h"

#include <cctype>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace jsonxx {

// Try to consume characters from the input stream and match the
// pattern string.
bool match(const char* pattern, std::istream& input) {
    input >> std::ws;
    const char* cur(pattern);
    char ch(0);
    while(input && !input.eof() && *cur != 0) {
        input.get(ch);
        if (ch != *cur) {
            input.putback(ch);
            return false;
        } else {
            cur++;
        }
    }
    return *cur == 0;
}

bool parse_string(std::istream& input, std::string* value) {
    if (!match("\"", input))  {
        return false;
    }
    char ch;
    while(!input.eof() && input.good()) {
        input.get(ch);
        if (ch == '"') {
            break;
        }
        if (ch == '\\') {
            input.get(ch);
            switch(ch) {
            case '"':
            case '\\':
            case '/':
                value->push_back(ch);
                break;
            case 'b':
                value->push_back('\b');
                break;
            case 'f':
                value->push_back('\f');
                break;
            case 'n':
                value->push_back('\n');
                break;
            case 'r':
                value->push_back('\r');
                break;
            case 't':
                value->push_back('\t');
                break;
            default:
                value->push_back('\\');
                value->push_back(ch);
                break;
            }
        } else {
            value->push_back(ch);
        }
    }
    if (input && ch == '"') {
        return true;
    } else {
        return false;
    }
}

bool parse_bool(std::istream& input, bool* value) {
    if (match("true", input))  {
        *value = true;
        return true;
    }
    if (match("false", input)) {
        *value = false;
        return true;
    }
    return false;
}

bool parse_null(std::istream& input) {
    if (match("null", input))  {
        return true;
    }
    return false;
}

bool parse_number(std::istream& input, long* value) {
    input >> std::ws;
    input >> *value;
    if (input.fail()) {
        input.clear();
        return false;
    }
    return true;
}

Object::Object() : value_map_() {}

Object::~Object() {
    std::map<std::string, Value*>::iterator i;
    for (i = value_map_.begin(); i != value_map_.end(); ++i) {
        delete i->second;
    }
}

bool Object::parse(std::istream& input) {
    if (!match("{", input)) {
        return false;
    }

    do {
        std::string key;
        if (!parse_string(input, &key)) {
            return false;
        }
        if (!match(":", input)) {
            return false;
        }
        Value* v = new Value();
        if (!v->parse(input)) {
            delete v;
            break;
        }
        value_map_[key] = v;
    } while (match(",", input));

    if (!match("}", input)) {
        return false;
    }
    return true;
}

Value::Value() : type_(INVALID_) {}

Value::~Value() {
    if (type_ == STRING_) {
        delete string_value_;
    }
    if (type_ == OBJECT_) {
        delete object_value_;
    }
    if (type_ == ARRAY_) {
        delete array_value_;
    }
}

bool Value::parse(std::istream& input) {
    std::string string_value;
    if (parse_string(input, &string_value)) {
        string_value_ = new std::string();
        string_value_->swap(string_value);
        type_ = STRING_;
        return true;
    }
    if (parse_number(input, &integer_value_)) {
        type_ = INTEGER_;
        return true;
    }

    if (parse_bool(input, &bool_value_)) {
        type_ = BOOL_;
        return true;
    }
    if (parse_null(input)) {
        type_ = NULL_;
        return true;
    }
    if (input.peek() == '[') {
        array_value_ = new Array();
        if (array_value_->parse(input)) {
            type_ = ARRAY_;
            return true;
        }
        delete array_value_;
    }
    object_value_ = new Object();
    if (object_value_->parse(input)) {
        type_ = OBJECT_;
        return true;
    }
    delete object_value_;
    return false;
}

Array::Array() : values_() {}

Array::~Array() {
    for (unsigned int i = 0; i < values_.size(); ++i) {
        delete values_[i];
    }
}

bool Array::parse(std::istream& input) {
    if (!match("[", input)) {
        return false;
    }

    do {
        Value* v = new Value();
        if (!v->parse(input)) {
            delete v;
            break;
        }
        values_.push_back(v);
    } while (match(",", input));

    if (!match("]", input)) {
        return false;
    }
    return true;
}

static std::ostream& stream_string(std::ostream& stream,
        const std::string& string) {
    stream << '"';
    for (std::string::const_iterator i = string.begin(),
            e = string.end(); i != e; ++i) {
		switch (*i) {
		case '"':
			stream << "\\\"";
			break;
		case '\\':
			stream << "\\\\";
			break;
		case '/':
			stream << "\\/";
			break;
		case '\b':
			stream << "\\b";
			break;
		case '\f':
			stream << "\\f";
			break;
		case '\n':
			stream << "\\n";
			break;
		case '\r':
			stream << "\\r";
			break;
		case '\t':
			stream << "\\t";
			break;
		default:
			if (*i < 32) {
				stream << "\\u" << std::hex << std::setw(6) <<
					std::setfill('0') << static_cast<int>(*i) << std::dec <<
					std::setw(0);
			} else {
				stream << *i;
			}
        }
    }
    stream << '"';
    return stream;
}

}  // namespace jsonxx

std::ostream& operator<<(std::ostream& stream, const jsonxx::Value& v) {
  if (v.is<long>()) {
    return stream << v.get<long>();
  } else if (v.is<std::string>()) {
    return jsonxx::stream_string(stream, v.get<std::string>());
  } else if (v.is<bool>()) {
    if (v.get<bool>()) {
      return stream << "true";
    } else {
      return stream << "false";
    }
  } else if (v.is<jsonxx::Value::Null>()) {
    return stream << "null";
  } else if (v.is<jsonxx::Object>()) {
    return stream << v.get<jsonxx::Object>();
  } else if (v.is<jsonxx::Array>()){
    return stream << v.get<jsonxx::Array>();
  }
  // Shouldn't reach here.
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const jsonxx::Array& v) {
  stream << "[";
  const std::vector<jsonxx::Value*>& values(v.values());
  for (unsigned int i = 0; i < values.size()-1; ++i) {
    stream << *(values[i]) << ", ";
  }
  return stream << *(values[values.size()-1]) << "]";
}

std::ostream& operator<<(std::ostream& stream, const jsonxx::Object& v) {
  stream << "{";
  const std::map<std::string, jsonxx::Value*>& kv(v.kv_map());
  for (std::map<std::string, jsonxx::Value*>::const_iterator i = kv.begin();
       i != kv.end(); /**/) {
    jsonxx::stream_string(stream, i->first);
    stream << ": " << *(i->second);
    ++i;
    if ( i != kv.end()) {
      stream << ", ";
    }
  }
  return stream << "}";
}
