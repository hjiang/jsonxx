// Author: Hong Jiang <hong@hjiang.net>

#include <iostream>

namespace jsonxx {

// A JSON Object
class Object {
  public:
    bool parse(std::istream& input);
};

}  // namespace jsonxx
