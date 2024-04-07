use Config;
if ($Config{osvers} == 5) {
  $self->{CCFLAGS} = $Config{ccflags};
  $self->{CCFLAGS} =~ s/-ansiposix //;
  $self->{CCFLAGS} =~ s/-D_POSIX_SOURCE /-D_POSIX_4SOURCE /;
}
