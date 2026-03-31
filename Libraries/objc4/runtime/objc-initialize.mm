/*
 * Copyright (c) 1999-2007 Apple Inc.  All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/***********************************************************************
* objc-initialize.m
* +initialize support
**********************************************************************/

/***********************************************************************
 * Thread-safety during class initialization (GrP 2001-9-24)
 *
 * Initial state: CLS_INITIALIZING and CLS_INITIALIZED both clear. 
 * During initialization: CLS_INITIALIZING is set
 * After initialization: CLS_INITIALIZING clear and CLS_INITIALIZED set.
 * CLS_INITIALIZING and CLS_INITIALIZED are never set at the same time.
 * CLS_INITIALIZED is never cleared once set.
 *
 * Only one thread is allowed to actually initialize a class and send 
 * +initialize. Enforced by allowing only one thread to set CLS_INITIALIZING.
 *
 * Additionally, threads trying to send messages to a class must wait for 
 * +initialize to finish. During initialization of a class, that class's 
 * method cache is kept empty. objc_msgSend will revert to 
 * class_lookupMethodAndLoadCache, which checks CLS_INITIALIZED before 
 * messaging. If CLS_INITIALIZED is clear but CLS_INITIALIZING is set, 
 * the thread must block, unless it is the thread that started 
 * initializing the class in the first place. 
 *
 * Each thread keeps a list of classes it's initializing. 
 * The global classInitLock is used to synchronize changes to CLS_INITIALIZED 
 * and CLS_INITIALIZING: the transition to CLS_INITIALIZING must be 
 * an atomic test-and-set with respect to itself and the transition 
 * to CLS_INITIALIZED.
 * The global classInitWaitCond is used to block threads waiting for an 
 * initialization to complete. The classInitLock synchronizes
 * condition checking and the condition variable.
 **********************************************************************/

/***********************************************************************
 *  +initialize deadlock case when a class is marked initializing while 
 *  its superclass is initialized. Solved by completely initializing 
 *  superclasses before beginning to initialize a class.
 *
 *  OmniWeb class hierarchy:
 *                 OBObject 
 *                     |    ` OBPostLoader
 *                 OFObject
 *                 /     \
 *      OWAddressEntry  OWController
 *                        | 
 *                      OWConsoleController
 *
 *  Thread 1 (evil testing thread):
 *    initialize OWAddressEntry
 *    super init OFObject
 *    super init OBObject		     
 *    [OBObject initialize] runs OBPostLoader, which inits lots of classes...
 *    initialize OWConsoleController
 *    super init OWController - wait for Thread 2 to finish OWController init
 *
 *  Thread 2 (normal OmniWeb thread):
 *    initialize OWController
 *    super init OFObject - wait for Thread 1 to finish OFObject init
 *
 *  deadlock!
 *
 *  Solution: fully initialize super classes before beginning to initialize 
 *  a subclass. Then the initializing+initialized part of the class hierarchy
 *  will be a contiguous subtree starting at the root, so other threads 
 *  can't jump into the middle between two initializing classes, and we won't 
 *  get stuck while a superclass waits for its subclass which waits for the 
 *  superclass.
 **********************************************************************/

#include "objc-private.h"
#include "message.h"
#include "objc-initialize.h"
#include "DenseMapExtras.h"

/* classInitLock protects CLS_INITIALIZED and CLS_INITIALIZING, and 
 * is signalled when any class is done initializing. 
 * Threads that are waiting for a class to finish initializing wait on this. */
monitor_t classInitLock;


struct _objc_willInitializeClassCallback {
    _objc_func_willInitializeClass f;
    void *context;
};
static GlobalSmallVector<_objc_willInitializeClassCallback, 1> willInitializeFuncs;


/***********************************************************************
* struct _objc_initializing_classes
* Per-thread list of classes currently being initialized by that thread. 
* During initialization, that thread is allowed to send messages to that 
* class, but other threads have to wait.
* The list is a simple array of metaclasses (the metaclass stores 
* the initialization state). 
**********************************************************************/
typedef struct _objc_initializing_classes {
    int classesAllocated;
    Class *metaclasses;
} _objc_initializing_classes;


/***********************************************************************
* _fetchInitializingClassList
* Return the list of classes being initialized by this thread.
* If create == YES, create the list when no classes are being initialized by this thread.
* If create == NO, return nil when no classes are being initialized by this thread.
**********************************************************************/
static _objc_initializing_classes *_fetchInitializingClassList(bool create)
{
    _objc_pthread_data *data;
    _objc_initializing_classes *list;
    Class *classes;

    data = _objc_fetch_pthread_data(create);
    if (data == nil) return nil;

    list = data->initializingClasses;
    if (list == nil) {
        if (!create) {
            return nil;
        } else {
            list = (_objc_initializing_classes *)
                calloc(1, sizeof(_objc_initializing_classes));
            data->initializingClasses = list;
        }
    }

    classes = list->metaclasses;
    if (classes == nil) {
        // If _objc_initializing_classes exists, allocate metaclass array, 
        // even if create == NO.
        // Allow 4 simultaneous class inits on this thread before realloc.
        list->classesAllocated = 4;
        classes = (Class *)
            calloc(list->classesAllocated, sizeof(Class));
        list->metaclasses = classes;
    }
    return list;
}


/***********************************************************************
* _destroyInitializingClassList
* Deallocate memory used by the given initialization list. 
* Any part of the list may be nil.
* Called from _objc_pthread_destroyspecific().
**********************************************************************/

void _destroyInitializingClassList(struct _objc_initializing_classes *list)
{
    if (list != nil) {
        if (list->metaclasses != nil) {
            free(list->metaclasses);
        }
        free(list);
    }
}


/***********************************************************************
* _thisThreadIsInitializingClass
* Return TRUE if this thread is currently initializing the given class.
**********************************************************************/
bool _thisThreadIsInitializingClass(Class cls)
{
    int i;

    _objc_initializing_classes *list = _fetchInitializingClassList(NO);
    if (list) {
        cls = cls->getMeta();
        for (i = 0; i < list->classesAllocated; i++) {
            if (cls == list->metaclasses[i]) return YES;
        }
    }

    // no list or not found in list
    return NO;
}


/***********************************************************************
* _setThisThreadIsInitializingClass
* Record that this thread is currently initializing the given class. 
* This thread will be allowed to send messages to the class, but 
*   other threads will have to wait.
**********************************************************************/
static void _setThisThreadIsInitializingClass(Class cls)
{
    int i;
    _objc_initializing_classes *list = _fetchInitializingClassList(YES);
    cls = cls->getMeta();
  
    // paranoia: explicitly disallow duplicates
    for (i = 0; i < list->classesAllocated; i++) {
        if (cls == list->metaclasses[i]) {
            _objc_fatal("thread is already initializing this class!");
            return; // already the initializer
        }
    }
  
    for (i = 0; i < list->classesAllocated; i++) {
        if (! list->metaclasses[i]) {
            list->metaclasses[i] = cls;
            return;
        }
    }

    // class list is full - reallocate
    list->classesAllocated = list->classesAllocated * 2 + 1;
    list->metaclasses = (Class *) 
        realloc(list->metaclasses,
                          list->classesAllocated * sizeof(Class));
    // zero out the new entries
    list->metaclasses[i++] = cls;
    for ( ; i < list->classesAllocated; i++) {
        list->metaclasses[i] = nil;
    }
}


/***********************************************************************
* _setThisThreadIsNotInitializingClass
* Record that this thread is no longer initializing the given class. 
**********************************************************************/
static void _setThisThreadIsNotInitializingClass(Class cls)
{
    int i;

    _objc_initializing_classes *list = _fetchInitializingClassList(NO);
    if (list) {
        cls = cls->getMeta();
        for (i = 0; i < list->classesAllocated; i++) {
            if (cls == list->metaclasses[i]) {
                list->metaclasses[i] = nil;
                return;
            }
        }
    }

    // no list or not found in list
    _objc_fatal("thread is not initializing this class!");  
}


typedef struct PendingInitialize {
    Class subclass;
    struct PendingInitialize *next;

    PendingInitialize(Class cls) : subclass(cls), next(nullptr) { }
} PendingInitialize;

typedef objc::DenseMap<Class, PendingInitialize *> PendingInitializeMap;
static PendingInitializeMap *pendingInitializeMap;

/***********************************************************************
* _finishInitializing
* cls has completed its +initialize method, and so has its superclass.
* Mark cls as initialized as well, then mark any of cls's subclasses 
* that have already finished their own +initialize methods.
**********************************************************************/
static void _finishInitializing(Class cls, Class supercls)
{
    PendingInitialize *pending;

    classInitLock.assertLocked();
    ASSERT(!supercls  ||  supercls->isInitialized());

    if (PrintInitializing) {
        _objc_inform("INITIALIZE: thread %p: %s is fully +initialized",
                     objc_thread_self(), cls->nameForLogging());
    }

    // mark this class as fully +initialized
    cls->setInitialized();
    classInitLock.notifyAll();
    _setThisThreadIsNotInitializingClass(cls);
    
    // mark any subclasses that were merely waiting for this class
    if (!pendingInitializeMap) return;

    auto it = pendingInitializeMap->find(cls);
    if (it == pendingInitializeMap->end()) return;

    pending = it->second;
    pendingInitializeMap->erase(it);

    // Destroy the pending table if it's now empty, to save memory.
    if (pendingInitializeMap->size() == 0) {
        delete pendingInitializeMap;
        pendingInitializeMap = nil;
    }

    while (pending) {
        PendingInitialize *next = pending->next;
        if (pending->subclass) _finishInitializing(pending->subclass, cls);
        delete pending;
        pending = next;
    }
}


/***********************************************************************
* _finishInitializingAfter
* cls has completed its +initialize method, but its superclass has not.
* Wait until supercls finishes before marking cls as initialized.
**********************************************************************/
static void _finishInitializingAfter(Class cls, Class supercls)
{

    classInitLock.assertLocked();

    if (PrintInitializing) {
        _objc_inform("INITIALIZE: thread %p: class %s will be marked as fully "
                     "+initialized after superclass +[%s initialize] completes",
                     objc_thread_self(), cls->nameForLogging(),
                     supercls->nameForLogging());
    }

    if (!pendingInitializeMap) {
        pendingInitializeMap = new PendingInitializeMap{10};
        // fixme pre-size this table for CF/NSObject +initialize
    }

    PendingInitialize *pending = new PendingInitialize{cls};
    auto result = pendingInitializeMap->try_emplace(supercls, pending);
    if (!result.second) {
        pending->next = result.first->second;
        result.first->second = pending;
    }
}


// Provide helpful messages in stack traces.
OBJC_EXTERN __attribute__((noinline, used, visibility("hidden")))
void waitForInitializeToComplete(Class cls)
    asm("_WAITING_FOR_ANOTHER_THREAD_TO_FINISH_CALLING_+initialize");
OBJC_EXTERN __attribute__((noinline, used, visibility("hidden")))
void callInitialize(Class cls)
    asm("_CALLING_SOME_+initialize_METHOD");


void waitForInitializeToComplete(Class cls)
{
    if (PrintInitializing) {
        _objc_inform("INITIALIZE: thread %p: blocking until +[%s initialize] "
                     "completes", objc_thread_self(), cls->nameForLogging());
    }

    monitor_locker_t lock(classInitLock);
    while (!cls->isInitialized()) {
        classInitLock.wait();
    }
    asm("");
}


void callInitialize(Class cls)
{
    ((void(*)(Class, SEL))objc_msgSend)(cls, @selector(initialize));
    asm("");
}


/***********************************************************************
* classHasTrivialInitialize
* Returns true if the class has no +initialize implementation or 
* has a +initialize implementation that looks empty.
* Any root class +initialize implemetation is assumed to be trivial.
**********************************************************************/
static bool classHasTrivialInitialize(Class cls)
{
    if (cls->isRootClass() || cls->isRootMetaclass()) return true;

    Class rootCls = cls->ISA()->ISA()->superclass;
    
    IMP rootImp = lookUpImpOrNil(rootCls, @selector(initialize), rootCls->ISA());
    IMP imp = lookUpImpOrNil(cls, @selector(initialize), cls->ISA());
    return (imp == nil  ||  imp == (IMP)&objc_noop_imp  ||  imp == rootImp);
}


/***********************************************************************
* lockAndFinishInitializing
* Mark a class as finished initializing and notify waiters, or queue for later.
* If the superclass is also done initializing, then update 
*   the info bits and notify waiting threads.
* If not, update them later. (This can happen if this +initialize 
*   was itself triggered from inside a superclass +initialize.)
**********************************************************************/
static void lockAndFinishInitializing(Class cls, Class supercls)
{
    monitor_locker_t lock(classInitLock);
    if (!supercls  ||  supercls->isInitialized()) {
        _finishInitializing(cls, supercls);
    } else {
        _finishInitializingAfter(cls, supercls);
    }
}


/***********************************************************************
* performForkChildInitialize
* +initialize after fork() is problematic. It's possible for the 
* fork child process to call some +initialize that would deadlock waiting 
* for another +initialize in the parent process. 
* We wouldn't know how much progress it made therein, so we can't
* act as if +initialize completed nor can we restart +initialize
* from scratch.
*
* Instead we proceed introspectively. If the class has some
* +initialize implementation, we halt. If the class has no
* +initialize implementation of its own, we continue. Root
* class +initialize is assumed to be empty if it exists.
*
* We apply this rule even if the child's +initialize does not appear 
* to be blocked by anything. This prevents races wherein the +initialize
* deadlock only rarely hits. Instead we disallow it even when we "won" 
* the race. 
*
* Exception: processes that are single-threaded when fork() is called 
* have no restrictions on +initialize in the child. Examples: sshd and httpd.
*
* Classes that wish to implement +initialize and be callable after 
* fork() must use an atfork() handler to provoke +initialize in fork prepare.
**********************************************************************/

// Called before halting when some +initialize 
// method can't be called after fork().
BREAKPOINT_FUNCTION(
    void objc_initializeAfterForkError(Class cls)
);

void performForkChildInitialize(Class cls, Class supercls)
{
    if (classHasTrivialInitialize(cls)) {
        if (PrintInitializing) {
            _objc_inform("INITIALIZE: thread %p: skipping trivial +[%s "
                         "initialize] in fork() child process",
                         objc_thread_self(), cls->nameForLogging());
        }
        lockAndFinishInitializing(cls, supercls);
    }
    else {
        if (PrintInitializing) {
            _objc_inform("INITIALIZE: thread %p: refusing to call +[%s "
                         "initialize] in fork() child process because "
                         "it may have been in progress when fork() was called",
                         objc_thread_self(), cls->nameForLogging());
        }
        _objc_inform_now_and_on_crash
            ("+[%s initialize] may have been in progress in another thread "
             "when fork() was called.",
             cls->nameForLogging());
        objc_initializeAfterForkError(cls);
        _objc_fatal
            ("+[%s initialize] may have been in progress in another thread "
             "when fork() was called. We cannot safely call it or "
             "ignore it in the fork() child process. Crashing instead. "
             "Set a breakpoint on objc_initializeAfterForkError to debug.",
             cls->nameForLogging());
    }
}


/***********************************************************************
* class_initialize.  Send the '+initialize' message on demand to any
* uninitialized class. Force initialization of superclasses first.
**********************************************************************/
void initializeNonMetaClass(Class cls)
{
    ASSERT(!cls->isMetaClass());

    Class supercls;
    bool reallyInitialize = NO;

    // Make sure super is done initializing BEFORE beginning to initialize cls.
    // See note about deadlock above.
    supercls = cls->superclass;
    if (supercls  &&  !supercls->isInitialized()) {
        initializeNonMetaClass(supercls);
    }
    
    // Try to atomically set CLS_INITIALIZING.
    SmallVector<_objc_willInitializeClassCallback, 1> localWillInitializeFuncs;
    {
        monitor_locker_t lock(classInitLock);
        if (!cls->isInitialized() && !cls->isInitializing()) {
            cls->setInitializing();
            reallyInitialize = YES;

            // Grab a copy of the will-initialize funcs with the lock held.
            localWillInitializeFuncs.initFrom(willInitializeFuncs);
        }
    }
    
    if (reallyInitialize) {
        // We successfully set the CLS_INITIALIZING bit. Initialize the class.
        
        // Record that we're initializing this class so we can message it.
        _setThisThreadIsInitializingClass(cls);

        if (MultithreadedForkChild) {
            // LOL JK we don't really call +initialize methods after fork().
            performForkChildInitialize(cls, supercls);
            return;
        }
        
        for (auto callback : localWillInitializeFuncs)
            callback.f(callback.context, cls);

        // Send the +initialize message.
        // Note that +initialize is sent to the superclass (again) if 
        // this class doesn't implement +initialize. 2157218
        if (PrintInitializing) {
            _objc_inform("INITIALIZE: thread %p: calling +[%s initialize]",
                         objc_thread_self(), cls->nameForLogging());
        }

        // Exceptions: A +initialize call that throws an exception 
        // is deemed to be a complete and successful +initialize.
        //
        // Only __OBJC2__ adds these handlers. !__OBJC2__ has a
        // bootstrapping problem of this versus CF's call to
        // objc_exception_set_functions().
#if __OBJC2__
        @try
#endif
        {
            callInitialize(cls);

            if (PrintInitializing) {
                _objc_inform("INITIALIZE: thread %p: finished +[%s initialize]",
                             objc_thread_self(), cls->nameForLogging());
            }
        }
#if __OBJC2__
        @catch (...) {
            if (PrintInitializing) {
                _objc_inform("INITIALIZE: thread %p: +[%s initialize] "
                             "threw an exception",
                             objc_thread_self(), cls->nameForLogging());
            }
            @throw;
        }
        @finally
#endif
        {
            // Done initializing.
            lockAndFinishInitializing(cls, supercls);
        }
        return;
    }
    
    else if (cls->isInitializing()) {
        // We couldn't set INITIALIZING because INITIALIZING was already set.
        // If this thread set it earlier, continue normally.
        // If some other thread set it, block until initialize is done.
        // It's ok if INITIALIZING changes to INITIALIZED while we're here, 
        //   because we safely check for INITIALIZED inside the lock 
        //   before blocking.
        if (_thisThreadIsInitializingClass(cls)) {
            return;
        } else if (!MultithreadedForkChild) {
            waitForInitializeToComplete(cls);
            return;
        } else {
            // We're on the child side of fork(), facing a class that
            // was initializing by some other thread when fork() was called.
            _setThisThreadIsInitializingClass(cls);
            performForkChildInitialize(cls, supercls);
        }
    }
    
    else if (cls->isInitialized()) {
        // Set CLS_INITIALIZING failed because someone else already 
        //   initialized the class. Continue normally.
        // NOTE this check must come AFTER the ISINITIALIZING case.
        // Otherwise: Another thread is initializing this class. ISINITIALIZED 
        //   is false. Skip this clause. Then the other thread finishes 
        //   initialization and sets INITIALIZING=no and INITIALIZED=yes. 
        //   Skip the ISINITIALIZING clause. Die horribly.
        return;
    }
    
    else {
        // We shouldn't be here. 
        _objc_fatal("thread-safe class init in objc runtime is buggy!");
    }
}

void _objc_addWillInitializeClassFunc(_objc_func_willInitializeClass _Nonnull func, void * _Nullable context) {
#if __OBJC2__
    unsigned count;
    Class *realizedClasses;

    // Fetch all currently initialized classes. Do this with classInitLock held
    // so we don't race with setting those flags.
    {
        monitor_locker_t initLock(classInitLock);
        realizedClasses = objc_copyRealizedClassList(&count);
        for (unsigned i = 0; i < count; i++) {
            // Remove uninitialized classes from the array.
            if (!realizedClasses[i]->isInitializing() && !realizedClasses[i]->isInitialized())
                realizedClasses[i] = Nil;
        }

        willInitializeFuncs.append({func, context});
    }

    // Invoke the callback for all realized classes that weren't cleared out.
    for (unsigned i = 0; i < count; i++) {
        if (Class cls = realizedClasses[i]) {
            func(context, cls);
        }
    }

    free(realizedClasses);
#endif
}
