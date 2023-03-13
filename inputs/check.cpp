#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<unordered_map>
#include<cstdint>

using namespace std;

struct Node {
  int parent;
};

vector<Node> rt = vector<Node>(1, {-2});

struct MetaData {
  uintptr_t bounds:27;
  uintptr_t perm:3;
  uintptr_t type:3;
  uintptr_t node_id:31; 
};

// set as unordered_map and free_idx as was trying something, 
// can work with just vector<int> shadowMem
unordered_map<int, MetaData> shadowMem = 
                    unordered_map<int, MetaData>(0);
vector<int> free_idx = vector<int>(0);

union LowFatPtr {
  void *cursor;
  struct {
    uintptr_t loc:54;
    uintptr_t idx:10;
  } meta;
};

extern "C" void check(void* p)
{
  LowFatPtr pt = {(void*)*(long*)p};
  // printf("Cursor at %p\n", p);
  // printf("idx: %x\n", pt->meta.idx);
  if(pt.meta.idx >= shadowMem.size() + free_idx.size() or 
      shadowMem[pt.meta.idx].node_id >= rt.size())
      return;

  printf("Cursor at %lx\n", pt.meta.loc);
  printf("pt.meta.idx: %d, shadowMem[pt.meta.idx].node_id: %d\n", pt.meta.idx, shadowMem[pt.meta.idx].node_id);

  if(rt[shadowMem[pt.meta.idx].node_id].parent == -1)
  {
    printf("Use after free!\n");
    abort();
  }
}

extern "C" void *tmalloc(size_t n)
{

  printf("\nTMALLOC\n");

  rt.push_back({0});
  void *loc = malloc(n);

  MetaData m = {n, 0, 0, rt.size() - 1};

  if(free_idx.size() == 0)
    shadowMem.insert({shadowMem.size(), m});
  else {
    shadowMem.insert({free_idx[free_idx.size() - 1], m});
    free_idx.pop_back();
  }

  LowFatPtr ret = {.meta = {(uintptr_t)loc, 
                            (uintptr_t)(shadowMem.size() - 1)
                            }};


  printf("In tmalloc, allocated %p\n", loc);
  printf("idx %x, ptr %lx, all %p\n", ret.meta.idx, ret.meta.loc, ret.cursor);
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
  printf("\nTFREE\n");
  
  LowFatPtr pt = {p};
  printf("In tfree, looking at %p\n", pt.cursor);

  if(pt.meta.idx < shadowMem.size() + free_idx.size()and 
      shadowMem[pt.meta.idx].node_id < rt.size()) {
    
    if(rt[shadowMem[pt.meta.idx].node_id].parent != -1) {

      printf("Freeable memory\n");
      free((void*) pt.meta.loc);
      rt[shadowMem[pt.meta.idx].node_id].parent = -1;
      // shadowMem.erase(pt.meta.idx);
      // free_idx.push_back(pt.meta.idx);
      printf("Freed %lx\n", pt.meta.loc);
    }
    else {
      printf("Use after free!\n");
      abort();
    }
  }
  else {
    printf("Not a good idea, undef behavior for now\n");
    printf("%d\n", pt.meta.idx);
    free(p);
    printf("Freed %p\n", p);
  }
}