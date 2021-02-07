#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSString.h>
#include <Foundation/NSUUID.h>
#include "Testing.h"
#include "ObjectTesting.h"

int main(int argc, char **argv)
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];
  NSUUID *uuid1, *uuid2;
  NSString *uuidString = @"2139CD2E-15E6-4C37-84DA-1E18EEBFAB4A";

  unsigned char uuidBytes[16] = {
    0x80, 0x1f, 0x3a, 0x01, 0x95, 0x7c, 0x45, 0x0f,
    0xaf, 0xf2, 0x1b, 0xe9, 0x59, 0xf5, 0x89, 0x54 };

  TEST_FOR_CLASS(@"NSUUID", [NSUUID alloc],
		 "+[NSUUID alloc] returns a NSUUID");
  TEST_FOR_CLASS(@"NSUUID", [NSUUID UUID],
		 "+[NSUUID UUID] returns a UUID");

  uuid1 = [[NSUUID alloc] initWithUUIDString: nil];
  PASS(uuid1 == nil, "Don't create a UUID from a nil string");

  uuid1 = [[NSUUID alloc] initWithUUIDString: @"test"];
  PASS(uuid1 == nil, "Don't create a UUID from an invalid string");

  uuid1 = [[NSUUID alloc] initWithUUIDString: uuidString];
  PASS(uuid1 != nil, "Create a UUID from a valid string");
  PASS_EQUAL([uuid1 UUIDString], uuidString,
	     "Derive a stable UUID string value");

  uuid2 = [[NSUUID alloc] initWithUUIDString: uuidString];
  PASS_EQUAL(uuid1, uuid2, "UUIDs representing the same value are considered equal");
  PASS([uuid1 hash] == [uuid2 hash], "Equal objects have equal hashes");

  DESTROY(uuid2);
  uuid2 = [[NSUUID alloc] initWithUUIDBytes: uuidBytes];
  PASS(![uuid1 isEqual: uuid2], "UUIDs representing different values should not be considered equal");

  uuid_t otherBytes = {0};
  [uuid2 getUUIDBytes: otherBytes];

  int comparison = memcmp(uuidBytes, otherBytes, 16);
  PASS(comparison == 0, "Get a stable value for the UUID bytes");
  DESTROY(uuid2);

  uuid2 = [uuid1 copy];
  PASS_EQUAL(uuid1, uuid2, "-[NSUUID copy] returns an identical object");
  DESTROY(uuid2);
  
  NSData *coded = [NSKeyedArchiver archivedDataWithRootObject: uuid1];
  uuid2 = [NSKeyedUnarchiver unarchiveObjectWithData: coded];
  PASS_EQUAL(uuid1, uuid2, "UUID survives a round-trip through archiver");
  DESTROY(uuid1);

  [arp release];
  arp = nil;
  return 0;
}
