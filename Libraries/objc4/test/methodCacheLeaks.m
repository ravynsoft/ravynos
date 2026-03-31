// TEST_CONFIG MEM=mrc LANGUAGE=objective-c
/*
TEST_RUN_OUTPUT
[\S\s]*0 leaks for 0 total leaked bytes[\S\s]*
END
*/

#include "test.h"
#include "testroot.i"

#include <spawn.h>
#include <stdio.h>

void noopIMP(id self __unused, SEL _cmd __unused) {}

id test(int n, int methodCount) {
    char *name;
    asprintf(&name, "TestClass%d", n);
    Class c = objc_allocateClassPair([TestRoot class], name, 0);
    free(name);
    
    SEL *sels = malloc(methodCount * sizeof(*sels));
    for(int i = 0; i < methodCount; i++) {
        asprintf(&name, "selector%d", i);
        sels[i] = sel_getUid(name);
        free(name);
    }
    
    for(int i = 0; i < methodCount; i++) {
        class_addMethod(c, sels[i], (IMP)noopIMP, "v@:");
    }
    
    objc_registerClassPair(c);
    
    id obj = [[c alloc] init];
    for (int i = 0; i < methodCount; i++) {
        ((void (*)(id, SEL))objc_msgSend)(obj, sels[i]);
    }
    free(sels);
    return obj;
}

int main()
{
    int classCount = 16;
    id *objs = malloc(classCount * sizeof(*objs));
    for (int i = 0; i < classCount; i++) {
        objs[i] = test(i, 1 << i);
    }
    
    char *pidstr;
    int result = asprintf(&pidstr, "%u", getpid());
    testassert(result);
    
    extern char **environ;
    char *argv[] = { "/usr/bin/leaks", pidstr, NULL };
    pid_t pid;
    result = posix_spawn(&pid, "/usr/bin/leaks", NULL, NULL, argv, environ);
    if (result) {
        perror("posix_spawn");
        exit(1);
    }
    wait4(pid, NULL, 0, NULL);
    printf("objs=%p\n", objs);
}
