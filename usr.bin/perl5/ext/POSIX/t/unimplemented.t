#!./perl -w

use strict;
use Test::More;
use Config;

plan(skip_all => "POSIX is unavailable")
    unless $Config{extensions} =~ /\bPOSIX\b/;

require POSIX;

foreach ([atexit => 'C-specific: use END {} instead'],
	 [atof => 'C-specific, stopped'],
	 [atoi => 'C-specific, stopped'],
	 [atol => 'C-specific, stopped'],
	 [bsearch => 'not supplied'],
	 [calloc => 'C-specific, stopped'],
	 [clearerr => \'IO::Handle::clearerr'],
	 [div => 'C-specific: use /, % and int instead'],
	 [execl => 'C-specific, stopped'],
	 [execle => 'C-specific, stopped'],
	 [execlp => 'C-specific, stopped'],
	 [execv => 'C-specific, stopped'],
	 [execve => 'C-specific, stopped'],
	 [execvp => 'C-specific, stopped'],
	 [fclose => \'IO::Handle::close'],
	 [fdopen => \'IO::Handle::new_from_fd'],
	 [feof => \'IO::Handle::eof'],
	 [ferror => \'IO::Handle::error'],
	 [fflush => \'IO::Handle::flush'],
	 [fgetc => \'IO::Handle::getc'],
	 [fgetpos => \'IO::Seekable::getpos'],
	 [fgets => \'IO::Handle::gets'],
	 [fileno => \'IO::Handle::fileno'],
	 [fopen => \'IO::File::open'],
	 [fprintf => 'C-specific: use printf instead'],
	 [fputc => 'C-specific: use print instead'],
	 [fputs => 'C-specific: use print instead'],
	 [fread => 'C-specific: use read instead'],
	 [free => 'C-specific, stopped'],
	 [freopen => 'C-specific: use open instead'],
	 [fscanf => 'C-specific: use <> and regular expressions instead'],
	 [fseek => \'IO::Seekable::seek'],
	 [fsetpos => \'IO::Seekable::setpos'],
	 [fsync => \'IO::Handle::sync'],
	 [ftell => \'IO::Seekable::tell'],
	 [fwrite => 'C-specific: use print instead'],
	 [labs => 'C-specific: use abs instead'],
	 [ldiv => 'C-specific: use /, % and int instead'],
	 [longjmp => 'C-specific: use die instead'],
	 [malloc => 'C-specific, stopped'],
	 [memchr => 'C-specific: use index() instead'],
	 [memcmp => 'C-specific: use eq instead'],
	 [memcpy => 'C-specific: use = instead'],
	 [memmove => 'C-specific: use = instead'],
	 [memset => 'C-specific: use x instead'],
	 [offsetof => 'C-specific, stopped'],
	 [putc => 'C-specific: use print instead'],
	 [putchar => 'C-specific: use print instead'],
	 [puts => 'C-specific: use print instead'],
	 [qsort => 'C-specific: use sort instead'],
	 [rand => 'non-portable, use Perl\'s rand instead'],
	 [realloc => 'C-specific, stopped'],
	 [scanf => 'C-specific: use <> and regular expressions instead'],
	 [setbuf => \'IO::Handle::setbuf'],
	 [setjmp => 'C-specific: use eval {} instead'],
	 [setvbuf => \'IO::Handle::setvbuf'],
	 [siglongjmp => 'C-specific: use die instead'],
	 [sigsetjmp => 'C-specific: use eval {} instead'],
	 [srand => 'not supplied; refer to Perl\'s srand documentation'],
	 [sscanf => 'C-specific: use regular expressions instead'],
	 [strcat => 'C-specific: use .= instead'],
	 [strchr => 'C-specific: use index() instead'],
	 [strcmp => 'C-specific: use eq instead'],
	 [strcpy => 'C-specific: use = instead'],
	 [strcspn => 'C-specific: use regular expressions instead'],
	 [strlen => 'C-specific: use length instead'],
	 [strncat => 'C-specific: use .= instead'],
	 [strncmp => 'C-specific: use eq instead'],
	 [strncpy => 'C-specific: use = instead'],
	 [strpbrk => 'C-specific, stopped'],
	 [strrchr => 'C-specific: use rindex() instead'],
	 [strspn => 'C-specific, stopped'],
	 [strtok => 'C-specific, stopped'],
	 [tmpfile => \'IO::File::new_tmpfile'],
	 [tmpnam => \'use File::Temp'],
	 [ungetc => \'IO::Handle::ungetc'],
	 [vfprintf => 'C-specific, stopped'],
	 [vprintf => 'C-specific, stopped'],
	 [vsprintf => 'C-specific, stopped'],
	 [L_tmpnam => 'C-specific, stopped'],
	) {
    my ($func, $action) = @$_;
    my $expect = ref $action
	? qr/Unimplemented: POSIX::$func\(\): .*$$action(?:\(\))? instead at \(eval/
	: qr/Unimplemented: POSIX::$func\(\): \Q$action\E at \(eval/;
    is(eval "POSIX::$func(); 1", undef, "POSIX::$func fails as expected");
    like($@, $expect, "POSIX::$func gives expected error message");
}

done_testing();
