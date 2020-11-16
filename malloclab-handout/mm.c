/* 
 * mm-implicit.c -  Simple allocator based on implicit free lists, 
 *                  first fit placement, and boundary tag coalescing. 
 *
 * Each block has header and footer of the form:
 * 
 *      31                     3  2  1  0 
 *      -----------------------------------
 *     | s  s  s  s  ... s  s  s  0  0  a/f
 *      ----------------------------------- 
 * 
 * where s are the meaningful size bits and a/f is set 
 * iff the block is allocated. The list has the following form:
 *
 * begin                                                          end
 * heap                                                           heap  
 *  -----------------------------------------------------------------   
 * |  pad   | hdr(8:a) | ftr(8:a) | zero or more usr blks | hdr(8:a) |
 *  -----------------------------------------------------------------
 *          |       prologue      |                       | epilogue |
 *          |         block       |                       | block    |
 *
 * The allocated prologue and epilogue blocks are overhead that
 * eliminate edge conditions during coalescing.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
  /* Team name */
  "Nick's",
  /* First member's full name */
  "Nicolas Mavromatis",
  /* First member's email address */
  "nima6629@colorado.edu",
  /* Second member's full name (leave blank if none) */
  " ",
  /* Second member's email address (leave blank if none) */
  " "
};
/*
//Solution adapted from CSAP 3E (Bryant and O'Hallaron) malloc implicit list implementation 
//To instead use an explicit free list with next and prev pointers stored in each block.
//New blocks are always added at the head of the list (LIFO) as list_ptr.
//The minimum block size is 24 bytes (8 for header+footer, +16 for 2 pointers)

* General structure of blocks
 *  -----------------------------------------------------------------   
 * |0:pad(4)|4:pro hdr|8:prev|16:next|24: pro ftr|USER BLOCKS|??:epi hdr
 *  -----------------------------------------------------------------
      
 *Free User Block
 *  -----------------------------------------------------------------   
 * |+0:hdr|+8:prev|+16:next|+??:ftr|
 *  -----------------------------------------------------------------
 
 *Occupied User Block
 *  -----------------------------------------------------------------   
 * |+0:hdr|PAYLOAD........|+??:ftr|
 *  -----------------------------------------------------------------
 */
/////////////////////////////////////////////////////////////////////////////
// Constants and macros
//
// These correspond to the material in Figure 9.43 of the text
// The macros have been turned into C++ inline functions to
// make debugging code easier.
//
/////////////////////////////////////////////////////////////////////////////
#define WSIZE       4       /* word size (bytes) */  
#define DSIZE       8       /* doubleword size (bytes) */
#define CHUNKSIZE  (1<<12)  /* initial heap size (bytes) */
#define OVERHEAD    8       /* overhead of header and footer (bytes) */

//my macro
#define MINSIZE     24 //8 bytes for header+footer, 16 bytes for 2 ptrs

static inline int MAX(int x, int y) {
  return x > y ? x : y;
}

//
// Pack a size and allocated bit into a word
// We mask of the "alloc" field to insure only
// the lower bit is used
//
static inline uint32_t PACK(uint32_t size, int alloc) {
  return ((size) | (alloc & 0x1));
}

//
// Read and write a word at address p
//
static inline uint32_t GET(void *p) { return  *(uint32_t *)p; }
static inline void PUT( void *p, uint32_t val)
{
  *((uint32_t *)p) = val;
}

//
// Read the size and allocated fields from address p
//
static inline uint32_t GET_SIZE( void *p )  { 
  return GET(p) & ~0x7;
}

static inline int GET_ALLOC( void *p  ) {
  return GET(p) & 0x1;
}

//
// Given block ptr bp, compute address of its header and footer
//
static inline void *HDRP(void *bp) {

  return ( (char *)bp) - WSIZE;
}
static inline void *FTRP(void *bp) {
  return ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE);
}

//
// Given block ptr bp, compute address of next and previous blocks
//
static inline void *NEXT_BLKP(void *bp) {
  return  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)));
}

static inline void* PREV_BLKP(void *bp){
  return  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)));
}

//More macros
//align to multiples of 8
#define ALIGN(p) (((size_t)(p) + (DSIZE -1)) & ~0x7)
//get address of next free block
#define NEXT(bp)  (*(void **)(bp + DSIZE)) 
//get address of prev free block
#define PREV(bp)  (*(void **)(bp))
/////////////////////////////////////////////////////////////////////////////
//
// Global Variables
//

static char *heap_listp=NULL;  /* pointer to first block */  
static char* list_ptr=NULL; //points to head of free list

//
// function prototypes for internal helper routines
//
static void *extend_heap(uint32_t words);
static void place(void *bp, uint32_t asize);
static void *find_fit(uint32_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp); 
static void checkblock(void *bp);

//my functions

//
// mm_init - Initialize the memory manager 
//
int mm_init(void) 
{
    //create initial memory heap
    
    //extend brk by 48 bytes. 
    //new brk is at 12 words.
    if((heap_listp=mem_sbrk(2*MINSIZE)) == NULL)
    {
        return -1;
    }
    //create alignment padding of one word.
    PUT(heap_listp, 0);
    //create prologue header.
    PUT(heap_listp + WSIZE, PACK(MINSIZE, 1));
    //initialize previous pointer
    PUT(heap_listp + DSIZE, 0);
    //Initialize NEXT pointer
    PUT(heap_listp + (DSIZE+DSIZE), 0);
    //free block footer
    PUT(heap_listp + MINSIZE, PACK(OVERHEAD, 1));  
    //epilogue header
    PUT(heap_listp + MINSIZE+WSIZE, PACK(0,1));
    //Initialize start of free list. 
    list_ptr=heap_listp+DSIZE;
    
    //extend the heap with a free block of 4096 bytes
    if(extend_heap(CHUNKSIZE/WSIZE)==NULL)
    {
       return -1;
    }
   
    return 0;
    
}

//ALways add the free block to the head of the free list. 
//This method uses LIFO organization.
static void add_node(void *bp)
{
 
    //set next to the old head of list
    NEXT(bp)=list_ptr;
    //set prev to NULL 
    PREV(bp)=NULL;
    //set previous of old head to bp
    PREV(list_ptr)=bp;
    //Update head to point to newly added node.
    list_ptr=bp;
    return;
}

//Delete the node by updating its next and prev pointers.
static void delete_node(void *bp)
{
    //store old prev and next links to update
    void* oldprev=PREV(bp);
    void* oldnext=NEXT(bp);
    //if prev link is not NULL
    if(PREV(bp))
    { 
        //set next link of node proceeding bp to skip bp
        NEXT(oldprev)=NEXT(bp);
    }
    else
    {
        //else, if bp was head, set new head to be next node after bp
        list_ptr=NEXT(bp);
    }
    //fix prev link to skip bp, by setting to old prev
    PREV(oldnext)=PREV(bp);
    return;
}


//
// extend_heap - Extend heap with free block and return its block pointer
//
static void *extend_heap(uint32_t words) 
{
    //*bp points to payload of block.
    char *bp;
    size_t size;
    
    //allocate even number of words to maintain alignment.
    size=(words %2) ? (words+1) * WSIZE : words *WSIZE;
  //enforce min size of MINSIZE
    if (size < MINSIZE)
    {
        size=MINSIZE;
    }
    //try to extend heap by size, keeping bp as old break value.
    if((long) (bp=mem_sbrk(size)) == -1)
    {//error
        return NULL;
    }
    //initialize free block header and footer, then epilogue header.
    //free block header.
    PUT(HDRP(bp), PACK(size,0));
    //free block footer.
    PUT(FTRP(bp), PACK(size, 0));
    //Update NEW epilogue header.
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1));
    
    //coalesce with adjacent blocks if possible.
    return coalesce(bp);
}


//
// Practice problem 9.8
//
// find_fit - Find a fit for a block with asize bytes 
//
static void *find_fit(uint32_t asize)
{
    //first fit search.
    void *bp;
     
    
    //initliaze search at heap_listp, check condition: the current block is non allocated
    for(bp=list_ptr; GET_ALLOC(HDRP(bp))==0; bp=NEXT(bp))
    { 
        //if current block is big enough, return it.
        if(asize<= GET_SIZE(HDRP(bp)))
        {
            return bp;
        }
    }
    
    return NULL; //no fit found.
}

// 
// mm_free - Free a block 
//
void mm_free(void *bp)
{
    //don't attempt to free NULL pointers
    if(!bp)
    {
        return;
    }
    //get current block size
    size_t size=GET_SIZE(HDRP(bp));
    //clear headers and footers to nonallocated.
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

//
// coalesce - boundary tag coalescing. Return ptr to coalesced block
//This updates pointers if block is coalesced by calling delete,
//Then adds the coalesced block to be the head of the free list.
static void *coalesce(void *bp) 
{
    //determine if prev and next block are allocated,
    //and the current block's size.
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))) || PREV_BLKP(bp) == bp; //if prev footer is alloc, or if bp is at head of list
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
  
    //case 1: if both are allocated, coalesce nothing
    /*
  if(prev_alloc && next_alloc)
  {
      
  }
  */
    //case 2: merge with next block
  if(prev_alloc && !next_alloc)
  {
      size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
      //adjust pointers in free list
      delete_node(NEXT_BLKP(bp));
      PUT(HDRP(bp), PACK(size, 0));
      PUT(FTRP(bp), PACK(size, 0));
  }
   ////RECHECK
    //case 3: merge with prev block
  else if(!prev_alloc && next_alloc)
  {
      size += GET_SIZE(HDRP(PREV_BLKP(bp)));
      bp = PREV_BLKP(bp);
      delete_node(bp);
      PUT(HDRP(bp), PACK(size, 0));
      PUT(FTRP(bp), PACK(size, 0));
  }
  
    //case 4: merge all 3 blocks.
  else if(!prev_alloc && !next_alloc)
  {
      size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
          GET_SIZE(HDRP(NEXT_BLKP(bp)));
      delete_node(PREV_BLKP(bp));
      delete_node(NEXT_BLKP(bp));
      bp = PREV_BLKP(bp);
      PUT(HDRP(bp), PACK(size, 0));
      PUT(FTRP(bp), PACK(size, 0));
  }
    
    //Add new coalesced block to start of free list.
  add_node(bp);
  return bp;
}

//
// mm_malloc - Allocate a block with at least size bytes of payload 
//
void *mm_malloc(uint32_t size) 
{
    size_t asize; //adjusted block size
    size_t finalsize; //amount to extend heap if no fit found.
    char *bp;
    
    if(size<=0)
    {
        return NULL;//bad request
    }
    
    //adjust block size to include overhead and alignment requirements.
    //each block must be at least 24 bytes, and be aligned to multiples of 8.
     asize = MAX(ALIGN(size) + DSIZE, MINSIZE);    
    
    //search free list for a fit. Place if found.
    if ((bp=find_fit(asize)))
        {
            place(bp, asize);
            return bp;
        }
        
    //No fit exists, extend the heap.
    //extend heap by Minimum of 4096 bytes 
    finalsize=MAX(asize, CHUNKSIZE);
    if((bp=extend_heap(finalsize/WSIZE)) ==NULL)
        return NULL; //error
    
    place(bp, asize); //place according to first fit, and possibly split.
    return bp;
} 

//
//
// Practice problem 9.9
//
// place - Place block of asize bytes at start of free block bp 
//         and split if remainder would be at least minimum block size
//This calls delete_node and add_node to update free list.
static void place(void *bp, uint32_t asize)
{
    //store current size of block.
    size_t csize=GET_SIZE(HDRP(bp));
    
    //min block size is 24 bytes. 
    //If 24 blocks is left over after placing new block, then we split the block
    if ((csize-asize) >= (MINSIZE))
    {
        //place current header and footer.
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        //update pointers 
        delete_node(bp);
        //move bp to next block payload (part being split)
        //This was changed by first placing new block. 
        bp = NEXT_BLKP(bp);
        //Place header and footer as empty for split part. 
        PUT(HDRP(bp), PACK(csize-asize,0));
        PUT(FTRP(bp), PACK(csize-asize,0));
        coalesce(bp);
    }
    
    //else, just place block, adjust headers and footers.
    else
    {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
        //update pointers
        delete_node(bp);
    }
}


//
// mm_realloc
//Return a reallocated block of ptr that holds size bytes
void *mm_realloc(void *ptr, uint32_t size)
{
  void *newp;
  uint32_t copysize;
    uint32_t oldsize;
    
    //Impossible request
    if(size<0)
    {
     return NULL;   
    }
    //If size==0, it is the same as a free request. 
    else if (size==0)
    {
        mm_free(ptr);
        return NULL;
    }
    
    else if (size>0)
    {
        oldsize=GET_SIZE(HDRP(ptr));
        //copy size should be aligned to 8, and be a min of 24 bytes
        copysize=MAX(ALIGN(size) + DSIZE, MINSIZE);
        
        //If the copysize is bigger than current block:
        if(copysize>oldsize)
        {
            //Check if next block is allocated
            size_t next_alloc=GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
            //Save total size of combined blocks
            uint32_t totalsize=(oldsize + GET_SIZE(HDRP(NEXT_BLKP(ptr))));
            //If not, and the two blocks are bigger in size than copysize,
            //then Combine them and place the realloc block
            if(!next_alloc && totalsize>= copysize)
            {
                //delete node to update free list pointers
                delete_node(NEXT_BLKP(ptr));
                //place node as current size
                PUT(HDRP(ptr), PACK(totalsize, 1));
                PUT(FTRP(ptr), PACK(totalsize, 1)); 
                return ptr; 
            }
            else//allocate new memory, and copy block(expensive)
            {
                //alloc block of copysize
                newp=mm_malloc(copysize);
                //Place new block at start of free list
                place(newp, copysize);
                //copy old block payload to new block
                memcpy(newp, ptr, copysize);
                //free old block
                mm_free(ptr);
                return newp;
            }
        }
        //if block is right size, return it
        else if(copysize<=oldsize)
        {
            return ptr;
        }
    }
        
    return NULL;
}





//
// mm_checkheap - Check the heap for consistency 
//
void mm_checkheap(int verbose) 
{
  //
  // This provided implementation assumes you're using the structure
  // of the sample solution in the text. If not, omit this code
  // and provide your own mm_checkheap
  //
  void *bp = heap_listp;
  
  if (verbose) {
    printf("Heap (%p):\n", heap_listp);
  }

  if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp))) {
	printf("Bad prologue header\n");
  }
  checkblock(heap_listp);

  for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
    if (verbose)  {
      printblock(bp);
    }
    checkblock(bp);
  }
     
  if (verbose) {
    printblock(bp);
  }

  if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp)))) {
    printf("Bad epilogue header\n");
  }
}

static void printblock(void *bp) 
{
  uint32_t hsize, halloc, fsize, falloc;

  hsize = GET_SIZE(HDRP(bp));
  halloc = GET_ALLOC(HDRP(bp));  
  fsize = GET_SIZE(FTRP(bp));
  falloc = GET_ALLOC(FTRP(bp));  
    
  if (hsize == 0) {
    printf("%p: EOL\n", bp);
    return;
  }

  printf("%p: header: [%d:%c] footer: [%d:%c]\n",
	 bp, 
	 (int) hsize, (halloc ? 'a' : 'f'), 
	 (int) fsize, (falloc ? 'a' : 'f')); 
}

static void checkblock(void *bp) 
{
  if ((uintptr_t)bp % 8) {
    printf("Error: %p is not doubleword aligned\n", bp);
  }
  if (GET(HDRP(bp)) != GET(FTRP(bp))) {
    printf("Error: header does not match footer\n");
  }
}

