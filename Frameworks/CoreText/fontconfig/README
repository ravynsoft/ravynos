                              Fontconfig
             Font configuration and customization library
                            Version 2.14.2
                              2023-01-27


Check INSTALL for compilation and installation instructions.
Report bugs to https://gitlab.freedesktop.org/fontconfig/fontconfig/issues/new.

2.14.2

Akira TAGOH (13):
      Fix the build issue on meson when -g option is added to c_args
      Store artifacts for meson windows CI
      Add FC_DESKTOP_NAME property
      Add --with-default-sub-pixel-rendering option
      Update po-conf/POTFILES.in
      Ignore null pointer on Fc*Destroy functions
      Convert tabs to spaces
      Convert more tabs to spaces in docs
      src/meson.build: Store correct paths to fontconfig.pc.
      Fix a typo in description for HAVE_STDATOMIC_PRIMITIVES
      Report more detailed logs instead of assertion.
      Add some missing constant names for weight.
      Adujst indentation between programlisting in fontconfig-user.sgml

Christopher Degawa (1):
      meson: modify gperf test to remove sh dependency

Jason Francis (1):
      meson: Update freetype2 git repository to upstream

Jean Abou Samra (1):
      Ignore LC_CTYPE if set to "UTF-8"

Ondrej Balaz (1):
      Expand ~ in glob

lilinjie (1):
      fix typo

2.14.1

Akira TAGOH (6):
      Use the latest stable release image for CI for mingw
      Real fix of 91114d18c
      Fix test cases for wrong use of remap-dir
      Add back sort command
      Add config files to enable/disable antialias
      Enable 10-sub-pixel-rgb.conf by default

Alan Coopersmith (1):
      Bump the cache version to 8 in doc/fontconfig-user.sgml

Jeremy Huddleston Sequoia (3):
      meson: Fix linking libexpat on darwin
      meson: Use fc_templatedir and fc_baseconfigdir are used when installing configs
      meson: Use fc_configdir where appropriate during build

NorwayFun (4):
      add Georgian
      add Georgian
      po: add Georgian translation
      po: Add Georgian translation

Sam James (1):
      configure.ac: allow disabling docbook

Taylor R Campbell (1):
      Avoid misuse of ctype(3)

Tim-Philipp Müller (7):
      meson: add 'default-hinting' option
      meson: add 'default-fonts-dirs' option
      meson: add 'additional-fonts-dirs' option
      meson: add 'cache-dir' option
      meson: add 'template-dir', 'baseconfig-dir', 'config-dir', and 'xml-dir' options
      ci: update windows image to a 2022-server based one
      ci: update macOS image

Xavier Claessens (3):
      meson: Do not run fc-cache when installing into a DESTDIR
      meson: Fix configuration warnings
      link_confs.py: Fix prepending DESTDIR to absolute path

2.14

Alan Coopersmith (1):
      Update address for reporting msgid bugs from bugzilla to gitlab

2.13.96 (2.14 RC6)

Akira TAGOH (2):
      Add a missing file 48-spacing.conf
      Merge branch 'main' of ssh://gitlab.freedesktop.org/fontconfig/fontconfig

2.13.95 (2.14 RC5)

Akira TAGOH (18):
      Do not set different score to non-string values
      Enable 11-lcdfilter-default.conf by default
      Bump the cache version to 8
      Reflect matching results to binding in FcPattern
      Fix a memory leak when trying to open a non-existing file
      Fix score estimation for postscriptname
      Resolves symlinks against <dir prefix="relative">
      Add the option to not build fontconfig cache during installation
      conf.d/60-latin.conf: Make Noto default.
      Fix some testcase fails for 14c265a1
      Fix the issue fail to obtain the style name
      Apply the change made by 23e46d1 again
      Initialize variable
      Add more description for fc-conflist.1 and FcConfigFileInfoIterInit.3
      Update CaseFolding.txt to Unicode 14
      Add an user font directory for Win32 to the default font path
      Add test/wrapper-script.sh to the archive
      Fix possible memory leaks in FcPatternObjectAddWithBinding

Alex Richardson (3):
      fcint: add casts to allow building with stricter compilers
      Add support for C11 stdatomic atomics
      FcCharSetPutLeaf(): Fix missing move of new_leaves contents

Behdad Esfahbod (1):
      If a varfont has 'opsz' axis, set FC_SIZE on default instant pattern

Ben Wagner (6):
      Add memory order constraints to C11 atomics
      Free local FcCache lock on contention
      Extend test thread args lifetime
      Fix warning about os2->achVendID cannot be NULL
      Back FcSerialize with open addressing hash table.
      Actually skip leading spaces in style name

Francesco Pretto (1):
      WIN32: Fix pGetSystemWindowsDirectory found initialized during FcConfigParseAndLoadFromMemory

Mehdi Sabwat (1):
      fcstat: add support for wasm-emscripten

Nirbheek Chauhan (1):
      meson: Remove summary() from version_compare() block

Pierre Ducroquet (5):
      Add a configuration to switch to monospace if spacing=100 is requested
      Reference the new configuration file
      Remove configuration file from POTFILES
      It seems this qual doesn't work on integers
      Always add the family name from spacing=100

Ryan Gonzalez (1):
      Ensure config.h is always included before stdlib headers

Ryan Schmidt (5):
      Avoid PCRE syntax when using grep
      Remove Bugzilla references
      Fix run-test.sh to work with BSD mktemp
      Restore fcatomic compatibility with Mac OS X 10.4.
      Fix FC_DEFAULT_FONTS on macOS and with BSD sed

2.13.94 (2.14 RC4)

Akira TAGOH (10):
      Add back fullname property at scan matching phase
      Overwrite symlinks for config files
      Fix missing <dir> element for WINDOWSFONTDIR in meson
      Refactoring configure.ac to add <dir> element around font paths
      Fix build fail when missing docbook and/or disabling doc-build
      ci: Update CA cert related thing for Python on Windows
      Add support for XDG_DATA_DIRS
      Better wording for comments in config
      Revert constructing fullname property from family and style properties
      Fix score evaluation for multiple values in properties

Albert Astals Cid (1):
      Fix potential memory leak in _get_real_paths_from_prefix

Ben Wagner (11):
      Skip leading whitespace in style name.
      Remove abort from FcCompareSize.
      Add line between licenses in COPYING.
      Portable trap conditions in run-test.sh.
      Fix leaks in fcxml.c, fc-match.c, and tests.
      Fix wild frees and leak of fs in test-conf.
      Always run-test-conf, but skip if not built.
      Fix test-conf string to integer conversion.
      Test all not_eq for family names.
      Clean up test-family-matching test.
      Fix stack use after scope in FcConfigCompareValue

Carmina16 (1):
      ie.orth: Corrected; mistaken source replaced

Heiko Becker (1):
      Handle absolute sysconfdir when installing symlinks

Jacko Dirks (1):
      fccfg.c: lock_config: Fix potential memory leak

Szunti (3):
      Fix locale dependent behaviour in run-test.sh
      Check qual and compare for family tests
      Fix stripping whitespace from end of family in FcPatternAddFullname

Tim-Philipp Müller (6):
      meson: remove unused stdin_wrapper.py script
      fcformat: fix compiler warnings with clang on Android
      ci: add meson android aarch64 build
      meson: error out in script if gperf preprocessing failed
      meson: fix cross-compilation issues with gperf header file preprocessing
      meson: fix subproject build regression

Xavier Claessens (3):
      Meson: Fallback to gperf subproject on all platforms
      Meson: Do not wrap fccache insallation script
      Windows: Fix symlink privilege error detection

ratijas (1):
      Fix closing tag bracket typo in doc/fontconfig-user.sgml

2.13.93 (2.14 RC3)

Akira TAGOH (48):
      Affect FC_FONT_HAS_HINT property to score on matcher
      Do not return FcFalse from FcConfigParseAndLoad*() if complain is set to false
      Warn as well if no directory name for cachedir provided
      Take effect sysroot functionality to the default config file
      Read latest cache in paths
      Fix a memory leak caused by the previous commit
      Use FcConfigReference/Destroy appropriately instead of FcConfigGetCurrent
      Fix potential race condition in FcConfigSetCurrent and FcConfigReference
      Fix gcc warnings with -Wpointer-sign
      Don't add a value for FC_FULLNAME in meta face
      Fix a test fail when no bwrap was available
      Add proper fullname for named-instances
      Fix the process substitution doesn't work with FONTCONFIG_FILE
      Fix memory leaks
      Fix assertion in FcFini()
      Set exact boolean value to color property
      Fix assertion in FcCacheFini() again
      Fix errors on shellcheck
      Fix cache conflicts on OSTree based system
      Drop unmaintained files
      Drop elements with its namespace from conf
      Add FC_ORDER property into cache
      Drop Bitstream Vera fonts from 60-latin.conf
      Fix a typo in doc/confdir.sgml.in
      Fix empty XDG_CACHE_HOME Behavior
      Fix build issues regarding formatter for Win32
      Add some tweaks into runtest.sh to see progress
      Integrate python scripts to autotools build
      Make sure a minimum version of python
      Make more clearer the license terms
      Add CONFIGDIR to the fallback config where can be specified by --with-configdir
      fc-scan: add --sysroot option
      Construct fullname from family and style
      Add fullname later once FcConfigSubstitute() is done
      Update meson.build
      Add Regular style when no meta data available to guess a style
      Make sure a combination of family and familylang is available
      Split up a code again coming from different copyright notice
      Update COPYING
      Use memcpy instead of strcpy
      Evaluate mingw64_env to setup properly on CI
      Add examples section in fc-match(1)
      Drop duplicated BUILT_SOURCES in doc/Makefile.am
      Initialize shell variables to be sure
      Update README that missed changes mistakenly
      new-version.sh: update version in meson.build
      Update version in meson.build to sync up with configure.ac
      new-version.sh: commit meson.build when bumpping

Alan Coopersmith (1):
      Fix some typos/spelling errors

Ben Wagner (2):
      Replace FT_UNUSED with FC_UNUSED.
      Fix fc_atomic_ptr_get and use.

Chun-wei Fan (2):
      meson: Look for FreeType using CMake too
      meson: Don't use .def files for Visual Studio builds

James Lee (1):
      Fix 'meson install' when cross compiling

Jan Tojnar (4):
      conf: Add JoyPixels emoji font
      Correct reset-dirs in DTD
      Drop elements with its namespace from fonts.conf.in
      Turn unknown attributes into warning

Jonathan Kew (1):
      Set name_mapping to NULL after freeing

Mathieu Duponchelle (1):
      Some build fixes to support compilation with MSVC on Windows

Matthias Clasen (17):
      Avoid a crash in FcPatternHash with ranges
      Special-case some of the string walking code
      Add a hash function for families
      Use a hash table for family matching
      Add a shortcut for FcQualAny matching
      Speed up fonthashint matching
      Speed up FcConfigCompareValue
      Speed up FcConfigCompareValue
      Speed up FcCompareLang and FcCompareBool
      Use a hash table for families in FcConfigSubstitute
      Use __builtin_expect in a few places
      Fixup: Promote ints to ranges when appropriate
      Add FC_LIKELY and FC_UNLIKELY macros
      Use FC_UNLIKELY
      Fixup: Handle patterns without family
      Fix up FC_LIKELY macros
      Fix a problem in FcConfigSubstitute

Nicolas Mailhot (1):
      Use an URN for DTD ID

Niklas Guertler (3):
      Allow multiple default system font directories in the fallback config, and set them to the default dirs on Darwin.
      Add <dir> XML tags to default values for FC_DEFAULT_FONTS on non-Darwin systems
      Increased timeout for meson tests to 600sec to make tests work on Darwin

Nirbheek Chauhan (2):
      fcatomic: Fix EXC_BAD_ACCESS on iOS ARM64
      meson: Fix build failure with gcc10 on mingw

Szunti (1):
      Add missing return type for FcFontSet* functions

Tim-Philipp Müller (8):
      doc: fix FcPatternFilter documentation
      Use FC_PATH_MAX to fix build on Windows
      Fix build on Windows some more
      fccompat: fix build on Windows without unistd.h
      Guard dirent.h includes
      Add Meson build system
      meson: print configuration summary()
      ci: allow meson mingw build to fail

Xavier Claessens (3):
      meson: Fix build when 'tools' option is disabled
      meson: Use version comparison function
      meson: Fix build failure when compiler is not in PATH

xiota (1):
      Add Courier Std aliases.  Fixes #262.

2.13.92 (2.14 RC2)

Akira TAGOH (18):
      Fix a typo on masking face id
      Don't clean up pre-built docs if no docbook installed.
      Fix obtaining real path from pre-defined names for Win32
      Fix a crash when running with FC_DEBUG=256
      Improve the performance a bit
      Fix a typo
      Add English name first into a cache
      FcConfigParseAndLoad*() should returns false on config errors
      Clean up temporary directory for tests
      Add docs for missing properties
      Fix the fail on fc-cache
      Fix memory leaks
      Fix a memory leak in FcFreeTypeQuery*()
      Add 35-lang-normalize.conf
      Add FC_FONT_HAS_HINT property to see if font has hinting or not.
      Fix failing the check of prep table in some fonts
      Fix the fails of make check when SOURCE_DATE_EPOCH is set
      Improve the performance a bit

Egmont Koblinger (1):
      Fix the linear interpolation during weight mapping

2.13.91 (2.14 RC1)

Akira TAGOH (74):
      Fix the build issue with --enable-static
      Fix the issue that '~' wasn't extracted to the proper homedir
      Add a test case for d1f48f11
      Fix CI
      Add more prefix support in <dir> element
      Update fonts.dtd for last commit
      Update docs for 1aa8b700
      add missing the case of prefix="default" as documented
      Fix test case
      CI: Add more logs
      Do not update mtime when removing .uuid file
      Do not try updating mtime when unlink was failed
      Do not run a test case for .uuid deletion
      Drop Mitra Mono from 65-nonlatin.conf
      Enable bubblewrap test case
      Use FC_PATH_MAX instead of PATH_MAX
      Use Rachana instead of Meera for Malayalam
      Add doc for description element and update fonts.dtd
      Fix FcFontList doesn't return a font with FC_COLOR=true
      Add a test case for FcFontList
      Warn when constant name is used for unexpected object
      covscan fix: get rid of unnecessary condition check
      Don't call unlink_dirs if basedir is null
      covscan: fix compiler warnings
      Fix a dereference of a null pointer
      Fix a crash with invalid matrix element
      Add system-ui generic family
      Fix misleading summary in docs for FcStrStrIgnoreCase
      Fix build issue on Win32.
      autogen.sh: Make AUTORECONF_FLAGS overwritable
      Ifdef'ed unnecessary code for Win32
      Fix make check on cross-compiled env
      Add build test for MinGW
      Fix make distcheck error
      Update requirement for gettext
      Correct configure option to cross-compile
      Install wine for CI on MinGW
      Don't test bind-mount thing for MinGW
      Reset errno to do error handling properly
      Add FcDirCacheCreateUUID doc back to pass make check
      Drop a line to include uuid.h
      Fix make check fail on run-test-conf.sh
      Add new element remap-dir instead of extending dir element
      Trim the last slash
      Update testcase
      Update deps to run CI
      Drop unnecessary line to include uuid.h
      Fix a typo
      Add reset-dirs element
      Add salt attribute to dir and remap-dir elements
      Update doc for salt
      trivial testcase update
      Add back if !OS_WIN32 line
      Fix build issues on MinGW
      Use alternative function for realpath on Win32
      Fix make check fail on MinGW again
      Add more data to artifacts for debugging purpose
      Don't share fonts and cache dir for testing
      Don't warn if path can't be converted with prefix
      Add some debugging output
      Oops, Terminate string
      fc-cache: Show font directories to generate cache with -v
      Allow overriding salt with new one coming later
      Don't show salt in debugging message if salt is null
      Fix unexpected cache name by double-slash in path
      Fallback uuid-based name to read a cache if no MD5-based cache available
      No need to remap for uuid based
      Update the test case that is looking for uuid based on host
      Distribute archive in xz instead of bz2
      Update CaseFolding.txt to Unicode 12.1
      fc-validate: returns an error code when missing some glyphs
      Correct the comment for FC_LANG in fontconfig.h
      Fix a typo in the description of FcWeightFromOpenTypeDouble
      Fix endianness on generating MD5 cache name

Behdad Esfahbod (1):
      Fix name-table language code mapping for Mongolian

Ben Wagner (1):
      Better document sysroot.

Chris McDonald (2):
      Respect sysroot option for file path passed to stat
      Lowered temporary rooted_dir variable inside loop

Jon Turney (1):
      Only use test wrapper-script if host is MinGW

Keith Packard (6):
      Do not remove UUID file when a scanned directory is empty
      Fetch FONTCONFIG_SYSROOT in FcConfigCreate
      Remove '-u' option from run-test-conf.sh
      Add delays to test-bz106632, check UptoDate separately
      Remove UUID-related tests
      Replace UUID file mechanism with per-directory 'map' attribute [v2]

Robert Yang (1):
      src/fccache.c: Fix define for HAVE_POSIX_FADVISE

2.13.1

Akira TAGOH (48):
      Use the builtin uuid for OSX
      Fix the build issue again on MinGW with enabling nls
      Add uuid to Requires.private in .pc only when pkgconfig macro found it
      Allow the constant names in the range
      Do not override locale if already set by app
      Add the value of the constant name to the implicit object in the pattern
      Add a testcase for FcNameParse
      Leave the locale setting to applications
      call setlocale
      Fix make check fail when srcdir != builddir.
      Do not ship fcobjshash.h
      Fix typo in doc
      Change the emboldening logic again
      Bug 43367 - RFE: iterator to peek objects in FcPattern
      Add a testrunner for conf
      Add a test case for 90-synthetic.conf
      Bug 106497 - better error description when problem reading font configuration
      Bug 106459 - fc-cache doesn't use -y option for .uuid files
      Fix leaks
      Fix -Wstringop-truncation warning
      Fix double-free
      Add a test case for bz#106618
      Update CaseFolding.txt to Unicode 11
      Remove .uuid when no font files exists on a directory
      Fix the leak of file handle
      Fix memory leak
      Fix memory leaks
      Fix memory leak
      Fix memory leak
      Fix memory leak
      Fix unterminated string issue
      Fix array access in a null pointer dereference
      Fix access in a null pointer dereference
      do not pass null pointer to memcpy
      Fix dereferencing null pointer
      Fix a typo
      Fix possibly dereferencing a null pointer
      Fix allocating insufficient memory for terminating null of the string
      Make a call fail on ENOMEM
      Allocate sufficient memory to terminate with null
      Drop the redundant code
      Fix memory leak
      Fix the build issue with gperf
      Fix missing closing bracket in FcStrIsAbsoluteFilename()
      Update the issue tracker URL
      Fix distcheck fail
      Add .gitlab-ci.yml
      Bump the libtool revision

Alexander Larsson (3):
      Add FcCacheAllocate() helper
      Cache: Rewrite relocated paths in earlier
      Cache: Remove alias_table

Behdad Esfahbod (4):
      Minor: fix warnings
      Fix name scanning
      Share name-mapping across instances
      Use FT_HAS_COLOR

Chris Lamb (1):
      Ensure cache checksums are deterministic

Matthieu Herrb (1):
      FcCacheFindByStat(): fix checking of nanoseconds field.

Tom Anderson (7):
      Fix undefined-shift UBSAN errors
      Use realfilename for FcOpen in _FcConfigParse
      Add FONTCONFIG_SYSROOT environment variable
      Fix CFI builds
      Fix heap use-after-free
      Return canonicalized paths from FcConfigRealFilename
      Fix build with CFLAGS="-std=c11 -D_GNU_SOURCE"

2.13

Akira TAGOH (4):
      Add Simplified Chinese translations
      Fix a build issue on MinGW with enabling nls
      Initialize an array explicitly
      Bump the libtool revision

2.12.93 (2.13 RC3)

Akira TAGOH (12):
      trivial fix
      Add files to enable ITS support in gettext
      Use the native ITS support in gettext
      Remove POTFILES.in until new release of gettext is coming...
      export GETTEXTDATADIR to refer the local .its/.loc file instead of using --its option
      clean up
      Do not add cflags and libs coming from pkg-config file.
      Revert some removal from 7ac6af6
      Take effects on dir, cachedir, acceptfont, and rejectfont only when loading
      Do not mix up font dirs into the list of config dirs
      Ensure the user config dir is available in the list of config dirs on the fallback config
      Add missing files to ship

Alexander Larsson (1):
      FcHashTableAddInternal: Compare against the right key

Behdad Esfahbod (5):
      Remove hack for OS/2 weights 1..9
      Support FC_WIDTH as double as well
      Fix leak
      Use FT_Done_MM_Var if available
      Fix undefined-behavior signed shifts

Olivier Crête (1):
      Fix cross-compilation by passing CPPFLAGS to CPP

Tom Anderson (1):
      Allow overriding symbol visibility.

2.12.92 (2.13 RC2)

Akira TAGOH (13):
      cleanup files
      Update .uuid only when -r is given but not -f.
      Returns false if key is already available in the table
      Add missing doc of FcDirCacheCreateUUID
      Replace uuid in the table properly when -r
      Add a test case for uuid creation
      Do not update mtime with creating .uuid
      Disable uuid related code on Win32
      Try to get current instance of FcConfig as far as possible
      do not check the existence of itstool on win32
      Fix the mis-ordering of ruleset evaluation in a file with include element
      Fix compiler warnings
      Add FcReadLink to wrap up readlink impl.

Alexander Larsson (1):
      fchash: Fix replace

Behdad Esfahbod (7):
      Don't crash
      Remove a debug abort()
      Minor
      Set font-variations settings for standard axes in variable fonts
      Let pattern FC_FONT_VARIATIONS override standard axis variations
      Put back accidentally removed code
      Add FcWeightTo/FromOpenTypeDouble()

2.12.91 (2.13 RC1)

Akira TAGOH (37):
      und_zsye.orth: polish to get for NotoEmoji-Regular.ttf
      Revert "Keep the same behavior to the return value of FcConfigParseAndLoad"
      Fix again to keep the same behavior to the return value of FcConfigParseAndLoad
      cleanup
      Fix a compiler warning
      Update libtool revision
      Bump version to 2.12.6
      doc: trivial update
      Add the ruleset description support
      workaround to avoid modifying by gettextize
      missing an open parenthesis
      another workaround to avoid modifying by gettextize...
      Validate cache more carefully
      Allow autoreconf through autopoint for gettext things
      Correct debugging messages to load/scan config
      Add the check of PCF_CONFIG_OPTION_LONG_FAMILY_NAMES back
      Use uuid-based cache filename if uuid is assigned to dirs
      Add new API to find out a font from current search path
      Replace the font path in FcPattern to what it is actually located.
      Replace the original path to the new one
      Replace the path of subdirs in caches as well
      Don't call FcStat when the alias has already been added
      Destroy the alias and UUID tables when all of caches is unloaded
      cleanup
      abstract hash table functions
      update
      Fix memory leak
      Fix a typo
      Don't call FcStat when the alias has already been added
      Add a testcase for bind-mounted cachedir
      cleanup
      Use smaller prime for hash size
      Fix the testcase for env not enabled PCF_CONFIG_OPTION_LONG_FAMILY_NAMES in freetype
      thread-safe functions in fchash.c
      Fix distcheck error
      Fix "make check" fail again
      Bump the libtool revision

Alban Browaeys (1):
      Fixes cleanup

Alexander Kanavin (1):
      src/fcxml.c: avoid double free() of filename

Bastien Nocera (1):
      conf: Prefer system emoji fonts to third-party ones

Behdad Esfahbod (76):
      Minor
      Remove stray printf()
      [fc-query] Fix linking order
      Instead of loading glyphs (with FreeType), just check loca table
      Don't even check loca for glyph outline detection
      Check for non-empty outline for U+0000..U+001F
      Add back code for choosing strike, and cleanup
      Minor: adjust debug output
      Remove unnecessary check
      Remove a few unused blanks parameters
      Remove check that cannot fail
      Remove use of psnames for charset construction
      Remove unused variable
      Remove fc-glyphname
      Remove blanks facility from the library
      Remove blanks support from fc-scan
      Mark more parameters FC_UNUSED
      Move variables to narrower scope and indent
      Remove unneeded check
      Use multiplication instead of division
      Use inline functions instead of macros for a couple of things
      Simplify advance-width calculations
      Inline FcFreeTypeCheckGlyph()
      Call FT_Get_Advance() only as long as we need to determine font width type
      Minor
      Update documentation for removal of blanks
      Merge branch 'faster'
      Add FcFreeTypeQueryAll()
      Document FcFreeTypeQueryAll()
      Accept NULL in for spacing in FcFreeTypeCharSetAndSpacing()
      Remove FcCompareSize()
      Rename FcCompareSizeRange() to FcCompareRange()
      Rewrite FcCompareRange()
      In FcSubstituteDefault(), handle size range
      Check instance-index before accessing array
      Indent
      [varfonts] Add FC_FONT_VARIATIONS
      [varfonts] Add FC_VARIABLE
      [varfonts] Change id argument in FcFreeTypeQuery* to unsigned int
      Print ranges as closed as opposed to half-open
      [varfonts] Change FC_WEIGHT and FC_WIDTH into ranges
      [varfonts] Query varfonts if id >> 16 == 0x8000
      Fix instance-num handling in collections
      [varfonts] Query variable font in FcFreeTypeQueryAll()
      [varfonts] Fetch optical-size for named instances
      In RenderPrepare(), handle ranges smartly
      [fc-query] Remove --ignore-blanks / -b
      [fc-match/fc-list/fc-query/fc-scan] Add --brief that is like --verbose without charset
      Add separate match compare function for size
      Fix range comparision operators implementation
      Adjust emboldening logic
      [varfonts] Map from OpenType to Fontconfig weight values
      Add FcDontCare value to FcBool
      Implement more config bool operations for boolean types
      Fix possible div-by-zero
      [varfonts] Use fvar data even if there's no variation in it
      Minor
      Revert "[varfonts] Use fvar data even if there's no variation in it"
      [varfonts] Minor
      [varfonts] Comment
      [varfonts] Don't set style for variable-font pattern
      [varfonts] Skip named-instance that is equivalent to base font
      [varfonts] Do not set postscriptname for varfont pattern
      [varfonts] Don't reopen face for each named instance
      Separate charset and spacing code
      [varfonts] Reuse charset for named instances
      Move whitespace-trimming code to apply to all name-table strings
      Fix whitespace-trimming loop and empty strings...
      Whitespace
      Don't convert nameds to UTF-8 unless we are going to use them
      Simplify name-table platform mathcing logic
      Use binary-search for finding name table entries
      [varfonts] Share lang across named-instances
      Merge branch 'varfonts2'
      Require freetype >= 2.8.1
      Remove assert

David Kaspar [Dee'Kej] (1):
      conf.d: Drop aliases for (URW)++ fonts

Florian Müllner (1):
      build: Remove references to deleted file

2.12.6

Akira TAGOH (4):
      und_zsye.orth: polish to get for NotoEmoji-Regular.ttf
      Revert "Keep the same behavior to the return value of FcConfigParseAndLoad"
      Fix again to keep the same behavior to the return value of FcConfigParseAndLoad
      Update libtool revision

Behdad Esfahbod (2):
      Minor
      [fc-query] Fix linking order

David Kaspar [Dee'Kej] (1):
      conf.d: Drop aliases for (URW)++ fonts

Florian Müllner (1):
      build: Remove references to deleted file

2.12.5

Akira TAGOH (17):
      Add FcPatternGetWithBinding() to obtain the binding type of the value in FcPattern.
      Add FcConfigParseAndLoadFromMemory() to load a configuration from memory.
      Bug 101726 - Sans config pulls in Microsoft Serifed font
      Fix gcc warnings with enabling libxml2
      Add und-zsye.orth to support emoji in lang
      Add more code points to und-zsye.orth
      Keep the same behavior to the return value of FcConfigParseAndLoad
      Do not ship fcobjshash.gperf in archive
      Accept 4 digit script tag in FcLangNormalize().
      Fix to work the debugging option on fc-validate
      Add und_zmth.orth to support Math in lang
      Polish und_zmth.orth for Libertinus Math
      Polish und_zmth.orth more for Cambria Math and Minion Math
      Update similar to emoji's
      fc-blanks: fall back to the static data available in repo if downloaded data is corrupted
      Update docs
      Update libtool versioning

Behdad Esfahbod (14):
      Pass --pic to gperf
      Add generic family matching for "emoji" and "math"
      [fc-query] Support listing named instances
      Add Twitter Color Emoji
      Add EmojiOne Mozilla font
      [fc-lang] Allow using ".." instead of "-" in ranges
      Minor
      Remove unneeded codepoints
      Adjust color emoji config some more
      Ignore 'und-' prefix for in FcLangCompare
      Minor
      Fix sign-difference compare warning
      Fix warning
      Fix weight mapping

2.12.4

Akira TAGOH (5):
      Force regenerate fcobjshash.h when updating Makefile
      Fix the build failure when srcdir != builddir and have gperf 3.1 or later installed
      Add a testcase for Bug#131804
      Update libtool revision
      Fix distcheck error

Florent Rougon (6):
      FcCharSetHash(): use the 'numbers' values to compute the hash
      fc-lang: gracefully handle the case where the last language initial is < 'z'
      Fix an off-by-one error in FcLangSetIndex()
      Fix erroneous test on language id in FcLangSetPromote()
      FcLangSetCompare(): fix bug when two charsets come from different "buckets"
      FcCharSetFreezeOrig(), FcCharSetFindFrozen(): use all buckets of freezer->orig_hash_table

Helmut Grohne (1):
      fix cross compilation

Jan Alexander Steffens (heftig) (1):
      Fix testing PCF_CONFIG_OPTION_LONG_FAMILY_NAMES (CFLAGS need to be right)

Josselin Mouette (1):
      Treat C.UTF-8 and C.utf8 locales as built in the C library.

Masamichi Hosoda (1):
      Bug 99360 - Fix cache file update on MinGW

2.12.3

Akira TAGOH (1):
      Fix make check fail with freetype-2.7.1 and 2.8 with PCF_CONFIG_OPTION_LONG_FAMILY_NAMES enabled.

2.12.2

Akira TAGOH (8):
      Don't call perror() if no changes happens in errno
      Fix FcCacheOffsetsValid()
      Fix the build issue with gperf 3.1
      Fix the build issue on GNU/Hurd
      Update a bit for the changes in FreeType 2.7.1
      Add the description of FC_LANG envvar to the doc
      Bug 101202 - fontconfig FTBFS if docbook-utils is installed
      Update libtool revision

Alan Coopersmith (1):
      Correct cache version info in doc/fontconfig-user.sgml

Khem Raj (1):
      Avoid conflicts with integer width macros from TS 18661-1:2014

Masamichi Hosoda (2):
      Fix PostScript font alias name
      Update aliases for URW June 2016

2.12.1

Akira TAGOH (6):
      Add --with-default-hinting to configure
      Update CaseFolding.txt to Unicode 9.0
      Check python installed in autogen.sh
      Fix some errors related to python3
      Bug 96676 - Check range of FcWeightFromOpenType argument
      Update libtool revision

Tobias Stoeckmann (1):
      Properly validate offsets in cache files.

2.12

Akira TAGOH (8):
      Modernize fc-blanks.py
      Update URL
      Bug 95477 - FcAtomicLock fails when SELinux denies link() syscall with EACCES
      45-latin.conf: Add some Windows fonts to categorize them properly
      Correct one for the previous change
      Bug 95481 - Build fails on Android due to broken lconv struct
      Add the static raw data to generate fcblanks.h
      Remove unused code

Erik de Castro Lopo (1):
      Fix a couple of minor memory leaks

Petr Filipsky (1):
      Fix memory leak in FcDirCacheLock

2.11.95 (2.12 RC5)

Akira TAGOH (22):
      Add one more debugging option to see transformation on font-matching
      Fix a crash when no objects are available after filtering
      No need to be public
      mark as private at this moment
      Don't return FcFalse even when no fonts dirs is configured
      Add a warning for blank in fonts.conf
      Fix a memory leak in FcFreeTypeQueryFace
      Update CaseFolding.txt to Unicode 8.0
      Bug 90867 - Memory Leak during error case in fccharset
      Fix the broken cache more.
      Fail on make runtime as needed instead of configure if no python installed
      Use long long to see the same size between LP64 and LLP64
      Fix build issue on MinGW
      Use int64_t instead of long long
      Fix compiler warnings on MinGW
      Fix assertion on 32bit arch
      remomve unnecessary code
      Bug 93075 - Possible fix for make check failure on msys/MinGW...
      Avoid an error message on testing when no fonts.conf installed
      Add hintstyle templates and make hintslight default
      Revert "Workaround another race condition issue"
      Update libtool revision

Behdad Esfahbod (6):
      Revert changes made to FcConfigAppFontAddDir() recently
      Call FcFreeTypeQueryFace() from fcdir.c, instead of FcFreeTypeQuery()
      [GX] Support instance weight, width, and style name
      [GX] Enumerate all named-instances in TrueType GX fonts
      Improve OpenType to Fontconfig weight mapping
      [GX] Improve weight mapping

Patrick Haller (1):
      Optimizations in FcStrSet

2.11.94 (2.12 RC4)

Akira TAGOH (16):
      Remove the dead code
      Bug 89617 - FcConfigAppFontAddFile() returns false on any font file
      Fix unknown attribute in Win32
      Fix SIGFPE
      Fix a typo for the latest cache version
      Fix a typo in fontconfig-user.sgml
      Drop unmaintained code
      Observe blanks to compute correct languages in fc-query/fc-scan
      Add missing description for usage
      Make FC_SCALE deprecated
      Bug 90148 - Don't warn if cachedir isn't specified
      Fix memory leaks after FcFini()
      Fix a typo
      Fix a crash
      Detect the overflow for the object ID
      Revert the previous change

Behdad Esfahbod (11):
      Fix bitmap scaling
      Add su[pport for symbol fonts
      Write ranges using a [start finish) format
      Only set FC_SIZE for scalable fonts if OS/2 version 5 is present
      Add bitmap-only font size as Double, not Range
      Accept Integer for FC_SIZE
      Don't set FC_SIZE for bitmap fonts
      Fix compiler warnings
      Simplify FcRange
      Reduce number of places that cache version is specified to 1
      Bump cache version number to 6, because of recent FcRange changes

Руслан Ижбулатов (1):
      W32: Support cache paths relative to the root directory

2.11.93 (2.12 RC3)

Akira TAGOH (18):
      Fix a typo in docs
      Add pkg.m4 to git
      Fix a build fail on some non-POSIX platforms
      ifdef'd the unnecessary code for win32
      Fix pointer cast warning on win32
      filter can be null
      Copy the real size of struct dirent
      Rework again to copy the struct dirent
      Hardcode the blanks in the library
      Update the script to recognize the escaped space
      Fix a build issue when $(srcdir) != $(builddir)
      Don't add FC_LANG when it has "und"
      Fix the array allocation
      Improve the performance on searching blanks
      Fix a segfault when OOM happened.
      Fix a bug in the previous change forFcBlanksIsMember()
      Fix an infinite loop in FcBlanksIsMember()
      Fix a trivial bug for dist

Alan Coopersmith (1):
      Fix configure to work with Solaris Studio compilers

Behdad Esfahbod (3):
      Fix symbol cmap handling
      Remove dead code after previous commit
      Simplify some more

Michael Haubenwallner (1):
      Ensure config.h is included first, bug#89336.

2.11.92 (2.12 RC2)

Akira TAGOH (1):
      Add missing docs

2.11.91 (2.12 RC1)

Akira TAGOH (28):
      Bug 71287 - size specific design selection support in OS/2 table version 5
      Fix a build issue with freetype <2.5.1
      Fix missing docs
      Fix a typo
      Fix fc-cache fail with -r
      Rebase ja.orth against Joyo kanji characters
      Allow the modification on FcTypeVoid with FcTypeLangSet and FcTypeCharSet
      Workaround another race condition issue
      Read the config files and fonts on the sysroot when --sysroot is given to fc-cache
      Fix a segfault
      Update CaseFolding.txt to Unicode 7.0
      Don't read/write from/to the XDG dirs if the home directory is disabled
      Rework for 5004e8e01f5de30ad01904e57ea0eda006ab3a0c
      Fix a crash when no sysroot is given and failed to load the default fonts.conf
      Fix a gcc warning
      Don't add duplicate lang
      fallback to the another method to lock when link() failed
      Increase the refcount in FcConfigSetCurrent()
      Fix the memory leak in fc-cat
      Note FcConfigSetCurrent() increases the refcount in document
      Add FcRangeGetDouble()
      Revert "Bug 73291 - poppler does not show fl ligature"
      Update aliases for new URW fonts
      Returns False if no fonts found
      fc-cache: make a fail if no fonts processed on a given path
      fc-cache: Add an option to raise an error if no fonts found
      Bump the cache version to 5
      Fix a typo

Behdad Esfahbod (39):
      Remove unused code
      Simplify hash code
      Further simplify hash code
      Rewrite hashing to use FT_Stream directly
      Allow passing NULL for file to FcFreeTypeQueryFace()
      [ko.orth] Remove U+3164 HANGUL FILLER
      Deprecate FC_HASH and don't compute it
      Remove unused FcHash code now that FC_HASH is deprecated
      Update list of blanks to Unicode 6.3.0
      Update blanks to Unicode 7.0
      Change charset parse/unparse format to be human readable
      Minor
      Fix charset unparse after recent changes
      Comments
      Remove HASH from matching priorities
      Fixup previous commit
      Update mingw32 MemoryBarrier from HarfBuzz
      More mingw32 MemoryBarrier() fixup
      Symlinks fix for DESTDIR
      Revert "Symlinks fix for DESTDIR"
      Call FcInitDebug from FcFreeTypeQueryFace
      Decode MacRoman encoding in name table without iconv
      Ouch, fix buffer
      Use lang=und instead of lang=xx for "undetermined"
      Remove unused regex code
      Improve / cleanup namelang matching
      Add FC_WEIGHT_DEMILIGHT
      Change DemiLight from 65 to 55
      Linearly interpolate weight values
      Export recently added API
      Remove unneeded FcPublic
      Fix assertion failure
      If OS/2 table says weight is 1 to 9, multiply by 100
      Trebuchet MS is a sans-serif font, not serif
      Fix previous commit
      Revert "[fcmatch] When matching, reserve score 0 for when elements don't exist"
      Fix buffer overflow in copying PS name
      Add FC_COLOR
      Treat color fonts as scalable

Nick Alcock (1):
      Generate documentation for FcWeight* functions.

2.11.1

Akira TAGOH (31):
      do not build test-migration for Win32
      Fix build issue on Debian/kFreeBSD 7.0
      Update ax_pthread.m4 to the latest version
      Fix the dynamic loading issue on NetBSD
      Use stat() if there are no d_type in struct dirent
      Fix a build issue on Solaris 10
      Change the default weight on match to FC_WEIGHT_NORMAL
      Warn if no <test> nor <edit> elements in <match>
      Correct DTD
      Re-scan font directories only when it contains subdirs
      Fix typo
      Bug 72086 - Check for gperf in autogen.sh
      Simplify to validate the availability of posix_fadvise
      Simplify to validate the availability of scandir
      Fix a typo
      Fix a build issue on platforms where doesn't support readlink()
      Improve the performance issue on rescanning directories
      Bug 73686 - confdir is not set correctly in fontconfig.pc
      Update zh_hk.orth
      clean up the unused files
      Add missing license headers
      Update the use of autotools' macro
      Fix a crash issue when empty strings are set to the BDF properties
      Add a doc for FcDirCacheRescan
      Add missing #include <sys/statvfs.h> in fcstat.c
      Fix incompatible API on AIX with random_r and initstate_r
      Fallback to lstat() in case the filesystem doesn't support d_type in struct dirent
      Update doc to include the version info of `since when'
      Bug 73291 - poppler does not show fl ligature
      Add README describes the criteria to add/modify the orthography files
      Fix autoconf warning, warning: AC_COMPILE_IFELSE was called before AC_USE_SYSTEM_EXTENSIONS

Alan Coopersmith (3):
      Leave room for null terminators in arrays
      Avoid memory leak when NULL path passed to FcStrBuildFilename
      Avoid null pointer dereference in FcNameParse if malloc fails

Behdad Esfahbod (1):
      Bug 72380 - Never drop first font when trimming

Frederic Crozat (2):
      Fix inversion between Tinos and Cousine in the comment
      Add metric aliases for additional Google ChromeOS fonts

Jehan (1):
      Defaulting <cachedir> to LOCAL_APPDATA_FONTCONFIG_CACHE for Win32 build

Ross Burton (1):
      fc-cache: --sysroot option takes an argument

2.11

Akira TAGOH (15):
      Do not create a config dir for migration when no config files nor dirs
      Add a test case of the migration for config place
      Fix memory leaks in FcFreeTypeQueryFace
      Bug 68955 - Deprecate / remove FC_RASTERIZER
      Copy all values from the font to the pattern if the pattern doesn't have the element
      Fix a crash when FcPattern is set to null on FcFontSetList() and FcFontList()
      Add the description of -q option to the man page
      avoid reading config.h twice
      clean up
      Add the relative path for <include> to fonts.conf if the parent path is same to fonts.conf
      Workaround the race condition issue on updating cache
      exit with the error code when FcNameParse() failed
      Add missing doc for FcStrListFirst and fix a typo
      Bump libtool revision
      Update CaseFolding.txt to Unicode 6.3

Jan Alexander Steffens (heftig) (1):
      Further changes to 30-metric-aliases.conf

W. Trevor King (1):
      doc/fccharset.fncs: Describe the map format in more detail

2.10.95 (2.11 RC5)

Akira TAGOH (2):
      Fix a typo
      Fix a crash

2.10.94 (2.11 RC4)

Akira TAGOH (25):
      Bug 64906 - FcNameParse() should ignore leading whitespace in parameters
      Fix a comparison of constant warning with clang
      Fix a shift count overflow on 32bit box
      Fix a incompatible pointer warning on NetBSD
      Add FcTypeUnknown to FcType to avoid comparison of constant -1
      Fix the behavior of intermixed tests end edits in match
      Ignore scandir() check on mingw
      Use INT_MAX instead of unreliable hardcoding value
      Add FC_UNUSED to FC_ASSERT_STATIC macro to avoid compiler warning
      Rework to apply the intermixed test and edit elements in one-pass
      trivial code optimization
      Correct fontconfig.pc to add certain dependencies for build
      Correct fontconfig.pc to add certain dependencies for static build
      Fix wrong edit position
      Bug 67809 - Invalid read/write with valgrind when assigning something twice
      warn deprecated only when migration failed
      Bug 67845 - Match on FC_SCALABLE
      Bug 16818 - fontformat in match pattern is not respected?
      Bug 68340 - More metric compat fonts
      Bug 63399 - Add default aliases for Georgia, Garamond, Palatino Linotype, Trebuchet MS
      Fix a typo
      Fix a crash when non-builtin objects are edited
      Fix a wrong edit position when 'kind' is different
      Bug 68587 - copy qu.orth to quz.orth
      Add quz.orth to Makefile.am

Behdad Esfahbod (2):
      Minor
      Fix assertion

2.10.93 (2.11 RC3)

Akira TAGOH (10):
      Bug 62980 - matching native fonts with even :lang=en
      Ensure closing fp on error
      Obtain fonts data via FT_Face instead of opening a file directly
      Revert the previous change and rework to not export freetype API outside fcfreetype.c
      documented FC_HASH and FC_POSTSCRIPT_NAME
      Bug 63329 - make check fails: .. contents:: :depth: 2
      Use the glob matching for filename
      Bug 63452 - conf.d/README outdated
      Fix missing OSAtomicCompareAndSwapPtrBarrier() on Mac OS X 10.4
      Bug 63922 - FcFreeTypeQueryFace fails on postscripts fonts loaded from memory

Sebastian Freundt (1):
      build-chain, replace INCLUDES directive by AM_CPPFLAGS

2.10.92 (2.11 RC2)

Akira TAGOH (33):
      Fix the build fail on MinGW
      Bug 50497 - RFE: Add OpenType feature tags support
      Improve FcGetPrgname() to work on BSD
      Better fix for 2fe5ddfd
      Add missing file descriptor to F_DUPFD_CLOEXEC
      Fix mkstemp absence for some platform
      Fix installation on MinGW32
      Add another approach to FC_PRGNAME for Solaris 10 or before
      remove the unnecessary code
      Bug 59385 - Do the right thing for intermixed edit and test elements
      Bug 23757 - Add mode="delete" to <edit>
      Modernize configure.ac
      Use AM_MISSING_PROG instead of hardcoding missing
      Revert "test: Use SH_LOG_COMPILER and AM_TESTS_ENVIRONMENT"
      Use AM_MISSING_PROG instead of hardcoding missing
      Bug 50733 - Add font-file hash?
      Bug 60312 - DIST_SUBDIRS should never appear in a conditional
      Update _FcMatchers definition logic
      Bump the cache version to 4
      Add Culmus foundry to the vendor list
      Bug 60748 - broken conf.d/10-autohint.conf and conf.d/10-unhinted.conf
      Bug 60783 - Add Liberation Sans Narrow to 30-metric-aliases.conf
      Fix a typo
      Fix a crash when the object is non-builtin object
      Fix broken sort order with FcFontSort()
      Fix a memory leak
      Bug 59456 - Adding a --sysroot like option to fc-cache
      Do not copy FC_*LANG_OBJECT even if it's not available on the pattern
      Fix a SIGSEGV on FcPatternGet* with NULL pattern
      Bug 38737 - Wishlist: support FC_POSTSCRIPT_NAME
      Minor cleanup
      Bump libtool revision
      Minor fix

Behdad Esfahbod (12):
      Resepct $NOCONFIGURE
      Ensure we find the uninstalled fontconfig header
      Copy all values from pattern to font if the font doesn't have the element
      Minor
      Bug 59379 - FC_PRGNAME
      Remove unused checks for common functions
      Minor
      Fix fc-cache crash caused by looking up NULL object incorrectly
      Fix FC_PRGNAME default
      Fix readlink failure
      Accept digits as part of OpenType script tags
      Fix crash with FcConfigSetCurrent(NULL)

Christoph J. Thompson (1):
      Use the PKG_INSTALLDIR macro.

Colin Walters (1):
      build: Only use PKG_INSTALLDIR if available

Quentin Glidic (2):
      test: Use SH_LOG_COMPILER and AM_TESTS_ENVIRONMENT
      Use LOG_COMPILER and AM_TESTS_ENVIRONMENT

2.10.91 (2.11 RC1)

Akira TAGOH (19):
      Fix a potability issue about stdint.h
      Fix build issues on clean tree
      Do not show the deprecation warning if it is a symlink
      Fix a typo
      Fix the wrong estimation for the memory usage information in fontconfig
      Remove the duplicate null-check
      Remove the dead code
      clean up
      Fix a typo that accessing to the out of array
      Fix a memory leak
      Check the system font to be initialized
      Missing header file for _mkdir declaration
      Clean up the unused variable
      Bug 47705 - Using O_CLOEXEC
      missing header file to declare _mkdir
      Fix a build fail on mingw
      Fix a typo in the manpages template
      Bug 29312 - RFE: feature to indicate which characters are missing to satisfy the language support
      Update the date in README properly

Behdad Esfahbod (73):
      Fix typo
      Parse matrices of expressions
      Fix compiler warnings
      Fix unused-parameter warnings
      Fix more warnings
      Fix sign-compare warnings
      Fix warning
      Fix more warnings
      Fixup from 4f6767470f52b287a2923e7e6d8de5fae1993f67
      Remove memory accounting and reporting
      Allow target="font/pattern/default" in <name> elements
      Don't warn if an unknown element is used in an expression
      Unbreak build when FC_ARCHITECTURE is defined
      Remove unneeded stuff
      Enable fcarch assert checks even when FC_ARCHITECTURE is explicitly given
      Make tests run on Windows
      Initialize matrix during name parsing
      Adjust docs for recent changes
      Warn if <name target="font"> appears in <match target="pattern">
      Make FC_DBG_OBJTYPES debug messages into warnings
      Refuse to set value to unsupported types during config too
      Add NULL check
      Don't crash in FcPatternDestroy with NULL pattern
      Don't crash in FcPatternFormat() with NULL pattern
      Minor
      Whitespace
      Deprecate FcName(Un)RegisterObjectTypes / FcName(Un)RegisterConstants
      Use a static perfect hash table for object-name lookup
      Switch .gitignore to git.mk
      Remove shared-str pool
      Fix build stuff
      Add build stuff for threadsafety primitives
      Add thread-safety primitives
      Make refcounts, patterns, charsets, strings, and FcLang thread-safe
      Make FcGetDefaultLang and FcGetDefaultLangs thread-safe
      Make FcInitDebug() idempotent
      Make FcDefaultFini() threadsafe
      Refactor; contain default config in fccfg.c
      Minor
      Make default-FcConfig threadsafe
      Minor
      Make FcCacheIsMmapSafe() threadsafe
      Minor
      Make cache refcounting threadsafe
      Add a big cache lock
      Make random-state initialization threadsafe
      Make cache hash threadsafe
      Make FcDirCacheDispose() threadsafe
      Make fcobjs.c thread-safe
      Warn about undefined/invalid attributes during config parsing
      Fixup fcobjs.c
      Remove FcSharedStr*
      Fix compiler warnings
      Minor
      Fix build and warnings on win32
      Use CC_FOR_BUILD to generate source files
      Fix more warnings.
      Trying to fix distcheck
      Fix build around true/false
      Work around Sun CPP
      Really fix cross-compiling and building of tools this time
      Second try to make Sun CPP happy
      Ugh, add Tools.mk
      Minor
      Don't use blanks for fc-query
      Remove FcInit() calls from tools
      Add 10-scale-bitmap-fonts.conf and enable by default
      Oops, add the actual file
      Fix pthreads setup
      Fix memory corruption!
      Add pthread test
      Add atomic ops for Solaris
      Make linker happy

Jon TURNEY (1):
      Fix build when srcdir != builddir

2.10.2

Akira TAGOH (13):
      Bug 53585 - Two highly-visible typos in src/fcxml.c
      Fix for libtoolize's warnings
      Bug 54138 - X_OK permission is invalid for win32 access(..) calls
      Bug 52573 - patch required to build 2.10.x with oldish GNU C library headers
      deal with warnings as errors for the previous change
      Fix wrongly squashing for the network path on Win32.
      Fix syntax errors in fonts.dtd.
      autogen.sh: Add -I option to tell aclocal a place for external m4 files
      Use automake variable instead of cleaning files in clean-local
      Bug 56531 - autogen.sh fails due to missing 'm4' directory
      Bug 57114 - regression on FcFontMatch with namelang
      Update CaseFolding.txt to Unicode 6.2
      Bug 57286 - Remove UnBatang and Baekmuk Batang from monospace in 65-nonlatin.conf

Behdad Esfahbod (1):
      Fix N'ko orthography

Jeremy Huddleston Sequoia (1):
      Remove _CONFIG_FIXUPS_H_ guards, so multiple includes of "config.h" result in the correct values

2.10.1

Akira TAGOH (2):
      Fix a typo in fontconfig.pc
      Install config files first

2.10.0

Akira TAGOH (5):
      Bug 34266 - configs silently ignored if libxml2 doesn't support SAX1 interface
      Update CaseFolding.txt to Unicode 6.1
      Fix a build fail with gcc 2.95, not supporting the flexible array members.
      Bump libtool revision
      Update INSTALL

2.9.92 (2.10 RC2)

Akira TAGOH (9):
      Bug 50835 - Deprecate FC_GLOBAL_ADVANCE
      Fix a typo and build fail.
      Fix a build fail on MINGW
      Fix the fail of make install with --disable-shared on Win32
      clean up the lock file properly on even hardlink-not-supported filesystem.
      Rename configure.in to configure.ac
      Bug 18726 - RFE: help write locale-specific tests
      Bump libtool revision
      Update INSTALL

Marius Tolzmann (2):
      Fix newline in warning about deprecated config includes
      Fix warning about deprecated, non-existent config includes

2.9.91 (2.10 RC1)

Akira TAGOH (60):
      [doc] Update the path for cache files and the version.
      [doc] Update for cachedir.
      Revert "Fix a build fail on some environment."
      Revert "Fix a build fail on some environment"
      Fix a build issue due to the use of non-portable variables
      Get rid of the prerequisites from the sufix rules
      Bug 39914 - Please tag the cache directory with CACHEDIR.TAG
      fc-cache: improvement of the fix for Bug#39914.
      fcmatch: Set FcResultMatch at the end if the return value is valid.
      Bug 47703 - SimSun default family
      Bug 17722 - Don't overwrite user's configurations in default config
      Fix a memory leak in FcDirScanConfig()
      Bug 17832 - Memory leaks due to FcStrStaticName use for external patterns
      fcpat: Increase the number of buckets in the shared string hash table
      Fix the hardcoded cache file suffix
      Move workaround macros for fat binaries into the separate header file
      Bug 48020 - Fix for src/makealias on Solaris 10
      Bug 24729 - [ne_NP] Fix ortho file
      doc: Add contains and not_contains operators and elements
      Use AC_HELP_STRING instead of formatting manually
      Use pkgconfig to check builddeps
      Bug 29341 - Make some fontconfig paths configurable
      Bug 22862 - <alias> ignores <match> <test>s
      Bug 26830 - Add search for libiconv non-default directory
      Bug 28491 - Allow matching on FC_FILE
      Bug 48573 - platform without regex do not have also REG_XXX defines
      Bug 27526 - Compatibility fix for old windows systems
      Add --with-expat, --with-expat-includes and --with-expat-lib back.
      doc: Fix a typo of the environment variable name.
      Bug 25151 - Move cleanCacheDirectory() from fc-cache.c into
      Rework to avoid adding the unexpected value to ICONV_CFLAGS and ICONV_LIBS
      Fix a build issue again when no regex functions available
      C++11 requires a space between literal and identifier
      Bug 47721 - Add ChromeOS fonts to 30-metric-aliases.conf
      Create CACHEDIR.TAG when fc-cache is run or only when the cache directory is created at the runtime.
      Add --enable-iconv option to configure
      Bug 27765 - FcMatch() returns style in wrong language
      Disable iconv support anyway...
      Bug 39278 - make usage of mmap optional
      Output more verbose debugging log to show where to insert the element into the value list
      fonts.conf: keeps same binding for alternatives
      fcarch.c: get rid of the duplicate definition of FC_MAX
      Bug 19128 - Handling whitespace in aliases
      Bug 20411 - fontconfig doesn't match FreeDesktop directories specs
      Correct the example
      Bug 33644 - Fontconfig doesn't match correctly in <test>
      fcatomic: fallback to create a directory with FcAtomicLock
      Move statfs/statvfs wrapper to fcstat.c and add a test for the mtime broken fs
      Fix the build fail on Solaris
      Fix a typo and polish the previous change
      Fix the wrong estimation for the memory usage information in fontconfig
      Bug 32853 - Export API to get the default language
      fcdefault: fallback if the environment variables are empty
      Add the default language to the pattern prior to do build the substitution
      fcdefault: no need to set FC_LANG in FcDefaultSubstitute() anymore
      fcdefault: Add the lang object at FcConfigSubstituteWithPat() only when kind is FcMatchPattern
      Bug 50525 - superfluous whitespace in the style
      Bump libtool revision
      doc: Fix distcheck error again...
      Generate bzip2-compressed tarball too

Jeremy Huddleston (1):
      fcarch: Check for architecture signature at compile time rather than configure time

Keith Packard (3):
      Use posix_fadvise to speed startup
      Extra ',' in AC_ARG_WITH(arch causes arch to never be autodetected
      Deal with architectures where ALIGNOF_DOUBLE < 4

Mark Brand (1):
      fix building for WIN32

Mikhail Gusarov (2):
      Move FcStat to separate compilation unit
      Fix cache aging for fonts on FAT filesystem under Linux

2.9

Akira TAGOH (28):
      Add charset editing feature.
      add some document for range and charset.
      Add the range support in blank element
      Add editing langset feature.
      add some documents
      Bug 24744 - No n'ko orthography
      Remove the unnecessary comment in ks.orth
      Bug 32965 - Asturian (ast-ES) language matching missing ḷḷḥ
      Add a missing file
      Bug 35517 - Remove Apple Roman cmap support
      Bug 40452 - Running 'fc-match --all' core dumps when no fonts are installed
      Get rid of the unexpected family name
      Bug 44826 - <alias> must contain only a single <family>
      Bug 46169 - Pointer error in FcConfigGlobMatch
      Do not update stream->pos when seeking is failed.
      Bug 27385 - lcdfilter settings for freetype-2.3.12 not available in fontconfig-2.8.0
      Add brx.orth and sat.orth
      Bug 41694 - FcCache functions have random-number-generator side effects
      Bug 23336 - unable to display bitmap-only (SFNT) TrueType or OpenType
      Check null value for given object to avoid possibly segfaulting
      Bug 19128 - Handling whitespace in aliases
      Fix distcheck error
      Update the version info
      Update to detect the uncommited changes properly
      Fix a build issue
      Fix a build fail on some environment
      Fix a build fail on some environment.
      Get rid of $< from Makefile.am

Alan Coopersmith (1):
      Fix compiler warnings

Behdad Esfahbod (54):
      [fc-cache] Document -r argument in man page
      [doc] Fix typo
      Bug 25508 configure assumes bash > 2.0 is on system
      Update INSTALL
      Add note about autogen.sh to INSTALL
      Fix doc typo
      More doc typo fixes
      Bug 18886 installation crashes if fontconfig already installed
      Bug 26157 Solaris/Sun C 5.8: compilation of 2.8.0 and 2.7.3 fails
      Bug 25152 Don't sleep(2) if all caches were uptodate
      Don't include unistd.h in fontconfig.h
      Accept TT_PLATFORM_MICROSOFT, TT_MS_ID_SYMBOL_CS from name table
      Whitespace
      More whitespace
      Remove all training whitespaces
      Fix comment
      Add fc-pattern cmdline tool
      Bug 29338 - fc-pattern.sgml, open para tag
      Add comments
      Bug 29995 - fc-cat does not invoke FcFini()
      Add new public API: FcCharSetDelChar()
      [fc-lang] Support excluding characters
      Bug 24729 - [ne_NP] Fix ortho file
      Add more copyright owners
      Cleanup copyright notices to replace "Keith Packard" with "the author(s)"
      Fix returned value
      Bug 28958 - lang=en matches other langs
      Make most generated-files cross-compiling-safe
      Make fc-arch stuff cross-compiling-safe
      Bump version
      Allow editing charset and lang in target="scan"
      Add <range> support for <blank> into the DTD
      Skip <range> elements with begin > end
      Doc nit
      Fix assertion failure on le32d4
      Remove AM_MAINTAINER_MODE
      Update CaseFolding.txt to Unicode 6.0
      Remove --enable-maintainer-mode from autogen.sh
      Bug 20113 - Uighur (ug) orthography incomplete
      Bug 30566 - fcformat.c:interpret_enumerate() passes uninitialized idx to FcPatternGetLangSet()
      Mark constant strings as constant
      More doc typo fixes
      Always define FcStat as a function
      Fix warning
      Bug 35587 - Add padding to make valgrind and glibc not hate each other
      [.gitignore] Update
      Bug 36577 - Updating cache with no-bitmaps disables bitmap fonts...
      Bug 26718 - "fc-match sans file" doesn't work
      Switch fc-match to use FcPatternFormat()
      Switch fc-cat to use FcPatternFormat()
      Fix stupid bug in FcFontSort()
      Bug 41171 - Invalid use of memset
      Fix parallel build
      Add FcPublic to FcLangSetUnion and FcLangSetSubtract

Brad Hards (1):
      Documentation fixes

Jeremy Huddleston (2):
      fontconfig.pc: Add variables for confdir and cachedir
      fontconfig.pc.in: Add sysconfdir, localstatedir, and PACKAGE

Jinkyu Yi (1):
      Bug 42423 - make default Korean font from Un to Nanum

MINAMI Hirokazu (1):
      Bug 43406 - typo of Japanese font name in conf.d/65-nonlatin.conf

Mike Frysinger (9):
      FcStrPlus: optimize a little
      delete unused variables
      FcStat: change to FcChar8 for first arg
      fc-cat: fix pointer warning
      FcName{,Get}Constant: constify string input
      fc-{list,match}: constify format string
      fix build warnings when using --with-arch
      FcObjectValidType: tweak -1 checking
      makealias: handle missing funcs better

Parag Nemade (2):
      Bug 25651 - Add ortho file for locale brx_IN
      Bug 25650 - Add ortho file for locale sat_IN

Pravin Satpute (4):
      Bug 27195 - need updates to ks.orth file
      Bug 43321 - Required corrections in urdu.orth file
      Bug 25653 - Add ortho file for locale doi_IN
      Bug 25652 - Add ortho file for locale mni_IN

2.8

Behdad Esfahbod (24):
      Clarify default confdir and cachedir better.
      Move FcAlign to fcint.h
      [fc-arch] Add FcAlign to arch signature
      [int] Define MIN/MAX/ABS macros
      Bump cache version up from 2 to 3 and fix FcLangSet caching/crash
      Remove unused macros
      [int] Remove fc_storage_type() in favor of direct access to v->type
      [int] Remove fc_value_* macros that did nothing other than renaming
      Enable automake silent rules
      [int] Remove more unused macros
      [xml] Remove unused code
      [arch] Try to ensure proper FcLangSet alignment in arch
      [lang] Fix serializing LangSet from older versions
      Make sure fclang.h and fcarch.h are built
      Remove bogus comment
      [fc-glyphname] Cleanup Makefile.am
      [src] Create fcglyphname.h automatically
      [fc-glyphname] Rename internal arrays to prefix with _fc_
      Clean up Makefile's a bit
      [fc-glyphname] Remove Adobe glyphlist
      [fc-case] Update CaseFolding.txt to Unicode 5.2.0
      [fc-arch] Beautify the arch template
      [fc-arch] Rename architecture names to better reflect what they are
      Bump libtool revision in preparation for release

2.7.3

Behdad Esfahbod (2):
      Use default config in FcFileScan() and FcDirScan()
      Bump libtool version in preparation for release

Roozbeh Pournader (2):
      Correct Ewe (ee) orthography to use U+025B (bug #20711)
      Updated Arabic, Persian, and Urdu orthographies

2.7.2

Behdad Esfahbod (6):
      Improve charset printing
      [ja.orth] Comment out FULLWIDTH YEN SIGN (#22942)
      Bug 22037 - No Fonts installed on a default install on Windows Server 2003
      Bug 23419 - "contains" expression seems not working on the fontconfig rule
      Revert "Fix FcNameUnparseLangSet()" and redo it
      Bump libtool version for release

Tor Lillqvist (3):
      Fix MinGW compilation
      Fix heap corruption on Windows in FcEndElement()
      Use multi-byte codepage aware string function on Windows

2.7.1

Behdad Esfahbod (16):
      git-tag -s again
      Fix win32 build
      Replace spaces with tabs in conf files
      Remove unused ftglue code
      Add Inconsolata to monospace config (#22710)
      Fix leak with string VStack objects
      Improve libtool version parsing (#22122)
      Use GetSystemWindowsDirectory() instead of GetWindowsDirectory() (#22037)
      Remove unused macros
      Fix FcNameUnparseLangSet()
      Fix doc syntax (#22902)
      TT_MS_ID_UCS_4 is really UTF-16BE, not UTF-32
      [doc] Add ~/fonts.conf.d to user docs
      Hardcode /etc/fonts instead of @CONFDIR@ in docs (#22911)
      Bump libtool versions that 2.7.0 (I forgot to do back then)
      Update .gitignore

Karl Tomlinson (1):
      Don't change the order of names unnecessarily (#20128)

2.7

Alexey Khoroshilov (1):
      Use human-readable file names in the docs (bug #16278)

Behdad Esfahbod (119):
      Avoid C99ism in Win32 code (#16651)
      [doc] Fix inaccuracy in FcFontRenderPrepare docs (#16985)
      When canonizing filenames, squash // and remove final / (#bug 16286)
      Add orth file for Maithili mai.orth (#15821)
      Replace RCS Id tags with the file name
      [doc] Fix signatures of FcPatternGetFTFace and FcPatternGetLangSet (#16272)
      Update Thai default families (#16223)
      Add ~/.fonts.conf.d to default config (#17100)
      [fc-match] Fix list of getopt options in --help
      Update man pages
      Add fc-query (#13019)
      Implement fc-list --verbose (#13015)
      [doc] Add const decorator for FcPatternDuplicate()
      Add FcPatternFilter() (#13016)
      [doc] Document that a zero rescanInterval disables automatic checks (#17103)
      Get rid of $Id$ tags
      [doc] Fix signature of FcConfigHome()
      Fix docs re 'orig' argument of FcPatternBuild and family
      Update sr.orth to actul subset of Cyrillic used by Serbian (#17208)
      Add Sindhi .orth file. (#17140)
      Add WenQuanYi fonts to default conf (#17262, from Mandriva)
      Handle -h and --help according to GNU Coding Standards (#17104)
      Document when config can be NULL (#17105)
      Add FcConfigReference() (#17124)
      Document how to free return value of FcNameUnparse()
      Don't leak FcValues string loaded through fcxml.c (#17661)
      Don't call FcPatternGetCharSet in FcSortWalk unless we need to (#17361)
      Fix two more doc typos
      [.gitignore] Update
      Cleanup symlinks in "make uninstall" (bug #18885)
      [fccache] Consistently use FcStat() over stat() (bug #18195)
      Consistently use FcStat() over stat() in all places
      Use __builtin_popcount() when available (bug #17592)
      Fix compile with old FreeType that doesn't have FT_Select_Size() (bug #17498)
      Implement fc-list --quiet ala grep (bug #17141)
      [65-fonts-persian.conf] Set foundry in target=scan instead of target=font
      Don't use identifier named complex
      Explicitly chmod() directories (bug #18934)
      Remove special-casing of FC_FILE in FcPatternPrint()
      [.gitignore] Update
      Implement FcPatternFormat and use it in cmdline tools (bug #17107)
      Fix comparison of family names to ignore leading space properly
      [fcmatch.c] Fix debug formatting
      [fcmatch] Use larger multipliers to enforce order
      [fcmatch] When matching, reserve score 0 for when elements don't exist
      [fcmatch] Move FcFontSetMatch() functionality into FcFontSetMatchInternal()
      [doc] Note that fontset returned by FcConfigGetFonts should not be modified
      Make FcCharSetMerge() public
      Don't use FcCharSetCopy in FcCharSetMerge
      Oops.  Fix usage output.
      Revive FcConfigScan() (bug #17121)
      Add fc-scan too that runs FcFileScan/FcDirScan
      Oops, fix FcPatternFilter
      [fc-match] Accept list of elements like fc-list (bug #13017)
      Cleanup all manpage.* files
      [fcmatch] Fix crash when no fonts are available.
      [fcfreetype] Fix typo in GB2312 encoding name string (#19845)
      Add ICONV_LIBS to fontconfig.pc.in (#19606)
      [win32] Fix usage of GetFullPathName()
      [win32] Expand "APPSHAREFONTDIR" to ../share/fonts relative to binary location
      [win32] Do not remove leading '\\' such that network paths work
      [fccache] Make sure the cache is current when reusing from open caches
      Update Sinhala orthography (#19288)
      [cache] After writing cache to file, update the internal copy to reflect this
      Further update Sinhala orthography (#19288)
      [fcformat] Add support for width modifiers
      [fcformat] Refactor and restructure code for upcoming changes
      [fcformat] Add support for subexpressions
      [fcformat] Add element filtering and deletion
      [fcformat] Add conditionals
      [fcformat] Add simple converters
      [fcformat] Implement 'cescape', 'shescape', and 'xmlescape' converters
      [FcStrBuf] better handle malloc failure
      [fcformat] Add value-count syntax
      [fcformat] Implement 'delete', 'escape', and 'translate' filter functions
      [fcformat] Start adding builtins
      [fcformat] Refactor code to avoid malloc
      [fcformat] Add support for builtin formats
      [fcformat] Support indexing simple tags
      [fcformat] Support 'default value' for simple tags
      [fcformat] Implement array enumeration
      [fclang] Implement FcLangSetGetLangs() (#18846)
      [fcformat] Enumerate langsets like we do arrays of values
      [fcformat] Add a 'pkgkit' builtin that prints tags for font packages
      [fcformat] Add list of undocumented language features
      [fc-lang] Continue parsing after an "include" (#20179)
      Fix Fanti (fat) orth file (#20390)
      Fix Makefile's to not create target file in case of failure
      [fcstr.c] Embed a static 64-byte buffer in FcStrBuf
      [fcstr,fcxml] Don't copy FcStrBuf contents when we would free it soon
      [fcxml] Don't allocate attr array if there are no attributes
      [fcxml] Embed 8 static FcPStack objects in FcConfigParse
      [fcxml] Embed 64 static FcVStack objects in FcConfigParse
      [fcxml.c] Embed a static 64-byte attr buffer in FcPStack
      Call git tools using "git cmd" instead of "git-cmd" syntax
      Replace 'KEITH PACKARD' with 'THE AUTHOR(S)' in license text in all files
      [fcformat] Fix default-value handling
      Document FcPatternFormat() format
      [Makefile.am] Don't clean ChangeLog in distclean
      Revert "[conf] Disable hinting when emboldening (#19904)" (#20599)
      [fc-lang] Fix bug in country map generation
      [fcstr] Remove unused variable
      [fc-lang] Make LangSet representation in the cache files stable
      [fc-cache] Remove obsolete sentence from man page
      Detect TrueType Collections by checking the font data header
      Mark matchers array const (#21935)
      Use/prefer WWS family/style (name table id 21/22)
      Simplify FcValueSave() semantics
      Add XXX note about Unicode Plane 16
      Always set *changed in FcCharsetMerge
      [charset] Grow internal FcCharset arrays exponentially
      Remove unused prototypes and function
      [xml] Centralize FcExpr allocation
      [xml] Mark more symbols static
      [xml] Allocate FcExpr's in a pool in FcConfig
      [xml] Intern more strings
      Bug 22154 -- fontconfig.pc doesn't include libxml2 link flags
      Fix distcheck
      Remove keithp's GPG key id

Benjamin Close (1):
      Remove build manpage logfile if it exists

Chris Wilson (1):
      Reduce number of allocations during FcSortWalk().

Dan Nicholson (1):
      Let make expand fc_cachedir/FC_CACHEDIR (bug #18675)

Harald Fernengel (1):
      Don't use variables named 'bool' (bug #18851)

Harshula Jayasuriya (1):
      Fix Sinhala coverage (bug #19288)

Karl Tomlinson (1):
      Change FcCharSetMerge API

Mike FABIAN (1):
      [conf] Disable hinting when emboldening (#19904)

Peter (1):
      Make sure alias files are built first (bug 16464)

Rahul Bhalerao (1):
      Add config for new Indic fonts (bug #17856)

Roozbeh Pournader (60):
      Correct Sindhi orthography to use Arabic script (bug #17140)
      Remove Sinhala characters not in modern use (bug #19288)
      Add Filipino orth, alias Tagalog to Filipino (bug #19846)
      Split Mongolian orth to Mongolia and China (bug #19847)
      Fix doubly encoded UTF-8 in comments (bug #19848)
      Change Turkmen orth from Cyrillic to Latin (bug #19849)
      Rename Venda from "ven" to "ve" (bug #19852)
      Rename "ku" to "ku_am", add "ku_iq" (bug #19853).
      Add Kashubian (csb) orth file (bug #19866)
      Add Malay (ms) orthography (bug #19867)
      Add Kinyarwanda (rw) orthography (bug #19868)
      Add Upper Sorbian (hsb) orthography (bug #19870)
      Add Berber orthographies in Latin and Tifinagh scripts (bug #19881)
      Renamed az to az_az (bug #19889)
      Rename Igbo from "ibo" to "ig" (bug #19892)
      Remove punctuation symbols from Asturian orthography (bug #19893)
      Add Chhattisgarhi (hne) orthography (bug #19891)
      Use newly added Cyrillic letters for Kurdish (bug #20049)
      Add Kurdish in Turkey (ku_tr) orthography (bug #19891)
      Add Aragonese (an) orthography (bug #19891)
      Add Haitian Creole (ht) orthography (bug #19891)
      Ad Ganda (lg) orthography (bug #19891)
      Add Limburgan (li) orthography (bug #19891)
      Add Sardinian (sc) orthography (bug #19891)
      Add Sidamo (sid) and Wolaitta (wal) orthographies (bug #19891)
      Fix Bengali (bn) and Assamese (as) orthographies (bug #22924)
      Remove Euro Sign from all orthographies (bug #19865)
      Add Ottoman Turkish (ota) orthography (bug #20114)
      Divide Panjabi (pa) to that of Pakistan and India (bug #19890)
      Add Blin (byn) orthography (bug #19891)
      Add Papiamento (pap_aw, pap_an) orthographies (bug #19891)
      Add Crimean Tatar (crh) orthography (bug #19891)
      Switch Uzbek (uz) orthography to Latin (bug #19851)
      Update Azerbaijani in Latin (az_az) to present usage (bug #20173)
      Rename Avaric orthography from 'ava' to 'av' (bug #20174)
      Rename Bambara orthography from 'bam' to 'bm' (bug #20175)
      Rename Fulah orthography from 'ful' to 'ff' (bug #20177)
      Change Kashmiri (ks) orthography to Arabic script (bug #20200)
      Tighten Central Khmer (km) orthography (bug #20202)
      Remove digits and symbols from some Indic orthographies (bug #20204)
      Add Divehi (dv) orthography (bug #20207)
      Extend Crimean Tatar (crh) orthography (bug #19891)
      Update Serbo-Croatian (sh) orthography (bug #20368)
      Add Ewe (ee) orthography (bug #20386)
      Add Herero (hz) orthograhy (bug #20387)
      Add Akan (ak) and Fanti (fat) orthographies (bug #20390)
      Added Quechua (qu) orthography (bug #20392)
      Add Sango (sg) orthography (bug #20393)
      Add Tahitian (ty) orthography (bug #20391)
      Add Navajo (nv) orthography (bug #20395)
      Add Rundi (rn) orthography (bug #20398)
      Add Zhuang (za) orthography (bug #20399)
      Add orthographies for Oshiwambo languages (bug #20401)
      Add Shona (sn) orthography (bug #20394)
      Add Sichuan Yi (ii) orthography (bug #20402)
      Add Javanese (jv) orthography (bug #20403)
      Add Nauru (na) orthography (bug #20418)
      Add Kanuri (kr) orthography (bug #20438)
      Add Sundanese (su) orthography (bug #20440)
      Reorganize Panjabi/Punjabi and Lahnda orthographies (bug #19890)

Serge van den Boom (1):
      Correctly handle mmap() failure (#21062)

2.6

2.5.93 (2.6 RC3)

Alexey Khoroshilov (1):
      Fix FcStrDirname documentation. (bug 16068)

Behdad Esfahbod (1):
      Persian conf update. (bug 16066).

Evgeniy Stepanov (1):
      Fix index/offset for 'decorative' matcher. Bug 15890.

Glen Low (1):
      Fix Win32 build error: install tries to run fc-cache locally (bug 15928).

Keith Packard (8):
      Call FcFini to make memory debugging easier
      Fix a few memory tracking mistakes.
      Add extended, caps, dunhill style mappings.
      Freetype 2.3.5 (2007-jul-02) fixes indic font hinting. re-enable (bug 15822)
      Add a copy of dolt.m4 to acinclude.m4.
      Libs.private needs freetype libraries
      Oops. Fix for bug 15928 used wrong path for installed fc-cache.
      Ignore empty <dir></dir> elements

Neskie Manuel (1):
      Add Secwepemctsin Orthography. Bug 15996.

Sayamindu Dasgupta (1):
      FcConfigUptoDate breaks if directory mtime is in the future. Bug 14424.

2.5.92 (2.6 RC2)

Carlo Bramini (1):
      Add FreeType-dependent functions to fontconfig.def file. (bug 15415)

Changwoo Ryu (1):
      Korean font in the default config - replacing baekmuk with un (bug 13569)

Dennis Schridde (1):
      Proper config path for static libraries in win32

Eric Anholt (1):
      Fix build with !ENABLE_DOCS and no built manpages.

Frederic Crozat (1):
      Merge some of Mandriva configuration into upstream configuration. Bug 13247

Keith Packard (11):
      Use DOLT if available
      Work around for bitmap-only TrueType fonts that are missing the glyf table.
      Remove size and dpi values from bitmap fonts. Bug 8765.
      Add some sample cursive and fantasy families.
      Add --all flag to fc-match to show the untrimmed list. Bug 13018.
      Remove doltcompile in distclean
      Use of ":=" in src/Makefile.am is unportable (bug 14420)
      Make fc-match behave better when style is unknown (bug 15332)
      Deal with libtool 2.2 which doesn't let us use LT_ variables. (bug 15692)
      Allow for RC versions in README update
      git ignore doltcompile

Ryan Schmidt (1):
      fontconfig build fails if "head" is missing or unusable (bug 14304)

Sylvain Pasche (1):
      Fontconfig options for freetype sub-pixel filter configuration

2.5.91 (2.6 RC1)

Hongbo Zhao (1):
      Not_contain should use strstr, not strcmp on strings. (bug 13632)

Keith Packard (11):
      Move conf.avail/README to conf.d/README (bug 13392)
      Fix OOM failure case in FcPStackPush.
      Remove freetype requirement for build-time applications.
      Include fcftaliastail.h so that the freetype funcs are exported.
      Eliminate references to freetype from utility Makefile.am's
      Distribute new fcftint.h file
      Create new-version.sh to help with releases, update INSTALL instructions
      Distribute khmer font aliases
      Add more files to .gitignore
      new-version.sh was mis-editing files
      git-tag requires space after -m flag

2.5

Keith Packard (4):
      Document several function return values (Bug 13145).
      Document that Match calls FcFontRenderPrepare (bug 13162).
      Document that FcConfigGetFonts returns the internal fontset (bug 13197)
      Revert "Remove fcprivate.h, move the remaining macros to fcint.h."

Tor Lillqvist (1):
      Workaround for stat() brokenness in Microsoft's C library (bug 8526)

2.4.92 (2.5 RC2)

Behdad Esfahbod (14):
      Make fc-match --sort call FcFontRenderPrepare.
      Port fonts-persian.conf to new alias syntax with binding="same"
      Fix trivial bugs in edit-sgml.c
      Add FcGetLangs() and FcLangGetCharSet().
      Add/update config files from Fedora.
      Split 40-generic.conf into 40-nonlatin.conf and 45-latin.conf
      Use binding="same" in 30-urw-aliases.conf and remove duplicate entries.
      Remove redundant/obsolete comments from conf files.
      Remove 20-lohit-gujarati.conf.  It's covered by 25-unhint-nonlatin.conf now.
      Oops, fix Makefile.am.
      Remove 25-unhint-nonlatin.conf from default configuration by not linking it.
      Fix documented conf-file naming format in README
      Remove list of available conf files from README.
      Simplify/improve 30-metric-aliases.conf

Keith Packard (25):
      Also check configDirs mtimes in FcConfigUptoDate
      Respect "binding" attribute in <alias> entries.
      Correct documentation for FcAtomicLock (Bug 12947).
      Remove fcprivate.h, move the remaining macros to fcint.h.
      Correct documentation for FcConfigUptoDate (bug 12948).
      Document skipping of fonts from FcFileScan/FcDirScan.
      Make file_stat argument to FcDirCacheLoadFile optional.
      Clean up exported names in fontconfig.h.
      Track line numbers in sgml edit tool input.
      Typo error in function name: Inverval -> interval
      Don't check cache file time stamps when cleaning cache dir.
      Use FcLangDifferentTerritory instead of FcLangDifferentCountry.
      Verify documentation covers exposed symbols.
      Document previously undocumented functions. (bug 12963)
      Update documentation for FcStrCopyFilename (bug 12964).
      Update documentation for stale FcConfigGetConfig function.
      Have FcConfigSetCurrent accept the current configuration and simply return
      Remove references to FcConfigParse and FcConfigLoad.
      Replace incorrect documentation uses of 'char' with 'FcChar8' (bug 13002).
      Fix formatting syntax in doc/fccache.fncs
      Generate fccache.sgml, fcdircache.sgml and fclangset.sgml.
      Formatting syntax mistake in doc/fclangset.fncs.
      Link new function documentation into the fontconfig-devel.sgml
      Ignore new generated documentation
      Export FcConfig{G,S}etRescanInverval from .so, mark as deprecated.

2.4.91 (2.5 RC1)

Behdad Esfahbod (1):
      Update CaseFolding.txt to Unicode 5.1.0

Dwayne Bailey (1):
      Add/fix *.orth files for South African languages

Hideki Yamane (1):
      Handle Japanese fonts better. (debian bug #435971)

Keith Packard (32):
      rehash increment could be zero, causing rehash infinite loop.
      Work around FreeType bug when glyph name buffer is too small.
      Free temporary string in FcDirCacheUnlink (Bug #11758)
      Fix ChangeLog generation to avoid circular make dependency
      Store font directory mtime in cache file.
      Comment about mmaping cache files was misleading.
      Make FC_FULLNAME include all fullname entries, elide nothing. [bug 12827]
      Remove unneeded call to access(2) in fc-cache.
      Improve verbose messages from fc-cache.
      Verbose message about cleaning directories was imprecise
      Don't use X_OK bit when checking for writable directories (bug 12438)
      Have fc-cache remove invalid cache files from cache directories.
      FcConfigParseAndLoad doc was missing the last param.
      Place language name in constant array instead of pointer.
      Must not insert cache into hash table before completely validating.
      Eliminate relocations for glyph name table.
      Eliminate relocations from FcCodePageRange structure (bug 10982).
      Leave generated headers out of distribution (bug 12734).
      Move <cachedir> elements to the end of fonts.conf.
      Add BRAILLE PATTERN BLANK to list of blank glyphs.
      Replace makealias pattern with something supported by POSIX grep (bug 11083)
      FcInit should return FcFalse when FcInitLoadConfigAndFonts fails. (bug 10976)
      There is no U+1257 (bug 10899).
      Spelling errors in documentation. (bug 10879).
      Oops. Left debugging printf in previous commit.
      Handle UltraBlack weight.
      Fix parallel build in fontconfig/docs (bug 10481).
      Distribute man source files for command line programs (bug 9678).
      Ensure weight/slant values present even when style is supplied (bug 9313).
      fontconfig needs configure option to use gnu iconv (bug 4083).
      Match 'ultra' on word boundaries to detect ultra bold fonts. (bug 2511)
      Build fix for Solaris 10 with GCC.

Mike FABIAN (1):
      Avoid crashes if config files contain junk.

Stephan Kulow (1):
      Make FcPatternDuplicate copy the binding instead of always using Strong.

Tilman Sauerbeck (2):
      Store FcNoticeFoundries in read-only memory.
      Store FcVendorFoundries in read-only memory.

2.4.2

Han-Wen Nienhuys:
      FcStrCanonFileName buggy for mingw. (bug 8311)
      More fixes for Win32 building (bug 8311)

Kean Johnston:
      Don't use varargs CPP macros in fccache.c. (bug 8733)

Keith Packard:
      Remove documentation for non-existant FcConfigNormalizeFontDir.
      Build fontconfig.def from header files when needed.
      Detect and use available random number generator (bug 8308)
      Add sparc64 architecture string.
      FcStrCanonAbsoluteFilename should be static.
      Use explicit platform/nameid order when scanning ttf files.
      Warn (and recover) from config file without <cachedir> elements.
      Avoid writing uninitialized structure pad bytes to cache files.
      Fix grep pattern in makealias to work on non-Gnu grep (bug 8368).
      Add FcFreeTypeQueryFace external API. Bug #7311.
      Segfault scanning non-font files. Disallow scan edit of user vars. (#8767)
      Add space between type and formal in devel man pages (bug 8935)

Mike FABIAN:
      Do not clean cache files for different architectures

Peter Breitenlohner:
      A VPATH build of fontconfig-2.4.1 fails for various reasons. Bug 8933.
      Use <literal> instead of <sgmltag> when documenting fonts.conf. Bug 8935.
      Fix fc-cat documentation (bug 8935).


2.4.1

Keith Packard:
      Update installation notes for 2.4 base.
      Add ppc64 signature. Bug 8227
      Add signatures for m68k and mipsel (thanks debian buildd)
      Add warning flags to fc-cache build. Clean up warnings in fc-cache.
      Reimplement FcConfigAppFontAddDir; function was lost in 2.4.0.

2.4.0

David Turner:
      Replace character discovery loop with simpler, faster version.

James Cloos:
      Move files from conf.d to conf.avail
      Standardize conf.avail number prefixing convention
      Support all five possibilities for sub-pixel
      Move user and local conf file loading into conf.avail files
      Number the remaining conf.avail files
      Update Makefile.am to match conf.avail changes
      Replace load of conf.d in fonts.conf.in
      Make room for chunks from fonts.conf in conf.avail
      Re-order old conf.d files
      Move some section from fonts.conf into conf.avail files
      Update Makefile.am files
      Make conf.avail and conf.d work

Keith Packard:
      Create fc_cachedir at install time. Bug 8157.
      Reference patterns in FcCacheCopySet.
      Replace gnu-specific sed command with simple grep.
      Attempt to fix makealias usage for build on Mac OS X.
      Accept locale environment variables that do not contain territory.
      Merge branch 'jhcloos'
      Insert newly created caches into reference data structure.
      Add XML headers to new conf files. Move link make commands to conf.avail dir
      Rename conf.avail to conf.d
      Fix conf.d directory sorting.
      Include cachedir in fonts.dtd.
      Don't display tests for DESTDIR on make install.
      Split much of the configuration into separate files. Renumber files

2.3.97

Carl Worth:
      Rename FcPatternThawAll to FcPatternFini.
      Add a configuration file that disables hinting for the Lohit Gujarati font

Keith Packard:
      Various GCC 4 cleanups for signed vs unsigned char
      Finish INSTALL changes. .gitignore ChangeLog
      Merge branch 'fc-2_4_branch' to master
      Remove all .cvsignore files
      Hide private functions in shared library. Export functionality for utilities.
      Hide FreeType glue code from library ABI.
      Can't typecheck values for objects with no known type.
      Leave cache files mapped permanently.
      Reference count cache objects.
      Make cache reference counting more efficient.
      Oops, fc-lang broke when I added cache referencing.
      Correct reference count when sharing cache file objects.
      Eliminate .so PLT entries for local symbols. (thanks to Arjan van de Ven)
      Update architecture signatures for x86-64 and ppc.
      Parallel build fix for fcalias.h and fcaliastail.h
      Charset hashing depended on uniqueness of leaves.

Patrick Lam:
      file Makefile.am was initially added on branch fc-2_4_branch.
      Modify config file to use Greek fonts before Asian fonts with Greek glyphs.
      Use libtool -no-undefined flag on all platforms.
      file ftglue.c was initially added on branch fc-2_4_branch.
      2005-11-23 Frederic Crozat <fcrozat@mandriva.com>: reviewed by: plam
      file 10-fonts-persian.conf was initially added on branch fc-2_4_branch.
      Sort directory entries while scanning them from disk; prevents Heisenbugs
      file ln.orth was initially added on branch fc-2_4_branch.
      Fix typos in orth files. Reported by Denis Jacquerye.
      On Windows, unlink before rename. Reported by Tim Evans.
      file fc-match.sgml was initially added on branch fc-2_4_branch.

2.3.96

Keith Packard:
      Make path names in cache files absolute (NB, cache format change) Stop
      Eliminate pattern freezing
      Add .gitignore
      Construct short architecture name from architecture signature.
      Write caches to first directory with permission. Valid cache in FcDirCacheOpen.
      Eliminate NormalizeDir. Eliminate gratuitous stat/access calls per dir.
      Add architecture to cache filename.
      Eliminate global cache. Eliminate multi-arch cache code.
      Fix up fc-cache and fc-cat for no global cache changes.
      Eliminate ./ and ../ elements from font directory names when scanning.
      Regenerate x86 line in fcarch.tmpl.h to match change in cache data.
      Add x86-64 architecture and signature.
      During test run, remove cache directory to avoid stale cache usage.
      Add ppc architecture
      Revert to original FcFontSetMatch algorithm to avoid losing fonts.
      Rework cache files to use offsets for all data structures.
      Fix build problems caused by cache rework.
      FcCharSetSerialize was using wrong offset for leaves. Make fc-cat work.
      Rework Object name database to unify typechecking and object lookup.
      Skip broken caches. Cache files are auto-written, don't rewrite in fc-cache.
      Fix fc-cat again. Sigh.
      Use intptr_t instead of off_t inside FcCache structure.
      Serialized value lists were only including one value.
      Automatically remove invalid cache files.
      With no args, fc-cat now dumps all directories.
      Revert ABI changes from version 2.3
      Change $(pkgcachedir) to $(fc_cachedir) in fc-cat and fc-cache Makefile.am
      Allow FcTypeLangSet to match either FcTypeLangSet or FcTypeString.
      Remove stale architecture signatures.
      Pass directory information around in FcCache structure. Freeze charsets.
      Fix fc-lang to use new charset freezer API.
      Fontset pattern references are relative to fontset, not array.
      Add some ignores
      Only rebuild caches for system fonts at make install time.
      Fix memory leaks in fc-cache directory cleaning code.
      Add @EXPAT_LIBS@ to Libs.private in fontconfig.pc (bug 7683)
      Avoid #warning directives on non-GCC compilers. (bug 7683)
      Chinese/Macau needs the Hong Kong orthography instead of Taiwan (bug 7884)
      Add Assamese orthography (as.orth). Bug #8050
      Really only rebuild caches for system fonts at make install time.
      Fonts matching lang not territory should satisfy sort pattern lang.
      Prefer Bitstream Vera to DejaVu families.
      Guess that mac roman names with lots of high bits are actually SJIS.
      Document FC_DEBUG values (bug 6393). Document name \ escape syntax.
      Move Free family names to bottom of respective aliases. (bug 7429)
      Unify directory canonicalization into FcStrAddFilename.
      Allow font caches to contain newer version numbers
      Add FcMatchScan to resolve Delicious font matching issues (bug #6769)
      Fix missing initialization/destruction of new 'scan' target subst list.
      Don't segfault when string values can't be parsed as charsets or langsets.
      Using uninitialized (and wrong) variable in FcStrCopyFilename.
      Oops; missed the 60-delicious.conf file.

Patrick Lam:
      Keith Packard <keithp@keithp.com>
      2006-04-27 Paolo Borelli (pborelli@katamail.com) reviewed by: plam
      2006-05-31 Yong Li (rigel863@gmail.com) reviewed by: plam, Bedhad Esfahbod
      2006-07-19 Jon Burgess (jburgess@uklinux.net) reviewed by: plam
      2006-08-04 Keith Packard (keithp@keithp.com) reviewed by: plam

2.3.95

Match 'Standard Symbols L' for 'Symbol'.  Add URW fonts as aliases for
all of the PostScript fonts.  (reported by Miguel Rodriguez).  Fix a
number of Coverity defects (Frederic Crozat).  Speed up FcFontSort
(fix suggested by Kenichi Handa).  Fix error with charsets.  Survive
missing docbook2pdf.  Compile on HP-UX, AIX, SGI and Windows (Cygwin,
MinGW).  Fix intel compiler warnings.  Fix multiarch support (don't
destroy multiarch files!)  Require pkg-config.  (Thanks Behdad; better
solution wanted for libxml2 detection!)  Fix typos in orth files and
add orth for Lingala (reported by Denis Jacquerye).  Remove debian/
directory.  Add a configuration file that disables hinting for the
Lohit Gujarati font (since the hinting distorts some glyphs quite
badly).  Sort directory entries while scanning them from disk;
prevents Heisenbugs due to file ordering in a directory (due to Egmont
Koblinger).  Fix Wine's problem with finding fonts.  (Reported by
Bernhard Rosenkraenzer.)  Fix the issues with GNU libiconv vs. libc
iconv (which especially appear on Solarii); patch by Behdad Esfahbod,
approach suggested by Tim Mooney.

2.3.94

fc-cat can take directories as input and creates old-style fonts.cache
listings.
fc-cache takes -r --really-force which blows away all old caches and
regenerates.
Robustness fixes, integer overflow fixes (notably to cache handling
code), toast broken global cache files.
Change binary format to make it compatible with static langset
information (thanks to Takashi Iwai).
Open hashed caches before fonts.cache-2 (Takashi Iwai).
Fix FcFontSetMatch's algorithm, which used to unjustly kill fonts for
not declaring certain elements (Takashi Iwai).
Fix matching bug when multiple elements match; don't use
the sum of all scores, but the best score (James Su).
Make fc-lang more friendly to Windows systems.
Remove archaic chars from Georgian charset; add Euro character to
charsets for European languages.
Fix treatment of broken PCF fonts that don't declare family names.
Pass O_BINARY to open if appropriate (reported by Doodle).
Normalize font directories to the form in which they appear in 
config files.  
Add a record of the cached directory to the cache file.
Perf optimizations (Dirk Mueller; some reported by Michael Meeks.)
Don't loop infinitely on recursive symlinks.
Make 'make distcheck' work with automake 1.6.3.
Replace 'stamp' target with mkinstalldirs.
Don't stop scanning if a directory in fonts.conf doesn't exist,
because subsequent directories might exist.
Put directory names into global cache (reported by Ronny V. Vindenes).
Treat zh-hk fonts differently from zh-tw fonts.  This patch may cause
fontconfig to treat A-X fonts differently from A-Y fonts; please mail
the fontconfig list if this causes any problems.
Fix for unaligned memory accesses (Andreas Schwab).
Fix treatment of cache directory as read from cache file; don't use
string equality to determine if we have the right file, use inode
equality.
Properly skip past dir caches that contain zero fonts, as occurs
in global caches (reported by Mike Fabian).
Print out full pathname in fc-match -v (reported by Frederic Crozat).
Fix bug where fc-match crashes when given __DUMMY__ property to
match on.

2.3.93

Create cache files in /var/cache/fontconfig with hashed filenames, if
possible, for added FHS compliance.  
Make fc-cat read both per-directory and global cache files.  
Add config file for Persian fonts from Sharif FarsiWeb, Inc.  
Major performance improvements by Dirk Mueller, Stephen Kulow, and Michael Matz at SuSE: in particular, speed up FcFontSetMatch, and inline many functions.
Fix treatment of globs in config files, broken since 2.3.2 and discovered by Mathias Clasen.
Don't use freetype internal headers (patch by Matthias Clasen).  
Further space improvements: create langsets statically, so that they can live in .rodata.
Properly align mmapped data structures to make e.g. ia64 happy.  
Bug fixes.

2.3.92

Fix corrupted caches bugs from 2.3.91 (reported by Mike Fabian).
Store only basename in the cache, reconstitute on demand 
(reported by James Cloos).
Change the rule for artificial emboldening in fonts.conf.in.  This
enables the support for artificial emboldening included in cairo
(patch by Zhe Su).
Add FC_EMBEDDED_BITMAP object type to tell Xft/Cairo whether
to load embedded bitmaps or not (patch by Jinghua Luo).
Fix GCC4 warnings (some by Behdad Esfahbod).
Support localized font family and style names; this has been reported
to break old apps like xfd, but modern (gtk+/qt/mozilla) apps work
fine (patch by Zhe Su).
Prevent fc-list from escaping strings when printing them (reported by
Matthias Clasen).
Add valist sentinel markup for FcObjectSetBuild and 
FcPatternBuild (patch by Marcus Meissner).
Add consts to variables so as to move arrays into .rodata (patch by
Ross Burton).
Modify config file to use Greek fonts before Asian fonts with
Greek glyphs. (patch by Simos Xenitellis).
Use libtool -no-undefined flag on all platforms (patch by Christian
Biesinger).

2.3.91

Use libxml2 if requested or if expat not available. (Mathias Hasselmann)
Fix multi-arch cache files: compute the position for the
block to be added using info from OrigFile, not NewFile. (plam)
Cast results of sizeof() to unsigned int to get rid of
warnings on x86_64 (reported by Matthias Clasen).
Use FcAtomic to rewrite cache files; don't unlink the fonts.cache-2
file even if there's no data to write; just write an empty cache file.
(Reported by Lubos Lunak)
Allocate room for the subdirectory names in each directory cache. 
(Reported by James Cloos)

2.3.90

Development release of mmap patch: load pattern information
directly from cache files.  (Patrick Lam)

2.3.2

Patch memory leaks in using iconv.  (Reported by Chris Capoccia)
Patch memory leaks in fc-cache. (Reported by Chris Capoccia)
Fetch bitmap glyphs to get widths during font evaluation. (keithp)
Share strings through FcObjectStaticName (Ross Burton)
Windows build updates (Tor Lillqvist)

2.3.1

Be more careful about broken GSUB/GPOS tables (Manish Singh)
Include debian packaging stuff in CVS (Josselin Mouette)
Add more conf.d examples (Keith Packard)
Make manuals build again (Keith Packard)
Johap -> Johab (Funda Wang)

2.3.0

Fix memory leak of patterns rejected by configuration (#2518)

Create prototype /etc/fonts/conf.d directory and populate it with a few
sample files.  These samples are unused as the file names don't start with
numbers.

Update documentation.

2.2.99

Verify cache for FC_FILE and FC_FAMILY in every entry (#2219)

Update blanks list from recent Unicode docs (#86)

Various small build fixes (#280, #2278, 

Documentation fixes (#2085, #2284, #2285)

Add polite typechecking to config file loader (#229)

2.2.98

Share object name strings (Michael Meeks)

Eliminate a couple of codepoints from Russian orthography (John Thacker)

Add synthetic emboldening configuration changes (Jakub Pavelek)

Change FcFontSetSort to ignore language after fonts with the requested
languages have been found. (Owen Taylor)

Add some RedHat font configuration changes (Owen Tayler).

Add full Unicode case folding support to case-ignoring string functions
(Keith Packard)

Remove Han characters from Korean orthography (Tor Andersson)

2.2.97

Fc-cache sleeps before exiting to ensure filesystem timestamps are well
ordered.

Added Punjai orthography.

The timestamp in fonts.conf is gone now.  Too many problems.

The default font path includes all of the X fonts; use selectfont/rejectfont
to eliminate bitmaps, as shown in the sample local.conf file.

<include> configuration elements may now reference a directory.  Files
in that directory matching [0-9]* are loaded in UTF-8 collating sequence order.

<selectfont> configuration added to control which fonts are used.

fontformat font pattern elements built from the FT_Get_X11_Font_Format
function in newer versions of FreeType.

'capability' list constructed from gsub/gpos and silf values in TrueType
files.

Multi-lingual names (style, family, fullname) extracted and stored with
parallel <foo>lang properties marking language.

2.2.96

Fix FcConfigUpToDate to actually check all font directories and eliminate
a typo which completely prevented it from working (Lubos Lunak
<l.lunak@suse.cz>)

Remove comma at end of FcResult enum definition for picky compilers.

2.2.95

Add FcResultOutOfMemory so FcFontSetMatch can return accurate error.

Replace MIN/MAX/ABS macros which happened to be in old FreeType releases
with FC_MIN/FC_MAX/FC_ABS macros owned by fontconfig.

2.2.94

The 2.2.93 release was prepared with a broken libtool which created
the shared library without the '.so' in the file names.

2.2.93

This is the third prerelease of fontconfig 2.3.  Significant changes from
2.2.92 are:

 o	Use new FreeType #include syntax
 o	use y_ppem field instead of 'height' in bitmap sizes rec -
 	FreeType changed the semantics.  Still uses height for
	older versions of FreeType
 o	Don't construct program manuals unless docbook is available

2.2.92

 o	make distcheck work

2.2.91

 o	Switch to SGML manuals
 o	Add FC_DUAL width spacing value
 o	Add FcFini to close out fontconfig and release all memory
 
2.2

This is the third public release of fontconfig, a font configuration and
customization library.  Fontconfig is designed to locate fonts within the
system and select them according to requirements specified by applications.

Fontconfig is not a rasterization library, nor does it impose a particular
rasterization library on the application.  The X-specific library
'Xft' uses fontconfig along with freetype to specify and rasterize fonts.

Keith Packard
keithp@keithp.com
