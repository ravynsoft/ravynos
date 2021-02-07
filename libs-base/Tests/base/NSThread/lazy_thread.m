#import "ObjectTesting.h"
#import <Foundation/NSThread.h>
#include <pthread.h>

void *thread(void *ignored)
{
  return [NSThread currentThread];
}

int main(void)
{
  pthread_t thr;
  void *ret;

  pthread_create(&thr, NULL, thread, NULL);
  pthread_join(thr, &ret);
  PASS(ret != 0, "NSThread lazily created from POSIX thread");
  testHopeful = YES;
  PASS((ret != 0) && (ret != [NSThread mainThread]),
    "Spawned thread is not main thread");
  pthread_create(&thr, NULL, thread, NULL);
  pthread_join(thr, &ret);
  PASS(ret != 0, "NSThread lazily created from POSIX thread");
  PASS((ret != 0) && (ret != [NSThread mainThread]),
    "Spawned thread is not main thread");

  NSThread *t = [NSThread currentThread];
  [t setName: @"xxxtestxxx"];
  NSLog(@"Thread description is '%@'", t);
  NSRange r = [[t description] rangeOfString: @"name = xxxtestxxx"];
  PASS(r.length > 0, "thread description contains name");

  return 0;
}

