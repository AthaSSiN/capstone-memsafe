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
  // int *p = (int*)malloc(16);
  void *p = malloc(16);
  free(p);
  p = malloc(20);
  // void *temp = (void*)((long)p & 0x3FFFFFFFFFFFFF);
  // printf("%lx, %lx\n", p, temp);
  // char *q = p + 15;
  int *z = (int*)((int*)p + 3);
  *z = 100;
  printf("%d\n", *z);
  // void *q = p + 1;
  // free(p);
  // p = (int*)malloc(20);
  // *(p + 1) = 5;
  // long idx = (long)p & 0xFFC0000000000000;
  // // printf("idx: %lx, p: %lx\n", idx, p);
  // p = (void*)((long)p ^ idx);
  // call load or store here
  // // printf("idx: %lx, p: %lx\n", idx, p);
  // p = (void*)((long)p | idx);
  // printf("idx: %lx, p: %lx\n", idx, p);
  // void *q = malloc(30);
  free(p);
  // free(p);
  // free(q);
  // free(q);
  return 0;
}
