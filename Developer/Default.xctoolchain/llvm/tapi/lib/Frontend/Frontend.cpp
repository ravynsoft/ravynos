//===- lib/Frontend/Frontend.cpp - TAPI Frontend ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the TAPI Frontend
///
//===----------------------------------------------------------------------===//

#include "tapi/Frontend/Frontend.h"
#include "APIVisitor.h"
#include "tapi/Defines.h"
#include "tapi/Frontend/FrontendContext.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Options.h"
#include "clang/Driver/Tool.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/HeaderMap.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/Path.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/TextAPI/TextAPIError.h"

using namespace llvm;
using namespace clang;

TAPI_NAMESPACE_INTERNAL_BEGIN

static StringRef getLanguageOptions(clang::Language lang) {
  switch (lang) {
  default:
    return "";
  case clang::Language::C:
    return "-xc";
  case clang::Language::CXX:
    return "-xc++";
  case clang::Language::ObjC:
    return "-xobjective-c";
  case clang::Language::ObjCXX:
    return "-xobjective-c++";
  }
}

static StringRef getFileExtension(clang::Language lang) {
  switch (lang) {
  default:
    llvm_unreachable("Unexpected language option.");
  case clang::Language::C:
    return ".c";
  case clang::Language::CXX:
    return ".cpp";
  case clang::Language::ObjC:
    return ".m";
  case clang::Language::ObjCXX:
    return ".mm";
  }
}

static void addHeaderInclude(HeaderFile header, const FrontendJob &job,
                             SmallVectorImpl<char> &includes) {
  raw_svector_ostream os(includes);
  if (header.isPreInclude)
    os << "/* the next header is configured with pre-includes. It might be "
          "include-what-you-use violation. Can it be removed? */\n";

  if (job.language == clang::Language::C ||
      job.language == clang::Language::CXX)
    os << "#include ";
  else
    os << "#import ";

  if (job.useRelativePath && header.useIncludeName()) {
    os << "<" << header.includeName << ">\n";
    return;
  }

  StringRef headerName = header.fullPath;
  // If the header is quoted already, use as it is.
  if ((headerName.startswith("\"") && headerName.endswith("\"")) ||
      (headerName.startswith("<") && headerName.endswith(">"))) {
    os << headerName << "\n";
    return;
  }

  os << "\"" << headerName << "\"\n";
}

static const opt::ArgStringList *
getCC1Arguments(clang::DiagnosticsEngine *diagnostics,
                driver::Compilation *compilation) {
  const auto &jobs = compilation->getJobs();
  if (jobs.size() != 1 || !isa<driver::Command>(*jobs.begin())) {
    SmallString<256> error_msg;
    raw_svector_ostream error_stream(error_msg);
    jobs.Print(error_stream, "; ", true);
    diagnostics->Report(clang::diag::err_fe_expected_compiler_job)
        << error_stream.str();
    return nullptr;
  }

  // The one job we find should be to invoke clang again.
  const auto &cmd = cast<driver::Command>(*jobs.begin());
  if (StringRef(cmd.getCreator().getName()) != "clang") {
    diagnostics->Report(clang::diag::err_fe_expected_clang_command);
    return nullptr;
  }

  return &cmd.getArguments();
}

CompilerInvocation *newInvocation(clang::DiagnosticsEngine *diagnostics,
                                  const opt::ArgStringList &cc1Args) {
  assert(!cc1Args.empty() && "Must at least contain the program name!");
  CompilerInvocation *invocation = new CompilerInvocation;
  CompilerInvocation::CreateFromArgs(*invocation, cc1Args, *diagnostics);
  invocation->getFrontendOpts().DisableFree = false;
  invocation->getCodeGenOpts().DisableFree = false;
  return invocation;
}

static bool runClang(FrontendContext &context, ArrayRef<std::string> options,
                     std::unique_ptr<llvm::MemoryBuffer> input) {
  context.compiler = std::make_unique<CompilerInstance>();
  IntrusiveRefCntPtr<DiagnosticIDs> diagID(new DiagnosticIDs());
  IntrusiveRefCntPtr<DiagnosticOptions> diagOpts(new DiagnosticOptions());
  const llvm::opt::OptTable &opts = driver::getDriverOptTable();

  std::vector<const char *> argv;
  for (const std::string &str : options)
    argv.push_back(str.c_str());
  const char *const binaryName = argv[0];
  unsigned MissingArgIndex, MissingArgCount;
  llvm::opt::InputArgList parsedArgs = opts.ParseArgs(
      ArrayRef<const char *>(argv).slice(1), MissingArgIndex, MissingArgCount);
  ParseDiagnosticArgs(*diagOpts, parsedArgs);
  TextDiagnosticPrinter diagnosticPrinter(llvm::errs(), &*diagOpts);
  clang::DiagnosticsEngine diagnosticsEngine(diagID, &*diagOpts,
                                             &diagnosticPrinter, false);

  const std::unique_ptr<clang::driver::Driver> driver(new clang::driver::Driver(
      binaryName, llvm::sys::getDefaultTargetTriple(), diagnosticsEngine,
      "tapi", &context.fileManager->getVirtualFileSystem()));
  // Since the input might only be virtual, don't check whether it exists.
  driver->setCheckInputsExist(false);
  const std::unique_ptr<clang::driver::Compilation> compilation(
      driver->BuildCompilation(llvm::ArrayRef(argv)));
  if (!compilation)
    return false;
  const llvm::opt::ArgStringList *const cc1Args =
      getCC1Arguments(&diagnosticsEngine, compilation.get());
  if (!cc1Args)
    return false;

  std::unique_ptr<clang::CompilerInvocation> invocation(
      newInvocation(&diagnosticsEngine, *cc1Args));

  // Show the invocation, with -v.
  if (invocation->getHeaderSearchOpts().Verbose) {
    llvm::errs() << "clang Invocation:\n";
    compilation->getJobs().Print(llvm::errs(), "\n", true);
    llvm::errs() << "\n";
  }

  if (input)
    invocation->getPreprocessorOpts().addRemappedFile(
        input->getBufferIdentifier(), input.release());

  // Create a compiler instance to handle the actual work.
  context.compiler->setInvocation(std::move(invocation));
  context.compiler->setFileManager(&*(context.fileManager));
  auto action = std::make_unique<APIVisitorAction>(context);

  // Create the compiler's actual diagnostics engine.
  context.compiler->createDiagnostics();
  if (!context.compiler->hasDiagnostics())
    return false;

  context.compiler->createSourceManager(*(context.fileManager));
  context.verifier->setSourceManager(context.compiler->getSourceManager());

  return context.compiler->ExecuteAction(*action);
}

static std::string getClangExecutablePath() {
  static int staticSymbol;
  static std::string clangExecutablePath;

  if (!clangExecutablePath.empty())
    return clangExecutablePath;

  // Try to find clang first in the toolchain. If that fails, then fall-back to
  // the default search PATH.
  auto mainExecutable = sys::fs::getMainExecutable("tapi", &staticSymbol);
  StringRef toolchainBinDir = sys::path::parent_path(mainExecutable);
  auto clangBinary = sys::findProgramByName("clang", ArrayRef(toolchainBinDir));
  if (clangBinary.getError())
    clangBinary = sys::findProgramByName("clang");
  if (auto ec = clangBinary.getError())
    clangExecutablePath = "clang";
  else
    clangExecutablePath = clangBinary.get();

  return clangExecutablePath;
}


static void populateFilelists(const FrontendJob &job, FrontendContext &context) {
  for (const auto &header : job.headerFiles) {
    if (header.type != job.type || header.isExcluded || header.isPreInclude)
      continue;

    auto file = context.fileManager->getFile(header.fullPath);
    if (!file)
      continue; // File do not exist.

    context.knownFiles.emplace(*file, header.type);

    if (!header.useIncludeName())
      continue;

    context.knownIncludes.emplace(header.includeName, header.type);

    // Construct additional includeName to Workaround for rdar://92350575.
    // When resolved all references of productName can be removed.
    if (job.productName.empty())
      continue;
    auto additionalName =
        (job.productName + "/" + llvm::sys::path::filename(header.fullPath))
            .str();
    context.knownIncludes.emplace(additionalName, header.type);
  }
}

static std::unique_ptr<MemoryBuffer>
createInputBuffer(const FrontendJob &job, FrontendContext &context,
                  StringRef inputFilename) {
  SmallString<4096> headerContents;
  for (const auto &header : job.headerFiles) {
    if (header.isExcluded)
      continue;

    if (header.type != job.type)
      continue;

    if (!job.useUmbrellaHeaderOnly || header.isUmbrellaHeader)
      addHeaderInclude(header, job, headerContents);
  }
  return llvm::MemoryBuffer::getMemBufferCopy(headerContents, inputFilename);
}

static void createClangReproducer(const FrontendJob &job,
                                  const std::vector<std::string> &args,
                                  FrontendContext &context) {
  std::string tempFileTemplate = job.clangReproducerPath.empty()
                                     ? "/tmp/tapi_include_headers-%%%%%%"
                                     : job.clangReproducerPath;
  tempFileTemplate += getFileExtension(job.language).str();
  SmallString<PATH_MAX> tempFile;
  int fd;
  auto ec = sys::fs::createUniqueFile(tempFileTemplate, fd, tempFile);
  if (ec) {
    errs() << "Cannot create temporary file for clang reproducer\n";
    return;
  }
  raw_fd_ostream fs(fd, /*shouldClose=*/ true);
  auto fileContent = createInputBuffer(job, context, "");
  fs << fileContent->getBuffer();
  fs.close();
  std::string tempSrc = tempFile.str().str();
  sys::path::replace_extension(tempFile, "sh");
  raw_fd_ostream sh(tempFile, ec);
  if (ec) {
    errs() << "Cannot create temporary file for clang reproducer\n";
    return;
  }
  SmallString<2048> argStr;
  // The last argument is the input file.
  for (unsigned i = 0; i < args.size() - 1; ++i) {
    argStr.append("\"");
    argStr.append(args[i]);
    argStr.append("\" ");
  }
  argStr.append(tempSrc);
  sh << argStr << "\n";
  SmallString<PATH_MAX> diagPath(tempFile);
  sys::path::replace_extension(
      diagPath,
      "{" + getFileExtension(job.language).drop_front(1).str() + ",sh}");
  errs() << "\nNote: a reproducer of the error is written to: \"" << diagPath
         << "\".\n";
  errs() << "Note: the reproducer is intended to help users to debug the issue "
            "under a more familiar context using clang.\n";
  errs() << "Note: the paths in the reproducer might need to be adjusted.\n";
}

extern Expected<FrontendContext> runFrontend(const FrontendJob &job,
                                             StringRef inputFilename) {
  FrontendContext context(job.target, job.verifier.get(), job.vfs, job.type);
  std::unique_ptr<MemoryBuffer> input;
  std::string inputFilePath;
  if (inputFilename.empty()) {
    inputFilePath =
        ("tapi_include_headers" + getFileExtension(job.language)).str();
    input = createInputBuffer(job, context, inputFilePath);
  } else {
    auto file = context.fileManager->getFile(inputFilename);
    assert(file && "file do not exist");
    context.knownFiles.emplace(*file, HeaderType::Public);
    inputFilePath = inputFilename.str();
  }
  populateFilelists(job, context);

  // No more work to do if there are no files to parse.
  if (context.knownFiles.empty())
    return make_error<TextAPIError>(TextAPIErrorCode::EmptyResults);

  if (job.verbose && input)
    outs() << job.label << " Headers:\n" << input->getBuffer() << "\n";

  std::string clangExecutablePath;
  if (job.clangExecutablePath)
    clangExecutablePath = job.clangExecutablePath.value();
  else
    clangExecutablePath = getClangExecutablePath();

  std::vector<std::string> args;
  args.emplace_back(clangExecutablePath);
  args.emplace_back("-fsyntax-only");
  args.emplace_back(getLanguageOptions(job.language));
  args.emplace_back("-target");
  args.emplace_back(job.target.str());

  if (!job.clangResourcePath.empty()) {
    args.emplace_back("-resource-dir");
    args.emplace_back(job.clangResourcePath);
  }

  if (!job.language_std.empty())
    args.emplace_back("-std=" + job.language_std);

  if (job.overwriteRTTI)
    args.emplace_back("-frtti");

  if (job.overwriteNoRTTI)
    args.emplace_back("-fno-rtti");

  if (!job.visibility.empty())
    args.emplace_back("-fvisibility=" + job.visibility);

  if (job.enableModules)
    args.emplace_back("-fmodules");

  if (!job.moduleCachePath.empty())
    args.emplace_back("-fmodules-cache-path=" + job.moduleCachePath);

  if (job.validateSystemHeaders)
    args.emplace_back("-fmodules-validate-system-headers");

  if (job.useObjectiveCARC)
    args.emplace_back("-fobjc-arc");

  if (job.useObjectiveCWeakARC)
    args.emplace_back("-fobjc-weak");

  if (job.verbose)
    args.emplace_back("-v");

  // Add a default macro for TAPI.
  args.emplace_back("-D__clang_tapi__=1");

  // FIXME: Disable implicit definitions for TARGET_OS* rdar://121206188
  args.emplace_back("-fno-define-target-os-macros");

  for (auto &macro : job.macros) {
    if (macro.second)
      args.emplace_back("-U" + macro.first);
    else
      args.emplace_back("-D" + macro.first);
  }

  if (!job.isysroot.empty())
    args.emplace_back("-isysroot" + job.isysroot);

  if (job.useRelativePath) {
    // Add the header search paths.
    transform(job.includePaths, std::back_inserter(args),
              [](const std::string &path) { return "-I" + path; });

    // Add quoted header search paths.
    transform(job.quotedIncludePaths, std::back_inserter(args),
              [](const std::string &path) { return "-iquote" + path; });
  }

  // Add SYSTEM framework search paths.
  for (const auto &path : job.systemFrameworkPaths)
    args.emplace_back("-iframework" + path);

  // Add SYSTEM header search paths.
  for (const auto &path : job.systemIncludePaths)
    args.emplace_back("-isystem" + path);

  // Add AFTER header search paths.
  for (const auto &path : job.afterIncludePaths)
    args.emplace_back("-idirafter" + path);

  // Add the framework search paths.
  for (const auto &path : job.frameworkPaths)
    args.emplace_back("-F" + path);

  // Support old ordering of include paths when tapi was not
  // given necessary paths into SRCROOT.
  if (!job.useRelativePath) {
    for (const auto &path : job.includePaths) {
      // Only add header maps for project headers.
      if (job.type == HeaderType::Project) {
        args.emplace_back("-I" + path);
        continue;
      }

      if (auto file = context.fileManager->getFile(path))
        if (HeaderMap::Create(*file, *context.fileManager))
          continue;

      args.emplace_back("-I" + path);
    }
  }

  // For c++ and objective-c++, add default stdlib to be libc++.
  if (job.language == clang::Language::CXX ||
      job.language == clang::Language::ObjCXX)
    args.emplace_back("-stdlib=libc++");

  // Add extra clang arguments.
  for (const auto &arg : job.clangExtraArgs)
    args.emplace_back(arg);

  // Add prefix headers.
  for (const auto &header : job.prefixHeaders)
    args.emplace_back("-include" + header);

  args.emplace_back(inputFilePath);
  if (runClang(context, args, std::move(input)))
    return context;

  // Create a reproducer.
  if (inputFilename.empty() && job.createClangReproducer)
    createClangReproducer(job, args, context);

  return make_error<TextAPIError>(TextAPIErrorCode::GenericFrontendError);
}

bool canIgnoreFrontendError(llvm::Error &error) {
  // Currently, the only non-fatal error occurs when a frontend task has no
  // input to parse.
  bool canIgnore = false;
  handleAllErrors(std::move(error),
                  [&canIgnore](std::unique_ptr<TextAPIError> tapiError) {
                    canIgnore = tapiError->EC == TextAPIErrorCode::EmptyResults;
                  });
  return canIgnore;
}

TAPI_NAMESPACE_INTERNAL_END
