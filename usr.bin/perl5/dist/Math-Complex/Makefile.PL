{ use 5.006; }

use ExtUtils::MakeMaker;

WriteMakefile(
	      NAME            => 'Math::Complex',
	      VERSION_FROM    => 'lib/Math/Complex.pm',
              INSTALLDIRS     => ( $] < 5.011 ? 'perl' : 'site' ),
	      PREREQ_PM	      =>
	      	{
		    'Config'		=> 0,
		    'Exporter'		=> 0,
		    'ExtUtils::MakeMaker' => 0,
		    'Scalar::Util'	=> '1.11',
		    'Test::More'	=> 0,
		    'overload'		=> 0,
		    'strict'		=> 0,
		    'warnings'		=> 0,
		},
              ($ExtUtils::MakeMaker::VERSION >= 6.3002 ?
	       ('LICENSE'=> 'perl_5', ) : ()),
	     );
