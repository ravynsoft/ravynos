use strict;
use warnings;

use Test::More;
use CPAN::Meta;
use CPAN::Meta::Merge;

delete $ENV{PERL_YAML_BACKEND};
delete $ENV{PERL_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_DECODER};

my %base = (
	abstract => 'This is a test',
	author => ['A.U. Thor'],
	generated_by => 'Myself',
	license => [ 'perl_5' ],
	resources => {
		license => [ 'http://dev.perl.org/licenses/' ],
                bugtracker => { web => 'https://rt.cpan.org/Dist/Display.html?Foo-Bar' },
	},
	prereqs => {
		runtime => {
			requires => {
				Foo => '0',
			},
		},
	},
	dynamic_config => 0,
	provides => {
		Baz => {
			file => 'lib/Baz.pm',
		},
	},
	'meta-spec' => {
		url => "http://search.cpan.org/perldoc?CPAN::Meta::Spec",
		version => 2,
	},
);

my %first = (
	author => [ 'I.M. Poster' ],
	generated_by => 'Some other guy',
	license => [ 'bsd' ],
	resources => {
		license => [ 'http://opensource.org/licenses/bsd-license.php' ],
	},
	prereqs => {
		runtime => {
			requires => {
				Foo => '< 1',
			},
			recommends => {
				Bar => '3.14',
			},
		},
		test => {
			requires => {
				'Test::Bar' => 0,
			},
		},
	},
	dynamic_config => 1,
	provides => {
		Quz => {
			file => 'lib/Quz.pm',
		},
	},
);
my %first_expected = (
	abstract => 'This is a test',
	author => [ 'A.U. Thor', 'I.M. Poster' ],
	generated_by => 'Myself, Some other guy',
	license => [ 'perl_5', 'bsd' ],
	resources => {
		license => [ 'http://dev.perl.org/licenses/', 'http://opensource.org/licenses/bsd-license.php' ],
                bugtracker => { web => 'https://rt.cpan.org/Dist/Display.html?Foo-Bar' },
	},
	prereqs => {
		runtime => {
			requires => {
				Foo => '>= 0, < 1',
			},
			recommends => {
				Bar => '3.14',
			},
		},
		test => {
			requires => {
				'Test::Bar' => 0,
			},
		},
	},
	provides => {
		Baz => {
			file => 'lib/Baz.pm',
		},
		Quz => {
			file => 'lib/Quz.pm',
		},
	},
	dynamic_config => 1,
	'meta-spec' => {
		url => "http://search.cpan.org/perldoc?CPAN::Meta::Spec",
		version => 2,
	},
);
my %provides_merge_expected = (
	abstract => 'This is a test',
	author => ['A.U. Thor'],
	generated_by => 'Myself',
	license => [ 'perl_5' ],
	resources => {
		license => [ 'http://dev.perl.org/licenses/' ],
		bugtracker => { web => 'https://rt.cpan.org/Dist/Display.html?Foo-Bar' },
	},
	prereqs => {
		runtime => {
			requires => {
				Foo => '0',
			},
		},
	},
	dynamic_config => 0,
	provides => {
		Baz => {
			file => 'lib/Baz.pm',
			version => '0.001',         # same as %base, but for this extra key
		},
	},
	'meta-spec' => {
		url => "http://search.cpan.org/perldoc?CPAN::Meta::Spec",
		version => 2,
	},
);

my $merger = CPAN::Meta::Merge->new(default_version => '2');

my $first_result = $merger->merge(\%base, \%first);

is_deeply($first_result, \%first_expected, 'First result is as expected');

is_deeply($merger->merge(\%base, { abstract => 'This is a test' }), \%base, 'Can merge in identical abstract');
is(
    eval { $merger->merge(\%base, { abstract => 'And now for something else' }) },
    undef,
    'Trying to merge different abstract gives an exception',
);
like $@, qr/^Can't merge attribute abstract/, 'Exception looks right';

is(
    eval { $merger->merge(\%base, { resources => { bugtracker => { web => 'http://foo.com' } } } ) },
    undef,
    'Trying to merge a different bugtracker URL gives an exception',
);
like $@, qr/^Duplication of element resources\.bugtracker\.web /, 'Exception looks right';

is(
    eval { $merger->merge(\%base, { provides => { Baz => { file => 'Baz.pm' } } }) },
    undef,
    'Trying to merge different provides.$module.file gives an exception',
);
like $@, qr/^Duplication of element provides\.Baz\.file /, 'Exception looks right';

my $provides_result = $merger->merge(\%base, { provides => { Baz => { file => 'lib/Baz.pm', version => '0.001' } } });
is_deeply(
	$provides_result,
	\%provides_merge_expected,
	'Trying to merge a new key for provides.$module is permitted; identical values are preserved',
);

my $extra_merger = CPAN::Meta::Merge->new(
	default_version => '2',
	extra_mappings => {
		'x_toolkit' => 'set_addition',
		'x_meta_meta' => {
			name => 'identical',
			tags => 'set_addition',
		}
	}
);

my $extra_results = $extra_merger->merge(\%base, {
		x_toolkit => [ 'marble' ],
		x_meta_meta => {
			name => 'Test',
			tags => [ 'Testing' ],
		}
	},
	{ x_toolkit => [ 'trike'],
		x_meta_meta => {
			name => 'Test',
			tags => [ 'TDD' ],
		}
	}
);

my $expected_nested_extra = {
	name => 'Test',
	tags => [ 'Testing', 'TDD' ],
};
is_deeply($extra_results->{x_toolkit}, [ 'marble', 'trike' ], 'Extra mapping fields are merged');
is_deeply($extra_results->{x_meta_meta}, $expected_nested_extra, 'Nested extra mapping fields are merged' );

my $adds_to = sub {
  my ($left, $right, $path) = @_;
  if ($right !~ /^\Q$left\E/) {
    die sprintf "Can't merge attribute %s: '%s' does not start with '%s'", join('.', @{$path}), $right, $left;
  }
  return $right;
};

$extra_merger = CPAN::Meta::Merge->new(default_version => '2', extra_mappings => { 'abstract' => \&$adds_to } );
my $extra_results2 = $extra_merger->merge({ abstract => 'This is a test.'}, { abstract => 'This is a test.  Includes more detail..' } );
is($extra_results2->{abstract}, 'This is a test.  Includes more detail..', 'Extra mapping fields overwrite existing mappings');
my $extra_failure = eval { $extra_merger->merge({ abstract => 'This is a test.'}, { abstract => 'This is a better test.' } ) };
is($extra_failure, undef, 'Extra mapping produces a failure');
like $@, qr/does not start with/, 'Exception looks right';



# issue 67
@base{qw/name version release_status/} = qw/Foo-Bar 0.01 testing/;
my $base_obj = CPAN::Meta->create(\%base);
ok my $first_result_obj = $merger->merge($base_obj, \%first), 'merging CPAN::Meta objects succeeds';

done_testing();
# vim: ts=4 sts=4 sw=4 tw=78 noet :
