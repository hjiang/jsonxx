// -*- mode: c++; c-basic-offset: 4; -*-

// Author: Hong Jiang <hong@hjiang.net>

#include "jsonxx.h"

#include <cctype>
#include <iostream>
#include <iomanip>
#include <sstream>
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
        if (settings::strict) {
            return false;
        }
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
    if (settings::strict) {
        return false;
    }
    return (input.peek()==',');
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
    container::iterator i;
    for (i = value_map_.begin(); i != value_map_.end(); ++i) {
        delete i->second;
    }
}

bool Object::parse(std::istream& input, Object& object) {
    object.reset();

    if (!match("{", input)) {
        return false;
    }
    if (match("}", input)) {
        return true;
    }

    do {
        std::string key;
        if (!parse_string(input, &key)) {
            if (!settings::strict) {
                if (input.peek() == '}')
                    break;
            }
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
    for (container::iterator i = values_.begin();
         i != values_.end(); ++i) {
        delete *i;
    }
}

bool Array::parse(std::istream& input, Array& array) {
    array.reset();

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
    using namespace jsonxx;
    if (v.is<Number>()) {
        return stream << v.get<Number>();
    } else if (v.is<String>()) {
        return stream_string(stream, v.get<std::string>());
    } else if (v.is<Boolean>()) {
        if (v.get<Boolean>()) {
            return stream << "true";
        } else {
            return stream << "false";
        }
    } else if (v.is<Null>()) {
        return stream << "null";
    } else if (v.is<Object>()) {
        return stream << v.get<Object>();
    } else if (v.is<Array>()){
        return stream << v.get<Array>();
    }
    // Shouldn't reach here.
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const jsonxx::Array& v) {
    stream << "[";
    for (jsonxx::Array::container::const_iterator i = v.values().begin(),
        end = v.values().end(); i != end; /**/) {
        stream << *(*i);
        ++i;
        if (i != end) {
            stream << ", ";
        }
    }
    return stream << "]";
}

std::ostream& operator<<(std::ostream& stream, const jsonxx::Object& v) {
    stream << "{";
    for (jsonxx::Object::container::const_iterator i = v.kv_map().begin(),
        end = v.kv_map().end(); i != end; /**/) {
        jsonxx::stream_string(stream, i->first);
        stream << ": " << *(i->second);
        ++i;
        if ( i != end) {
            stream << ", ";
        }
    }
    return stream << "}";
}


namespace jsonxx {
namespace {

typedef unsigned char byte;

std::string escape_string( const std::string &input ) {
    static std::string map[256], *once = 0;
    if( !once ) {
        for( int i = 0; i < 256; ++i )
            map[ i ] = std::string() + char(i);
        map[ byte('"') ] = "\\\"";
        map[ byte('\'') ] = "\\\'";
        once = map;
    }
    std::string output;
    for( std::string::const_iterator it = input.begin(), end = input.end(); it != end; ++it )
        output += map[ byte(*it) ];
    return output;
}

std::string escape_attrib( const std::string &input ) {
    static std::string map[256], *once = 0;
    if( !once ) {
        for( int i = 0; i < 256; ++i )
            map[ i ] = "_";
        for( int i = int('a'); i < int('z'); ++i )
            map[ i ] = std::string() + char(i);
        for( int i = int('A'); i < int('Z'); ++i )
            map[ i ] = std::string() + char(i);
        for( int i = int('0'); i < int('9'); ++i )
            map[ i ] = std::string() + char(i);
        once = map;
    }
    std::string output;
    for( std::string::const_iterator it = input.begin(), end = input.end(); it != end; ++it )
        output += map[ byte(*it) ];
    return output;
}

std::string escape_tag( const std::string &input ) {
    static std::string map[256], *once = 0;
    if( !once ) {
        for( int i = 0; i < 256; ++i )
            map[ i ] = std::string() + char(i);
        map[ byte('<') ] = "&lt;";
        map[ byte('>') ] = "&gt;";
        once = map;
    }
    std::string output;
    for( std::string::const_iterator it = input.begin(), end = input.end(); it != end; ++it )
        output += map[ byte(*it) ];
    return output;
}

std::string open_tag( unsigned format, char type, const std::string &name, const std::string &attr = std::string(), const std::string &text = std::string() ) {
    std::string tagname;
    switch( format )
    {
        default:
            return std::string();

        case jsonxx::format::jxml:
            if( name.empty() )
                tagname = std::string("j son=\"") + type + '\"';
            else
                tagname = std::string("j son=\"") + type + ':' + escape_string(name) + '\"';
            break;

        case jsonxx::format::jxmlex:
            if( name.empty() )
                tagname = std::string("j son=\"") + type + '\"';
            else
                tagname = std::string("j son=\"") + type + ':' + escape_string(name) + "\" " + escape_attrib(name) + "=\"" + escape_string(text) + "\"";
            break;

        case jsonxx::format::jsonx:
            if( !name.empty() )
                tagname = std::string(" name=\"") + escape_string(name) + "\"";
            switch( type ) {
                default:
                case '0': tagname = "json:null" + tagname; break;
                case 'b': tagname = "json:boolean" + tagname; break;
                case 'a': tagname = "json:array" + tagname; break;
                case 's': tagname = "json:string" + tagname; break;
                case 'o': tagname = "json:object" + tagname; break;
                case 'n': tagname = "json:number" + tagname; break;
            }
            break;
    }

    return std::string("<") + tagname + attr + ">";
}

std::string close_tag( unsigned format, char type ) {
    switch( format )
    {
        default:
            return std::string();

        case jsonxx::format::jxml:
        case jsonxx::format::jxmlex:
            return "</j>";

        case jsonxx::format::jsonx:
            switch( type ) {
                default:
                case '0': return "</json:null>";
                case 'b': return "</json:boolean>";
                case 'a': return "</json:array>";
                case 'o': return "</json:object>";
                case 's': return "</json:string>";
                case 'n': return "</json:number>";
            }
    }
}

std::string tag( unsigned format, unsigned depth, const std::string &name, const jsonxx::Value &t, const std::string &attr = std::string() ) {
    std::stringstream ss;
    const std::string tab(depth, '\t');

    switch( t.type_ )
    {
        default:
        case jsonxx::Value::NULL_:
            return tab + open_tag( format, '0', name, " /" ) + '\n';

        case jsonxx::Value::BOOL_:
            ss << ( t.bool_value_ ? "true" : "false" );
            return tab + open_tag( format, 'b', name, std::string(), format == jsonxx::format::jxmlex ? ss.str() : std::string() )
                       + ss.str()
                       + close_tag( format, 'b' ) + '\n';

        case jsonxx::Value::ARRAY_:
            for(Array::container::const_iterator it = t.array_value_->values().begin(),
                end = t.array_value_->values().end(); it != end; ++it )
              ss << tag( format, depth+1, std::string(), **it );
            return tab + open_tag( format, 'a', name, attr ) + '\n'
                       + ss.str()
                 + tab + close_tag( format, 'a' ) + '\n';

        case jsonxx::Value::STRING_:
            ss << escape_tag( *t.string_value_ );
            return tab + open_tag( format, 's', name, std::string(), format == jsonxx::format::jxmlex ? ss.str() : std::string() )
                       + ss.str()
                       + close_tag( format, 's' ) + '\n';

        case jsonxx::Value::OBJECT_:
            for(Object::container::const_iterator it=t.object_value_->kv_map().begin(),
                end = t.object_value_->kv_map().end(); it != end ; ++it)
              ss << tag( format, depth+1, it->first, *it->second );
            return tab + open_tag( format, 'o', name, attr ) + '\n'
                       + ss.str()
                 + tab + close_tag( format, 'o' ) + '\n';

        case jsonxx::Value::NUMBER_:
            ss << t.number_value_;
            return tab + open_tag( format, 'n', name, std::string(), format == jsonxx::format::jxmlex ? ss.str() : std::string() )
                       + ss.str()
                       + close_tag( format, 'n' ) + '\n';
    }
}

const char *defheader[] = {
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
         JSONXX_XML_TAG "\n",

    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
         JSONXX_XML_TAG "\n",

    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
         JSONXX_XML_TAG "\n"
};
const char *defrootattrib[] = {
    " xsi:schemaLocation=\"http://www.datapower.com/schemas/json jsonx.xsd\""
        " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
        " xmlns:json=\"http://www.ibm.com/xmlns/prod/2009/jsonx\"",

    "",

    ""
};

} // namespace

std::string Object::xml( unsigned format, const std::string &header, const std::string &attrib ) const {
    assert( format == format::jsonx || format == format::jxml || format == format::jxmlex );

    jsonxx::Value v;
    v.object_value_ = const_cast<jsonxx::Object*>(this);
    v.type_ = jsonxx::Value::OBJECT_;

    std::string result = tag( format, 0, std::string(), v, attrib.empty() ? std::string(defrootattrib[format]) : attrib );

    v.object_value_ = 0;
    return ( header.empty() ? std::string(defheader[format]) : header ) + result;
}

std::string Array::xml( unsigned format, const std::string &header, const std::string &attrib ) const {
    assert( format == format::jsonx || format == format::jxml || format == format::jxmlex );

    jsonxx::Value v;
    v.array_value_ = const_cast<jsonxx::Array*>(this);
    v.type_ = jsonxx::Value::ARRAY_;

    std::string result = tag( format, 0, std::string(), v, attrib.empty() ? std::string(defrootattrib[format]) : attrib );

    v.array_value_ = 0;
    return ( header.empty() ? std::string(defheader[format]) : header ) + result;
}

bool validate( std::istream &input ) {

    // trim chars
    for( char ch(0); !input.eof() && input.peek() <= 32; )
        input.get(ch);

    // validate json
    if( input.peek() == '{' )
    {
        jsonxx::Object o;
        if( o.parse( input, o ) )
            return true;
    }
    else
    if( input.peek() == '[' )
    {
        jsonxx::Array a;
        if( a.parse( input, a ) )
            return true;
    }

    // bad json, return empty xml
    return false;
}

bool validate( const std::string &input ) {
    return jsonxx::validate( std::istringstream(input) );
}

std::string xml( std::istream &input, unsigned format ) {
    assert( format == format::jsonx || format == format::jxml || format == format::jxmlex );

    // trim chars
    for( char ch(0); !input.eof() && input.peek() <= 32; )
        input.get(ch);

    // validate json, then transform
    if( input.peek() == '{' )
    {
        jsonxx::Object o;
        if( o.parse( input, o ) )
            return o.xml(format);
    }
    else
    if( input.peek() == '[' )
    {
        jsonxx::Array a;
        if( a.parse( input, a ) )
            return a.xml(format);
    }

    // bad json, return empty xml
    return defheader[format];
}

std::string xml( const std::string &input, unsigned format ) {
    return jsonxx::xml( std::istringstream(input), format );
}

}  // namespace jsonxx
