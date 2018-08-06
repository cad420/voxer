#include "Debugger.h"
#include <iostream>
#include <cstdlib>

using namespace std;

static const char *DEBUG = getenv("DEBUG");
static const bool DEBUG_MODE = DEBUG != nullptr;

void Debugger::log(string info) {
  if (!DEBUG_MODE) return;
  cout << this->name << ": " << info << endl;
}