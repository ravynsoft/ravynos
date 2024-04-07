#!./perl

use FileCache maxopen => 2;
our @files;
BEGIN { @files = map { "append_$_" } qw(foo bar baz quux Foo_Bar) }
END   { 1 while unlink @files }

use Test::More tests => 2;

{# Test 3: that we open for append on second viewing
     my @cat;
     for my $path ( @files ){
	 cacheout $path;
	 print $path "$path 3\n";
     }
     for my $path ( @files ){
	 cacheout $path;
	 print $path "$path 33\n";
     }
     for my $path ( @files ){
	 open($path, '<', $path);
	 push @cat, do{ local $/; <$path>};
         close($path);
     }

     ok(scalar(grep/\b3$/m, @cat) == scalar(@files));

     @cat = ();
     for my $path ( @files ){
	 cacheout $path;
	 print $path "$path 333\n";
     }
     for my $path ( @files ){
	 open($path, '<', $path);
	 push @cat, do{ local $/; <$path>};
         close($path);
     }
     ok(scalar(grep /\b33$/m, @cat) == scalar(@files));
}
