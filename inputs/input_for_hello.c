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
  // void *q = p + 1;
  free(p);
  p = malloc(20);
  void *q = malloc(30);
  free(p);
  free(q);
  free(q);
  return 0;
}
