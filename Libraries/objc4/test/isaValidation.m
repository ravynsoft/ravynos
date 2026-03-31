// TEST_CRASHES
// TEST_CONFIG MEM=mrc
/* 
TEST_RUN_OUTPUT
Testing object_getMethodImplementation
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing class_getInstanceMethod
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing class_getMethodImplementation
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing class_respondsToSelector
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing class_conformsToProtocol
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing class_copyProtocolList
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing class_getProperty
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing class_copyPropertyList
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing class_addMethod
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing class_replaceMethod
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing class_addIvar
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing class_addProtocol
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing class_addProperty
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing class_replaceProperty
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing class_setIvarLayout
objc\[\d+\]: \*\*\* Can't set ivar layout for already-registered class 'TestRoot'
objc\[\d+\]: \*\*\* Can't set ivar layout for already-registered class 'TestRoot'
objc\[\d+\]: \*\*\* Can't set ivar layout for already-registered class 'NSObject'
objc\[\d+\]: \*\*\* Can't set ivar layout for already-registered class 'NSObject'
objc\[\d+\]: \*\*\* Can't set ivar layout for already-registered class 'AllocatedTestClass2'
objc\[\d+\]: \*\*\* Can't set ivar layout for already-registered class 'AllocatedTestClass2'
objc\[\d+\]: \*\*\* Can't set ivar layout for already-registered class 'TestRoot'
objc\[\d+\]: \*\*\* Can't set ivar layout for already-registered class 'DuplicateClass'
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing class_setWeakIvarLayout
objc\[\d+\]: \*\*\* Can't set weak ivar layout for already-registered class 'TestRoot'
objc\[\d+\]: \*\*\* Can't set weak ivar layout for already-registered class 'TestRoot'
objc\[\d+\]: \*\*\* Can't set weak ivar layout for already-registered class 'NSObject'
objc\[\d+\]: \*\*\* Can't set weak ivar layout for already-registered class 'NSObject'
objc\[\d+\]: \*\*\* Can't set weak ivar layout for already-registered class 'AllocatedTestClass2'
objc\[\d+\]: \*\*\* Can't set weak ivar layout for already-registered class 'AllocatedTestClass2'
objc\[\d+\]: \*\*\* Can't set weak ivar layout for already-registered class 'TestRoot'
objc\[\d+\]: \*\*\* Can't set weak ivar layout for already-registered class 'DuplicateClass'
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing objc_registerClassPair
objc\[\d+\]: objc_registerClassPair: class 'TestRoot' was not allocated with objc_allocateClassPair!
objc\[\d+\]: objc_registerClassPair: class 'NSObject' was not allocated with objc_allocateClassPair!
objc\[\d+\]: objc_registerClassPair: class 'AllocatedTestClass2' was already registered!
objc\[\d+\]: objc_registerClassPair: class 'DuplicateClass' was not allocated with objc_allocateClassPair!
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing objc_duplicateClass
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Testing objc_disposeClassPair
objc\[\d+\]: objc_disposeClassPair: class 'TestRoot' was not allocated with objc_allocateClassPair!
objc\[\d+\]: objc_disposeClassPair: class 'NSObject' was not allocated with objc_allocateClassPair!
objc\[\d+\]: objc_disposeClassPair: class 'DuplicateClass' was not allocated with objc_allocateClassPair!
Completed test on good classes.
objc\[\d+\]: Attempt to use unknown class 0x[0-9a-f]+.
objc\[\d+\]: HALTED
Completed!
END
 */

#include "test.h"
#include "testroot.i"
#include <spawn.h>

@protocol P
@end

extern char **environ;

id dummyIMP(id self, SEL _cmd) { (void)_cmd; return self; }

char *dupeName(Class cls) {
    char *name;
    asprintf(&name, "%sDuplicate", class_getName(cls));
    return name;
}

typedef void (^TestBlock)(Class);
struct TestCase {
    const char *name;
    TestBlock block;
};

#define NAMED_TESTCASE(name, ...) { name, ^(Class cls) { __VA_ARGS__; } }
#define TESTCASE(...) NAMED_TESTCASE(#__VA_ARGS__, __VA_ARGS__)
#define TESTCASE_NOMETA(...) \
    NAMED_TESTCASE( #__VA_ARGS__, if(class_isMetaClass(cls)) return; __VA_ARGS__; )
#define TESTCASE_OBJ(...) NAMED_TESTCASE( \
    #__VA_ARGS__, \
    if(class_isMetaClass(cls)) return;          \
    id obj = [TestRoot alloc]; \
    *(Class *)obj = cls; \
    __VA_ARGS__; \
)

struct TestCase TestCases[] = {
    TESTCASE_OBJ(object_getMethodImplementation(obj, @selector(init))),
    
    TESTCASE(class_getInstanceMethod(cls, @selector(init))),
    TESTCASE(class_getMethodImplementation(cls, @selector(init))),
    TESTCASE(class_respondsToSelector(cls, @selector(init))),
    TESTCASE(class_conformsToProtocol(cls, @protocol(P))),
    TESTCASE(free(class_copyProtocolList(cls, NULL))),
    TESTCASE(class_getProperty(cls, "x")),
    TESTCASE(free(class_copyPropertyList(cls, NULL))),
    TESTCASE(class_addMethod(cls, @selector(nop), (IMP)dummyIMP, "v@:")),
    TESTCASE(class_replaceMethod(cls, @selector(nop), (IMP)dummyIMP, "v@:")),
    TESTCASE(class_addIvar(cls, "x", sizeof(int), sizeof(int), @encode(int))),
    TESTCASE(class_addProtocol(cls, @protocol(P))),
    TESTCASE(class_addProperty(cls, "x", NULL, 0)),
    TESTCASE(class_replaceProperty(cls, "x", NULL, 0)),
    TESTCASE(class_setIvarLayout(cls, NULL)),
    TESTCASE(class_setWeakIvarLayout(cls, NULL)),
    TESTCASE_NOMETA(objc_registerClassPair(cls)),
    TESTCASE_NOMETA(objc_duplicateClass(cls, dupeName(cls), 0)),
    TESTCASE_NOMETA(objc_disposeClassPair(cls)),
};

void parent(char *argv0)
{
    int testCount = sizeof(TestCases) / sizeof(*TestCases);
    for (int i = 0; i < testCount; i++) {
        char *testIndex;
        asprintf(&testIndex, "%d", i);
        char *argvSpawn[] = {
            argv0,
            testIndex,
            NULL
        };
        pid_t pid;
        int result = posix_spawn(&pid, argv0, NULL, NULL, argvSpawn, environ);
        if (result != 0) {
            fprintf(stderr, "Could not spawn child process: (%d) %s\n",
                    errno, strerror(errno));
            exit(1);
        }
        
        free(testIndex);
        
        result = waitpid(pid, NULL, 0);
        if (result == -1) {
            fprintf(stderr, "Error waiting for termination of child process: (%d) %s\n",
                    errno, strerror(errno));
            exit(1);
        }
    }
    fprintf(stderr, "Completed!\n");
}

void child(char *argv1)
{
    long index = strtol(argv1, NULL, 10);
    struct TestCase testCase = TestCases[index];
    TestBlock block = testCase.block;
    
    const char *name = testCase.name;
    if (strncmp(name, "free(", 5) == 0)
        name += 5;
    const char *paren = strchr(name, '(');
    long len = paren != NULL ? paren - name : strlen(name);
    fprintf(stderr, "Testing %.*s\n", (int)len, name);
    
    // Make sure plain classes work.
    block([TestRoot class]);
    block(object_getClass([TestRoot class]));
    
    // And framework classes.
    block([NSObject class]);
    block(object_getClass([NSObject class]));
    
    // Test a constructed, unregistered class.
    Class allocatedClass = objc_allocateClassPair([TestRoot class],
                                                  "AllocatedTestClass",
                                                  0);
    class_getMethodImplementation(allocatedClass, @selector(self));
    block(object_getClass(allocatedClass));
    block(allocatedClass);
    
    // Test a constructed, registered class. (Do this separately so
    // test cases can dispose of the class if needed.)
    allocatedClass = objc_allocateClassPair([TestRoot class],
                                            "AllocatedTestClass2",
                                            0);
    objc_registerClassPair(allocatedClass);
    block(object_getClass(allocatedClass));
    block(allocatedClass);
    
    // Test a duplicated class.
   
    Class duplicatedClass = objc_duplicateClass([TestRoot class],
                                                "DuplicateClass",
                                                0);
    block(object_getClass(duplicatedClass));
    block(duplicatedClass);
    
    fprintf(stderr, "Completed test on good classes.\n");
    
    // Test a fake class.
    Class templateClass = objc_allocateClassPair([TestRoot class],
                                                 "TemplateClass",
                                                 0);
    void *fakeClass = malloc(malloc_size(templateClass));
    memcpy(fakeClass, templateClass, malloc_size(templateClass));
    block((Class)fakeClass);
    fail("Should have died on the fake class");
}

int main(int argc, char **argv)
{
    // We want to run a bunch of tests, all of which end in _objc_fatal
    // (at least if they succeed). Spawn one subprocess per test and
    // have the parent process manage it all. The test will begin by
    // running parent(), which will repeatedly re-spawn this program to
    // call child() with the index of the test to run.
    if (argc == 1) {
        parent(argv[0]);
    } else {
        child(argv[1]);
    }
}
