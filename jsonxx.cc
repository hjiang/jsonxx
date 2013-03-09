// -*- mode: c++; c-basic-offset: 4; -*-

// Author: Hong Jiang <hong@hjiang.net>

#include "jsonxx.h"

#include <cctype>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

namespace jsonxx {

bool match(const char* pattern, std::istream& input);
bool parse_string(std::istream& input, String* value);
bool parse_number(std::istream& input, Number* value);

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
            while (cur > pattern) {
                cur--;
                input.putback(*cur);
            }
            return false;
        } else {
            cur++;
        }
    }
    return *cur == 0;
}

bool parse_string(std::istream& input, String* value) {
    char ch, delimiter = '"';
    if (!match("\"", input))  {
        delimiter = '\'';
        if (input.peek() != delimiter) {
            return false;
        }
        input.get(ch);
    }
    while(!input.eof() && input.good()) {
        input.get(ch);
        if (ch == delimiter) {
            break;
        }
        if (ch == '\\') {
            input.get(ch);
            switch(ch) {
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
                    if (ch != delimiter) {
                        value->push_back('\\');
                        value->push_back(ch);
                    } else value->push_back(ch);
                    break;
            }
        } else {
            value->push_back(ch);
        }
    }
    if (input && ch == delimiter) {
        return true;
    } else {
        return false;
    }
}

static bool parse_bool(std::istream& input, Boolean* value) {
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

static bool parse_null(std::istream& input) {
    if (match("null", input))  {
        return true;
    }
    return false;
}

bool parse_number(std::istream& input, Number* value) {
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

bool Object::parse(std::istream& input, Object& object) {
    object.value_map_.clear();

    if (!match("{", input)) {
        return false;
    }
    if (match("}", input)) {
        return true;
    }

    do {
        std::string key;
        if (!parse_string(input, &key)) {
            if (input.peek() == '}')
                break;
            return false;
        }
        if (!match(":", input)) {
            return false;
        }
        Value* v = new Value();
        if (!Value::parse(input, *v)) {
            delete v;
            break;
        }
        object.value_map_[key] = v;
    } while (match(",", input));


    if (!match("}", input)) {
        return false;
    }

    return true;
}

Value::Value() : type_(INVALID_) {}

void Value::reset() {
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

bool Value::parse(std::istream& input, Value& value) {
    value.reset();

    std::string string_value;
    if (parse_string(input, &string_value)) {
        value.string_value_ = new std::string();
        value.string_value_->swap(string_value);
        value.type_ = STRING_;
        return true;
    }
    if (parse_number(input, &value.number_value_)) {
        value.type_ = NUMBER_;
        return true;
    }

    if (parse_bool(input, &value.bool_value_)) {
        value.type_ = BOOL_;
        return true;
    }
    if (parse_null(input)) {
        value.type_ = NULL_;
        return true;
    }
    if (input.peek() == '[') {
        value.array_value_ = new Array();
        if (Array::parse(input, *value.array_value_)) {
            value.type_ = ARRAY_;
            return true;
        }
        delete value.array_value_;
    }
    value.object_value_ = new Object();
    if (Object::parse(input, *value.object_value_)) {
        value.type_ = OBJECT_;
        return true;
    }
    delete value.object_value_;
    return false;
}

Array::Array() : values_() {}

Array::~Array() {
    for (std::vector<jsonxx::Value*>::iterator i = values_.begin();
         i != values_.end(); ++i) {
        delete *i;
    }
}

bool Array::parse(std::istream& input, Array& array) {
    array.values_.clear();

    if (!match("[", input)) {
        return false;
    }

    do {
        Value* v = new Value();
        if (!Value::parse(input, *v)) {
            delete v;
            break;
        }
        array.values_.push_back(v);
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
    if (v.is<jsonxx::Number>()) {
        return stream << v.get<jsonxx::Number>();
    } else if (v.is<jsonxx::String>()) {
        return jsonxx::stream_string(stream, v.get<std::string>());
    } else if (v.is<jsonxx::Boolean>()) {
        if (v.get<jsonxx::Boolean>()) {
            return stream << "true";
        } else {
            return stream << "false";
        }
    } else if (v.is<jsonxx::Null>()) {
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
    for (std::vector<jsonxx::Value*>::const_iterator i = values.begin();
         i != values.end(); /**/) {
        stream << *(*i);
        ++i;
        if (i != values.end()) {
            stream << ", ";
        }
    }
    return stream << "]";
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


namespace jsonxx {
namespace {

std::string escape( const std::string &input, const std::vector< std::string > &map ) {
    std::string output;
    for( const auto &it : input )
        output += map[ int(it) ];
    return output;
}

void base( std::vector< std::string > &map ) {
    map.resize( 256 );
    for( int i = 0; i < 256; ++i )
        map[ i ] = std::string() + char(i);
}

const std::vector< std::string > &encxml() {
    static std::vector<std::string> map;
    if( !map.size() ) {
        base(map);
        map[ int('<') ] = "&lt;";
        map[ int('>') ] = "&gt;";
    }
    return map;
}

const std::vector< std::string > &encstr() {
    static std::vector<std::string> map;
    if( !map.size() ) {
        base(map);
        map[ int('"') ] = "\\\"";
        map[ int('\'') ] = "\\\'";
    }
    return map;
}

std::string tagname( const std::string &name ) {
    return !name.empty() ? std::string(" name=\"") + escape(name, encstr()) + "\"" : std::string();
}

template<typename T>
std::string tag( const T &t, const std::string &name = std::string(), unsigned depth = 0 ) {
    std::stringstream ss;
    std::string tab(depth, '\t');
    ss << tab << "<json:null" << tagname(name);
    ss << " />" << std::endl;
    return ss.str();
}

std::string tag( const jsonxx::Boolean &t, const std::string &name = std::string(), unsigned depth = 0 ) {
    std::stringstream ss;
    std::string tab(depth, '\t');
    ss << tab << "<json:boolean" << tagname(name) << ">";
    ss << ( t ? "true" : "false" );
    ss << "</json:boolean>" << std::endl;;
    return ss.str();
}

std::string tag( const jsonxx::Number &t, const std::string &name = std::string(), unsigned depth = 0 ) {
    std::stringstream ss;
    std::string tab(depth, '\t');
    ss << tab << "<json:number" << tagname(name) << ">";
    ss << t;
    ss << "</json:number>" << std::endl;;
    return ss.str();
}

std::string tag( const jsonxx::String &t, const std::string &name = std::string(), unsigned depth = 0 ) {
    std::stringstream ss;
    std::string tab(depth, '\t');
    ss << tab << "<json:string" << tagname(name) << ">";
    ss << escape(t, encxml());
    ss << "</json:string>" << std::endl;;
    return ss.str();
}

std::string tag( const jsonxx::Object &t, const std::string &name = std::string(), unsigned depth = 0 ) {
	std::stringstream ss;
    std::string tab(depth, '\t');
    ss << tab << "<json:object" << tagname(name) << ">" << std::endl;
    for( const auto &it : t.kv_map() )
      ss << it.second->jsonx( it.first, depth+1 );
    ss << tab << "</json:object>" << std::endl;
    return ss.str();
}

std::string tag( const jsonxx::Array &t, const std::string &name = std::string(), unsigned depth = 0 ) {
    std::stringstream ss;
    std::string tab(depth, '\t');
    ss << tab << "<json:array" << tagname(name) << ">" << std::endl;
    for( const auto &it : t.values() )
      ss << it->jsonx( std::string(), depth+1 );
    ss << tab << "</json:array>" << std::endl;
    return ss.str();
}

} // namespace

std::string Object::jsonx( const std::string &name, unsigned depth ) const {
    return tag( *this, name, depth );
}

std::string Array::jsonx( const std::string &name, unsigned depth ) const {
    return tag( *this, name, depth );
}

std::string Value::jsonx( const std::string &name, unsigned depth ) const {
    switch(type_) {
        case BOOL_:   return tag( bool_value_, name, depth );
        case ARRAY_:  return tag( *array_value_, name, depth );
        case NUMBER_: return tag( number_value_, name, depth );
        case STRING_: return tag( *string_value_, name, depth );
        case OBJECT_: return tag( *object_value_, name, depth );
        default:      return tag( jsonxx::Null(), name, depth );
    }
}

}  // namespace jsonxx
