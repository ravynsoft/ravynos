#include <mach/mach_types.h>
#include <sys/mman.h>
#include <string.h>

#include <darwintest.h>

static const char* qbf = "The quick brown fox jumps over the lazy dog";
static const char* lynx = "Lynx c.q. vos prikt bh: dag zwemjuf!";

T_DECL(strlcat, "strlcat(3)")
{

	void *ptr = mmap(NULL, PAGE_SIZE*2, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
	T_ASSERT_NE(ptr, MAP_FAILED, NULL);

	T_ASSERT_POSIX_ZERO(mprotect(ptr+PAGE_SIZE, PAGE_SIZE, PROT_READ), NULL);

	size_t offset = strlen(qbf)+strlen(lynx)+1;
	char *dst = (ptr+PAGE_SIZE)-offset;
	strcpy(dst, qbf);

	size_t res = strlcat(dst, lynx, offset);
	T_ASSERT_EQ(res, offset-1, "strlcat");
	T_ASSERT_EQ(memcmp(dst, qbf, strlen(qbf)), 0, NULL);
	T_ASSERT_EQ(memcmp(dst+strlen(qbf), lynx, strlen(lynx)), 0, NULL);
	T_ASSERT_EQ(dst[offset], 0, "null-term");

	memset(ptr, '\0', PAGE_SIZE);

	offset = strlen(qbf)+(strlen(lynx)/2)+1;
	dst = (ptr+PAGE_SIZE)-offset;
	strcpy(dst, qbf);

	res = strlcat(dst, lynx, offset);
	T_ASSERT_EQ(res, strlen(qbf)+strlen(lynx), "strlcat");
	T_ASSERT_EQ(memcmp(dst, qbf, strlen(qbf)), 0, NULL);
	T_ASSERT_EQ(memcmp(dst+strlen(qbf), lynx, offset-strlen(qbf)-1), 0, NULL);
	T_ASSERT_EQ(*(char*)(ptr+PAGE_SIZE), 0, NULL);
	T_ASSERT_EQ(dst[offset], 0, "null-term");

	memset(ptr, '\0', PAGE_SIZE);

	offset = strlen(qbf)-4;
	dst = (ptr+PAGE_SIZE)-offset;
	strncpy(dst, qbf, offset);

	res = strlcat(dst, lynx, offset);
	T_ASSERT_EQ(res, offset+strlen(lynx), "strlcat");
	T_ASSERT_EQ(memcmp(dst, qbf, offset), 0, NULL);
	T_ASSERT_EQ(*(char*)(ptr+PAGE_SIZE), 0, NULL);
	T_ASSERT_EQ(dst[offset], 0, "null-term");
}

T_DECL(strlcat_overlap, "strlcat(3) with overlap: PR-20105548")
{
	char buffer[21];
	memset(buffer,'x',sizeof(buffer));
	buffer[0]='\0';
	buffer[20]='\0';

	char *a = &buffer[0];
	char *b = &buffer[10];
	strlcat(a,b,10);
	T_PASS("did not abort");
}
