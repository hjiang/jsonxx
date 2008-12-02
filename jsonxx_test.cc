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
}
