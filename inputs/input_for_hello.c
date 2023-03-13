//=============================================================================
// FILE:
//      input_for_hello.c
//
// DESCRIPTION:
//      Sample input file for HelloWorld and InjectFuncCall
//
// License: MIT
//=============================================================================

#include <stdlib.h>

int main() {
  void *p = malloc(16);
  void *q = p + 1;
  free(p);
  free(p);
  return 0;
}
