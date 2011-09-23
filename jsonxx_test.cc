// -*- mode: c++; c-basic-offset: 4; -*-

// Author: Hong Jiang <hong@hjiang.net>

#include <cassert>
#include <sstream>
#include <string>

#include "jsonxx.h"

namespace jsonxx {
extern bool parse_string(std::istream& input, std::string* value);
extern bool parse_number(std::istream& input, number* value);
extern bool match(const char* pattern, std::istream& input);
}

int main() {
    using namespace jsonxx;
    using namespace std;
    {
        string teststr("\"field1\"");
        string value;
        istringstream input(teststr);
        assert(parse_string(input, &value));
        assert(value == "field1");
    }
    {
        string teststr("\"  field1\"");
        string value;
        istringstream input(teststr);
        assert(parse_string(input, &value));
        assert(value == "  field1");
    }
    {
        string teststr("  \"field1\"");
        string value;
        istringstream input(teststr);
        assert(parse_string(input, &value));
        assert(value == "field1");
    }
    {
        string teststr("6");
        istringstream input(teststr);
        number value;
        assert(parse_number(input, &value));
        assert(value == 6);
    }
    {
        string teststr(" }");
        istringstream input(teststr);
        assert(match("}", input));
    }
    {
        string teststr("{ \"field1\" : 6 }");
        istringstream input(teststr);
        Object o;
        assert(Object::parse(input, o));
    }
    {
        string teststr("{ \"field1 : 6 }");
        istringstream input(teststr);
        Object o;
        assert(!Object::parse(input, o));
    }
    {
        string teststr("6");
        istringstream input(teststr);
        Value v;
        assert(Value::parse(input, v));
        assert(v.is<number>());
        assert(v.get<number>() == 6);
    }
    {
        string teststr("+6");
        istringstream input(teststr);
        Value v;
        assert(Value::parse(input, v));
        assert(v.is<number>());
        assert(v.get<number>() == 6);
    }
    {
        string teststr("-6");
        istringstream input(teststr);
        Value v;
        assert(Value::parse(input, v));
        assert(v.is<number>());
        assert(v.get<number>() == -6);
    }
    {
        string teststr("asdf");
        istringstream input(teststr);
        Value v;
        assert(!Value::parse(input, v));
    }
    {
        string teststr("true");
        istringstream input(teststr);
        Value v;
        assert(Value::parse(input, v));
        assert(v.is<bool>());
        assert(v.get<bool>());
    }
    {
        string teststr("false");
        istringstream input(teststr);
        Value v;
        assert(Value::parse(input, v));
        assert(v.is<bool>());
        assert(!v.get<bool>());
    }
    {
        string teststr("null");
        istringstream input(teststr);
        Value v;
        assert(Value::parse(input, v));
        assert(v.is<Value::Null>());
        assert(!v.is<bool>());
    }
    {
        string teststr("\"field1\"");
        istringstream input(teststr);
        Value v;
        assert(Value::parse(input, v));
        assert(v.is<std::string>());
        assert("field1" == v.get<std::string>());
        ostringstream stream;
        stream << v;
        assert(stream.str() == "\"field1\"");
    }
    {
        string teststr("[\"field1\", 6]");
        istringstream input(teststr);
        Array a;
        assert(Array::parse(input, a));
        assert(a.has<std::string>(0));
        assert("field1" == a.get<std::string>(0));
        assert(a.has<number>(1));
        assert(6 == a.get<number>(1));
        assert(!a.has<bool>(2));
    }
    {
        string teststr(
                "{"
                "  \"foo\" : 1,"
                "  \"bar\" : false,"
                "  \"person\" : {\"name\" : \"GWB\", \"age\" : 60},"
                "  \"data\": [\"abcd\", 42, 54.7]"
                "}"
                       );
        istringstream input(teststr);
        Object o;
        assert(Object::parse(input, o));
        assert(1 == o.get<number>("foo"));
        assert(o.has<bool>("bar"));
        assert(o.has<Object>("person"));
        assert(o.get<Object>("person").has<number>("age"));
        assert(o.has<Array>("data"));
        assert(o.get<Array>("data").get<number>(1) == 42);
        assert(o.get<Array>("data").get<string>(0) == "abcd");
        assert(o.get<Array>("data").get<number>(2) == 54.7);
        assert(!o.has<number>("data"));
    }
    {
        string teststr("{\"bar\": \"a\\rb\\nc\\td\", \"foo\": true}");
        istringstream input(teststr);
        Object o;
        assert(Object::parse(input, o));
        assert(o.has<std::string>("bar"));
        assert(o.get<std::string>("bar") == "a\rb\nc\td");
    }
    {
        string teststr("[ ]");
        istringstream input(teststr);
        ostringstream output;
        Array root;
        assert(Array::parse(input, root));
        output << root;
    }

    {
        string teststr("{}");
        istringstream input(teststr);
        Object o;
        assert(Object::parse(input, o));
    }

    {
        string teststr("{\"attrs\":{}}");
        istringstream input(teststr);
        Object o;
        assert(Object::parse(input, o));
        assert(o.has<Object>("attrs"));
    }

    {
        string teststr("{\"place\":{\"full_name\":\"Limburg, The Netherlands\""
                       ",\"attributes\":{},\"name\":\"Limburg\","
                       "\"place_type\":\"admin\",\"bounding_box\":{"
                       "\"type\":\"Polygon\",\"coordinates\":"
                       "[[[5.5661376,50.750449],[6.2268848,50.750449],"
                       "[6.2268848,51.7784841],[5.5661376,51.7784841]]]},"
                       "\"url\":\"http:\\/\\/api.twitter.com\\/1\\/geo\\/id\\/"
                       "4ef0c00cbdff9ac8.json\",\"country_code\":\"NL\","
                       "\"id\":\"4ef0c00cbdff9ac8\","
                       "\"country\":\"The Netherlands\"}}");
        istringstream input(teststr);
        Object o;
        assert(Object::parse(input, o));
        assert(o.has<Object>("place"));
        assert(o.get<Object>("place").has<Object>("attributes"));
    }
}
