// -*- mode: c++; c-basic-offset: 4; -*-

// Author: Hong Jiang <hong@hjiang.net>

#include <cassert>
#include <sstream>
#include <string>

#include "jsonxx.h"

namespace jsonxx {
extern bool parse_string(std::istream& input, String* value);
extern bool parse_number(std::istream& input, Number* value);
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
        string teststr("'field1'");
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
        string teststr("'  field1'");
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
        string teststr("  'field1'");
        string value;
        istringstream input(teststr);
        assert(parse_string(input, &value));
        assert(value == "field1");
    }
    {
        string teststr("6");
        istringstream input(teststr);
        Number value;
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
        assert(v.is<Number>());
        assert(v.get<Number>() == 6);
    }
    {
        string teststr("+6");
        istringstream input(teststr);
        Value v;
        assert(Value::parse(input, v));
        assert(v.is<Number>());
        assert(v.get<Number>() == 6);
    }
    {
        string teststr("-6");
        istringstream input(teststr);
        Value v;
        assert(Value::parse(input, v));
        assert(v.is<Number>());
        assert(v.get<Number>() == -6);
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
        assert(v.is<Boolean>());
        assert(v.get<Boolean>());
    }
    {
        string teststr("false");
        istringstream input(teststr);
        Value v;
        assert(Value::parse(input, v));
        assert(v.is<Boolean>());
        assert(!v.get<Boolean>());
    }
    {
        string teststr("null");
        istringstream input(teststr);
        Value v;
        assert(Value::parse(input, v));
        assert(v.is<Null>());
        assert(!v.is<Boolean>());
    }
    {
        string teststr("\"field1\"");
        istringstream input(teststr);
        Value v;
        assert(Value::parse(input, v));
        assert(v.is<String>());
        assert("field1" == v.get<String>());
        ostringstream stream;
        stream << v;
        assert(stream.str() == "\"field1\"");
    }
    {
        string teststr("'field1'");
        istringstream input(teststr);
        Value v;
        assert(Value::parse(input, v));
        assert(v.is<String>());
        assert("field1" == v.get<String>());
        ostringstream stream;
        stream << v;
        assert(stream.str() == "\"field1\"");
    }
    {
        string teststr("[\"field1\", 6]");
        istringstream input(teststr);
        Array a;
        assert(Array::parse(input, a));
        assert(a.has<String>(0));
        assert("field1" == a.get<String>(0));
        assert(a.has<Number>(1));
        assert(6 == a.get<Number>(1));
        assert(!a.has<Boolean>(2));
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
        assert(1 == o.get<Number>("foo"));
        assert(o.has<Boolean>("bar"));
        assert(o.has<Object>("person"));
        assert(o.get<Object>("person").has<Number>("age"));
        assert(o.has<Array>("data"));
        assert(o.get<Array>("data").get<Number>(1) == 42);
        assert(o.get<Array>("data").get<String>(0) == "abcd");
        assert(o.get<Array>("data").get<Number>(2) == 54.7);
        assert(!o.has<Number>("data"));
    }
    {
        string teststr("{\"bar\": \"a\\rb\\nc\\td\", \"foo\": true}");
        istringstream input(teststr);
        Object o;
        assert(Object::parse(input, o));
        assert(o.has<String>("bar"));
        assert(o.get<String>("bar") == "a\rb\nc\td");
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

    {
        string teststr(
                "{"
                "  'this_array_has_formatting_issues': [true, 42, 54.7,],"
                "  'this_object_too' : {'name' : \"GWB\", \"age\" : 60,},"
                "}"
                       );
        istringstream input(teststr);
        Object o;
        assert(Object::parse(input, o));
    }
    {
        #define QUOTE(...) #__VA_ARGS__
        string input = QUOTE(
        {
          "name":"John Smith",
          'alias': 'Joe',
          "address": {
            "streetAddress": "21 2nd Street",
            "city": "New York",
            "state": "NY",
            "postalCode": 10021,
          },
          "phoneNumbers": [
            "212 555-1111",
            "212 555-2222",
          ],
          "additionalInfo": null,
          "remote": false,
          "height": 62.4,
          "ficoScore": " > 640",
        }
        );

        string sample_output = QUOTE(
        <?xml version="1.0" encoding="UTF-8"?>
        <json:object xsi:schemaLocation="http://www.datapower.com/schemas/json jsonx.xsd"
            xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:json="http://www.ibm.com/xmlns/prod/2009/jsonx">
          <json:string name="name">John Smith</json:string>
          <json:string name="alias">Joe</json:string>
          <json:object name="address">
            <json:string name="streetAddress">21 2nd Street</json:string>
            <json:string name="city">New York</json:string>
            <json:string name="state">NY</json:string>
            <json:number name="postalCode">10021</json:number>
          </json:object>
          <json:array name="phoneNumbers">
            <json:string>212 555-1111</json:string>
            <json:string>212 555-2222</json:string>
          </json:array>
          <json:null name="additionalInfo" />
          <json:boolean name="remote">false</json:boolean>
          <json:number name="height">62.4</json:number>
          <json:string name="ficoScore">&gt; 640</json:string>
        </json:object>
        );

        Object o;
        if( Object::parse( istringstream( input ), o ) ) {
            // jsonx header
            cout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
            // jsonx body
            cout << o.jsonx() << endl;
        }
    }

    cout << "All tests ok." << endl;
}
