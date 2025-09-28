# Android's linker will actually do relative paths just fine; the problem
# is that it won't search from the current directory, only on
# /vendor/lib, /system/lib, and whatever is in LD_LIBRARY_PATH.
# The core handles that just fine, but bits of CPAN rather rightfully
# expect things like these to work:
# use lib 'foo'            # puts foo/ in @INC
# use My::Module::In::Foo; # calls dlopen() with foo/My/Module/...
#                          # which will likely fail
# So we take this route instead.
$self->{CCFLAGS} = $Config{ccflags} . ' -DDLOPEN_WONT_DO_RELATIVE_PATHS';
1;
