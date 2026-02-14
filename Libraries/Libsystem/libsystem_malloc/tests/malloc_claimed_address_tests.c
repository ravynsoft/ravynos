//
//  malloc_claimed_address_tests.c
//  libmalloc
//
//  Tests for malloc_claimed_address() and malloc_zone_claimed_address().
//

#include <darwintest.h>
#include <stdlib.h>
#include <stdio.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <malloc/malloc.h>
#include <malloc_private.h>
#include <sys/mman.h>

T_GLOBAL_META(T_META_RUN_CONCURRENTLY(true));

T_DECL(malloc_claimed_address_default_zone_test,
		"Tests for malloc_claimed_address, default zone only",
		T_META_ENVVAR("MallocNanoZone=0"))
{
	// NULL is never a possible pointer.
	boolean_t result = malloc_claimed_address(NULL);
	T_EXPECT_FALSE(result, "NULL is never a valid pointer");

	// Allocate from tiny, check that it's claimed.
	void *ptr = malloc(16);
	result = malloc_claimed_address(ptr);
	T_EXPECT_TRUE(result, "allocation from tiny");

	// Offset from the pointer, check that it's still claimed.
	result = malloc_claimed_address(ptr + 8);
	T_EXPECT_TRUE(result, "allocation from tiny with offset");
	free(ptr);

	// Allocate from small, check that it's claimed.
	ptr =  malloc(2048);
	result = malloc_claimed_address(ptr);
	T_EXPECT_TRUE(result, "allocation from small");

	// Offset from the pointer, check that it's still claimed.
	result = malloc_claimed_address(ptr + 1000);
	T_EXPECT_TRUE(result, "allocation from small with offset");
	free(ptr);

	// Allocate from large, check that it's claimed.
	ptr =  malloc(140000);
	result = malloc_claimed_address(ptr);
	T_EXPECT_TRUE(result, "allocation from large");

	// Offset from the pointer, check that it's still claimed.
	result = malloc_claimed_address(ptr + 1000);
	T_EXPECT_TRUE(result, "allocation from large with offset");
	free(ptr);

	// Allocate some memory with vm_allocate() and make sure it's not claimed.
	mach_vm_address_t addr;
	kern_return_t kr = mach_vm_allocate(mach_task_self(), &addr, 1024, VM_FLAGS_ANYWHERE);
	T_ASSERT_TRUE(kr == KERN_SUCCESS, "allocate vm space");
	result = malloc_claimed_address((void *)addr);
	T_EXPECT_FALSE(result, "address in VM allocated memory");
	mach_vm_deallocate(mach_task_self(), addr, 1024);
}


T_DECL(malloc_zone_claimed_address_test,
		"Tests for malloc_zone_claimed_address",
		T_META_ENVVAR("MallocNanoZone=0"))
{
	malloc_zone_t *zone = malloc_create_zone(0, 0);

	// NULL is never a possible pointer.
	boolean_t result = malloc_zone_claimed_address(zone, NULL);
	T_EXPECT_FALSE(result, "NULL is never a valid pointer");

	// Allocate from tiny, check that it's claimed.
	void *ptr = malloc_zone_malloc(zone, 16);
	result = malloc_zone_claimed_address(zone, ptr);
	T_EXPECT_TRUE(result, "allocation from tiny");

	// Offset from the pointer, check that it's still claimed.
	result = malloc_zone_claimed_address(zone, ptr + 8);
	T_EXPECT_TRUE(result, "allocation from tiny with offset");
	free(ptr);

	// Allocate with tiny from the default zone, check that it's not claimed.
	ptr = malloc(16);
	result = malloc_zone_claimed_address(zone, ptr);
	T_EXPECT_FALSE(result, "allocation from tiny in default zone");
	result = malloc_zone_claimed_address(zone, ptr + 8);
	T_EXPECT_FALSE(result, "allocation from tiny in default zone with offset");
	free(ptr);

	// Allocate from small, check that it's claimed.
	ptr =  malloc_zone_malloc(zone, 2048);
	result = malloc_zone_claimed_address(zone, ptr);
	T_EXPECT_TRUE(result, "allocation from small");

	// Offset from the pointer, check that it's still claimed.
	result = malloc_zone_claimed_address(zone, ptr + 1000);
	T_EXPECT_TRUE(result, "allocation from small with offset");
	free(ptr);

	// Allocate with small from the default zone, check that it's not claimed.
	ptr = malloc(2048);
	result = malloc_zone_claimed_address(zone, ptr);
	T_EXPECT_FALSE(result, "allocation from small in default zone");
	result = malloc_zone_claimed_address(zone, ptr + 8);
	T_EXPECT_FALSE(result, "allocation from small in default zone with offset");
	free(ptr);

	// Allocate from large, check that it's claimed.
	ptr =  malloc_zone_malloc(zone, 140000);
	result = malloc_zone_claimed_address(zone, ptr);
	T_EXPECT_TRUE(result, "allocation from large");

	// Offset from the pointer, check that it's still claimed.
	result = malloc_zone_claimed_address(zone, ptr + 1000);
	T_EXPECT_TRUE(result, "allocation from large with offset");
	free(ptr);

	// Allocate with large from the default zone, check that it's not claimed.
	ptr = malloc(140000);
	result = malloc_zone_claimed_address(zone, ptr);
	T_EXPECT_FALSE(result, "allocation from large in default zone");
	result = malloc_zone_claimed_address(zone, ptr + 8);
	T_EXPECT_FALSE(result, "allocation from large in default zone with offset");
	free(ptr);

	// Allocate some memory with vm_allocate() and make sure it's not claimed.
	mach_vm_address_t addr;
	kern_return_t kr = mach_vm_allocate(mach_task_self(), &addr, 1024, VM_FLAGS_ANYWHERE);
	T_ASSERT_TRUE(kr == KERN_SUCCESS, "allocate vm space");
	result = malloc_zone_claimed_address(zone, (void *)addr);
	T_EXPECT_FALSE(result, "address in VM allocated memory");
	mach_vm_deallocate(mach_task_self(), addr, 1024);

	malloc_destroy_zone(zone);
}

T_DECL(malloc_claimed_address_zone_test,
		"Tests for malloc_claimed_address with another zone",
		T_META_ENVVAR("MallocNanoZone=0"))
{
	// Allocate in a custom zone, check that we can still use
	// malloc_claimed_address() to check whether an address is claimed.
	malloc_zone_t *zone = malloc_create_zone(0, 0);

	// Allocate from tiny, check that it's claimed.
	void *ptr = malloc_zone_malloc(zone, 16);
	boolean_t result = malloc_claimed_address(ptr);
	T_EXPECT_TRUE(result, "allocation from tiny");

	// Offset from the pointer, check that it's still claimed.
	result = malloc_claimed_address(ptr + 8);
	T_EXPECT_TRUE(result, "allocation from tiny with offset");
	free(ptr);

	// Allocate from small, check that it's claimed.
	ptr =  malloc_zone_malloc(zone, 2048);
	result = malloc_claimed_address(ptr);
	T_EXPECT_TRUE(result, "allocation from small");

	// Offset from the pointer, check that it's still claimed.
	result = malloc_claimed_address(ptr + 1000);
	T_EXPECT_TRUE(result, "allocation from small with offset");
	free(ptr);

	// Allocate from large, check that it's claimed.
	ptr =  malloc_zone_malloc(zone, 140000);
	result = malloc_claimed_address(ptr);
	T_EXPECT_TRUE(result, "allocation from large");

	// Offset from the pointer, check that it's still claimed.
	result = malloc_claimed_address(ptr + 1000);
	T_EXPECT_TRUE(result, "allocation from large with offset");
	free(ptr);

	malloc_destroy_zone(zone);
}

T_DECL(malloc_claimed_address_nanozone_test,
		"Tests for malloc_claimed_address with nano",
		T_META_ENVVAR("MallocNanoZone=1"))
{
	// NULL is never a possible pointer.
	boolean_t result = malloc_claimed_address(NULL);
	T_EXPECT_FALSE(result, "NULL is never a valid pointer");

	// Allocate various sizes, check that they are claimed and that offset
	// pointers are also claimed.
	for (size_t sz = 16; sz <= 256; sz += 16) {
		void *ptr = malloc(sz);
		result = malloc_claimed_address(ptr);
		T_EXPECT_TRUE(result, "nano allocation size %d", (int)sz);
		result = malloc_claimed_address(ptr + sz/2);
		T_EXPECT_TRUE(result, "nano allocation size %d offset %d", (int)sz, (int)sz/2);
		free(ptr);
	}

	// Allocate a non-Nano size, which Nano will pass to its helper zone.
	// Verify that it still claims the address as valid when asked via the
	// default zone.
	void *ptr = malloc(512);
	result = malloc_claimed_address(ptr);
	T_EXPECT_TRUE(result, "Above nano pointer check");
	result = malloc_zone_claimed_address(malloc_default_zone(), ptr);
	T_EXPECT_TRUE(result, "Above nano pointer check via default zone");
	free(ptr);

	// Allocate some memory with vm_allocate() and make sure it's not claimed.
	mach_vm_address_t addr;
	kern_return_t kr = mach_vm_allocate(mach_task_self(), &addr, 1024, VM_FLAGS_ANYWHERE);
	T_ASSERT_TRUE(kr == KERN_SUCCESS, "allocate vm space");
	result = malloc_claimed_address((void *)addr);
	T_EXPECT_FALSE(result, "address in VM allocated memory");
	mach_vm_deallocate(mach_task_self(), addr, 1024);
}


T_DECL(malloc_claimed_address_custom_zone_test,
		"Tests for malloc_claimed_address in a zone that does not implement it",
		T_META_ENVVAR("MallocNanoZone=0"))
{
	// Custom zones that do not support claimed_address must always appear
	// to return true.
	malloc_zone_t *zone = malloc_create_zone(0, 0);
	mprotect(zone, sizeof(*zone), PROT_READ | PROT_WRITE);
	zone->version = 9;
	zone->claimed_address = NULL;
	mprotect(zone, sizeof(*zone), PROT_READ);

	// NULL must still be disclaimed.
	boolean_t result = malloc_zone_claimed_address(zone, NULL);
	T_EXPECT_FALSE(result, "NULL is never a valid pointer");

	// Allocate from tiny, check that it's claimed.
	void *ptr = malloc_zone_malloc(zone, 16);
	result = malloc_claimed_address(ptr);
	T_EXPECT_TRUE(result, "allocation from tiny");

	// Offset from the pointer, check that it's still claimed.
	result = malloc_claimed_address(ptr + 8);
	T_EXPECT_TRUE(result, "allocation from tiny with offset");
	free(ptr);

	// Allocate from small, check that it's claimed.
	ptr =  malloc_zone_malloc(zone, 2048);
	result = malloc_claimed_address(ptr);
	T_EXPECT_TRUE(result, "allocation from small");

	// Offset from the pointer, check that it's still claimed.
	result = malloc_claimed_address(ptr + 1000);
	T_EXPECT_TRUE(result, "allocation from small with offset");
	free(ptr);

	// Allocate from large, check that it's claimed.
	ptr =  malloc_zone_malloc(zone, 140000);
	result = malloc_claimed_address(ptr);
	T_EXPECT_TRUE(result, "allocation from large");

	// Offset from the pointer, check that it's still claimed.
	result = malloc_claimed_address(ptr + 1000);
	T_EXPECT_TRUE(result, "allocation from large with offset");
	free(ptr);

	malloc_destroy_zone(zone);
}

