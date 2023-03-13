#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<cstdint>

using namespace std;

struct Node {
  int parent;
};

vector<Node> rt = vector<Node>(1, {-2});

union LowFatPtr {
  void *cursor;
  struct {
    uintptr_t loc:54;
    uintptr_t parent:10;
  } meta;
};

extern "C" void check(void* p)
{
  LowFatPtr* pt = (LowFatPtr*)p;
  // printf("Cursor at %p\n", p);
  // printf("Parent: %x\n", pt->meta.parent);

  if(pt->meta.parent == 0x3ff)
  {
    printf("Use after free!\n");
    abort();
  }
}

extern "C" void *tmalloc(size_t n)
{

  rt.push_back({0});
  void *loc = malloc(n);

  LowFatPtr ret = {loc};

  printf("In tmalloc, allocated %p\n", loc);
  printf("Parent %x, ptr %lx, all %p\n", ret.meta.parent, ret.meta.loc, ret.cursor);
  return ret.cursor;

  /*
  if 
  void* val = tmalloc(8);
  I want val = loc i.e *val = *loc
  and *((int*)val - 1) = node_id
  */
}

extern "C" void tfree(void* p)
{
  printf("\n\nTFREE\n\n");
  check(p);
  
  LowFatPtr pt = {p};
  printf("In tfree, looking at %p\n", pt.cursor);

  if(pt.meta.parent == 0)
  {
    printf("Freeable memory\n");
    free(p);
    pt.meta.parent = 0x3ff;
  }

  else
  {
    printf("Not a good idea, undef behavior for now\n");
    free(p);
  }

  printf("Freed, now the root is parent is set as 1023, so look at %p\n", pt.cursor);

  *&p = 0;
}