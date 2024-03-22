/*
 * Copyright © 2020 Valve Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */
#include "aco_ir.h"

#include <llvm-c/Target.h>

#include "framework.h"
#include <getopt.h>
#include <map>
#include <set>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>

static const char* help_message =
   "Usage: %s [-h] [-l --list] [--no-check] [TEST [TEST ...]]\n"
   "\n"
   "Run ACO unit test(s). If TEST is not provided, all tests are run.\n"
   "\n"
   "positional arguments:\n"
   "  TEST        Run TEST. If TEST ends with a '.', run tests with names\n"
   "              starting with TEST. The test variant (after the '/') can\n"
   "              be omitted to run all variants\n"
   "\n"
   "optional arguments:\n"
   "  -h, --help  Show this help message and exit.\n"
   "  -l --list   List unit tests.\n"
   "  --no-check  Print test output instead of checking it.\n";

std::map<std::string, TestDef> tests;
FILE* output = NULL;

static TestDef current_test;
static unsigned tests_written = 0;
static FILE* checker_stdin = NULL;
static char* checker_stdin_data = NULL;
static size_t checker_stdin_size = 0;

static char* output_data = NULL;
static size_t output_size = 0;
static size_t output_offset = 0;

static char current_variant[64] = {0};
static std::set<std::string>* variant_filter = NULL;

bool test_failed = false;
bool test_skipped = false;
static char fail_message[256] = {0};

void
write_test()
{
   if (!checker_stdin) {
      /* not entirely correct, but shouldn't matter */
      tests_written++;
      return;
   }

   fflush(output);
   if (output_offset == output_size && !test_skipped && !test_failed)
      return;

   char* data = output_data + output_offset;
   uint32_t size = output_size - output_offset;

   fwrite("test", 1, 4, checker_stdin);
   fwrite(current_test.name, 1, strlen(current_test.name) + 1, checker_stdin);
   fwrite(current_variant, 1, strlen(current_variant) + 1, checker_stdin);
   fwrite(current_test.source_file, 1, strlen(current_test.source_file) + 1, checker_stdin);
   if (test_failed || test_skipped) {
      const char* res = test_failed ? "failed" : "skipped";
      fwrite("\x01", 1, 1, checker_stdin);
      fwrite(res, 1, strlen(res) + 1, checker_stdin);
      fwrite(fail_message, 1, strlen(fail_message) + 1, checker_stdin);
   } else {
      fwrite("\x00", 1, 1, checker_stdin);
   }
   fwrite(&size, 4, 1, checker_stdin);
   fwrite(data, 1, size, checker_stdin);

   tests_written++;
   output_offset += size;
}

bool
set_variant(const char* name)
{
   if (variant_filter && !variant_filter->count(name))
      return false;

   write_test();
   test_failed = false;
   test_skipped = false;
   strncpy(current_variant, name, sizeof(current_variant) - 1);

   printf("Running '%s/%s'\n", current_test.name, name);

   return true;
}

void
fail_test(const char* fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   test_failed = true;
   vsnprintf(fail_message, sizeof(fail_message), fmt, args);

   va_end(args);
}

void
skip_test(const char* fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   test_skipped = true;
   vsnprintf(fail_message, sizeof(fail_message), fmt, args);

   va_end(args);
}

void
run_test(TestDef def)
{
   current_test = def;
   output_data = NULL;
   output_size = 0;
   output_offset = 0;
   test_failed = false;
   test_skipped = false;
   memset(current_variant, 0, sizeof(current_variant));

   if (checker_stdin)
      output = open_memstream(&output_data, &output_size);
   else
      output = stdout;

   current_test.func();
   write_test();

   if (checker_stdin)
      fclose(output);
   free(output_data);
}

int
check_output(char** argv)
{
   fflush(stdout);
   fflush(stderr);

   fclose(checker_stdin);

   int stdin_pipe[2];
   pipe(stdin_pipe);

   pid_t child_pid = fork();
   if (child_pid == -1) {
      fprintf(stderr, "%s: fork() failed: %s\n", argv[0], strerror(errno));
      return 99;
   } else if (child_pid != 0) {
      /* Evaluate test output externally using Python */
      dup2(stdin_pipe[0], STDIN_FILENO);
      close(stdin_pipe[0]);
      close(stdin_pipe[1]);

      execlp(ACO_TEST_PYTHON_BIN, ACO_TEST_PYTHON_BIN, ACO_TEST_SOURCE_DIR "/check_output.py",
             NULL);
      fprintf(stderr, "%s: execlp() failed: %s\n", argv[0], strerror(errno));
      return 99;
   } else {
      /* Feed input data to the Python process. Writing large streams to
       * stdin will block eventually, so this is done in a forked process
       * to let the test checker process chunks of data as they arrive */
      write(stdin_pipe[1], checker_stdin_data, checker_stdin_size);
      close(stdin_pipe[0]);
      close(stdin_pipe[1]);
      _exit(0);
   }
}

bool
match_test(std::string name, std::string pattern)
{
   if (name.length() < pattern.length())
      return false;
   if (pattern.back() == '.')
      name.resize(pattern.length());
   return name == pattern;
}

int
main(int argc, char** argv)
{
   int print_help = 0;
   int do_list = 0;
   int do_check = 1;
   const struct option opts[] = {{"help", no_argument, &print_help, 1},
                                 {"list", no_argument, &do_list, 1},
                                 {"no-check", no_argument, &do_check, 0},
                                 {NULL, 0, NULL, 0}};

   int c;
   while ((c = getopt_long(argc, argv, "hl", opts, NULL)) != -1) {
      switch (c) {
      case 'h': print_help = 1; break;
      case 'l': do_list = 1; break;
      case 0: break;
      case '?':
      default: fprintf(stderr, "%s: Invalid argument\n", argv[0]); return 99;
      }
   }

   if (print_help) {
      fprintf(stderr, help_message, argv[0]);
      return 99;
   }

   if (do_list) {
      for (auto test : tests)
         printf("%s\n", test.first.c_str());
      return 99;
   }

   std::vector<std::pair<std::string, std::string>> names;
   for (int i = optind; i < argc; i++) {
      std::string name = argv[i];
      std::string variant;
      size_t pos = name.find('/');
      if (pos != std::string::npos) {
         variant = name.substr(pos + 1);
         name = name.substr(0, pos);
      }
      names.emplace_back(std::pair<std::string, std::string>(name, variant));
   }

   if (do_check)
      checker_stdin = open_memstream(&checker_stdin_data, &checker_stdin_size);

   LLVMInitializeAMDGPUTargetInfo();
   LLVMInitializeAMDGPUTarget();
   LLVMInitializeAMDGPUTargetMC();
   LLVMInitializeAMDGPUDisassembler();

   aco::init();

   for (auto pair : tests) {
      bool found = names.empty();
      bool all_variants = names.empty();
      std::set<std::string> variants;
      for (const std::pair<std::string, std::string>& name : names) {
         if (match_test(pair.first, name.first)) {
            found = true;
            if (name.second.empty())
               all_variants = true;
            else
               variants.insert(name.second);
         }
      }

      if (found) {
         variant_filter = all_variants ? NULL : &variants;
         printf("Running '%s'\n", pair.first.c_str());
         run_test(pair.second);
      }
   }
   if (!tests_written) {
      fprintf(stderr, "%s: No matching tests\n", argv[0]);
      return 99;
   }

   if (checker_stdin) {
      printf("\n");
      return check_output(argv);
   } else {
      printf("Tests ran\n");
      return 99;
   }
}
