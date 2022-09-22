// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 11
struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head;
} bache[NBUCKET];

char *bache_name[] = {
  "bache1",
  "bache2",
  "bache3",
  "bache4",
  "bache5",
  "bache6",
  "bache7",
  "bache8",
  "bache9",
  "bache10",
  "bache11",
};

void
binit(void)
{
  for(int i=0;i<NBUCKET;i++){
    struct buf *b;
    initlock(&bache[i].lock, bache_name[i]);

    // Create linked list of buffers
    bache[i].head.prev = &bache[i].head;
    bache[i].head.next = &bache[i].head;
    for(b = bache[i].buf; b < bache[i].buf+NBUF; b++){
      b->next = bache[i].head.next;
      b->prev = &bache[i].head;
      initsleeplock(&b->lock, "buffer");
      bache[i].head.next->prev = b;
      bache[i].head.next = b;
    }
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int i = blockno%NBUCKET;
  acquire(&bache[i].lock);

  // Is the block already cached?
  for(b = bache[i].head.next; b != &bache[i].head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bache[i].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for(b = bache[i].head.prev; b != &bache[i].head; b = b->prev){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bache[i].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");
  int i=b->blockno% NBUCKET;
  releasesleep(&b->lock);
  acquire(&bache[i].lock); 
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bache[i].head.next;
    b->prev = &bache[i].head;
    bache[i].head.next->prev = b;
    bache[i].head.next = b;
  }
  
  release(&bache[i].lock);
}

void
bpin(struct buf *b) {
  int i=b->blockno% NBUCKET;
  acquire(&bache[i].lock);
  b->refcnt++;
  release(&bache[i].lock);
}

void
bunpin(struct buf *b) {
  int i=b->blockno% NBUCKET;
  acquire(&bache[i].lock);
  b->refcnt--;
  release(&bache[i].lock);
}


