// Author: Hong Jiang <hong@hjiang.net>

#include <cassert>
#include <sstream>
#include <string>

#include "jsonxx.h"

namespace jsonxx {
bool parse_string(std::istream& input, std::string* value);
bool parse_number(std::istream& input, long* value);
bool match(const char* pattern, std::istream& input);
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
        long value;
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
        assert(v.is<long>());
        assert(v.get<long>() == 6);
    }
    {
        string teststr("+6");
        istringstream input(teststr);
        Value v;
        assert(Value::parse(input, v));
        assert(v.is<long>());
        assert(v.get<long>() == 6);
    }
    {
        string teststr("-6");
        istringstream input(teststr);
        Value v;
        assert(Value::parse(input, v));
        assert(v.is<long>());
        assert(v.get<long>() == -6);
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
        assert(a.has<long>(1));
        assert(6 == a.get<long>(1));
        assert(!a.has<bool>(2));
    }
    {
        string teststr(
                "{"
                "  \"foo\" : 1,"
                "  \"bar\" : false,"
                "  \"person\" : {\"name\" : \"GWB\", \"age\" : 60},"
                "  \"data\": [\"abcd\", 42]"
                "}"
                       );
        istringstream input(teststr);
        Object o;
        assert(Object::parse(input, o));
        assert(1 == o.get<long>("foo"));
        assert(o.has<bool>("bar"));
        assert(o.has<Object>("person"));
        assert(o.get<Object>("person").has<long>("age"));
        assert(o.has<Array>("data"));
        assert(o.get<Array>("data").get<long>(1) == 42);
        assert(o.get<Array>("data").get<string>(0) == "abcd");
        assert(!o.has<long>("data"));
    }
	{
		string teststr("{\"bar\": \"a\\rb\\nc\\td\", \"foo\": true}");
		istringstream input(teststr);
		Object o;
		assert(Object::parse(input, o));
		ostringstream output;
		cout << output.str() << endl;
		assert(output.str() == teststr);
	}
}
