#!./perl -w
#
# Tests to make sure the regexp engine doesn't run into limits too soon.
#

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;

my $email = qr {
    (?(DEFINE)
      (?<address>         (?&mailbox) | (?&group))
      (?<mailbox>         (?&name_addr) | (?&addr_spec))
      (?<name_addr>       (?&display_name)? (?&angle_addr))
      (?<angle_addr>      (?&CFWS)? < (?&addr_spec) > (?&CFWS)?)
      (?<group>           (?&display_name) : (?:(?&mailbox_list) | (?&CFWS))? ;
                                             (?&CFWS)?)
      (?<display_name>    (?&phrase))
      (?<mailbox_list>    (?&mailbox) (?: , (?&mailbox))*)

      (?<addr_spec>       (?&local_part) \@ (?&domain))
      (?<local_part>      (?&dot_atom) | (?&quoted_string))
      (?<domain>          (?&dot_atom) | (?&domain_literal))
      (?<domain_literal>  (?&CFWS)? \[ (?: (?&FWS)? (?&dcontent))* (?&FWS)?
                                    \] (?&CFWS)?)
      (?<dcontent>        (?&dtext) | (?&quoted_pair))
      (?<dtext>           (?&NO_WS_CTL) | (?[ [:ascii:] & [:graph:] & [^][ \\ ] ]))

      (?<atext>           (?&ALPHA) | (?&DIGIT) | [-!#\$%&'*+/=?^_`{|}~])
      (?<atom>            (?&CFWS)? (?&atext)+ (?&CFWS)?)
      (?<dot_atom>        (?&CFWS)? (?&dot_atom_text) (?&CFWS)?)
      (?<dot_atom_text>   (?&atext)+ (?: \. (?&atext)+)*)

      (?<text>            (?[ [:ascii:] & [^ \000 \n \r ] ]))
      (?<quoted_pair>     \\ (?&text))

      (?<qtext>           (?&NO_WS_CTL) | (?[ [:ascii:] & [:graph:] & [^ " \\ ] ]))
      (?<qcontent>        (?&qtext) | (?&quoted_pair))
      (?<quoted_string>   (?&CFWS)? (?&DQUOTE) (?:(?&FWS)? (?&qcontent))*
                           (?&FWS)? (?&DQUOTE) (?&CFWS)?)

      (?<word>            (?&atom) | (?&quoted_string))
      (?<phrase>          (?&word)+)

      # Folding white space
      (?<FWS>             (?: (?&WSP)* (?&CRLF))? (?&WSP)+)
      (?<ctext>           (?&NO_WS_CTL) | (?[ [:ascii:] & [:graph:] & [^ () ] & [^ \\ ] ]))
      (?<ccontent>        (?&ctext) | (?&quoted_pair) | (?&comment))
      (?<comment>         \( (?: (?&FWS)? (?&ccontent))* (?&FWS)? \) )
      (?<CFWS>            (?: (?&FWS)? (?&comment))*
                          (?: (?:(?&FWS)? (?&comment)) | (?&FWS)))

      # No whitespace control
      (?<NO_WS_CTL>       (?[ [:ascii:] & [:cntrl:] & [^ \000 \h \r \n ] ]))

      (?<ALPHA>           [A-Za-z])
      (?<DIGIT>           [0-9])
      (?<CRLF>            \r \n)
      (?<DQUOTE>          ")
      (?<WSP>             [ \t])
    )

    (?&address)
}x;

run_tests() unless caller;

sub run_tests {
    # rewinding DATA is necessary with PERLIO=stdio when this
    # test is run from another thread
    seek *DATA, 0, 0;
    while (<DATA>) { last if /^__DATA__/ }
    while (<DATA>) {
	chomp;
	next if /^#/;
	like($_, qr/^$email$/, $_);
    }

    done_testing();
}

1; # Because reg_email_thr.t will (indirectly) require this script.

#
# Acme::MetaSyntactic ++
#
__DATA__
Jeff_Tracy@thunderbirds.org
"Lady Penelope"@thunderbirds.org
"The\ Hood"@thunderbirds.org
fred @ flintstones.net
barney (rubble) @ flintstones.org
bammbamm (bam! bam! (bam! bam! (bam!)) bam!) @ flintstones.org
Michelangelo@[127.0.0.1]
Donatello @ [127.0.0.1]
Raphael (He as well) @ [127.0.0.1]
"Leonardo" @ [127.0.0.1]
Barbapapa <barbapapa @ barbapapa.net>
"Barba Mama" <barbamama @ [127.0.0.1]>
Barbalala (lalalalalalalala) <barbalala (Yes, her!) @ (barba) barbapapa.net>
