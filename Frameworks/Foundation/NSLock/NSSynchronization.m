/* Copyright (c) 2008 Johannes Fortmann

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#if !defined(GCC_RUNTIME_3) && !defined(APPLE_RUNTIME_4)

#import <Foundation/NSMutableDictionary.h>
#import <Foundation/NSRecursiveLock.h>
#import <Foundation/NSValue.h>

#define NUM_CHAINS 16
#define ID_HASH(a) (((long)a >> 5) & (NUM_CHAINS - 1))

// lock to serialize accesses to the lock chain; also serves as a marker if we
// are currently locking (locking is disabled as long as there's no multithreading)
static NSLock **lockChainLock=NULL;

struct LockChain;
typedef struct LockChain
	{
		id lock;
		id object;
      unsigned owningThreads;
		struct LockChain *next;
	} LockChain;

static LockChain *allLocks[NUM_CHAINS]={0};

void _NSInitializeSynchronizedDirective()
{
	if(!lockChainLock)
	{
      int i;
      lockChainLock=NSZoneCalloc(NULL, NUM_CHAINS, sizeof(id));
      for(i=0; i<NUM_CHAINS; i++)
      {
         allLocks[i]=NSZoneCalloc(NULL, 1, sizeof(LockChain));

         allLocks[i]->object=0;
         allLocks[i]->next=0;
         allLocks[i]->owningThreads=0;
         allLocks[i]->lock=[NSRecursiveLock new];

         lockChainLock[i]=[NSLock new];
      }
	}
}

enum {
	OBJC_SYNC_SUCCESS                 = 0,
	OBJC_SYNC_NOT_OWNING_THREAD_ERROR = -1,
	OBJC_SYNC_TIMED_OUT               = -2,
	OBJC_SYNC_NOT_INITIALIZED         = -3
};

LockChain* lockForObject(id object, BOOL entering)
{
   NSLock *chainLock=lockChainLock[ID_HASH(object)];
	LockChain *result=allLocks[ID_HASH(object)];
	LockChain *firstFree=NULL;

   [chainLock lock];

	while(result)
	{
		if(result->object==object)
			goto done;
		if(result->object==NULL)
			firstFree=result;
      result=result->next;
	}

   if(!entering)
      goto done;

	if(firstFree)
	{
		firstFree->object=object;
		result=firstFree;
		goto done;
	}

	result=NSZoneCalloc(NULL, 1, sizeof(LockChain));
	result->object=object;
   result->owningThreads=0;
	result->next=allLocks[ID_HASH(object)];
	result->lock=[NSRecursiveLock new];
	allLocks[ID_HASH(object)]=result;

done:

   if(entering)
   {
      result->owningThreads++;
   }
   else
   {
      if(result)
      {
         result->owningThreads--;
         if(result->owningThreads==0)
            result->object=NULL;
      }
   }

	[chainLock unlock];
	return result;
}

FOUNDATION_EXPORT int objc_sync_enter(id obj)
{
	if(!obj)
		return OBJC_SYNC_SUCCESS;
	if(!lockChainLock)
		return OBJC_SYNC_NOT_INITIALIZED;

	LockChain *result=lockForObject(obj, YES);

   [result->lock lock];

   return OBJC_SYNC_SUCCESS;
}

FOUNDATION_EXPORT int objc_sync_exit(id obj)
{
	if(!obj)
		return OBJC_SYNC_SUCCESS;
	if(!lockChainLock)
		return OBJC_SYNC_NOT_INITIALIZED;

	LockChain *result=lockForObject(obj, NO);
   if(!result)
   {
      // this may happen only in the case where locking was initialized after a sync_enter,
      // but before the corresponding sync_exit.
		return OBJC_SYNC_NOT_INITIALIZED;
   }

   [result->lock unlock];

	return OBJC_SYNC_SUCCESS;
}

#endif
