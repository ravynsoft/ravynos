#include <spawn.h>
#include <err.h>
#include <unistd.h>
#include <zlib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void *zalloc(__unused void *opaque, uInt items, uInt size)
{
	return malloc(items * size);
}

void zfree(__unused void *opaque, void *ptr)
{
	free(ptr);
}

int main(int argc, char *argv[])
{
	const int fixed_args = 4;
	char *args[argc + fixed_args];

	char template[] = "/tmp/img-to-c.XXXXXXXX";
	char *dir = mkdtemp(template);

	if (!dir)
		err(1, "mkdtemp failed");

	char *path;
	asprintf(&path, "%s/img.sparseimage", dir);

	args[0] = "hdiutil";
	args[1] = "create";
	args[2] = path;
	args[3] = "-quiet";

	int i;
	for (i = 1; i < argc; ++i)
		args[i + fixed_args - 1] = argv[i];
	args[i + fixed_args - 1] = NULL;

	pid_t pid;
	posix_spawn(&pid, "/usr/bin/hdiutil", NULL, NULL, args, NULL);

	int status;
	waitpid(pid, &status, 0);

	if (!WIFEXITED(status) || WEXITSTATUS(status))
		err(1, "hdiutil failed");

	int fd = open(path, O_RDONLY);

	z_stream zs = {
		.zalloc = zalloc,
		.zfree = zfree,
	};

	deflateInit(&zs, Z_DEFAULT_COMPRESSION);

	const size_t buf_size = 1024 * 1024;

	unsigned char *out_buf = malloc(buf_size);
	int flush = 0;

	unsigned char *in_buf = malloc(buf_size);

	printf("unsigned char data[] = {");
	int offset = 0;

	zs.next_in = in_buf;

	do {
		if (!flush) {
			ssize_t amt = read(fd, zs.next_in, &in_buf[buf_size] - zs.next_in);

			if (!amt)
				flush = Z_FINISH;

			zs.avail_in += amt;
		}

		zs.next_out = out_buf;
		zs.avail_out = buf_size;

		deflate(&zs, flush);

		memmove(in_buf, zs.next_in, zs.avail_in);
		zs.next_in = in_buf;

		for (unsigned i = 0; i < buf_size - zs.avail_out; ++i) {
			if (!(offset & 15))
				printf("\n  ");
			printf("0x%02x, ", out_buf[i]);
			++offset;
		}
	} while (!flush || zs.avail_in);

	printf("\n};\n");

	// Clean up
	unlink(path);
	rmdir(dir);
}
