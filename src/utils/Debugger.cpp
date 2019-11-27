#include "utils/Debugger.hpp"
#include <cstdlib>
#include <iostream>

using namespace std;

static const char *DEBUG = getenv("DEBUG");
static const bool DEBUG_MODE = DEBUG != nullptr;

namespace voxer {

void Debugger::log(string info) {
  if (!DEBUG_MODE)
    return;
  cout << this->name << ": " << info << endl;
}

}
