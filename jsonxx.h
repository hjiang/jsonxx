// Author: Hong Jiang <hong@hjiang.net>

#include <iostream>

namespace jsonxx {

// A JSON Object
class Object {
  public:
    bool parse(std::istream& input);
};

// A value could be a number, an array, a string, an object, a
// boolean, or null
class Value {
  public:
    bool parse(std::istream& input);
};

}  // namespace jsonxx
