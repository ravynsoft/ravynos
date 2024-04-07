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

my $fragment1 = {
	'optional_features' => {
		'FeatureName' => {
			'description' => 'desc',
			'x_default' => 1,
			'prereqs' => { 'runtime' => { 'requires' => { 'A' => '0' } } }
		}
	}
};
my $fragment2 = {
	'optional_features' => {
		'FeatureName' => {
			'description' => 'desc',
			'prereqs' => { 'test' => { 'requires' => { 'B' => '0' } } }
		}
	}
};

my $merger = CPAN::Meta::Merge->new(default_version => "2");
my $meta1 = $merger->merge(\%base, $fragment1);

is_deeply(
	$meta1,
	{
		%base,
		%$fragment1,
	},
	'merged first optional_feature fragment into base',
);

my $meta2 = $merger->merge($meta1, $fragment2);

is_deeply(
	$meta2,
	{
		%base,
		'optional_features' => {
			'FeatureName' => {
				'description' => 'desc',
				'x_default' => 1,
				'prereqs' => {
					'runtime' => { 'requires' => { 'A' => '0' } },
					'test' => { 'requires' => { 'B' => '0' } },
				}
			}
		}
	},
	'merged second optional_feature fragment into the first',
);

my $fragment3 = {
	'optional_features' => {
		'FeatureName' => {
			'description' => 'other desc',
			'prereqs' => { 'test' => { 'requires' => { 'B' => '0' } } }
		}
	}
};

is( eval { $merger->merge($meta1, $fragment3) }, undef, 'Trying to merge optional_features with same feature name and different descriptions gives an exception');
like $@, qr/^Cannot merge two optional_features named 'FeatureName' with different 'description' values/, 'Exception looks right';

my $fragment4 = {
	'optional_features' => {
		'FeatureName' => {
			'description' => 'desc',
			'x_default' => 0,
			'prereqs' => { 'test' => { 'requires' => { 'B' => '0' } } }
		}
	}
};

is( eval { $merger->merge($meta1, $fragment4) }, undef, 'Trying to merge optional_features with same feature name and differences in other keys gives an exception');
like $@, qr/^Cannot merge two optional_features named 'FeatureName' with different 'x_default' values/, 'Exception looks right';

my $fragment5 = {
	'optional_features' => {
		'Another FeatureName' => {
			'description' => 'desc',
			'prereqs' => { 'test' => { 'requires' => { 'B' => '0' } } }
		}
	}
};

my $meta5 = $merger->merge($meta1, $fragment5);
is_deeply(
	$meta5,
	{
		%base,
		'optional_features' => {
			'FeatureName' => {
				'description' => 'desc',
				'x_default' => 1,
				'prereqs' => { 'runtime' => { 'requires' => { 'A' => '0' } } },
			},
			'Another FeatureName' => {
				'description' => 'desc',
				'prereqs' => { 'test' => { 'requires' => { 'B' => '0' } } },
			}
		}
	},
	'can merge optional_features with different names without collisions',
);

done_testing;
# vim: ts=4 sts=4 sw=4 noet :
