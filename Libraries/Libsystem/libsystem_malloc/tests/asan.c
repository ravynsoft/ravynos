#include <TargetConditionals.h>
#include <darwintest.h>
#include <dlfcn.h>
#include <setjmp.h>
#include <stdlib.h>
#include "sanitizer/asan_interface.h"

T_GLOBAL_META(T_META_RUN_CONCURRENTLY(true));

T_DECL(asan_sanity, "ASan Sanity Check", T_META_CHECK_LEAKS(NO))
{
	const char *dylib_path =
		TARGET_OS_OSX    ? "@rpath/libclang_rt.asan_osx_dynamic.dylib" :
		TARGET_OS_IOS    ? "@rpath/libclang_rt.asan_ios_dynamic.dylib" :
		TARGET_OS_TV     ? "@rpath/libclang_rt.asan_tvos_dynamic.dylib" :
		TARGET_OS_WATCH  ? "@rpath/libclang_rt.asan_watchos_dynamic.dylib" :
		TARGET_OS_BRIDGE ? "@rpath/libclang_rt.asan_bridgeos_dynamic.dylib" :
		NULL;

	void *asan_dylib = dlopen(dylib_path, RTLD_NOLOAD);
	T_ASSERT_NOTNULL(asan_dylib, "ASan dylib loaded");

	void *ptr = malloc(16);
	free(ptr);
	
	T_PASS("I didn't crash!");
}

jmp_buf longjmp_env = {0};
bool asan_report_hit = false;
char *asan_report = NULL;

void asan_report_handler(const char *report) {
	asan_report_hit = true;
	asan_report = strdup(report);
	longjmp(longjmp_env, 1);
}

__attribute__((optnone, noinline))
void write_byte(void *ptr, size_t offset) {
	((char *)ptr)[offset] = 'x';
}

T_DECL(asan_use_after_free, "ASan Detects use-after-free", T_META_CHECK_LEAKS(NO))
{
	asan_report = NULL;
	asan_report_hit = false;
	__asan_set_error_report_callback(asan_report_handler);

	if (setjmp(longjmp_env) == 0) {
		char *ptr = malloc(16);
		free(ptr);
		write_byte(ptr, 10);

		T_FAIL("use-after-free not detected");
	}

	T_EXPECT_EQ(asan_report_hit, true, "asan finds use-after-free");
	T_EXPECT_NOTNULL(strstr(asan_report, "AddressSanitizer: heap-use-after-free"), "asan header");
}

T_DECL(asan_heap_buffer_overflow, "ASan Detects heap-buffer-overflow", T_META_CHECK_LEAKS(NO))
{
	asan_report = NULL;
	asan_report_hit = false;
	__asan_set_error_report_callback(asan_report_handler);

	if (setjmp(longjmp_env) == 0) {
		char *ptr = malloc(16);
		write_byte(ptr, 17);
		free(ptr);

		T_FAIL("heap-buffer-overflow not detected");
	}

	T_EXPECT_EQ(asan_report_hit, true, "asan finds heap-buffer-overflow");
	T_EXPECT_NOTNULL(strstr(asan_report, "AddressSanitizer: heap-buffer-overflow"), "asan header");
}
