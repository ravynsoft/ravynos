/* vmem.h
 *
 * (c) 1999 Microsoft Corporation. All rights reserved. 
 * Portions (c) 1999 ActiveState Tool Corp, http://www.ActiveState.com/
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 * Options:
 *
 * Defining _USE_MSVCRT_MEM_ALLOC will cause all memory allocations
 * to be forwarded to the compiler's MSVCR*.DLL. Defining _USE_LINKED_LIST as
 * well will track all allocations in a doubly linked list, so that the host can
 * free all memory allocated when it goes away.
 * If _USE_MSVCRT_MEM_ALLOC is not defined then Knuth's boundary tag algorithm
 * is used; defining _USE_BUDDY_BLOCKS will use Knuth's algorithm R
 * (Buddy system reservation)
 *
 */

#ifndef ___VMEM_H_INC___
#define ___VMEM_H_INC___

#define _USE_MSVCRT_MEM_ALLOC
#define _USE_LINKED_LIST

// #define _USE_BUDDY_BLOCKS

// #define _DEBUG_MEM
#ifdef _DEBUG_MEM
#define ASSERT(f) if(!(f)) DebugBreak();

inline void MEMODS(char *str)
{
    OutputDebugString(str);
    OutputDebugString("\n");
}

inline void MEMODSlx(char *str, long x)
{
    char szBuffer[512];	
    sprintf(szBuffer, "%s %lx\n", str, x);
    OutputDebugString(szBuffer);
}

#define WALKHEAP() WalkHeap(0)
#define WALKHEAPTRACE() WalkHeap(1)

#else

#define ASSERT(f)
#define MEMODS(x)
#define MEMODSlx(x, y)
#define WALKHEAP()
#define WALKHEAPTRACE()

#endif

#ifdef _USE_MSVCRT_MEM_ALLOC

#ifndef _USE_LINKED_LIST
// #define _USE_LINKED_LIST
#endif

/* 
 * Pass all memory requests through to the compiler's msvcr*.dll.
 * Optionally track by using a doubly linked header.
 */

#ifdef _USE_LINKED_LIST
class VMem;

/*
 * Address an alignment issue with x64 mingw-w64 ports of gcc-12 and
 * (presumably) later. We do the same thing again 16 lines further down.
 * See https://github.com/Perl/perl5/issues/19824
 */

#if defined(__MINGW64__) && __GNUC__ > 11
typedef struct _MemoryBlockHeader* PMEMORY_BLOCK_HEADER __attribute__ ((aligned(16)));
#else
typedef struct _MemoryBlockHeader* PMEMORY_BLOCK_HEADER;
#endif

typedef struct _MemoryBlockHeader {
    PMEMORY_BLOCK_HEADER    pNext;
    PMEMORY_BLOCK_HEADER    pPrev;
    VMem *owner;

#if defined(__MINGW64__) && __GNUC__ > 11
} MEMORY_BLOCK_HEADER __attribute__ ((aligned(16))), *PMEMORY_BLOCK_HEADER;
#else
} MEMORY_BLOCK_HEADER, *PMEMORY_BLOCK_HEADER;
#endif

#endif

class VMem
{
public:
    VMem();
    ~VMem();
    void* Malloc(size_t size);
    void* Realloc(void* pMem, size_t size);
    void Free(void* pMem);
    void GetLock(void);
    void FreeLock(void);
    int IsLocked(void);
    long Release(void);
    long AddRef(void);

    inline BOOL CreateOk(void)
    {
        return TRUE;
    };

protected:
#ifdef _USE_LINKED_LIST
    void LinkBlock(PMEMORY_BLOCK_HEADER ptr)
    {
        PMEMORY_BLOCK_HEADER next = m_Dummy.pNext;
        m_Dummy.pNext = ptr;
        ptr->pPrev = &m_Dummy;
        ptr->pNext = next;
        ptr->owner = this;
        next->pPrev = ptr;
    }
    void UnlinkBlock(PMEMORY_BLOCK_HEADER ptr)
    {
        PMEMORY_BLOCK_HEADER next = ptr->pNext;
        PMEMORY_BLOCK_HEADER prev = ptr->pPrev;
        prev->pNext = next;
        next->pPrev = prev;
    }

    MEMORY_BLOCK_HEADER	m_Dummy;
    CRITICAL_SECTION	m_cs;		// access lock
#endif

    long		m_lRefCount;	// number of current users
};

VMem::VMem()
{
    m_lRefCount = 1;
#ifdef _USE_LINKED_LIST
    InitializeCriticalSection(&m_cs);
    m_Dummy.pNext = m_Dummy.pPrev =  &m_Dummy;
    m_Dummy.owner = this;
#endif
}

VMem::~VMem(void)
{
#ifdef _USE_LINKED_LIST
    while (m_Dummy.pNext != &m_Dummy) {
        Free(m_Dummy.pNext+1);
    }
    DeleteCriticalSection(&m_cs);
#endif
}

void* VMem::Malloc(size_t size)
{
#ifdef _USE_LINKED_LIST
    GetLock();
    PMEMORY_BLOCK_HEADER ptr = (PMEMORY_BLOCK_HEADER)malloc(size+sizeof(MEMORY_BLOCK_HEADER));
    if (!ptr) {
        FreeLock();
        return NULL;
    }
    LinkBlock(ptr);
    FreeLock();
    return (ptr+1);
#else
    return malloc(size);
#endif
}

void* VMem::Realloc(void* pMem, size_t size)
{
#ifdef _USE_LINKED_LIST
    if (!pMem)
        return Malloc(size);

    if (!size) {
        Free(pMem);
        return NULL;
    }

    GetLock();
    PMEMORY_BLOCK_HEADER ptr = (PMEMORY_BLOCK_HEADER)(((char*)pMem)-sizeof(MEMORY_BLOCK_HEADER));
    UnlinkBlock(ptr);
    ptr = (PMEMORY_BLOCK_HEADER)realloc(ptr, size+sizeof(MEMORY_BLOCK_HEADER));
    if (!ptr) {
        FreeLock();
        return NULL;
    }
    LinkBlock(ptr);
    FreeLock();

    return (ptr+1);
#else
    return realloc(pMem, size);
#endif
}

void VMem::Free(void* pMem)
{
#ifdef _USE_LINKED_LIST
    if (pMem) {
        PMEMORY_BLOCK_HEADER ptr = (PMEMORY_BLOCK_HEADER)(((char*)pMem)-sizeof(MEMORY_BLOCK_HEADER));
        if (ptr->owner != this) {
            if (ptr->owner) {
#if 1
                int *nowhere = NULL;
                Perl_warn_nocontext("Free to wrong pool %p not %p",this,ptr->owner);
                *nowhere = 0; /* this segfault is deliberate, 
                                 so you can see the stack trace */
#else
                ptr->owner->Free(pMem);	
#endif
            }
            return;
        }
        GetLock();
        UnlinkBlock(ptr);
        ptr->owner = NULL;
        free(ptr);
        FreeLock();
    }
#else /*_USE_LINKED_LIST*/
    free(pMem);
#endif
}

void VMem::GetLock(void)
{
#ifdef _USE_LINKED_LIST
    EnterCriticalSection(&m_cs);
#endif
}

void VMem::FreeLock(void)
{
#ifdef _USE_LINKED_LIST
    LeaveCriticalSection(&m_cs);
#endif
}

int VMem::IsLocked(void)
{
#if 0
    /* XXX TryEnterCriticalSection() is not available in some versions
     * of Windows 95.  Since this code is not used anywhere yet, we 
     * skirt the issue for now. */
    BOOL bAccessed = TryEnterCriticalSection(&m_cs);
    if(bAccessed) {
        LeaveCriticalSection(&m_cs);
    }
    return !bAccessed;
#else
    ASSERT(0);	/* alarm bells for when somebody calls this */
    return 0;
#endif
}

long VMem::Release(void)
{
    long lCount = InterlockedDecrement(&m_lRefCount);
    if(!lCount)
        delete this;
    return lCount;
}

long VMem::AddRef(void)
{
    long lCount = InterlockedIncrement(&m_lRefCount);
    return lCount;
}

#else	/* _USE_MSVCRT_MEM_ALLOC */

/*
 * Knuth's boundary tag algorithm Vol #1, Page 440.
 *
 * Each block in the heap has tag words before and after it,
 *  TAG
 *  block
 *  TAG
 * The size is stored in these tags as a long word, and includes the 8 bytes
 * of overhead that the boundary tags consume.  Blocks are allocated on long
 * word boundaries, so the size is always multiples of long words.  When the
 * block is allocated, bit 0, (the tag bit), of the size is set to 1.  When 
 * a block is freed, it is merged with adjacent free blocks, and the tag bit
 * is set to 0.
 *
 * A linked list is used to manage the free list. The first two long words of
 * the block contain double links.  These links are only valid when the block
 * is freed, therefore space needs to be reserved for them.  Thus, the minimum
 * block size (not counting the tags) is 8 bytes.
 *
 * Since memory allocation may occur on a single threaded, explicit locks are not
 * provided.
 * 
 */

const long lAllocStart = 0x00020000; /* start at 128K */
const long minBlockSize = sizeof(void*)*2;
const long sizeofTag = sizeof(long);
const long blockOverhead = sizeofTag*2;
const long minAllocSize = minBlockSize+blockOverhead;
#ifdef _USE_BUDDY_BLOCKS
const long lSmallBlockSize = 1024;
const size_t nListEntries = ((lSmallBlockSize-minAllocSize)/sizeof(long));

inline size_t CalcEntry(size_t size)
{
    ASSERT((size&(sizeof(long)-1)) == 0);
    return ((size - minAllocSize) / sizeof(long));
}
#endif

typedef BYTE* PBLOCK;	/* pointer to a memory block */

/*
 * Macros for accessing hidden fields in a memory block:
 *
 * SIZE	    size of this block (tag bit 0 is 1 if block is allocated)
 * PSIZE    size of previous physical block
 */

#define SIZE(block)	(*(ULONG*)(((PBLOCK)(block))-sizeofTag))
#define PSIZE(block)	(*(ULONG*)(((PBLOCK)(block))-(blockOverhead)))
inline void SetTags(PBLOCK block, long size)
{
    SIZE(block) = size;
    PSIZE(block+(size&~1)) = size;
}

/*
 * Free list pointers
 * PREV	pointer to previous block
 * NEXT	pointer to next block
 */

#define PREV(block)	(*(PBLOCK*)(block))
#define NEXT(block)	(*(PBLOCK*)((block)+sizeof(PBLOCK)))
inline void SetLink(PBLOCK block, PBLOCK prev, PBLOCK next)
{
    PREV(block) = prev;
    NEXT(block) = next;
}
inline void Unlink(PBLOCK p)
{
    PBLOCK next = NEXT(p);
    PBLOCK prev = PREV(p);
    NEXT(prev) = next;
    PREV(next) = prev;
}
#ifndef _USE_BUDDY_BLOCKS
inline void AddToFreeList(PBLOCK block, PBLOCK pInList)
{
    PBLOCK next = NEXT(pInList);
    NEXT(pInList) = block;
    SetLink(block, pInList, next);
    PREV(next) = block;
}
#endif

/* Macro for rounding up to the next sizeof(long) */
#define ROUND_UP(n)	(((ULONG)(n)+sizeof(long)-1)&~(sizeof(long)-1))
#define ROUND_UP64K(n)	(((ULONG)(n)+0x10000-1)&~(0x10000-1))
#define ROUND_DOWN(n)	((ULONG)(n)&~(sizeof(long)-1))

/*
 * HeapRec - a list of all non-contiguous heap areas
 *
 * Each record in this array contains information about a non-contiguous heap area.
 */

const int maxHeaps = 32; /* 64 was overkill */
const long lAllocMax   = 0x80000000; /* max size of allocation */

#ifdef _USE_BUDDY_BLOCKS
typedef struct _FreeListEntry
{
    BYTE    Dummy[minAllocSize];	// dummy free block
} FREE_LIST_ENTRY, *PFREE_LIST_ENTRY;
#endif

#ifndef _USE_BUDDY_BLOCKS
#define USE_BIGBLOCK_ALLOC
#endif
/*
 * performance tuning
 * Use VirtualAlloc() for blocks bigger than nMaxHeapAllocSize since
 * Windows 95/98/Me have heap managers that are designed for memory 
 * blocks smaller than four megabytes.
 */

#ifdef USE_BIGBLOCK_ALLOC
const int nMaxHeapAllocSize = (1024*512);  /* don't allocate anything larger than this from the heap */
#endif

typedef struct _HeapRec
{
    PBLOCK	base;	/* base of heap area */
    ULONG	len;	/* size of heap area */
#ifdef USE_BIGBLOCK_ALLOC
    BOOL	bBigBlock;  /* was allocate using VirtualAlloc */
#endif
} HeapRec;

class VMem
{
public:
    VMem();
    ~VMem();
    void* Malloc(size_t size);
    void* Realloc(void* pMem, size_t size);
    void Free(void* pMem);
    void GetLock(void);
    void FreeLock(void);
    int IsLocked(void);
    long Release(void);
    long AddRef(void);

    inline BOOL CreateOk(void)
    {
#ifdef _USE_BUDDY_BLOCKS
        return TRUE;
#else
        return m_hHeap != NULL;
#endif
    };

    void ReInit(void);

protected:
    void Init(void);
    int Getmem(size_t size);

    int HeapAdd(void* ptr, size_t size
#ifdef USE_BIGBLOCK_ALLOC
        , BOOL bBigBlock
#endif
    );

    void* Expand(void* block, size_t size);

#ifdef _USE_BUDDY_BLOCKS
    inline PBLOCK GetFreeListLink(int index)
    {
        if (index >= nListEntries)
            index = nListEntries-1;
        return &m_FreeList[index].Dummy[sizeofTag];
    }
    inline PBLOCK GetOverSizeFreeList(void)
    {
        return &m_FreeList[nListEntries-1].Dummy[sizeofTag];
    }
    inline PBLOCK GetEOLFreeList(void)
    {
        return &m_FreeList[nListEntries].Dummy[sizeofTag];
    }

    void AddToFreeList(PBLOCK block, size_t size)
    {
        PBLOCK pFreeList = GetFreeListLink(CalcEntry(size));
        PBLOCK next = NEXT(pFreeList);
        NEXT(pFreeList) = block;
        SetLink(block, pFreeList, next);
        PREV(next) = block;
    }
#endif
    inline size_t CalcAllocSize(size_t size)
    {
        /*
         * Adjust the real size of the block to be a multiple of sizeof(long), and add
         * the overhead for the boundary tags.  Disallow negative or zero sizes.
         */
        return (size < minBlockSize) ? minAllocSize : (size_t)ROUND_UP(size) + blockOverhead;
    }

#ifdef _USE_BUDDY_BLOCKS
    FREE_LIST_ENTRY	m_FreeList[nListEntries+1];	// free list with dummy end of list entry as well
#else
    HANDLE		m_hHeap;		    // memory heap for this script
    char		m_FreeDummy[minAllocSize];  // dummy free block
    PBLOCK		m_pFreeList;		    // pointer to first block on free list
#endif
    PBLOCK		m_pRover;		    // roving pointer into the free list
    HeapRec		m_heaps[maxHeaps];	    // list of all non-contiguous heap areas 
    int			m_nHeaps;		    // no. of heaps in m_heaps 
    long		m_lAllocSize;		    // current alloc size
    long		m_lRefCount;		    // number of current users
    CRITICAL_SECTION	m_cs;			    // access lock

#ifdef _DEBUG_MEM
    void WalkHeap(int complete);
    void MemoryUsageMessage(char *str, long x, long y, int c);
    FILE*		m_pLog;
#endif
};

VMem::VMem()
{
    m_lRefCount = 1;
#ifndef _USE_BUDDY_BLOCKS
    BOOL bRet = (NULL != (m_hHeap = HeapCreate(HEAP_NO_SERIALIZE,
                                lAllocStart,	/* initial size of heap */
                                0)));		/* no upper limit on size of heap */
    ASSERT(bRet);
#endif

    InitializeCriticalSection(&m_cs);
#ifdef _DEBUG_MEM
    m_pLog = 0;
#endif

    Init();
}

VMem::~VMem(void)
{
#ifndef _USE_BUDDY_BLOCKS
    ASSERT(HeapValidate(m_hHeap, HEAP_NO_SERIALIZE, NULL));
#endif
    WALKHEAPTRACE();

    DeleteCriticalSection(&m_cs);
#ifdef _USE_BUDDY_BLOCKS
    for(int index = 0; index < m_nHeaps; ++index) {
        VirtualFree(m_heaps[index].base, 0, MEM_RELEASE);
    }
#else /* !_USE_BUDDY_BLOCKS */
#ifdef USE_BIGBLOCK_ALLOC
    for(int index = 0; index < m_nHeaps; ++index) {
        if (m_heaps[index].bBigBlock) {
            VirtualFree(m_heaps[index].base, 0, MEM_RELEASE);
        }
    }
#endif
    BOOL bRet = HeapDestroy(m_hHeap);
    ASSERT(bRet);
#endif /* _USE_BUDDY_BLOCKS */
}

void VMem::ReInit(void)
{
    for(int index = 0; index < m_nHeaps; ++index) {
#ifdef _USE_BUDDY_BLOCKS
        VirtualFree(m_heaps[index].base, 0, MEM_RELEASE);
#else
#ifdef USE_BIGBLOCK_ALLOC
        if (m_heaps[index].bBigBlock) {
            VirtualFree(m_heaps[index].base, 0, MEM_RELEASE);
        }
        else
#endif
            HeapFree(m_hHeap, HEAP_NO_SERIALIZE, m_heaps[index].base);
#endif /* _USE_BUDDY_BLOCKS */
    }

    Init();
}

void VMem::Init(void)
{
#ifdef _USE_BUDDY_BLOCKS
    PBLOCK pFreeList;
    /*
     * Initialize the free list by placing a dummy zero-length block on it.
     * Set the end of list marker.
     * Set the number of non-contiguous heaps to zero.
     * Set the next allocation size.
     */
    for (int index = 0; index < nListEntries; ++index) {
        pFreeList = GetFreeListLink(index);
        SIZE(pFreeList) = PSIZE(pFreeList+minAllocSize) = 0;
        PREV(pFreeList) = NEXT(pFreeList) = pFreeList;
    }
    pFreeList = GetEOLFreeList();
    SIZE(pFreeList) = PSIZE(pFreeList+minAllocSize) = 0;
    PREV(pFreeList) = NEXT(pFreeList) = NULL;
    m_pRover = GetOverSizeFreeList();
#else
    /*
     * Initialize the free list by placing a dummy zero-length block on it.
     * Set the number of non-contiguous heaps to zero.
     */
    m_pFreeList = m_pRover = (PBLOCK)(&m_FreeDummy[sizeofTag]);
    PSIZE(m_pFreeList+minAllocSize) = SIZE(m_pFreeList) = 0;
    PREV(m_pFreeList) = NEXT(m_pFreeList) = m_pFreeList;
#endif

    m_nHeaps = 0;
    m_lAllocSize = lAllocStart;
}

void* VMem::Malloc(size_t size)
{
    WALKHEAP();

    PBLOCK ptr;
    size_t lsize, rem;
    /*
     * Disallow negative or zero sizes.
     */
    size_t realsize = CalcAllocSize(size);
    if((int)realsize < minAllocSize || size == 0)
        return NULL;

#ifdef _USE_BUDDY_BLOCKS
    /*
     * Check the free list of small blocks if this is free use it
     * Otherwise check the rover if it has no blocks then
     * Scan the free list entries use the first free block
     * split the block if needed, stop at end of list marker
     */
    {
        int index = CalcEntry(realsize);
        if (index < nListEntries-1) {
            ptr = GetFreeListLink(index);
            lsize = SIZE(ptr);
            if (lsize >= realsize) {
                rem = lsize - realsize;
                if(rem < minAllocSize) {
                    /* Unlink the block from the free list. */
                    Unlink(ptr);
                }
                else {
                    /*
                     * split the block
                     * The remainder is big enough to split off into a new block.
                     * Use the end of the block, resize the beginning of the block
                     * no need to change the free list.
                     */
                    SetTags(ptr, rem);
                    ptr += SIZE(ptr);
                    lsize = realsize;
                }
                SetTags(ptr, lsize | 1);
                return ptr;
            }
            ptr = m_pRover;
            lsize = SIZE(ptr);
            if (lsize >= realsize) {
                rem = lsize - realsize;
                if(rem < minAllocSize) {
                    /* Unlink the block from the free list. */
                    Unlink(ptr);
                }
                else {
                    /*
                     * split the block
                     * The remainder is big enough to split off into a new block.
                     * Use the end of the block, resize the beginning of the block
                     * no need to change the free list.
                     */
                    SetTags(ptr, rem);
                    ptr += SIZE(ptr);
                    lsize = realsize;
                }
                SetTags(ptr, lsize | 1);
                return ptr;
            }
            ptr = GetFreeListLink(index+1);
            while (NEXT(ptr)) {
                lsize = SIZE(ptr);
                if (lsize >= realsize) {
                    size_t rem = lsize - realsize;
                    if(rem < minAllocSize) {
                        /* Unlink the block from the free list. */
                        Unlink(ptr);
                    }
                    else {
                        /*
                         * split the block
                         * The remainder is big enough to split off into a new block.
                         * Use the end of the block, resize the beginning of the block
                         * no need to change the free list.
                         */
                        SetTags(ptr, rem);
                        ptr += SIZE(ptr);
                        lsize = realsize;
                    }
                    SetTags(ptr, lsize | 1);
                    return ptr;
                }
                ptr += sizeof(FREE_LIST_ENTRY);
            }
        }
    }
#endif

    /*
     * Start searching the free list at the rover.  If we arrive back at rover without
     * finding anything, allocate some memory from the heap and try again.
     */
    ptr = m_pRover;	/* start searching at rover */
    int loops = 2;	/* allow two times through the loop  */
    for(;;) {
        lsize = SIZE(ptr);
        ASSERT((lsize&1)==0);
        /* is block big enough? */
        if(lsize >= realsize) {	
            /* if the remainder is too small, don't bother splitting the block. */
            rem = lsize - realsize;
            if(rem < minAllocSize) {
                if(m_pRover == ptr)
                    m_pRover = NEXT(ptr);

                /* Unlink the block from the free list. */
                Unlink(ptr);
            }
            else {
                /*
                 * split the block
                 * The remainder is big enough to split off into a new block.
                 * Use the end of the block, resize the beginning of the block
                 * no need to change the free list.
                 */
                SetTags(ptr, rem);
                ptr += SIZE(ptr);
                lsize = realsize;
            }
            /* Set the boundary tags to mark it as allocated. */
            SetTags(ptr, lsize | 1);
            return ((void *)ptr);
        }

        /*
         * This block was unsuitable.  If we've gone through this list once already without
         * finding anything, allocate some new memory from the heap and try again.
         */
        ptr = NEXT(ptr);
        if(ptr == m_pRover) {
            if(!(loops-- && Getmem(realsize))) {
                return NULL;
            }
            ptr = m_pRover;
        }
    }
}

void* VMem::Realloc(void* block, size_t size)
{
    WALKHEAP();

    /* if size is zero, free the block. */
    if(size == 0) {
        Free(block);
        return (NULL);
    }

    /* if block pointer is NULL, do a Malloc(). */
    if(block == NULL)
        return Malloc(size);

    /*
     * Grow or shrink the block in place.
     * if the block grows then the next block will be used if free
     */
    if(Expand(block, size) != NULL)
        return block;

    size_t realsize = CalcAllocSize(size);
    if((int)realsize < minAllocSize)
        return NULL;

    /*
     * see if the previous block is free, and is it big enough to cover the new size
     * if merged with the current block.
     */
    PBLOCK ptr = (PBLOCK)block;
    size_t cursize = SIZE(ptr) & ~1;
    size_t psize = PSIZE(ptr);
    if((psize&1) == 0 && (psize + cursize) >= realsize) {
        PBLOCK prev = ptr - psize;
        if(m_pRover == prev)
            m_pRover = NEXT(prev);

        /* Unlink the next block from the free list. */
        Unlink(prev);

        /* Copy contents of old block to new location, make it the current block. */
        memmove(prev, ptr, cursize);
        cursize += psize;	/* combine sizes */
        ptr = prev;

        size_t rem = cursize - realsize;
        if(rem >= minAllocSize) {
            /*
             * The remainder is big enough to be a new block.  Set boundary
             * tags for the resized block and the new block.
             */
            prev = ptr + realsize;
            /*
             * add the new block to the free list.
             * next block cannot be free
             */
            SetTags(prev, rem);
#ifdef _USE_BUDDY_BLOCKS
            AddToFreeList(prev, rem);
#else
            AddToFreeList(prev, m_pFreeList);
#endif
            cursize = realsize;
        }
        /* Set the boundary tags to mark it as allocated. */
        SetTags(ptr, cursize | 1);
        return ((void *)ptr);
    }

    /* Allocate a new block, copy the old to the new, and free the old. */
    if((ptr = (PBLOCK)Malloc(size)) != NULL) {
        memmove(ptr, block, cursize-blockOverhead);
        Free(block);
    }
    return ((void *)ptr);
}

void VMem::Free(void* p)
{
    WALKHEAP();

    /* Ignore null pointer. */
    if(p == NULL)
        return;

    PBLOCK ptr = (PBLOCK)p;

    /* Check for attempt to free a block that's already free. */
    size_t size = SIZE(ptr);
    if((size&1) == 0) {
        MEMODSlx("Attempt to free previously freed block", (long)p);
        return;
    }
    size &= ~1;	/* remove allocated tag */

    /* if previous block is free, add this block to it. */
#ifndef _USE_BUDDY_BLOCKS
    int linked = FALSE;
#endif
    size_t psize = PSIZE(ptr);
    if((psize&1) == 0) {
        ptr -= psize;	/* point to previous block */
        size += psize;	/* merge the sizes of the two blocks */
#ifdef _USE_BUDDY_BLOCKS
        Unlink(ptr);
#else
        linked = TRUE;	/* it's already on the free list */
#endif
    }

    /* if the next physical block is free, merge it with this block. */
    PBLOCK next = ptr + size;	/* point to next physical block */
    size_t nsize = SIZE(next);
    if((nsize&1) == 0) {
        /* block is free move rover if needed */
        if(m_pRover == next)
            m_pRover = NEXT(next);

        /* unlink the next block from the free list. */
        Unlink(next);

        /* merge the sizes of this block and the next block. */
        size += nsize;
    }

    /* Set the boundary tags for the block; */
    SetTags(ptr, size);

    /* Link the block to the head of the free list. */
#ifdef _USE_BUDDY_BLOCKS
        AddToFreeList(ptr, size);
#else
    if(!linked) {
        AddToFreeList(ptr, m_pFreeList);
    }
#endif
}

void VMem::GetLock(void)
{
    EnterCriticalSection(&m_cs);
}

void VMem::FreeLock(void)
{
    LeaveCriticalSection(&m_cs);
}

int VMem::IsLocked(void)
{
#if 0
    /* XXX TryEnterCriticalSection() is not available in some versions
     * of Windows 95.  Since this code is not used anywhere yet, we 
     * skirt the issue for now. */
    BOOL bAccessed = TryEnterCriticalSection(&m_cs);
    if(bAccessed) {
        LeaveCriticalSection(&m_cs);
    }
    return !bAccessed;
#else
    ASSERT(0);	/* alarm bells for when somebody calls this */
    return 0;
#endif
}


long VMem::Release(void)
{
    long lCount = InterlockedDecrement(&m_lRefCount);
    if(!lCount)
        delete this;
    return lCount;
}

long VMem::AddRef(void)
{
    long lCount = InterlockedIncrement(&m_lRefCount);
    return lCount;
}


int VMem::Getmem(size_t requestSize)
{   /* returns -1 is successful 0 if not */
#ifdef USE_BIGBLOCK_ALLOC
    BOOL bBigBlock;
#endif
    void *ptr;

    /* Round up size to next multiple of 64K. */
    size_t size = (size_t)ROUND_UP64K(requestSize);

    /*
     * if the size requested is smaller than our current allocation size
     * adjust up
     */
    if(size < (unsigned long)m_lAllocSize)
        size = m_lAllocSize;

    /* Update the size to allocate on the next request */
    if(m_lAllocSize != lAllocMax)
        m_lAllocSize <<= 2;

#ifndef _USE_BUDDY_BLOCKS
    if(m_nHeaps != 0
#ifdef USE_BIGBLOCK_ALLOC
        && !m_heaps[m_nHeaps-1].bBigBlock
#endif
                    ) {
        /* Expand the last allocated heap */
        ptr = HeapReAlloc(m_hHeap, HEAP_REALLOC_IN_PLACE_ONLY|HEAP_NO_SERIALIZE,
                m_heaps[m_nHeaps-1].base,
                m_heaps[m_nHeaps-1].len + size);
        if(ptr != 0) {
            HeapAdd(((char*)ptr) + m_heaps[m_nHeaps-1].len, size
#ifdef USE_BIGBLOCK_ALLOC
                , FALSE
#endif
                );
            return -1;
        }
    }
#endif /* _USE_BUDDY_BLOCKS */

    /*
     * if we didn't expand a block to cover the requested size
     * allocate a new Heap
     * the size of this block must include the additional dummy tags at either end
     * the above ROUND_UP64K may not have added any memory to include this.
     */
    if(size == requestSize)
        size = (size_t)ROUND_UP64K(requestSize+(blockOverhead));

Restart:
#ifdef _USE_BUDDY_BLOCKS
    ptr = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
#else
#ifdef USE_BIGBLOCK_ALLOC
    bBigBlock = FALSE;
    if (size >= nMaxHeapAllocSize) {
        bBigBlock = TRUE;
        ptr = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
    }
    else
#endif
    ptr = HeapAlloc(m_hHeap, HEAP_NO_SERIALIZE, size);
#endif /* _USE_BUDDY_BLOCKS */

    if (!ptr) {
        /* try to allocate a smaller chunk */
        size >>= 1;
        if(size > requestSize)
            goto Restart;
    }

    if(ptr == 0) {
        MEMODSlx("HeapAlloc failed on size!!!", size);
        return 0;
    }

#ifdef _USE_BUDDY_BLOCKS
    if (HeapAdd(ptr, size)) {
        VirtualFree(ptr, 0, MEM_RELEASE);
        return 0;
    }
#else
#ifdef USE_BIGBLOCK_ALLOC
    if (HeapAdd(ptr, size, bBigBlock)) {
        if (bBigBlock) {
            VirtualFree(ptr, 0, MEM_RELEASE);
        }
    }
#else
    HeapAdd(ptr, size);
#endif
#endif /* _USE_BUDDY_BLOCKS */
    return -1;
}

int VMem::HeapAdd(void* p, size_t size
#ifdef USE_BIGBLOCK_ALLOC
    , BOOL bBigBlock
#endif
    )
{   /* if the block can be successfully added to the heap, returns 0; otherwise -1. */
    int index;

    /* Check size, then round size down to next long word boundary. */
    if(size < minAllocSize)
        return -1;

    size = (size_t)ROUND_DOWN(size);
    PBLOCK ptr = (PBLOCK)p;

#ifdef USE_BIGBLOCK_ALLOC
    if (!bBigBlock) {
#endif
        /*
         * Search for another heap area that's contiguous with the bottom of this new area.
         * (It should be extremely unusual to find one that's contiguous with the top).
         */
        for(index = 0; index < m_nHeaps; ++index) {
            if(ptr == m_heaps[index].base + (int)m_heaps[index].len) {
                /*
                 * The new block is contiguous with a previously allocated heap area.  Add its
                 * length to that of the previous heap.  Merge it with the dummy end-of-heap
                 * area marker of the previous heap.
                 */
                m_heaps[index].len += size;
                break;
            }
        }
#ifdef USE_BIGBLOCK_ALLOC
    }
    else {
        index = m_nHeaps;
    }
#endif

    if(index == m_nHeaps) {
        /* The new block is not contiguous, or is BigBlock.  Add it to the heap list. */
        if(m_nHeaps == maxHeaps) {
            return -1;	/* too many non-contiguous heaps */
        }
        m_heaps[m_nHeaps].base = ptr;
        m_heaps[m_nHeaps].len = size;
#ifdef USE_BIGBLOCK_ALLOC
        m_heaps[m_nHeaps].bBigBlock = bBigBlock;
#endif
        m_nHeaps++;

        /*
         * Reserve the first LONG in the block for the ending boundary tag of a dummy
         * block at the start of the heap area.
         */
        size -= blockOverhead;
        ptr += blockOverhead;
        PSIZE(ptr) = 1;	/* mark the dummy previous block as allocated */
    }

    /*
     * Convert the heap to one large block.  Set up its boundary tags, and those of
     * marker block after it.  The marker block before the heap will already have
     * been set up if this heap is not contiguous with the end of another heap.
     */
    SetTags(ptr, size | 1);
    PBLOCK next = ptr + size;	/* point to dummy end block */
    SIZE(next) = 1;	/* mark the dummy end block as allocated */

    /*
     * Link the block to the start of the free list by calling free().
     * This will merge the block with any adjacent free blocks.
     */
    Free(ptr);
    return 0;
}


void* VMem::Expand(void* block, size_t size)
{
    /*
     * Disallow negative or zero sizes.
     */
    size_t realsize = CalcAllocSize(size);
    if((int)realsize < minAllocSize || size == 0)
        return NULL;

    PBLOCK ptr = (PBLOCK)block; 

    /* if the current size is the same as requested, do nothing. */
    size_t cursize = SIZE(ptr) & ~1;
    if(cursize == realsize) {
        return block;
    }

    /* if the block is being shrunk, convert the remainder of the block into a new free block. */
    if(realsize <= cursize) {
        size_t nextsize = cursize - realsize;	/* size of new remainder block */
        if(nextsize >= minAllocSize) {
            /*
             * Split the block
             * Set boundary tags for the resized block and the new block.
             */
            SetTags(ptr, realsize | 1);
            ptr += realsize;

            /*
             * add the new block to the free list.
             * call Free to merge this block with next block if free
             */
            SetTags(ptr, nextsize | 1);
            Free(ptr);
        }

        return block;
    }

    PBLOCK next = ptr + cursize;
    size_t nextsize = SIZE(next);

    /* Check the next block for consistency.*/
    if((nextsize&1) == 0 && (nextsize + cursize) >= realsize) {
        /*
         * The next block is free and big enough.  Add the part that's needed
         * to our block, and split the remainder off into a new block.
         */
        if(m_pRover == next)
            m_pRover = NEXT(next);

        /* Unlink the next block from the free list. */
        Unlink(next);
        cursize += nextsize;	/* combine sizes */

        size_t rem = cursize - realsize;	/* size of remainder */
        if(rem >= minAllocSize) {
            /*
             * The remainder is big enough to be a new block.
             * Set boundary tags for the resized block and the new block.
             */
            next = ptr + realsize;
            /*
             * add the new block to the free list.
             * next block cannot be free
             */
            SetTags(next, rem);
#ifdef _USE_BUDDY_BLOCKS
            AddToFreeList(next, rem);
#else
            AddToFreeList(next, m_pFreeList);
#endif
            cursize = realsize;
        }
        /* Set the boundary tags to mark it as allocated. */
        SetTags(ptr, cursize | 1);
        return ((void *)ptr);
    }
    return NULL;
}

#ifdef _DEBUG_MEM
#define LOG_FILENAME ".\\MemLog.txt"

void VMem::MemoryUsageMessage(char *str, long x, long y, int c)
{
    char szBuffer[512];
    if(str) {
        if(!m_pLog)
            m_pLog = fopen(LOG_FILENAME, "w");
        sprintf(szBuffer, str, x, y, c);
        fputs(szBuffer, m_pLog);
    }
    else {
        if(m_pLog) {
            fflush(m_pLog);
            fclose(m_pLog);
            m_pLog = 0;
        }
    }
}

void VMem::WalkHeap(int complete)
{
    if(complete) {
        MemoryUsageMessage(NULL, 0, 0, 0);
        size_t total = 0;
        for(int i = 0; i < m_nHeaps; ++i) {
            total += m_heaps[i].len;
        }
        MemoryUsageMessage("VMem heaps used %d. Total memory %08x\n", m_nHeaps, total, 0);

        /* Walk all the heaps - verify structures */
        for(int index = 0; index < m_nHeaps; ++index) {
            PBLOCK ptr = m_heaps[index].base;
            size_t size = m_heaps[index].len;
#ifndef _USE_BUDDY_BLOCKS
#ifdef USE_BIGBLOCK_ALLOC
            if (!m_heaps[m_nHeaps].bBigBlock)
#endif
                ASSERT(HeapValidate(m_hHeap, HEAP_NO_SERIALIZE, ptr));
#endif

            /* set over reserved header block */
            size -= blockOverhead;
            ptr += blockOverhead;
            PBLOCK pLast = ptr + size;
            ASSERT(PSIZE(ptr) == 1); /* dummy previous block is allocated */
            ASSERT(SIZE(pLast) == 1); /* dummy next block is allocated */
            while(ptr < pLast) {
                ASSERT(ptr > m_heaps[index].base);
                size_t cursize = SIZE(ptr) & ~1;
                ASSERT((PSIZE(ptr+cursize) & ~1) == cursize);
                MemoryUsageMessage("Memory Block %08x: Size %08x %c\n", (long)ptr, cursize, (SIZE(ptr)&1) ? 'x' : ' ');
                if(!(SIZE(ptr)&1)) {
                    /* this block is on the free list */
                    PBLOCK tmp = NEXT(ptr);
                    while(tmp != ptr) {
                        ASSERT((SIZE(tmp)&1)==0);
                        if(tmp == m_pFreeList)
                            break;
                        ASSERT(NEXT(tmp));
                        tmp = NEXT(tmp);
                    }
                    if(tmp == ptr) {
                        MemoryUsageMessage("Memory Block %08x: Size %08x free but not in free list\n", (long)ptr, cursize, 0);
                    }
                }
                ptr += cursize;
            }
        }
        MemoryUsageMessage(NULL, 0, 0, 0);
    }
}
#endif	/* _DEBUG_MEM */

#endif	/* _USE_MSVCRT_MEM_ALLOC */

#endif	/* ___VMEM_H_INC___ */
