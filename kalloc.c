// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"


extern pte_t* walkpgdir_(pde_t *pgdir, const void *va, int alloc);
extern void remove_from_lru(char* mem);
extern void add_to_lru(char *mem, pde_t *pgdir);
extern void remove_from_swapspace(pte_t *pte);
extern int add_to_swapspace();

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;

// MYCODE
struct spinlock swap_lock;

struct page pages[PHYSTOP/PGSIZE];
struct page *page_lru_head;
char *swap_track;
int num_free_pages;
int num_lru_pages;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  num_free_pages = 0;
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  if(kmem.use_lock)
    release(&kmem.lock);
  // MYCODE
  struct page *p = &pages[V2P(v) / PGSIZE];
  p->vaddr = 0;
  num_free_pages++;
}

// MYCODE
void pages_init() { 
  char* mem;
  if((mem = kalloc()) == 0)
    panic("pages_init no memory");
  memset(mem, 0, PGSIZE);
  for(int i = 0; i < PHYSTOP / PGSIZE; i++){
    pages[i] = *(struct page*)(mem + sizeof(struct page) * i);
    pages[i].next = 0;
    pages[i].pgdir = 0;
    pages[i].prev = 0;
    pages[i].vaddr = 0;
  }
  num_lru_pages = 0;
  if((mem = kalloc()) == 0)
    panic("pages_init no memory");
  memset(mem, 0, PGSIZE);
  swap_track = mem;
  initlock(&swap_lock, "swaplock");
}

int evict(){
  cprintf("In evict: num_lru_pages: %d\n", num_lru_pages);
  cprintf("In evict: num_free_pages: %d\n", num_free_pages);

  if(num_lru_pages == 0){
    cprintf("Out of memory");
    return 0;
  }
  while(1){
    if(page_lru_head->pgdir == 0){
      cprintf("smt wrong");
      page_lru_head = page_lru_head->next;
      continue;
    }
    pde_t *pgdir = page_lru_head->pgdir;
    char* va = page_lru_head->vaddr;
    cprintf("ADDR: %x\n", (uint)va);
    pte_t *pte;
    if((pte = walkpgdir_(pgdir, va, 0)) == 0){
      cprintf("pgtable does not exist");
      return 0;
    }
    // Access bit 1
    if((*pte & PTE_A)){
      cprintf("access bit 1\n");
      *pte = *pte & !PTE_A;
    }
    // Access bit 0
    else{
      cprintf("access bit 0\n");
      int offset = add_to_swapspace();
      swapwrite((char*)PTE_ADDR(*pte), offset);
      
      kfree(va);
      panic("remove from lru done\n");
      remove_from_lru(va);
      
      *pte = (PTE_ADDR(*pte) ^ *pte) | offset;
      *pte = *pte & !PTE_P;
      break;
    }
    page_lru_head = page_lru_head->next;
  }
  return 1;
}
// ~

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

try_again:
  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(!r){
    release(&kmem.lock);
    if(evict() == 0){
      return 0;
    }
    panic("Done evict\n");
	  goto try_again;
  }
  if(r)
    kmem.freelist = r->next;
  if(kmem.use_lock)
    release(&kmem.lock);
  // MYCODE
  struct page *p = &pages[V2P((char*)r) / PGSIZE];
  p->vaddr = (char*)r;
  num_free_pages--;
  return (char*)r;
}