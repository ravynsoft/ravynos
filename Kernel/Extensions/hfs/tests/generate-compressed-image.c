//
// Copyright (c) 2019-2019 Apple Inc. All rights reserved.
//
// generate-compressed-image.c - Generates compressed disk images for
//                               file-system type passed as argument.
//

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <spawn.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sysexits.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <zlib.h>

//
// Rationale: We don't have `hdiutil` in iOS runtime and thus cannot generate
// disk-images for test inside iOS. We assume that the build enviornment has
// `hdiutil` and thus generate custom file-system images (passed as argument)
// during build, compress it using deflate and finally use the compressed byte
// stream to generate files which can then be be used from iOS runtime.
//

//
// Name template for the temporary directory where we create the file-system
// image before we can compress it.
//
#define GEN_COM_IMAGE_TMP_DIR "/tmp/generate_compressed_image.XXXXXXXX"

//
// Name of the temporary file-system image.
//
#define GEN_COM_IMAGE_TMP_FILENAME "img.sparseimage"

//
// Path to hdiutil utility inside build enviornment.
//
#define GEN_COM_IMAGE_HDIUTIL_PATH "/usr/bin/hdiutil"

enum {

	//
	// Count of fixed number of argumets needed to call hdiutil, additional
	// options are passed by the caller.
	//
	FIXED_NR_ARGUMENTS = 4,
	ADDITIONAL_NR_ARGUMENTS = 10,
	BYTES_PER_LINE_MASK = 0xF,
};

//
// Valid file-system types that is supported by this program.
//
static bool
is_fstype_valid(const char *fs_type)
{
	int idx;

	const char *const VALID_FSTYPES[] = {
		"JHFS+",
		"APFS",
		"EXFAT",
		"FAT32",

		NULL
	};

	for (idx = 0; VALID_FSTYPES[idx] != NULL; idx++) {
		if (strcmp(fs_type, VALID_FSTYPES[idx]) == 0) {
			return true;
		}
	}
	return false;
}

int
main(int argc, char *argv[])
{
	pid_t pid, child_state_changed;
	z_stream c_stream;
	int fd, idx, ret, status, flush, offset;
	char *tmp_dir, *tmp_disk_image_path, *fs_type;
	unsigned char *compressed_out_buf, *uncompressed_in_buf;
	const size_t chunk_size = (1ULL << 20);
	char *args[argc + FIXED_NR_ARGUMENTS];
	char tmp_dir_name_template[] = GEN_COM_IMAGE_TMP_DIR;
	const char *progname = (progname = strrchr(argv[0], '/')) ?
			progname+=1 : (progname = argv[0]);

	//
	// Disable stdout buffering.
	//
	setvbuf(stdout, NULL, _IONBF, 0);

	//
	// Validate that we have correct number of arguments passed (minimal,
	// this is only called from inside build internally).
	//
	if (argc != (ADDITIONAL_NR_ARGUMENTS + 1)) {

err_usage:
		fprintf(stderr, "Usage: %s -size [size-arg] -type "
			"[type-arg] -fs [APFS|JHFS+|EXFAT|FAT32] "
			"-uid [uid-arg] -gid [gid-arg]\n", progname);
		return EXIT_FAILURE;
	}

	//
	// Just to simplify this program and to avoid parsing input aruments
	// we assume that the arguments are passed in order and 7th argument
	// has the file-system type. We confirm this now.
	//
	if (!is_fstype_valid(argv[6])) {
		fprintf(stderr, "Unknown file-system type %s\n", argv[6]);
		goto err_usage;
	}
	fs_type = argv[6];
	if (!strcmp(argv[6], "JHFS+")) {
		fs_type = "JHFS";
	}

	//
	// First we create a temporary directory to host our newly created
	// disk image in the build environment.
	//
	tmp_dir = mkdtemp(tmp_dir_name_template);
	if (!tmp_dir)
		err(EX_NOINPUT, "mkdtemp failed");

	//
	// Path where we want to keep our temporary disk image.
	//
	asprintf(&tmp_disk_image_path, "%s/"GEN_COM_IMAGE_TMP_FILENAME,
			tmp_dir);

	//
	// Set up the fixed command line parameters to be passed to the hdiutil
	// child process.
	//
	//	- program name.
	//	- create a new disk-image.
	//	- path of disk-image file to create.
	//	- silent mode.
	//
	args[0] = "hdiutil";
	args[1] = "create";
	args[2] = tmp_disk_image_path;
	args[3] = "-quiet";

	//
	// Copy the additional arguments passed by the caller needed for
	// hdiutil.
	//
	for (idx = 1; idx < argc; ++idx) {
		args[idx + FIXED_NR_ARGUMENTS - 1] = argv[idx];
	}
	args[idx + FIXED_NR_ARGUMENTS - 1] = NULL;

	//
	// Spawn the hdiutil as a child process and wait for its completion.
	//
	ret = posix_spawn(&pid, GEN_COM_IMAGE_HDIUTIL_PATH, NULL, NULL,
			args, NULL);
	if (ret) {
		errno = ret;
		err(EX_OSERR, "posix_spawn failed");
	}

	//
	// Wait for the child process to finish.
	//
	do {
		errno = 0;
		child_state_changed = waitpid(pid, &status, 0);
	} while (child_state_changed == -1 && errno == EINTR);

	if (child_state_changed == -1) {
		err(EX_OSERR, "waitpid failed");
	}
	if (!WIFEXITED(status) || WEXITSTATUS(status)) {
		fprintf(stderr, "hdiutil failed, status %d", status);
		exit(EXIT_FAILURE);
	}

	//
	// We have successfully create the disk image, now we have to
	// open this disk image, read and finally write out a compess
	// stream of this disk image to a data file.
	//
	fd = open(tmp_disk_image_path, O_RDONLY);
	if (fd == -1) {
		err(EX_NOINPUT, "open failed for file %s",
				tmp_disk_image_path);
	}

	//
	// Initialize the compressed stream of bytes we will write out.
	//
	c_stream = (z_stream) {
		.zalloc = Z_NULL,
		.zfree = Z_NULL,
		.opaque = Z_NULL,
	};

	ret = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
	if (ret != Z_OK) {
		err(EX_SOFTWARE, "deflateInit faile, ret %d", ret);
	}

	compressed_out_buf = malloc(chunk_size);
	if (!compressed_out_buf) {
		err(EX_OSERR, "malloc faliled for compressed_out_buf");
	}

	uncompressed_in_buf = malloc(chunk_size);
	if (!uncompressed_in_buf) {
		err(EX_OSERR, "malloc faliled for uncompressed_in_buf");
	}

	fprintf(stdout, "unsigned char %s_data[] = {", fs_type);

	offset = 0;
	flush = Z_NO_FLUSH;
	do {
		ssize_t bytes_read;

		bytes_read = read(fd, uncompressed_in_buf, chunk_size);
		if (bytes_read == -1) {
			(void)deflateEnd(&c_stream);
			err(EX_OSERR, "read failed for file %s\n",
					tmp_disk_image_path);
		}

		//
		// Set the stream's input buffer and number of input bytes
		// available to be compressed.
		//
		c_stream.next_in = uncompressed_in_buf;
		c_stream.avail_in = bytes_read;

		//
		// If we have reached the end of the file, we have to flush the
		// compressed stream.
		//
		if (!bytes_read) {
			flush = Z_FINISH;
		}

		//
		// Run deflate() on input until output buffer is not full,
		// finish compression if all of source has been read in.
		//
		do {
			unsigned written;

			c_stream.avail_out = chunk_size;
			c_stream.next_out = compressed_out_buf;

			ret = deflate(&c_stream, flush);
			assert(ret != Z_STREAM_ERROR);

			written = chunk_size - c_stream.avail_out;
			for (idx = 0; idx < written; ++idx) {
				if (!(offset & BYTES_PER_LINE_MASK))
					fprintf(stdout, "\n  ");
				fprintf(stdout, "0x%02x, ",
						compressed_out_buf[idx]);
				++offset;
			}

		} while (c_stream.avail_out == 0);

		//
		// All  input should be used.
		//
		assert(c_stream.avail_in == 0);

	} while (flush != Z_FINISH);

	//
	// Stream will be complete.
	//
	assert(ret == Z_STREAM_END);
	(void)close(fd);

	//
	// stdout is line buffered by default and this should flush it.
	//
	fprintf(stdout, "\n};\n");

	//
	// Clean up
	//
	(void)deflateEnd(&c_stream);
	unlink(tmp_disk_image_path);
	free(tmp_disk_image_path);
	rmdir(tmp_dir);
}
