use strict;

use I18N::LangTags::Detect;

use Test::More;

my @in = grep m/\S/, split /\n/, q{

[ sv      ]  sv
[ en      ]  en
[ en fi   ]  en, fi
[ en-us   ]  en-us
[ en-us   ]  en-US
[ en-us   ]  EN-US

[ en-au en i-klingon x-klingon en-gb en-us mt-mt mt ja ]  EN-au, JA;q=0.14, i-klingon;q=0.83, en-gb;q=0.71, en-us;q=0.57, mt-mt;q=0.43, mt;q=0.29, en;q=0.86
[ en-au en i-klingon x-klingon en-gb en-us mt-mt mt tli ja ]  EN-au, tli;q=0.201, JA;q=0.14, i-klingon;q=0.83, en-gb;q=0.71, en-us;q=0.57, mt-mt;q=0.43, mt;q=0.29, en;q=0.86
[ en-au en en-gb en-us ja  ]  en-au, ja;q=0.20, en-gb;q=0.60, en-us;q=0.40, en;q=0.80

[ en-au en en-gb en-us mt-mt mt ja ]  EN-au, JA;q=0.14, en-gb;q=0.71, en-us;q=0.57, mt-mt;q=0.43, mt;q=0.29, en;q=0.86
[ en-au en en-gb en-us ja  ]  en-au, ja;q=0.20, en-gb;q=0.60, en-us;q=0.40, en;q=0.80
[ en fr           ]  en;q=1,fr;q=.5
[ en fr           ]  en;q=1,fr;q=.99
[ en ru ko        ]  en, ru;q=0.7, ko;q=0.3
[ en ru ko        ]  en, ru;q=0.7, KO;q=0.3
[ en-us en        ]  en-us, en;q=0.50
[ en fr           ]  fr ; q = 0.9, en
[ en fr           ]  en,fr;q=.90
[ ru en-uk en fr  ]  ru, en-UK;q=0.5, en;q=0.3, fr;q=0.1
[ en-us fr es-mx  ]  en-us,fr;q=0.7,es-mx;q=0.3 
[ en-us en        ]  en-us, en;q=0.50 

[ da en-gb en       ]  da, en-gb;q=0.8, en;q=0.7
[ da en-gb en       ]  da, en;q=0.7, en-gb;q=0.8
[ da en-gb en       ]  da, en-gb;q=0.8, en;q=0.7
[ da en-gb en       ]  da,en;q=0.7,en-gb;q=0.8
[ da en-gb en       ]  da, en-gb ; q=0.8, en ; q=0.7
[ da en-gb en       ]  da , en-gb ; q = 0.8 , en ; q  =0.7
[ da en-gb en       ]  da (yup, Danish) , en-gb ; q = 0.8 , en ; q  =0.7

[ no dk en-uk en-us ]  en-UK;q=0.7, en-US;q=0.6, no;q=1.0, dk;q=0.8
[ no dk en-uk en-us ]  en-US;q=0.6, en-UK;q=0.7, no;q=1.0, dk;q=0.8
[ no dk en-uk en-us ]  en-UK;q=0.7, no;q=1.0, en-US;q=0.6, dk;q=0.8
[ no dk en-uk en-us ]  en-UK;q=0.7, no;q=1.0, dk;q=0.8, en-US;q=0.6

[ fi en ]  fi;q=1, en;q=0.2
[ de-de de en en-us en-gb ]  de-DE, de;q=0.80, en;q=0.60, en-US;q=0.40, en-GB;q=0.20
[ ru          ]  ru; q=1, *; q=0.1
[ ru en       ]  ru, en; q=0.1
[ ja en       ]  ja,en;q=0.5
[ en          ]  en; q=1.0
[ ja          ]  ja; q=1.0
[ ja          ]  ja; q=1.0
[ en ja       ]  en; q=0.5, ja; q=0.5
[ fr-ca fr en ]  fr-ca, fr;q=0.8, en;q=0.7
[ NIX ] NIX
};

plan(tests => 2 * @in);
foreach my $in (@in) {
  $in =~ s/^\s*\[([^\]]+)\]\s*//s or die "Bad input: $in";
  my @should = do { my $x = $1; $x =~ m/(\S+)/g };

  if($in eq 'NIX') { $in = ''; @should = (); }

  local $ENV{'HTTP_ACCEPT_LANGUAGE'};
  
  foreach (
	   ['arg', sub {
      print "# Testing with arg...\n";
      $ENV{'HTTP_ACCEPT_LANGUAGE'} = 'PLORK';
      return $_[0];
  }],
	   ['HTTP_ACCEPT_LANGUAGE', sub {
      print "# Testing wath HTTP_ACCEPT_LANGUAGE...\n";
      $ENV{'HTTP_ACCEPT_LANGUAGE'} = $_[0];
     return();
  }],
	  ) {
    my ($type, $modus) = @$_;
    my @args = &$modus($in);

    # ////////////////////////////////////////////////////
    my @out = I18N::LangTags::Detect->http_accept_langs(@args);
    # \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

    is_deeply(\@out, \@should, "Testing [$in] with $type");
  }
}
