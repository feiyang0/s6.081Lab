// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

char* name[8]={
  "cpu1",
  "cpu2",
  "cpu3",
  "cpu4",
  "cpu5",
  "cpu6",
  "cpu7",
  "cpu8",
};

struct {
  struct spinlock lock[NCPU];
  struct run *freelist[NCPU];
} kmem;

void
kinit()
{
  for(int i=0;i<NCPU;i++){
    initlock(&kmem.lock[i], name[i]);
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();
  int i = cpuid();
  pop_off();
  acquire(&kmem.lock[i]);
  r->next = kmem.freelist[i];
  kmem.freelist[i] = r;
  release(&kmem.lock[i]);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();
  int i = cpuid();
  pop_off();

  acquire(&kmem.lock[i]);
  r = kmem.freelist[i];
  if(r)
    kmem.freelist[i] = r->next;
  else {  //内存不足
    for(int i=0;i<NCPU;i++){
      // 从有内存的cpu中拿
      if(kmem.freelist[i]){
        r=kmem.freelist[i];
        kmem.freelist[i]=r->next;
        break;
      }
    }
  }
  release(&kmem.lock[i]);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
