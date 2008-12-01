#include <sstream>
#include <string>

#include "jsonxx.hh"

int main() {
    using namespace jsonxx;
    using namespace std;
    {
        string teststr("{ \"field1\" : 6 }");
        istringstream input(teststr);
        Object o;
        assert(o.parse(input));
    }
}
