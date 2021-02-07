/* Implementation for the timsort sorting algorithm for GNUStep
   Copyright (C) 2012 Free Software Foundation, Inc.

   Written by:  Niels Grewe <niels.grewe@halbordnung.de>
   Date: September 2012

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
   */

#import "common.h"
#import "Foundation/NSSortDescriptor.h"

#import "Foundation/NSCoder.h"
#import "Foundation/NSException.h"
#import "Foundation/NSKeyValueCoding.h"

#import "GNUstepBase/GSObjCRuntime.h"
#import "GSPrivate.h"
#import "GSSorting.h"

/*
 * About this implementation.
 *
 * Timsort is a stable, adaptive hybrid of merge- and insertion-sort which
 * exploits existing structure in the data to be sorted, so that it takes
 * usually takes less than O(n*log(n)) comparisons for real world data. The
 * algorithm has been developed by Tim Peters and is described in [0].
 *
 * This implementation takes inspiration both from the C implementation in
 * Python [1] and from the Java implementation in OpenJDK [2].
 *
 *  [0] http://svn.python.org/projects/python/trunk/Objects/listsort.txt
 *  [1] http://svn.python.org/projects/python/trunk/Objects/listobject.c
 *  [2] http://cr.openjdk.java.net/~martin/webrevs/openjdk7/timsort/raw_files/new/src/share/classes/java/util/TimSort.java
 */

#define GS_MIN_MERGE 32
#define GS_MIN_GALLOP 7
#define GS_INITIAL_TEMP_STORAGE 256

/*
 * Galloping from left searches for an insertion point for key into the
 * already sorted buffer and returns the point immediately left of the first
 * equal element. We can also use this function, and gallopRight(), its twin, to
 * implement -[NSArray indexOfObject:inSortedRange:options:usingComparator:].
 * Since we want to do that, this implementation is a bit different than the one
 * in Python's listobject.c, since it takes into account the range and does just
 * the buffer's length starting from the base address. The hint argument is used
 * to give galloping a sensible start point for the search (it's usually 0 or
 * (r.length - 1)).
 */
static NSUInteger
gallopLeft(id key, id *buf, NSRange r, NSUInteger hint, id descOrComp,
  GSComparisonType type, void* context)
{
  NSInteger offset =  1;
  NSInteger lastOffset = 0;
  NSInteger k = 0;

  buf += (hint + r.location);
  if (NSOrderedAscending == GSCompareUsingDescriptorOrComparator(*buf,
    key,
    descOrComp,
    type,
    context))
    {
      /* In an ascending order, we gallop to the right until the key
       * is no longer greater than the element from the range
       */
      NSInteger maxOffset = (r.length - hint);
      while (offset < maxOffset)
        {
          if (NSOrderedAscending
            == GSCompareUsingDescriptorOrComparator(buf[offset],
            key,
            descOrComp,
            type,
            context))
            {
              lastOffset = offset;
              // We gallop by 1, 3, 7, 15,...
              offset = (offset << 1) + 1;

              if (offset <= 0)
                {
                  offset = maxOffset;
                }
            }
          else
            {
              break;
            }
        }
      if (offset > maxOffset)
        {
          offset = maxOffset;
        }
      /* we incremented the buf pointer by hint, so we need to
       * account for that in order to get the offset to the base address
       */
      lastOffset += r.location + hint;
      offset += r.location + hint;
    }
  else
    {
      /* In descending (or equal) order, we gallop to the left
       * until the key is no longer less than the element from the range
       */
      NSInteger maxOffset = hint + 1;
      while (offset < maxOffset)
        {
          if (NSOrderedAscending
            == GSCompareUsingDescriptorOrComparator(*(buf - offset),
            key, descOrComp, type, context))
            {
              break;
            }
          lastOffset = offset;
          offset = (offset << 1) + 1;
          if (offset <= 0)
            {
              offset = maxOffset;
            }
        }
      // Translate into positive offsets from array base address again.
      k = lastOffset;
      lastOffset = (r.location + hint) - offset;
      offset = (r.location + hint) - k;
    }
  // Restore base address:
  buf -= (hint + r.location);

  /*
   * We are now sure that we need to insert key somewhere between offset and
   * lastOffset, though the stride might have been to large for the range.
   * Fix the range and do a binary search with a vastly diminished search space.
   */
  offset = MIN(offset, NSMaxRange(r));
  if (lastOffset < (NSInteger)r.location)
    {
      lastOffset = (NSInteger)r.location;
    }
  while (lastOffset < offset)
    {
      NSInteger midPoint = lastOffset + ((offset - lastOffset) >> 1);

      if (NSOrderedAscending
        == GSCompareUsingDescriptorOrComparator(buf[midPoint],
        key, descOrComp, type, context))
        {
          lastOffset = midPoint + 1;
        }
      else
        {
          offset = midPoint;
        }
    }
  return (NSUInteger)offset;
}

/*
 * Equivalent to gallopLeft() except that it searches for an insertion point
 * right of the last equal element.
 */
static NSUInteger
gallopRight(id key, id *buf, NSRange r, NSUInteger hint,
  id descOrComp, GSComparisonType type, void *context)
{
  NSInteger offset = 1;
  NSInteger lastOffset = 0;
  NSInteger k = 0;

  buf += (hint + r.location);
  if (NSOrderedAscending == GSCompareUsingDescriptorOrComparator(key, *buf,
    descOrComp, type, context))
    {
      /* In an ascending order, we gallop to the right until
       * the key is no longer greater than the element from the range
       */
      NSInteger maxOffset = hint + 1;

      while (offset < maxOffset)
        {
          if (NSOrderedAscending == GSCompareUsingDescriptorOrComparator(key,
            *(buf - offset), descOrComp, type, context))
            {
              lastOffset = offset;
              offset = (offset << 1) + 1;
              if (offset <= 0)
                {
                  offset = maxOffset;
                }
            }
          else
            {
              break;
            }
        }
      if (offset > maxOffset)
        {
          offset = maxOffset;
        }
      // Translation to positive offsets against the base address.
      k = lastOffset;
      lastOffset = (r.location + hint) - offset;
      offset = (r.location + hint) - k;
    }
  else
    {
      // In descending (or equal) order, we gallop to the right

      NSInteger maxOffset = (r.length - hint);
      while (offset < maxOffset)
        {
          if (NSOrderedAscending
            == GSCompareUsingDescriptorOrComparator(key, buf[offset],
            descOrComp, type, context))
            {
              break;
            }
          lastOffset = offset;
          offset = (offset << 1) + 1;
          if (offset <= 0)
            {
              offset = maxOffset;
            }
        }
        // Translate into positive offsets from array base address again.
      lastOffset += (hint + r.location);
      offset += (hint + r.location);
    }
  // Restore base address:
  buf -= (hint + r.location);

  /*
   * We are now sure that we need to insert key somewhere between offset and
   * lastOffset, though the stride might have been to large for the range.
   * Fix the range and do a binary search with a vastly diminished search space.
   */
  lastOffset++;
  offset = MIN(offset, NSMaxRange(r));
  if (lastOffset < (NSInteger)r.location)
  {
    lastOffset = (NSInteger)r.location;
  }
  while (lastOffset < offset)
    {
      NSInteger midPoint = lastOffset + ((offset - lastOffset) >> 1);

      if (NSOrderedAscending
        == GSCompareUsingDescriptorOrComparator(key, buf[midPoint],
        descOrComp, type, context))
        {
          offset = midPoint;
        }
      else
        {
          lastOffset = midPoint + 1;
        }
    }
  return (NSUInteger)offset;
}


// Public versions of the galloping functions
NSUInteger
GSLeftInsertionPointForKeyInSortedRange(id key, id *buffer,
  NSRange range, NSComparator cmptr)
{
  return gallopLeft(key, buffer, range, 0, (id)cmptr,
    GSComparisonTypeComparatorBlock, NULL);
}


NSUInteger
GSRightInsertionPointForKeyInSortedRange(id key, id *buffer,
NSRange range, NSComparator cmptr)
{
  return gallopRight(key, buffer, range, 0, (id)cmptr,
    GSComparisonTypeComparatorBlock, NULL);
}

static inline void
reverseRange(id *buffer, NSRange r)
{
  NSUInteger loc = r.location;
  NSUInteger max = (NSMaxRange(r) - 1);

  while (loc < max)
    {
      id temp = buffer[loc];
      buffer[loc++] = buffer[max];
      buffer[max--] = temp;
    }
}

/* In-place binary insertion sorting for small arrays (i.e. those which are
 * smaller than GS_MIN_MERGE. We use this to generate minimal runs for timsort.
 */
static void
internalBinarySort(id *buffer,
  NSRange r,
  NSUInteger start,
  id compOrDesc,
  GSComparisonType type,
  void *context)
{
  NSUInteger min = r.location;
  NSUInteger max = NSMaxRange(r);

  NSCAssert2(NSLocationInRange(start, r),
    @"Start index %lu not in range %@",
    start, NSStringFromRange(r));

  if (min == start)
    {
      start++;
    }
  // We assume that everything before start is sorted.
  for (; start < max; ++start)
    {
      NSUInteger left = min;
      NSUInteger right = start;
      id pivot = buffer[right];
      int i = 0;

      do
        {
          NSUInteger midPoint = (left + ((right - left) >> 1));
          NSComparisonResult res = GSCompareUsingDescriptorOrComparator(pivot,
            buffer[midPoint],
            compOrDesc,
            type,
            context);
          if (NSOrderedAscending == res)
            {
              right = midPoint;
            }
          else
            {
              left = midPoint + 1;
            }
        } while (left < right);
      NSCAssert(left == right, @"Binary sort iteration did not end correctly,");
      // We make room for the pivot and place it at left.
      for (i = start; i > left; --i)
        {
          buffer[i] = buffer[(i - 1)];
        }
      buffer[left] = pivot;
    }
}


/*
 * Count the number of elements in the range that are already ordered.
 * If the order is a descending one, reverse it so that all runs are ordered the
 * same way.
 */
static inline NSUInteger
countAscendizedRun(id *buf, NSRange r, id descOrComp,
  GSComparisonType type, void*context)
{
  NSUInteger min = r.location;
  NSUInteger runMax = min + 1;
  NSUInteger rangeMax = NSMaxRange(r);

  if (runMax == rangeMax)
    {
      return 1;
    }
  if (NSOrderedDescending == GSCompareUsingDescriptorOrComparator(buf[min],
    buf[runMax++], descOrComp, type, context))
    {
      while ((runMax < rangeMax) && NSOrderedDescending
        == GSCompareUsingDescriptorOrComparator(buf[runMax - 1],
        buf[runMax], descOrComp, type, context))
        {
          runMax++;
        }
      reverseRange(buf, NSMakeRange(min, (runMax - min)));
    }
  else // ascending or equal
    {
      while ((runMax < rangeMax) && NSOrderedDescending
        != GSCompareUsingDescriptorOrComparator(buf[runMax - 1],
        buf[runMax], descOrComp, type, context))
        {
          runMax++;
        }
    }
  return (runMax - min);
}

/*
 * Calculate a sensible minimum length for the runs, these need to be powers of
 * two, or less than, but close to, one, but always at least GS_MIN_MERGE. For
 * details on why this is useful, see Python's listsort.txt.
 */
static inline NSUInteger
minimumRunLength(NSUInteger length)
{
  NSUInteger r = 0;

  while (length >= GS_MIN_MERGE)
    {
      r |= length & 1;
      length >>= 1;
    }

  return (length + r);
}

/*
 * For arrays up to GS_MIN_MERGE, we don't do merging. Instead, we identify
 * pre-ordering at the begining of the range and sort the rest using binary
 * sort.
 */
static inline void
miniTimSort(id *buf, NSRange r, id descOrComp, GSComparisonType ty, void *ctx)
{
  NSUInteger firstRunLength = countAscendizedRun(buf, r, descOrComp, ty, ctx);

  if (r.length == firstRunLength)
    {
      // In this case, we have already sorted the array here.
      return;
    }
  internalBinarySort(buf, r, (r.location + firstRunLength),
    descOrComp, ty, ctx);
}

/* These macros make calling the cached IMPs easier,
 * if we choose to do so later.
 */

#define GS_TIMSORT_CACHED_MSG(recv, sel) sel ## Imp(recv,@selector(sel))
#define GS_TIMSORT_CACHED_MSGV(recv, imp, sel, ...) imp(recv, @selector(sel), __VA_ARGS__)

#define GS_TIMSORT_SUGGEST_MERGE(desc) GS_TIMSORT_CACHED_MSG(desc, suggestMerge)
#define GS_TIMSORT_FORCE_MERGE(desc) GS_TIMSORT_CACHED_MSG(desc, forceMerge)
#define GS_TIMSORT_PUSH_RUN(desc, run) \
  GS_TIMSORT_CACHED_MSGV(desc, pushRunImp, pushRun:, run)
#define GS_TIMSORT_MERGE_AT_INDEX(desc, n) \
  GS_TIMSORT_CACHED_MSGV(desc, mergeAtIndexImp, mergeAtIndex:, n)
#define GS_TIMSORT_MERGE_LOW(desc, r1, r2) \
  GS_TIMSORT_CACHED_MSGV(desc, mergeLowImp, mergeLowRun:withRun:, r1, r2)
#define GS_TIMSORT_MERGE_HIGH(desc, r1, r2) \
  GS_TIMSORT_CACHED_MSGV(desc, mergeHighImp, mergeHighRun:withRun:, r1, r2)
#define GS_TIMSORT_ENSURE_TEMP_CAPACITY(desc, length) \
  GS_TIMSORT_CACHED_MSGV(desc, ensureCapImp, ensureTempCapacity:, length)


static IMP pushRunImp;
static IMP suggestMergeImp;
static IMP forceMergeImp;
static IMP mergeAtIndexImp;
static IMP mergeLowImp;
static IMP mergeHighImp;
static IMP ensureCapImp;

@interface GSTimSortPlaceHolder : NSObject
{
  id *objects;
  NSRange sortRange;
  id sortDescriptorOrComparator;
  GSComparisonType comparisonType;
  void *functionContext;
  NSUInteger minGallop;
  NSUInteger tempCapacity;
  id *tempBuffer;
  NSUInteger stackSize;
  NSRange* runStack;
}

- (id)initWithObjects: (id*)theObjects
            sortRange: (NSRange)theSortRange
           descriptor: (NSSortDescriptor*)descriptor;
- (id)initWithObjects: (id*)theObjects
            sortRange: (NSRange)theSortRange
           comparator: (NSComparator)comparator;
- (void)mergeAtIndex: (NSUInteger)index;
- (void)suggestMerge;
- (void)forceMerge;
@end



// Prototype for the actual timsort function.
static void
_GSTimSort(id *objects,
  NSRange sortRange,
  id sortDescriptorOrComparator,
  GSComparisonType comparisonType,
  void *context);

@implementation GSTimSortPlaceHolder
+ (void) load
{
  _GSSortStable = _GSTimSort;
}

+ (void) initialize
{
  if ([GSTimSortPlaceHolder class] == [self class])
    {
      // We need to be fast, so we cache a lot of IMPs
      pushRunImp =
        [self instanceMethodForSelector: @selector(pushRun:)];
      suggestMergeImp =
        [self instanceMethodForSelector: @selector(suggestMerge)];
      forceMergeImp =
        [self instanceMethodForSelector: @selector(forceMerge)];
      mergeAtIndexImp =
        [self instanceMethodForSelector: @selector(mergeAtIndex:)];
      mergeLowImp =
        [self instanceMethodForSelector: @selector(mergeLowRun:withRun:)];
      mergeHighImp =
        [self instanceMethodForSelector: @selector(mergeHighRun:withRun:)];
      ensureCapImp =
        [self instanceMethodForSelector: @selector(ensureTempCapacity:)];
    }
}

+ (void) setUnstable
{
  _GSSortUnstable = _GSTimSort; // Use for unstable even though we are stable
}

- (id) initWithObjects: (id*)theObjects
             sortRange: (NSRange)theSortRange
descriptorOrComparator: (id)descriptorOrComparator
        comparisonType: (GSComparisonType)ty
       functionContext: (void*)ctx
{
  NSUInteger sortLength = theSortRange.length;
  NSUInteger stackSpace = 0;

  if (nil == (self = [super init]))
    {
      return nil;
    }
  /* GSTimSortPlaceHolders are ephemeral objects that just track state, so we
   * don't bother making sure that the objects don't go away.
   */
  objects = theObjects;
  sortRange = theSortRange;
  sortDescriptorOrComparator = descriptorOrComparator;
  comparisonType = ty;
  functionContext = ctx;
  /* minGallop will be adjusted based on heuristics on whether we have a highly
   * structured array (in which case galloping is useful) or a more random one
   * (when it isn't).
   */
  minGallop = GS_MIN_GALLOP;
  stackSize = 0;
  /* timsort needs at most half the array size as temporary storage, so
   * we optimize for arrays that require less storage than we'd usually
   * allocate.
   */
  tempCapacity = ((sortLength < (2 * GS_INITIAL_TEMP_STORAGE))
    ? sortLength >> 1 : GS_INITIAL_TEMP_STORAGE);
  tempBuffer = malloc(sizeof(id) * tempCapacity );

  /* We also allocate the stack in which we track the runs based on the array
   * size. (The values are based of the OpenJDK implementation of timsort)
   */
  stackSpace = (sortLength < 120 ? 5 :
                  sortLength < 1542 ? 10 :
                   sortLength < 119151 ? 19 : 40);
  runStack = malloc(sizeof(NSRange) * stackSpace);
  return self;
}

- (id) initWithObjects: (id*)theObjects
             sortRange: (NSRange)theSortRange
            descriptor: (NSSortDescriptor*)descriptor
{
  return [self initWithObjects: theObjects
                     sortRange: theSortRange
        descriptorOrComparator: descriptor
                comparisonType: GSComparisonTypeSortDescriptor
               functionContext: NULL];
}

- (id) initWithObjects: (id*)theObjects
             sortRange: (NSRange)theSortRange
            comparator: (NSComparator)comparator
{
  return [self initWithObjects: theObjects
                     sortRange: theSortRange
        descriptorOrComparator: (id)comparator
                comparisonType: GSComparisonTypeComparatorBlock
               functionContext: NULL];
}


- (void) pushRun: (NSRange)r
{
  runStack[stackSize] = r;
  stackSize++;
  NSDebugMLLog(@"GSTimSort", @"Pushing run: %@", NSStringFromRange(r));
}

/**
 * Ensure that the invariant enabling the algorithm holds for the stack.
 *
 * see: http://www.envisage-project.eu/proving-android-java-and-python-sorting-algorithm-is-broken-and-how-to-fix-it/#sec3
 */
- (void) suggestMerge
{
  while (stackSize > 1)
    {
      NSInteger n = stackSize -2;

      if ((n >= 1 && runStack[n-1].length
	  <= (runStack[n].length + runStack[n+1].length))
        || (n >= 2 && runStack[n-2].length
	  <= (runStack[n].length + runStack[n-1].length)))
        {
          if (runStack[n-1].length < runStack[n+1].length)
            {
              n--;
            }
        }
      else if (runStack[n].length > runStack[n+1].length)
        {
          break; //invariant reached
        }
      GS_TIMSORT_MERGE_AT_INDEX(self, n);
    }
}

- (void) ensureTempCapacity: (NSUInteger)elementsRequired
{
  if (elementsRequired <= tempCapacity)
    {
      return;
    }
  /* We don't realloc any memory because we don't care about the contents from
   * previous merge iterations.
   */
  free(tempBuffer);
  tempBuffer = malloc(sizeof(id) * elementsRequired);
  tempCapacity = elementsRequired;
  //TODO: OOM exception
}


/*
 * Main merge algorithm: Does a pairwise merge in the general-case and
 * adaptively switches to galloping mode, which moves around whole chunks of the
 * array. This method is called if r1 is the shorter run (i.e. the one which
 * requires less temporary storage).
 */
- (void) mergeLowRun: (NSRange)r1 withRun: (NSRange)r2
{
  NSUInteger num1 = r1.length;
  NSUInteger num2 = r2.length;
  id *buf1 = objects + r1.location;
  id *buf2 = objects + r2.location;
  id *destination = buf1;
  NSUInteger k = 0;
  // Local variables for performance
  NSUInteger localMinGallop = minGallop;
  id descOrComp = sortDescriptorOrComparator;
  GSComparisonType ty = comparisonType;
  void *ctx = functionContext;

  /* We use the first run as our destination, so we copy out its contents into
   * the temporary storage (which needs to be large enough, though).
   */
  GS_TIMSORT_ENSURE_TEMP_CAPACITY(self, r1.length);
  memcpy(tempBuffer, buf1, (sizeof(id) * r1.length));

  destination = buf1;
  buf1 = tempBuffer;
  *destination++ = *buf2++;
  num2--;

  if (num2 == 0)
    {
      if (0 != num1)
        {
          memcpy(destination, buf1, num1 * sizeof(id));
        }
      return;
    }
  if (num1 == 1)
    {
      memmove(destination, buf2, num2 * sizeof(id));
      destination[num2] = *buf1;
      return;
    }

  NS_DURING
    {
      for (;;)
        {
          // Variables to track whether galloping is useful
          NSUInteger winners1 = 0;
          NSUInteger winners2 = 0;

          do
            {
              if (NSOrderedAscending
                == GSCompareUsingDescriptorOrComparator(*buf2, *buf1,
                descOrComp, ty, ctx))
                {
                  *destination++ = *buf2++;
                  winners2++;
                  winners1 = 0;
                  num2--;
                  if (num2 == 0)
                    {
                      goto Success;
                    }
                }
              else
                {
                  *destination++ = *buf1++;
                  winners1++;
                  winners2 = 0;
                  num1--;
                  if (num1 == 1)
                    {
                      goto CopyB;
                    }
                }
            } while ((winners1 | winners2) < localMinGallop);

          localMinGallop++;
          do
            {
              /* If we fall through here, one of the runs is very structured,
               * so we assume that galloping will also be useful in the future.
               */
              localMinGallop -= localMinGallop > 1;
              minGallop = localMinGallop;
              k = gallopRight(*buf2, buf1,
                NSMakeRange(0,num1), 0, descOrComp, ty, ctx);
              winners1 = k;
              if (0 != k)
                {
                  memcpy(destination, buf1, k * sizeof(id));
                  destination += k;
                  buf1 += k;
                  num1 -= k;
                  if (1 == num1)
                    {
                      goto CopyB;
                    }
                  if (0 == num1)
                    {
                      goto Success;
                    }
                }
              /* Since our galloping run finishes here, the next element
               * comes from r2
               */
              *destination++ = *buf2++;
              num2--;
              if (0 == num2)
                {
                  goto Success;
                }

              // Now we try to gallop into the other direction
              k = gallopLeft(*buf1, buf2, NSMakeRange(0, num2),
                0, descOrComp, ty, ctx);
              winners2 = k;
              if (0 != k)
                {
                  /* buf2 is part of the destination, not the temporary
                   * storage, so we need to memmove instead of memcpy to
                   * account for potential overlap.
                   */
                  memmove(destination, buf2, k * sizeof(id));
                  destination += k;
                  buf2 += k;
                  num2 -= k;
                  if (0 == num2)
                    {
                      goto Success;
                    }
                }
              /* Galloping run for r2 finished, next element comes from r1,
               * and starts the next loop iteration
               */
              *destination++ = *buf1++;
              num1--;
              if (1 == num1)
                {
                  goto CopyB;
                }
            } while (winners1 >= GS_MIN_GALLOP || winners2 >= GS_MIN_GALLOP);
          localMinGallop++;
          minGallop = localMinGallop;
        }
      Success:
        if (0 != num1)
          {
            memcpy(destination, buf1, num1 * sizeof(id));
          }
        NS_VOIDRETURN;
      CopyB:
        memmove(destination, buf2, num2 * sizeof(id));
        destination[num2] = *buf1;
        NS_VOIDRETURN;
    }
  NS_HANDLER
    {
      //In case of an exception, we need to copy back r1 into its original
      //position
      if (0 != num1)
        {
          memcpy(destination, buf1, num1 * sizeof(id));
        }
      [localException raise];
    }
  NS_ENDHANDLER
}

- (void) mergeHighRun: (NSRange)r1 withRun: (NSRange)r2
{
  NSUInteger num1 = r1.length;
  NSUInteger num2 = r2.length;
  id *buf1 = objects + r1.location;
  id *buf2 = objects + r2.location;
  id *base1 = buf1;
  id *base2 = NULL;
  // We are walking backwards, so destination pointer is at the high end
  // initially.
  id *destination = buf2 + num2 - 1;
  NSUInteger k = 0;
  // Local variables for performance
  NSUInteger localMinGallop = minGallop;
  id descOrComp = sortDescriptorOrComparator;
  GSComparisonType ty = comparisonType;
  void *ctx = functionContext;

  // We use the first run as our destination, so we copy out its contents into
  // the temporary storage (which needs to be large enough, though).
  GS_TIMSORT_ENSURE_TEMP_CAPACITY(self, r2.length);
  memcpy(tempBuffer, buf2, (sizeof(id) * r2.length));

  base2 = tempBuffer;
  buf2 = tempBuffer + num2 - 1;
  buf1 += num1 - 1;
  *destination-- = *buf1--;
  num1--;

  if (num1 == 0)
    {
      if (0 != num2)
        {
          memcpy(destination - (num2-1), base2, num2 * sizeof(id));
        }
      return;
    }
  if (num2 == 1)
    {
      destination -= num1;
      buf1 -= num1;
      memmove(destination + 1, buf1 + 1, num1 * sizeof(id));
      *destination = *buf2;
      return;
    }

  NS_DURING
    {
      for (;;)
        {
          // Variables to track whether galloping is useful
          NSUInteger winners1 = 0;
          NSUInteger winners2 = 0;

          do
            {
              if (NSOrderedAscending
                == GSCompareUsingDescriptorOrComparator(*buf2, *buf1,
                descOrComp, ty, ctx))
                {
                  *destination-- = *buf1--;
                  winners1++;
                  winners2 = 0;
                  num1--;
                  if (num1 == 0)
                    {
                      goto Success;
                    }
                }
              else
                {
                  *destination-- = *buf2--;
                  winners2++;
                  winners1 = 0;
                  num2--;
                  if (num2 == 1)
                    {
                      goto CopyA;
                    }
                }
            } while ((winners1 | winners2) < localMinGallop);

          localMinGallop++;
          do
            {
              /* If we fall through here, one of the runs is very structured,
               * so we assume that galloping will also be useful in the future.
               */
              localMinGallop -= localMinGallop > 1;
              minGallop = localMinGallop;
              k = gallopRight(*buf2, base1,
                NSMakeRange(0, num1), num1 - 1, descOrComp, ty, ctx);
              k = num1 - k;
              winners1 = k;
              if (0 != k)
                {
                  destination -= k;
                  buf1 -= k;
                  memmove(destination+1, buf1+1, k * sizeof(id));
                  num1 -= k;
                  if (0 == num1)
                    {
                      goto Success;
                    }
                }
              /* Since our galloping run finishes here,
               * the next element comes from r2
               */
              *destination-- = *buf2--;
              num2--;
              if (1 == num2)
                {
                  goto CopyA;
                }

              // Now we try to gallop into the other direction
              k = gallopLeft(*buf1, base2,
                NSMakeRange(0, num2), num2-1, descOrComp, ty, ctx);
              k = num2 - k;
              winners2 = k;
              if (0 != k)
                {
                  destination -= k;
                  buf2 -= k;
                  memcpy(destination + 1, buf2 + 1, k * sizeof(id));
                  num2 -= k;
                  if (1 == num2)
                    {
                      goto CopyA;
                    }
                  if (0 == num2)
                    {
                      goto Success;
                    }
                }
              /* Galloping run for r2 finished, next element comes from r1,
               * and starts the next loop iteration
               */
              *destination-- = *buf1--;
              num1--;
              if (0 == num1)
                {
                  goto Success;
                }
            } while (winners1 >= GS_MIN_GALLOP || winners2 >= GS_MIN_GALLOP);
          localMinGallop++;
          minGallop = localMinGallop;
        }
      Success:
        if (0 != num2)
          {
            memcpy(destination - (num2-1), base2, num2 * sizeof(id));
          }
        NS_VOIDRETURN;
      CopyA:
        destination -= num1;
        buf1 -= num1;
        memmove(destination + 1, buf1 + 1, num1 * sizeof(id));
        *destination = *buf2;
        NS_VOIDRETURN;
    }
  NS_HANDLER
    {
      //In case of an exception, we need to copy back r1 into its original
      //position
      if (0 != num2)
        {
          memcpy(destination - (num2-1), base2, num2 * sizeof(id));
        }
      [localException raise];
    }
  NS_ENDHANDLER
}

- (void) mergeAtIndex: (NSUInteger)i
{
  NSRange r1;
  NSRange r2;
  NSUInteger insert = 0;
  NSAssert((stackSize >= 2), @"Trying to merge without a plurality of runs.");
  NSAssert(((i == (stackSize - 2)) || (i == (stackSize - 3))),
    @"Trying at an index other than the penultimate or antepenultimate.");

  r1 = runStack[i];
  r2 = runStack[i+1];
  NSDebugMLLog(@"GSTimSort",
    @"Merging stack location %lu (stack size: %lu, run %@ with %@)", i,
    stackSize, NSStringFromRange(r1), NSStringFromRange(r2));

  /* Do some housekeeping on the stack: We combine the two runs
   * being merged and move around the last run on the stack
   * if we are merging on the antepenultimate run.
   * In any case, the run at i+1 is consumed in the merge and
   * the stack shrinks.
   */
  runStack[i] = NSUnionRange(r1, r2);
  if (i == (stackSize - 3))
    {
      runStack[i+1] = runStack[i+2];
    }
  stackSize--;

  // Find an insertion point for the first element in r2 into r1
  insert = gallopRight(objects[r2.location], objects, r1, 0,
    sortDescriptorOrComparator, comparisonType, functionContext);
  r1.length = r1.length - (insert - r1.location);
  r1.location = insert;
  if (r1.length == 0)
    {
      // The entire run r2 lies after r1, just return.
      return;
    }
  NSDebugMLLog(@"GSTimSort",
    @"Insertion point for r2 in r1: %lu, r1 for the merge is now %@.",
    insert, NSStringFromRange(r1));

  // Find an insertion point for the last element of r1 into r2. Subtracting the
  // location from that point gives us the length of the subrange we need to
  // merge.
  r2.length = (gallopLeft(objects[NSMaxRange(r1) - 1], objects, r2,
    (r2.length - 1),
    sortDescriptorOrComparator, comparisonType, functionContext)
     - r2.location);
  if (r2.length == 0)
    {
      return;
    }

  (r1.length <= r2.length)
    ? GS_TIMSORT_MERGE_LOW(self, r1, r2)
    : GS_TIMSORT_MERGE_HIGH(self, r1, r2);
}

/**
 * Force a final merge of the runs on the stack, so that only one run, covering
 * the whole array, remains.
 */
- (void) forceMerge
{
  while (stackSize > 1)
    {
      NSInteger n = stackSize - 2;
      if ((n > 0) && (runStack[n-1].length < runStack[n+1].length))
        {
          n--;
        }
      GS_TIMSORT_MERGE_AT_INDEX(self, n);
    }
}

- (void) dealloc
{
  free(runStack);
  free(tempBuffer);
  [super dealloc];
}

@end

static void
_GSTimSort(id *objects,
  NSRange sortRange,
  id sortDescriptorOrComparator,
  GSComparisonType comparisonType,
  void *context)
{
  NSUInteger sortStart = sortRange.location;
  NSUInteger sortEnd = NSMaxRange(sortRange);
  NSUInteger sortLen = sortRange.length;
  NSUInteger minimalRunLen = 0;
  GSTimSortPlaceHolder *desc = nil;
  if (sortLen < 2)
    {
      // Don't sort anything that doesn't contain at least two elements.
      return;
    }

  if (sortLen < GS_MIN_MERGE)
    {
      miniTimSort(objects, sortRange,
        sortDescriptorOrComparator, comparisonType, context);
      return;
    }

  // Now we need a timsort descriptor for state-tracking.
  desc = [[GSTimSortPlaceHolder alloc] initWithObjects: objects
    sortRange: sortRange
    descriptorOrComparator: sortDescriptorOrComparator
    comparisonType: comparisonType
    functionContext: context];

  NS_DURING
    {
      minimalRunLen = minimumRunLength(sortLen);
      do
        {
          NSUInteger runLen = countAscendizedRun(objects,
            NSMakeRange(sortStart, sortLen),
            sortDescriptorOrComparator,
            comparisonType, context);

          /* If the run is too short, coerce it up to minimalRunLen
           * or the end of the sortRange.
           */
          if (runLen < MIN(sortLen, minimalRunLen))
            {
              NSUInteger coercionLen;

              coercionLen = sortLen <= minimalRunLen ? sortLen : minimalRunLen;
              internalBinarySort(objects,
                NSMakeRange(sortStart, coercionLen),
                sortStart + runLen,
                sortDescriptorOrComparator,
                comparisonType,
                context);
              runLen = coercionLen;
            }

          GS_TIMSORT_PUSH_RUN(desc, NSMakeRange(sortStart, runLen));
          GS_TIMSORT_SUGGEST_MERGE(desc);
          sortStart += runLen;
          sortLen -= runLen;
        } while (sortLen != 0);

      NSCAssert(sortStart == sortEnd, @"Sorting did not complete");
      GS_TIMSORT_FORCE_MERGE(desc);
    }
  NS_HANDLER
    {
      [desc release];
      [localException raise];
    }
  NS_ENDHANDLER
  [desc release];
}

