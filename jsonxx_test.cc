// Author: Hong Jiang <hong@hjiang.net>

#include <cassert>
#include <sstream>
#include <string>

#include "jsonxx.h"

namespace jsonxx {
bool parse_string(std::istream& input);
bool parse_number(std::istream& input);
bool match(const std::string& pattern, std::istream& input, bool ignore_ws = true);
}

int main() {
    using namespace jsonxx;
    using namespace std;
    {
        string teststr("\"field1\"");
        istringstream input(teststr);
        assert(parse_string(input));
    }
    {
        string teststr("\"  field1\"");
        istringstream input(teststr);
        assert(parse_string(input));
    }
    {
        string teststr("  \"field1\"");
        istringstream input(teststr);
        assert(parse_string(input));
    }
    {
        string teststr("6");
        istringstream input(teststr);
        assert(parse_number(input));
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
        assert(o.parse(input));
    }
    {
        string teststr("{ \"field1 : 6 }");
        istringstream input(teststr);
        Object o;
        assert(!o.parse(input));
    }
    {
        string teststr("6");
        istringstream input(teststr);
        Value v;
        assert(v.parse(input));
    }
    {
        string teststr("asdf");
        istringstream input(teststr);
        Value v;
        assert(!v.parse(input));
    }
    {
        string teststr("true");
        istringstream input(teststr);
        Value v;
        assert(v.parse(input));
        assert(v.is<bool>());
    }
    {
        string teststr("false");
        istringstream input(teststr);
        Value v;
        assert(v.parse(input));
        assert(v.is<bool>());
        assert(v.get<bool>() == false);
    }
    {
        string teststr("null");
        istringstream input(teststr);
        Value v;
        assert(v.parse(input));
        assert(v.is<Value::Null>());
        assert(!v.is<bool>());
    }
}
