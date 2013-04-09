// -*- mode: c++; c-basic-offset: 4; -*-

// Author: Hong Jiang <hong@hjiang.net>
// Include a few sanity tests from https://github.com/isubiker/mljson/
// Include a few samples from http://www.json.org

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
    if( !Settings::Strict )
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
    if( !Settings::Strict )
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
    if( !Settings::Strict )
    {
        string teststr("  'field1'");
        string value;
        istringstream input(teststr);
        assert(parse_string(input, &value));
        assert(value == "field1");
    }
    {
        // 'escaped text to unescaped text' test
        string teststr("\"\\b\\f\\n\\r\\t\\u0014\\u0002\"");
        string value;
        istringstream input(teststr);
        assert(parse_string(input, &value));
        assert( value == "\b\f\n\r\t\xe\x2" );
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
    if( !Settings::Strict )
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

    if( !Settings::Strict )
    {
        #define QUOTE(...) #__VA_ARGS__
        string input = QUOTE(
        {
          "name/surname":"John Smith",
          'alias': 'Joe',
          "address": {
            "streetAddress": "21 2nd Street",
            "city": "New York",
            "state": "NY",
            "postal-code": 10021,
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
          <json:string name="name/surname">John Smith</json:string>
          <json:string name="alias">Joe</json:string>
          <json:object name="address">
            <json:string name="streetAddress">21 2nd Street</json:string>
            <json:string name="city">New York</json:string>
            <json:string name="state">NY</json:string>
            <json:number name="postal-code">10021</json:number>
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
        if( o.parse(input) ) {
            cout << o.xml(JSONx) << endl;            // XML output, JSONx flavor
            cout << o.xml(JXML) << endl;             // XML output, JXML flavor
            cout << o.xml(JXMLex) << endl;           // XML output, JXMLex flavor
        } else {
            assert( !"provided JSON is valid. jsonxx::Object::operator<<() is broken!" );
        }

        assert( jsonxx::validate(input) );
    }

#   define ASSERT_ARRAY(...)  for( Array  o; assert( Array::parse(istringstream(#__VA_ARGS__), o ) ), false; );
#   define ASSERT_OBJECT(...) for( Object o; assert( Object::parse(istringstream(#__VA_ARGS__), o ) ), false; );

    // Four samples from www.json.org
    ASSERT_OBJECT( {
        "name": "Jack (\"Bee\") Nimble",
        "format": {
            "type":       "rect",
            "width":      1920,
            "height":     1080,
            "interlace":  false,
            "frame rate": 24
        }
    } );
    ASSERT_OBJECT( { "color": "blue", "closed": true, "points": [[10,10], [20,10], [20,20], [10,20]] } );
    ASSERT_ARRAY( [ [0, -1, 0], [1, 0, 0], [0, 0, 1] ] );
    ASSERT_ARRAY( ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"] );

    // Empty/raw datatypes
    ASSERT_ARRAY( [true, false, null, [], {}] );

    // Various numbers
    ASSERT_ARRAY( [[-1], [1.2]] );

    // General array with all data types
    ASSERT_ARRAY( ["hello", 0, [], {}, null, false, true] );

    // Nested objects
    ASSERT_OBJECT( {"foo":1, "bar":{"baz":2, "yaz":3}} );

    // Nested objects with trailing key/value
    ASSERT_OBJECT( {"foo":1, "em":{"a":"b"}, "bar":"aa"} );

    // Nested arrays
    ASSERT_ARRAY( [1, 2, 3, [4, 5, [7, 8, 9], 6]] );

    // Nested arrays with trailing values
    ASSERT_ARRAY( [1, 2, 3, [4, 5, [7, 8, 9], 6], 10] );

    // UTF-8
    ASSERT_OBJECT( {"text":"は 2010/11/4 at 5:50 AM に 6'45\"/km のペースで 8.42 km を走りました http://go.nike.com/9rlcovd"} );

    // Escaped UTF-8
    ASSERT_OBJECT( {"text":"\u3050\u3089\u307e\u3041\u3067\u3061\u3085\u306d\u2665\u304a\u306f\u3088\u3046\u3067\u3059\uff01"} );

    // Empty values
    ASSERT_OBJECT( {"foo":"", "bar":""} );

    // Escaped quotes
    ASSERT_OBJECT( {"foo":"\"bar\""} );

    // Newline
    ASSERT_OBJECT( {"foo":"bar\nbaz"} );

    // Could use more tests around escaping odd characters in key names that can't be used as an element name
    // Escaping invalid xml element names
    ASSERT_OBJECT( {"f•o":"bar"} );

    // Escaping our invalid xml element name escaping
    ASSERT_OBJECT( {"_foo":"bar"} );

    // Empty key name (about to check in a fix)
    ASSERT_OBJECT( {"":"bar"} );

    // Trailing commas (if permissive mode is enabled)
    if( !jsonxx::Settings::Strict ) {
        ASSERT_ARRAY( [ true, 42, 54.7, ] );
        ASSERT_OBJECT( { "hello": "world",} )
    }

    // Single-quoted strings (if permissive mode is enabled)
    if( !jsonxx::Settings::Strict ) {
        ASSERT_OBJECT( { 'single-quoted-strings': 'are "handy"' } );
    }

    {
        jsonxx::Array a;
        a << true;
        a << false;
        a << 'A';
        a << 65;
        a << 65L;
        a << 65LL;
        a << 65U;
        a << 65UL;
        a << 65ULL;
        a << 65.f;
        a << 65.0;
        a << jsonxx::Value( "hello world" );
        a << std::string("hello world");
        a << "hello world";

        assert( jsonxx::Array().parse( a.json() ) );   // self-evaluation
        assert( validate( a.json() ) );                // self-evaluation
    }

    {
        // Generate JSON document dinamically
        jsonxx::Array a;
        a << jsonxx::Null();           // C++11: 'a << nullptr' is preferred
        a << 123;
        a << "hello \"world\"";
        a << 3.1415;
        a << 99.95f;
        a << 'h';

        jsonxx::Object o;
        o << jsonxx::Object( "key1", "value" );
        o << "key2" << 123;
        o << "key3" << a;

        a << o;

        assert( jsonxx::Array().parse( a.json() ) );   // self-evaluation
        assert( validate( a.json() ) );                // self-evaluation
    }

    {
        struct custom {};

        jsonxx::Array a;
        a << true;
        a << false;

        jsonxx::Object o;
        o << jsonxx::Object( "number", 123 );
        o << jsonxx::Object( "string", "hello world" );
        o << jsonxx::Object( "boolean", false );
        o << jsonxx::Object( "null", NULL );
        o << jsonxx::Object( "array", a );
        o << jsonxx::Object( "object", jsonxx::Object("child", "object") );
        o << jsonxx::Object( "undefined", custom() );

        std::cout << o.write(JSON) << std::endl;        // same than o.json()
        std::cout << o.write(JSONx) << std::endl;       // same than o.xml()
        std::cout << o.write(JXML) << std::endl;        // same than o.xml(JXML)
        std::cout << o.write(JXMLex) << std::endl;      // same than o.xml(JXMLex)

        assert( jsonxx::Object().parse( o.json() ) );   // self-evaluation
        assert( validate( o.json() ) );                 // self-evaluation
    }

    {
        // recursion test
        jsonxx::Array a;
        a << 123;
        a << "hello world";
        a << a;
        assert( a.size() == 4 );
    }

    {
        // recursion test
        jsonxx::Object o;
        o << jsonxx::Object( "number", 123 );
        o << jsonxx::Object( "string", "world" );
        o << jsonxx::Object( "recursion", o );
        assert( o.size() == 3 );
    }

    if( !jsonxx::Settings::Strict )
    {
        // C++ style comments test
        string teststr(
                "//this is comment #1 {\n"
                "{"
                "  \"foo\" : 1,"
                "  \"bar\" : false, //this is comment #2\n"
                "  \"person\" : {\"name //this shall not be removed\" : \"GWB\", \"age\" : 60},"
                "  \"data\": [\"abcd\", 42, 54.7]"
                "} //this is comment #3"
       );
        jsonxx::Object o;
        assert( o.parse(teststr) );
    }

    cout << "All tests ok." << endl;
    return 0;
}
