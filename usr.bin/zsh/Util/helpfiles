#!/usr/bin/env perl

# helpfiles:  make help files for Z-shell builtins from the manual entries.

# Create help files for zsh commands for the manpage file of zshbuildins.1
# passed as the first arg.
# The second arg is the directory in which the help files will be created.
# Assumes no other files are present.
# No overwriting check;  `.' becomes `dot', `:' becomes `colon'.

# Any command claiming to be `same as <foo>' or `equivalent to <foo>'
# has its help file appended to the end of <foo>'s and replaced by a
# link to <foo>.  (Arguably the help file should be put at the start
# instead.)

# If a third arg is given, the symlink is not created, but a
# list of symlinks is put into the file specified by that arg.

# This script is called automatically during `make install'
# unless specified otherwise.

# For usage and more information see zshcontrib(1).

sub Usage {
    print(STDERR "Usage: helpfiles zshbuiltins.1 dest-dir [link-file]\n");
    exit(1);
}

sub Info {
    print('helpfiles: ', @_, "\n");
}

sub Die {
    print(STDERR 'helpfiles: ', @_, "\n");
    exit(1);
}

&Usage() unless(@ARGV);
$manfile = shift(@ARGV);
&Usage() unless(@ARGV);
$destdir = shift(@ARGV);
$linkfile = ((@ARGV) ? shift(@ARGV) : '');
unless(-d $destdir) {
    mkdir($destdir) || &Die("$destdir is not a directory and cannot be created");
}

foreach (keys %ENV) {
	delete($ENV{$_}) if(/^((LC_)|(LESS)|(MAN))/);
}
$ENV{'LANG'} = 'C';
$ENV{'MANWIDTH'} = '80';
$ENV{'GROFF_NO_SGR'} = ''; # We need "classical" formatting of man pages.

$mantmp = $destdir . '/man.tmp';
$coltmpbase = 'col.tmp';
$coltmp = $destdir . '/' . $coltmpbase;
$args = "$manfile >$mantmp";
unlink($mantmp);
&Info('attempting man ', $args);
if(system('man ' . $args) || !(-s $mantmp)) {
    unlink($mantmp);
    &Info('attempting nroff -man ', $args);
    if(system('nroff -man ' . $args) || !(-s $mantmp)) {
        unlink($mantmp);
        &Die('man and nroff -man both failed for ', $manfile);
    }
}
$args = "$mantmp >$coltmp";
unlink($coltmp);
&Info('attempting col -bx <', $args);
# The x is necessary so that spaces don't turn into tabs, which messes
# up the calculations of indentation on machines which randomly wrap lines
# round to the previous line (so you see what we're up against).
if(system('col -bx <' . $args) || !(-s $coltmp)) {
    unlink($coltmp);
    &Info('attempting colcrt - ', $args);
    if(system('colcrt - ' . $args) || !(-s $coltmp)) {
        unlink($mantmp);
        unlink($coltmp);
        &Die('col -bx and colcrt - both failed');
    }
}
unlink($mantmp) || &Die('cannot remove tempfile ', $mantmp);

unless(open(MANPAGE, '<', $coltmp)) {
    unlink($coltmp);
    &Die('generated tmpfile cannot be read');
}

unless($linkfile eq '') {
    open(LINKFILE, '>', $linkfile) || &Die("cannot open $linkfile for writing")
}

chdir($destdir) || &Die("cannot cd to $destdir");

while (<MANPAGE>) {
    last if /^\s*SHELL BUILTIN COMMANDS/;
    /zshbuiltins/ && $zb++;
    last if ($zb && /^\s*DESCRIPTIONS/);
}

$print = 0;

sub namesub {
    local($cmd) = shift;
    if ($cmd =~ /^\w*$/ && lc($cmd) eq $cmd) {
	$cmd;
    } elsif ($cmd eq '.') {
	'dot';
    } elsif ($cmd eq ':') {
	'colon';
    } else {
	undef;
    }
}

sub getsame {
    local($_) = shift;
    if (/same\s*as\s*(\S+)/i || /equivalent\s*to\s*(\S+)/i) {
	local($name) = $1;
	($name =~ /[.,]$/) && chop($name);
	return $name;
    } else {
	return undef;
    }
}

sub newcmd {
    local($_) = shift;
    local($cmd);
    # new command
    if (defined($cmd = &namesub($_))) {
	# in case there's something nasty here like a link..
	unlink $cmd;
	open (OUT, ">$cmd");
	select OUT;
	$print = 1;
    } else {
	$print = 0;
    }
}

sub doprint {
    local($_) = shift;

    s/^$indentstr//o;		# won't work if too many tabs
    print;
}

while (<MANPAGE>) { last unless /^\s*$/; }

/^(\s+)(\S+)/;
$indentstr = $1;
$indent = length($1);
&newcmd($2);
print if $print;

BUILTINS: while (<MANPAGE>) {
    next if /^\w/;

    undef($undented);
    if (/^\s*$/ || ($undented = (/^(\s*)/  && length($1) < $indent))) {
	$undented && &doprint($_);
	while (defined($_ = <MANPAGE>) && /(^\w)|(^\s*$)/) {
	    # NAME is the start of the next section when in zshall.
	    # (Historical note: we used to exit on the page header,
	    # but text from the old section can continue to the
	    # new page).
	    last BUILTINS if /^\s*NAME\s*$/;
	    last BUILTINS if /^STARTUP\/SHUTDOWN FILES/;
	    last if /^zsh.*\s\d$/; # GNU nroff -man end-of-page marker
	}
        if (/^\s*Page/ || /^zsh.*\s\d$/) {
	    do {
		$_ = <MANPAGE>;
	    } while (defined($_) && /^\s*$/);
	    if (/^\s*ZSHBUILTINS/) {
		do {
		    $_ = <MANPAGE>;
		} while (defined($_) && /^\s*$/);
	    }
	}
	if (/^(\s*)/ && length($1) < $indent) {
	    # This may be just a bug on the SGI, or maybe something
	    # more sinister (don\'t laugh, this is nroff).
	    s/^\s*/ /;
	    $defer = $_;
	    do {
		$_ = <MANPAGE>;
	    } while (defined($_) && /^\s*$/);
	    last unless defined($_);
	}
	if (/^(\s+)(\S+)/ && length($1) == $indent) {
	    &newcmd($2);
	}  else {
	    print "\n";
	}
        if ($print) {
	    if (defined($defer)) {
		chop;
		&doprint("$_$defer");
		undef($defer);
	    } else {
		&doprint($_);
	    }
	}
    } else {
	&doprint($_) if $print;
    }
}

select STDOUT;
close OUT;
close(MANPAGE);
unlink($coltmpbase) || &Die('cannot remove tempfile ', $coltmpbase);

foreach $file (<*>) {
    open (IN, $file);
    if ($sameas = (&getsame($_ = <IN>) || &getsame($_ = <IN>))) {
	defined($sameas = &namesub($sameas)) || next;
#	print "$file is the same as $sameas\n";
	seek (IN, 0, 0);

	# Copy this to base builtin.
	open (OUT, ">>$sameas");
	select OUT;
	print "\n";
	while (<IN>) { print; }
	close IN;
	select STDOUT;
	close OUT;

	# Make this a link to that.
	unlink $file;
	if($linkfile eq '') {
	    symlink ($sameas, $file);
	} else {
	    print(LINKFILE "$sameas $file\n");
	}
    }
}
close(LINKFILE) unless($linkfile eq '');

# Make one sanity check
&Die('not all files were properly generated') unless(-r 'ztcp');

__END__
