#include "threads/malloc.h"
#include <debug.h>
#include <list.h>
#include <round.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

/* A simple implementation of malloc().

   The size of each request, in bytes, is rounded up to a power
   of 2 and assigned to the "descriptor" that manages blocks of
   that size.  The descriptor keeps a list of free blocks.  If
   the free list is nonempty, one of its blocks is used to
   satisfy the request.

   Otherwise, a new page of memory, called an "arena", is
   obtained from the page allocator (if none is available,
   malloc() returns a null pointer).  The new arena is divided
   into blocks, all of which are added to the descriptor's free
   list.  Then we return one of the new blocks.

   When we free a block, we add it to its descriptor's free list.
   But if the arena that the block was in now has no in-use
   blocks, we remove all of the arena's blocks from the free list
   and give the arena back to the page allocator.

   We can't handle blocks bigger than 2 kB using this scheme,
   because they're too big to fit in a single page with a
   descriptor.  We handle those by allocating contiguous pages
   with the page allocator and sticking the allocation size at
   the beginning of the allocated block's arena header. */

/* Descriptor. */
struct desc
  {
    size_t block_size;          /* Size of each element in bytes. */
    size_t blocks_per_arena;    /* Number of blocks in an arena. */
    struct list free_list;      /* List of free blocks. */
    struct lock lock;           /* Lock. */
  };

/* Magic number for detecting arena corruption. */
#define ARENA_MAGIC 0x9a548eed

/* Arena. */
struct arena
  {
    unsigned magic;             /* Always set to ARENA_MAGIC. */
    struct desc *desc;          /* Owning descriptor, null for big block. */
    size_t free_cnt;            /* Free blocks; pages in big block. */
    struct list_elem free_elem;
  };

/* Free block. */
struct block
  {
    uint8_t level;
    uint8_t occupied;
    struct list_elem free_elem; /* Free list element. */
  };

struct lock lock;

/* Our set of descriptors. */
static struct desc descs[10];   /* Descriptors. */
static size_t desc_cnt;         /* Number of descriptors. */

static struct arena *block_to_arena (struct block *);
static struct block *arena_to_block (struct arena *);//, size_t idx);
static struct list alist;
static bool smaller_block(const struct block * b1,const struct block * b2,void * aux);
static struct block * malloc_recursive(struct block * p,unsigned int hash);
static long int pow(int base, int power);
/* Initializes the malloc() descriptors. */
void
malloc_init (void)
{
  size_t block_size;

  for (block_size = 16; block_size < PGSIZE / 2; block_size *= 2)
    {
      struct desc *d = &descs[desc_cnt++];
      ASSERT (desc_cnt <= sizeof descs / sizeof *descs);

      list_init (&d->free_list);
      list_init(&alist);
      lock_init (&lock);
    }
}

/* Obtains and returns a new block of at least SIZE bytes.
   Returns a null pointer if memory is not available. */
void * malloc (size_t size)
{
  struct block *b;
  struct arena *a;

  /* A null pointer satisfies a request for 0 bytes. */
  if (size == 0)
    return NULL;
  size += 2 * sizeof(char);
  unsigned int hash;
  for (hash = 0; hash < 7; hash++)
    if (1 << (hash + 4) >= size)
      break;
  if (hash == 7)
    {
      size_t page_cnt = DIV_ROUND_UP (size + sizeof *a, PGSIZE);
      a = palloc_get_multiple (0, page_cnt);
      if (a == NULL)
        return NULL;
      a->magic = ARENA_MAGIC;
      list_push_front(&alist,&a->free_elem);
      b = arena_to_block(a);
      b->level = 7;
      b->occupied = 1;
      b = (void * )((char *)b + 2);
      // return a pointer just above stack
      return b;
    }

  lock_acquire (&lock);
  unsigned int hash_value = hash;

  // check for a bigger block if none are empty
  while (list_empty (&(descs[hash].free_list)) && hash < 7)
    {
      hash++;
    }

  if (hash == 7) // allocate a new page
  {
    a = palloc_get_page(0);
    if (a == NULL)
      {
        lock_release (&lock);
        return NULL;
      }
    a->magic = ARENA_MAGIC;
    list_push_back(&alist,&a->free_elem);
    b = arena_to_block(a);
  } else
  {
    b = list_entry (list_pop_front (&(descs[hash].free_list)), struct block, free_elem);
  }
  struct block * lower;
  while(hash_value != hash) {
    lower = malloc_recursive(b, hash - 1);
    lower->level = hash-1;
    lower->occupied = 0;
    list_insert_ordered(&(descs[hash-1].free_list),&lower->free_elem,smaller_block,NULL);
    hash--;
  }
  b->level = hash_value;
  b->occupied = 1;
  b = (void * )((char *)b + 2);
  lock_release(&lock);
  return b;
}

/* Allocates and return A times B bytes initialized to zeroes.
   Returns a null pointer if memory is not available. */
void *
calloc (size_t a, size_t b)
{
  void *p;
  size_t size;

  /* Calculate block size and make sure it fits in size_t. */
  size = a * b;
  if (size < a || size < b)
    return NULL;

  /* Allocate and zero memory. */
  p = malloc (size);
  if (p != NULL)
    memset (p, 0, size);

  return p;
}

/* Returns the number of bytes allocated for BLOCK. */
static size_t
block_size (void *block)
{
  struct block *b = (struct block *)((char *)block - 2);
  struct arena *a = block_to_arena (b);
  struct desc *d = a->desc;

  int current_level = b->level;
  return 1 << (current_level + 4);
}

/* Attempts to resize OLD_BLOCK to NEW_SIZE bytes, possibly
   moving it in the process.
   If successful, returns the new block; on failure, returns a
   null pointer.
   A call with null OLD_BLOCK is equivalent to malloc(NEW_SIZE).
   A call with zero NEW_SIZE is equivalent to free(OLD_BLOCK). */
void *
realloc (void *old_block, size_t new_size)
{
  if (new_size == 0)
    {
      free (old_block);
      return NULL;
    }
  else
    {
      void *new_block = malloc (new_size);
      if (old_block != NULL && new_block != NULL)
        {
          size_t old_size = block_size (old_block) - 2;
          // reducing storage space
          size_t min_size = new_size < old_size ? new_size : old_size;
          memcpy (new_block, old_block, min_size);
          free (old_block);
        }
      return new_block;
    }
}

/* Frees block P, which must have been previously allocated with
   malloc(), calloc(), or realloc(). */
void
free (void *p)
{
  if (p != NULL)
    {
      p = (char *) p -  2;
      struct block *b = p;
      struct arena *a = block_to_arena (b);
      struct desc *d = a->desc;

      lock_acquire (&lock);
      // printf("%u is the buddy of %u\n",buddy,b);

      struct block * top = malloc_recursive(b, b->level);

      while (top->level == b->level && !top->occupied){
        list_remove(&(top->free_elem));

        if (top < b)
          b = top;
        b->level += 1;
        if (b->level == 7) break;

        top = malloc_recursive(b,b->level);
      }
      if ( b->level >= 7 ) {
        a = block_to_arena(b);
        list_remove(&a->free_elem);
        palloc_free_page(a);
      }
      else{
        b->occupied = 0;
        list_insert_ordered (&(descs[b->level].free_list),&b->free_elem,smaller_block,NULL);
      }
      lock_release (&lock);
    }
}

/* Returns the arena that block B is inside. */
static struct arena *
block_to_arena (struct block *b)
{
  struct arena *a = pg_round_down (b);

  /* Check that the arena is valid. */
  ASSERT (a != NULL);
  ASSERT (a->magic == ARENA_MAGIC);
  ASSERT ((pg_ofs(b) - sizeof *a)%16==0);
  return a;
}

/* Returns the (IDX - 1)'th block within arena A. */
static struct block *
arena_to_block (struct arena *a)//, size_t idx)
{
  ASSERT (a != NULL);
  ASSERT (a->magic == ARENA_MAGIC);
  return (struct block *) ((uint8_t *) a + sizeof *a);
}

void printMemory()
{
  int i,j=0;
  struct list_elem * l;
  printf("------------------------------------------------------------------------------\n");
  if (list_empty(&alist))
    printf("No free blocks\n");
  else
    for (l = list_begin(&alist); l != list_end(&alist); l = list_next(l)){
      printf("\nPage %d : \n",j++);
      struct arena *a = list_entry(l, struct arena, free_elem);
      printf("Page address = %u \n",(unsigned int) a);
      for(i = 0; i < 7; i++)
      {
          struct list_elem *e;

          printf("\tSize %d:",1 << ( i + 4) );
          for (e = list_begin (&descs[i].free_list); e != list_end (&descs[i].free_list);e = list_next (e))
          {
            struct block *b = list_entry (e, struct block, free_elem);
            if (a == block_to_arena(b))
              printf("%u  ",(unsigned int)b);

          }
          printf("\n");
      }
    }
  printf("No. of pages allocated: %d\n", j);
  printf("------------------------------------------------------------------------------\n\n");
}

bool smaller_block(const struct block * b1,const struct block * b2,void * aux){
  return ((uintptr_t)b1 < (uintptr_t)b2);
}

struct block * malloc_recursive(struct block * p,unsigned int hash)
  {
      struct block * b = arena_to_block(block_to_arena(p));
      return (struct block *)((((uintptr_t) p - (uintptr_t) b)^(1 << ( hash + 4))) + (uintptr_t) b);
  }
