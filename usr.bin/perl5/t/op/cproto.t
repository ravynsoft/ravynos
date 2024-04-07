#!./perl
# Tests to ensure that we don't unexpectedly change prototypes of builtins

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan tests => 254;

while (<DATA>) {
    chomp;
    (my $keyword, my $proto, local $TODO) = split " ", $_, 3;
    if ($proto eq 'undef') {
	ok( !defined prototype "CORE::".$keyword, $keyword );
    }
    elsif ($proto eq 'unknown') {
	eval { prototype "CORE::".$keyword };
	like( $@, qr/Can't find an opnumber for/, $keyword );
    }
    else {
	is(
	    "(".(prototype("CORE::".$keyword) // 'undef').")", $proto,
	    $keyword
	);
    }
}

# the keyword list :

__DATA__
__FILE__ ()
__LINE__ ()
__PACKAGE__ ()
__DATA__ undef
__END__ undef
__SUB__ ()
AUTOLOAD undef
BEGIN undef
CORE unknown
DESTROY undef
END undef
INIT undef
CHECK undef
abs (_)
accept (**)
alarm (_)
and undef
atan2 ($$)
bind (*$)
binmode (*;$)
bless ($;$)
break ()
caller (;$)
chdir (;$)
chmod (@)
chomp undef
chop undef
chown (@)
chr (_)
chroot (_)
close (;*)
closedir (*)
cmp undef
connect (*$)
continue ()
cos (_)
crypt ($$)
dbmclose (\%)
dbmopen (\%$$)
default undef
defined undef
delete undef
die (@)
do undef
dump ()
each (\[%@])
else undef
elsif undef
endgrent ()
endhostent ()
endnetent ()
endprotoent ()
endpwent ()
endservent ()
eof (;*)
eq undef
eval undef
evalbytes (_)
exec undef
exists undef
exit (;$)
exp (_)
fc (_)
fcntl (*$$)
fileno (*)
flock (*$)
for undef
foreach undef
fork ()
format undef
formline ($@)
ge undef
getc (;*)
getgrent ()
getgrgid ($)
getgrnam ($)
gethostbyaddr ($$)
gethostbyname ($)
gethostent ()
getlogin ()
getnetbyaddr ($$)
getnetbyname ($)
getnetent ()
getpeername (*)
getpgrp (;$)
getppid ()
getpriority ($$)
getprotobyname ($)
getprotobynumber ($;)
getprotoent ()
getpwent ()
getpwnam ($)
getpwuid ($)
getservbyname ($$)
getservbyport ($$)
getservent ()
getsockname (*)
getsockopt (*$$)
given undef
glob (_;)
gmtime (;$)
goto undef
grep undef
gt undef
hex (_)
if undef
index ($$;$)
int (_)
ioctl (*$$)
join ($@)
keys (\[%@])
kill (@)
last undef
lc (_)
lcfirst (_)
le undef
length (_)
link ($$)
listen (*$)
local undef
localtime (;$)
lock (\[$@%&*])
log (_)
lstat (;*)
lt undef
m undef
map undef
mkdir (_;$)
msgctl ($$$)
msgget ($$)
msgrcv ($$$$$)
msgsnd ($$$)
my undef
ne undef
next undef
no undef
not ($;)
oct (_)
open (*;$@)
opendir (*$)
or undef
ord (_)
our undef
pack ($@)
package undef
pipe (**)
pop (;\@)
pos (;\[$*])
print undef
printf undef
prototype (_)
push (\@@)
q undef
qq undef
qr undef
quotemeta (_)
qw undef
qx undef
rand (;$)
read (*\$$;$)
readdir (*)
readline (;*)
readlink (_)
readpipe (_)
recv (*\$$$)
redo undef
ref (_)
rename ($$)
require undef
reset (;$)
return undef
reverse (@)
rewinddir (*)
rindex ($$;$)
rmdir (_)
s undef
say undef
scalar ($)
seek (*$$)
seekdir (*$)
select undef
semctl ($$$$)
semget ($$$)
semop ($$)
send (*$$;$)
setgrent ()
sethostent ($)
setnetent ($)
setpgrp (;$$)
setpriority ($$$)
setprotoent ($)
setpwent ()
setservent ($)
setsockopt (*$$$)
shift (;\@)
shmctl ($$$)
shmget ($$$)
shmread ($$$$)
shmwrite ($$$$)
shutdown (*$)
sin (_)
sleep (;$)
socket (*$$$)
socketpair (**$$$)
sort undef
splice (\@;$$@)
split undef
sprintf ($@)
sqrt (_)
srand (;$)
stat (;*)
state undef
study (_)
sub undef
substr ($$;$$)
symlink ($$)
syscall ($@)
sysopen (*$$;$)
sysread (*\$$;$)
sysseek (*$$)
system undef
syswrite (*$;$$)
tell (;*)
telldir (*)
tie (\[$@%*]$@)
tied (\[$@%*])
time ()
times ()
tr undef
truncate ($$)
uc (_)
ucfirst (_)
umask (;$)
undef (;\[$@%&*])
unless undef
unlink (@)
unpack ($_)
unshift (\@@)
untie (\[$@%*])
until undef
use undef
utime (@)
values (\[%@])
vec ($$$)
wait ()
waitpid ($$)
wantarray ()
warn (@)
when undef
while undef
write (;*)
x undef
xor undef
y undef
