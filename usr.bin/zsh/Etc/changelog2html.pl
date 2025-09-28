#!/usr/bin/perl -w

# This programme turns the ChangeLog into changelog.html for display
# on the website.  That lives at https://zsh.sourceforge.io/Etc/changelog.html.

my $out = "changelog.html";

open CL, "ChangeLog" or die "No ChangeLog --- run in top level directory.\n";

if (-f $out) {
    die "Will not overwrite existing $out.  Delete by hand.\n";
}

my $version;
my $changes = 0;

while (<CL>) {
    /^\d+/  and  $changes++;
    if (/version\.mk.*version\s+(\d+(\.\d+)*(-\S+)?)/i) {
	$version = $1;
	$version =~ s/\.$//;
	last;
    }
}

if (defined $version) {
    warn "Outputting changelog.html for version \"$version\".\n";
    if ($changes) {
	warn "WARNING: there are changes since this version.\n";
    }
} else {
    $version = "X.X.X";
    warn "WARNING: no version found.  Set by hand\n";
}

seek CL, 0, 0;

open NEW, ">changelog.html";

select NEW;

print <<"EOH";
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">
<html>
<head>
<title>ChangeLog for zsh version $version</title>
</head>
<body>
<h1>ChangeLog for zsh version $version</h1>
<pre>
EOH

while (<CL>) {
    s/&/&amp;/g;
    s/</&lt;/g;
    s/>/&gt;/g;

    print;
}

my $now = gmtime(time);

print <<"EOH";
</pre>
<hr>
Automatically generated from ChangeLog at $now
</body>
</html>
EOH
