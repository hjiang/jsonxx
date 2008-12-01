#include <iostream>

namespace jsonxx {

class Object {
  public:
    bool parse(std::istream& input);
};

bool Object::parse(std::istream& input) {
    return true;
}

}
