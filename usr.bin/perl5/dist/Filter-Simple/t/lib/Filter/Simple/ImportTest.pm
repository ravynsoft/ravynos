package Filter::Simple::ImportTest;

use parent qw(Exporter);
@EXPORT = qw(say);

sub say { print @_ }

use Filter::Simple;

sub import {
	my $class = shift;
	print "ok $_\n" foreach @_;
	__PACKAGE__->export_to_level(1,$class);
}

FILTER { s/not // };


1;
