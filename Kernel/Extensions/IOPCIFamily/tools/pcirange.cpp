/*
cc pcirange.cpp -o /tmp/pcirange -Wall -framework IOKit -framework CoreFoundation -arch i386 -g -lstdc++
*/

#include <assert.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOKitKeys.h>


typedef uint64_t IOPCIScalar;

struct IOPCIRange
{
    IOPCIScalar         start;
    IOPCIScalar         size;
    IOPCIScalar         end;
    IOPCIScalar         zero;

    IOPCIScalar         alignment;
    UInt32              type;
    UInt32              flags;
    struct IOPCIRange * next;
    struct IOPCIRange * nextSubRange;
    struct IOPCIRange * subRanges;
    struct IOPCIRange * allocations;
};

IOPCIScalar align(IOPCIScalar num, IOPCIScalar alignment)
{
    return (num + (alignment - 1) & ~(alignment - 1));
}

IOPCIRange * pciRangeAlloc(void)
{
    return ((IOPCIRange *) malloc(sizeof(IOPCIRange)));
}

void pciRangeFree(IOPCIRange * range)
{
    memset(range, 0xbb, sizeof(*range));
    free(range);
}

void pciRangeInit(IOPCIRange * range,
                  IOPCIScalar start, IOPCIScalar size, IOPCIScalar alignment = 0)
{
    bzero(range, sizeof(*range));
    range->start       = start;
    range->size        = size;
    range->end         = start + size;
    range->zero        = 0;
    range->alignment   = alignment ? alignment : size;
    range->allocations = (IOPCIRange *) &range->end;
}


bool pciRangeListAddRange(IOPCIRange ** rangeList,
                          IOPCIScalar start,
                          IOPCIScalar size,
                          IOPCIScalar alignment)
{
    IOPCIRange *  range;
    IOPCIRange *  nextRange;
    IOPCIRange ** prev;
    IOPCIScalar   end;
    bool          result = true;
    bool          alloc  = true;

    end = start + size;
    for (prev = rangeList; (range = *prev); prev = &range->next)
    {
        if (((start >= range->start) && (start < range->end))
         || ((end > range->start) && (end <= range->end))
         || ((start < range->start) && (end > range->end)))
        {
            range = NULL;
            result = false;
            break;
        }
        if (end == range->start)
        {
            range->start = start;
            range->size = range->end - range->start;
            alloc = false;
            break;
        }
        if (start == range->end)
        {
            if ((nextRange = range->next) && (nextRange->start == end))
            {
                if (nextRange->allocations != (IOPCIRange *) &nextRange->end)
                    assert(false);
                if (nextRange->subRanges != NULL)
                    assert(false);
                end = nextRange->end;
                range->next = nextRange->next;
                pciRangeFree(nextRange);
            }
            range->end = end;
            range->size = end - range->start;
            alloc = false;
            break;
        }
        if (range->start > end)
        {
            alloc = true;
            break;
        }
    }

    if (result && alloc)
    {
        nextRange = pciRangeAlloc();
        pciRangeInit(nextRange, start, size, alignment);
        nextRange->next = range;
        *prev = nextRange;
    }

    return (result);
}

bool pciRangeAllocateSubRange(IOPCIRange * headRange,
                              IOPCIRange * newRange,
                              IOPCIScalar  newSize = 0,
                              IOPCIScalar  newStart = 0)
{
    IOPCIScalar   pos, end, size, len;
    IOPCIRange *  range;
    IOPCIRange *  next;
    IOPCIRange ** prev;

    size = newSize  ? newSize : newRange->size;
    pos  = newStart ? newStart : newRange->start;
    if (pos)
    {
        end = pos + size;
        if ((pos < headRange->start) || (end > headRange->end))
            return (false);
    }
    else
    {
        pos = align(headRange->start, newRange->alignment);
        end = 0;
    }

    prev = &headRange->allocations;
    range = *prev;
    do
    {
        next = range->nextSubRange;
        if (range == newRange)
            continue;
        if (end)
        {
            len = range->start + range->size;
            if (((pos >= range->start) && (pos < len))
             || ((end > range->start) && (end <= len))
             || ((pos < range->start) && (end > len)))
            {
                range = NULL;
                break;
            }
        }
        if (range->start > pos)
        {
            len = range->start - pos;
            if (len >= size)
            {
                break;
            }
        }
        // keep walking down the list
        prev = &range->nextSubRange;
        if (!range->size)
        {
            range = NULL;
            break;
        }
        if (!end)
            pos = align(range->start + range->size, newRange->alignment);
    }
    while(range = next, true); 

    if (range)
    {
        newRange->start = pos;
        newRange->size  = size;
        newRange->end   = pos + size;
        newRange->nextSubRange = range;
        *prev = newRange;
    }

    return (range != 0);
}

bool pciRangeDeallocateSubRange(IOPCIRange * headRange,
                                IOPCIRange * oldRange)
{
    IOPCIRange *  range;
    IOPCIRange ** prev;

    prev = &headRange->allocations;
    do
    {
        range = *prev;
        if (range == oldRange)
            break;
        // keep walking down the list
        if (!range->size)
        {
            range = NULL;
            break;
        }
    }
    while (prev = &range->nextSubRange, true);

    if (range)
        *prev = range->nextSubRange;

    return (range != 0);
}

bool pciRangeListAllocateSubRange(IOPCIRange * headRange,
                                  IOPCIRange * newRange,
                                  IOPCIScalar  newSize = 0,
                                  IOPCIScalar  newStart = 0)
{
    bool ok = false;

    while (headRange && !ok)
    {
        ok = pciRangeAllocateSubRange(headRange, newRange, newSize, newStart);
        headRange = headRange->next;
    }

    return (ok);
}

bool pciRangeListDeallocateSubRange(IOPCIRange * headRange,
                                IOPCIRange * oldRange)
{
    bool ok = false;

    while (headRange && !ok)
    {
        ok = pciRangeDeallocateSubRange(headRange, oldRange);
        headRange = headRange->next;
    }

    return (ok);
}



bool pciRangeAppendSubRange(IOPCIRange * headRange,
                            IOPCIRange * newRange )
{
    bool result = false;

    IOPCIRange ** prev;
    IOPCIRange *  range;

    prev = &headRange->subRanges;
    do
    {
        range = *prev;
        if (!range)
            break;
        if (range->start && ((range->start < newRange->start) || !newRange->start))
            continue;
        if (newRange->start && !range->start)
            break;
        if (newRange->alignment > range->alignment)
            break;
    }
    while (prev = &range->nextSubRange, true);

    *prev = newRange;
    newRange->nextSubRange = range;
    result = true;

    return (result);
}

void dump(IOPCIRange * head)
{
    IOPCIRange * range;
    uint32_t idx;
    
    do
    {
        printf("head.start     0x%llx\n", head->start);
        printf("head.size      0x%llx\n", head->size);
        printf("head.end       0x%llx\n", head->end);
        printf("head.alignment 0x%llx\n", head->alignment);
    
        printf("allocs:\n");
        range = head->allocations;
        idx = 0;
        while (true)
        {
            if (range == (IOPCIRange *) &head->end)
            {
                printf("[end]\n");
                break;
            }
            printf("[%d].start     0x%llx\n", idx, range->start);
            printf("[%d].size      0x%llx\n", idx, range->size);
            printf("[%d].end       0x%llx\n", idx, range->end);
            printf("[%d].alignment 0x%llx\n", idx, range->alignment);
            idx++;
            range = range->nextSubRange;
        }
    
        printf("reqs:\n");
        range = head->subRanges;
        idx = 0;
        while (range)
        {
            printf("[%d].start     0x%llx\n", idx, range->start);
            printf("[%d].size      0x%llx\n", idx, range->size);
            printf("[%d].end       0x%llx\n", idx, range->end);
            printf("[%d].alignment 0x%llx\n", idx, range->alignment);
            idx++;
            range = range->nextSubRange;
        }
        printf("------\n");
    }
    while ((head = head->next));

    printf("------------------------------------\n");
}

int main(int argc, char **argv)
{
    IOPCIRange * head = NULL;
    IOPCIRange * range;
    IOPCIRange * elems[8];
    size_t       idx;
    bool         ok;

    pciRangeListAddRange(&head, 0x80000000, 0x08000000, 1);
    pciRangeListAddRange(&head, 0xA0000000, 0x10000000, 1);
    pciRangeListAddRange(&head, 0x98000000, 0x08000000, 1);
    pciRangeListAddRange(&head, 0x90000000, 0x08000000, 1);
    pciRangeListAddRange(&head, 0xB0000000, 0x10000000, 1);

//exit(0);

    for (idx = 0; idx < sizeof(elems) / sizeof(elems[0]); idx++)
    {
        elems[idx] = pciRangeAlloc();
    }

    idx = 0;
    pciRangeInit(elems[idx++], 0x80001000, 0x1000);
    pciRangeInit(elems[idx++], 0, 0x4000000);
    pciRangeInit(elems[idx++], 0, 0x20000000);
    pciRangeInit(elems[idx++], 0, 0x20000000);
    pciRangeInit(elems[idx++], 0x80002000, 0x800);
    pciRangeInit(elems[idx++], 0, 0x10000, 1);

    for (idx = 0; idx < sizeof(elems) / sizeof(elems[0]); idx++)
    {
        if (elems[idx]->size)
            pciRangeAppendSubRange(head, elems[idx]);
    }

    dump(head);

    while ((range = head->subRanges))
    {
        head->subRanges = range->nextSubRange;
        ok = pciRangeListAllocateSubRange(head, range);
        printf("alloc(%d) [0x%llx, 0x%llx]\n", ok, range->start, range->size);
    }

    dump(head);

    for (idx = 0; idx < sizeof(elems) / sizeof(elems[0]); idx++)
    {
        range = elems[idx];
        if (range->size && range->start)
        {
            ok = pciRangeListDeallocateSubRange(head, range);
            printf("dealloc(%d) [0x%llx, 0x%llx]\n", ok, range->start, range->size);
            ok = pciRangeListAllocateSubRange(head, range);
            printf("alloc(%d) [0x%llx, 0x%llx]\n", ok, range->start, range->size);
        }
    }

    // extend
    range = elems[5];
    ok = pciRangeListAllocateSubRange(head, range, 2 * range->size);
    printf("extalloc(%d) [0x%llx, 0x%llx]\n", ok, range->start, range->size);
    range = elems[0];
    ok = pciRangeListAllocateSubRange(head, range, 2 * range->size);
    printf("extalloc(%d) [0x%llx, 0x%llx]\n", ok, range->start, range->size);
    ok = pciRangeListAllocateSubRange(head, range, 2 * range->size, range->start - range->size);
    printf("extalloc(%d) [0x%llx, 0x%llx]\n", ok, range->start, range->size);

    dump(head);

    exit(0);    
}
