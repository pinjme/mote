#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include "V8Engine.h"

void log_notice(string message);
void fatal(string message);


int main(int argc, char **argv) {

  V8Engine* e=new V8Engine();
//  e->run("log_notice(\"Worked!\");");
  e->run("\"Worked!\"");

  return 0;
}

void log_notice(string message) {

    cout << message << endl;
}

void fatal(string message) {

  log_notice("fatal: " + message);
  exit(1);
}
