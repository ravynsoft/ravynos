#!perl

use strict;
use warnings;

use Test::More;
use Config;

#
# Module::Load; test new features:
#    autoload;
#    remote_load
#    autload_remote
#  and options: '','none',undef,'all','load','autoload','load_remote'
#
# License: This library is free software; you may redistribute and/or modify it under the same terms as Perl itself.
#
#  Author (jabber/email) : reisub@yandex.ru
#

my ($afx, $cnt, $r, $tcode) = ('TestXYZ_', 0);

sub _reset{
    undef %{Data::Dumper::};
    undef %{XYZ::Module::};
    no warnings q[uninitialized];
    eval "undef %{$afx$cnt::}";
    delete $INC{'Data/Dumper.pm'};
}

sub _test{
    $cnt++;
    $tcode = "package $afx$cnt; my \$WORLD='PEACE';" . join '', @_;
#    print "tcode:$tcode\n";
    $r = eval($tcode) || $@;
}

sub is_peace_in_world{
    like $r, qr/(WORLD\W+)?PEACE/o, $_[0] || '.';
    goto &_reset;
}

sub isnt_peace_in_world{
    unlike $r, qr/(WORLD\W+)?PEACE/o, $_[0] || '.';
    goto &_reset;
}

sub isnt_def_sub{
    like $r, qr/Undefined\s+subroutine/io, $_[0] || '.';
    goto &_reset;
}

sub cant_locate{
	like $r, qr/Can't\s+locate/io, $_[0] || '.';
    goto &_reset;
}

subtest 'load/prevcompat' => sub{
    _test('use Module::Load;
	    load("Data::Dumper");
    	    Data::Dumper->Dump([$WORLD]);');
    is_peace_in_world('default import');

    _test('use Module::Load "load";
	    load("Data::Dumper");
    	    Data::Dumper->Dump([$WORLD]);');
    is_peace_in_world();

    _test('use Module::Load;
	    load("Data::Dumper");
    	    Dumper([$WORLD]);');
    isnt_def_sub();

    _test('use Module::Load;
	    load("Data::Dumper","Dumper");
    	    Data::Dumper->Dump([$WORLD]);');
    is_peace_in_world();

    _test('use Module::Load "all";
		load("______");');
    cant_locate();

    _test('use Module::Load "";
			load("Data::Dumper");
    	    Data::Dumper->Dump([$WORLD]);');
    isnt_def_sub();

	_test('use Module::Load "none";
			load("Data::Dumper");
    	    Data::Dumper->Dump([$WORLD]);');
    isnt_def_sub();

	_test('use Module::Load undef;
			load("Data::Dumper");
    	    Data::Dumper->Dump([$WORLD]);');
    isnt_def_sub();

    done_testing();
};

subtest 'autoload' => sub{
    _test('use Module::Load;
			autoload("Data::Dumper");
    	    Data::Dumper->Dump([$WORLD]);');
    is_peace_in_world('default import');

    _test('use Module::Load;
			Module::Load::autoload("Data::Dumper");
    	    Data::Dumper->Dump([$WORLD]);');
    is_peace_in_world();

    _test('use Module::Load;
			Module::Load::autoload("Data::Dumper");
    	    Dumper($WORLD);');
    is_peace_in_world();

    _test('use Module::Load;
			Module::Load::autoload("Data::Dumper","Dumper");
    	    Dumper($WORLD);');
    is_peace_in_world();

    _test('use Module::Load "autoload";
			autoload("Data::Dumper");
    	    Dumper($WORLD);');
    is_peace_in_world();

    _test('use Module::Load "all";
			autoload("Data::Dumper");
    	    Dumper($WORLD);');
    is_peace_in_world();

    _test('use Module::Load "all";
			autoload("______");');
    cant_locate();

    _test('use Module::Load "";
			autoload("Data::Dumper");
    	    Data::Dumper->Dump([$WORLD]);');
    isnt_def_sub();

	_test('use Module::Load "none";
			autoload("Data::Dumper");
    	    Data::Dumper->Dump([$WORLD]);');
    isnt_def_sub();

	_test('use Module::Load undef;
			autoload("Data::Dumper");
    	    Data::Dumper->Dump([$WORLD]);');
    isnt_def_sub();

    done_testing();
};

subtest 'noimport' => sub{
    for my $asq('"none"', '""', 'undef'){
		_test('use Module::Load '.$asq.';
				load("Data::Dumper");
				Data::Dumper->Dump([$WORLD]);');
		isnt_def_sub();

		_test('use Module::Load '.$asq.';
				autoload("Data::Dumper");
				Data::Dumper->Dump([$WORLD]);');
		isnt_def_sub();

		_test('use Module::Load '.$asq.';
				load_remote("XYZ::Module" => "Data::Dumper");
				Data::Dumper->Dump([$WORLD]);');
		isnt_def_sub();

		_test('use Module::Load '.$asq.';
				autoload_remote("XYZ::Module" => "Data::Dumper");
				Data::Dumper->Dump([$WORLD]);');
		isnt_def_sub();
    }
    done_testing();
};

subtest 'load_remote' => sub{
    _test('use Module::Load;
	    load_remote("XYZ::Module","Data::Dumper");
    	    Data::Dumper->Dump([$WORLD]);');
    isnt_def_sub();

    _test('use Module::Load;
	    load_remote("XYZ::Module","Data::Dumper");
    	    Dumper([$WORLD]);');
    isnt_def_sub();

    _test('use Module::Load;
	    Module::Load::load_remote("XYZ::Module","Data::Dumper");
	    Data::Dumper->Dump([$WORLD]);');
    is_peace_in_world();

    _test('use Module::Load;
	    Module::Load::load_remote("XYZ::Module","Data::Dumper");
	    XYZ::Module::Dumper($WORLD);');
    isnt_def_sub();

    _test('use Module::Load;
	    Module::Load::load_remote("XYZ::Module","Data::Dumper","Dumper");
	    XYZ::Module::Dumper($WORLD);');
    is_peace_in_world();

	_test('use Module::Load "all";
	    load_remote("XYZ::Module","______","Data::Dumper");
	    XYZ::Module::Dumper($WORLD);');
    cant_locate();

    done_testing();
};

subtest 'autoload_remote' => sub{
    _test('use Module::Load;
	    autoload_remote("XYZ::Module","Data::Dumper");
	    Data::Dumper->Dump([$WORLD]);');
    isnt_def_sub();

    _test('use Module::Load;
	    autoload_remote("XYZ::Module","Data::Dumper");
	    Dumper([$WORLD]);');
    isnt_def_sub();

    _test('use Module::Load;
	    Module::Load::autoload_remote("XYZ::Module","Data::Dumper");
	    Data::Dumper->Dump([$WORLD]);');
    is_peace_in_world();

    _test('use Module::Load;
	    Module::Load::autoload_remote("XYZ::Module","Data::Dumper");
	    XYZ::Module::Dumper($WORLD);');
    is_peace_in_world();

    _test('use Module::Load "all";
	    autoload_remote("XYZ::Module","______","Data::Dumper");
	    XYZ::Module::Dumper($WORLD);');
    cant_locate();

	done_testing();
};

subtest 'complex' => sub{
	_test('use Module::Load "load","autoload","none";
			load("Data::Dumper");
	    Data::Dumper->Dump([$WORLD]);');
    isnt_def_sub();

	_test('use Module::Load "load","autoload","none";
			autoload("Data::Dumper");
	    Data::Dumper->Dump([$WORLD]);');
    isnt_def_sub();

	_test('use Module::Load "load","autoload","none";
			load_remote("Data::Dumper");
	    Data::Dumper->Dump([$WORLD]);');
    isnt_def_sub();

	_test('use Module::Load "load","autoload","none";
			autoload_remote("Data::Dumper");
	    Data::Dumper->Dump([$WORLD]);');
    isnt_def_sub();

	_test('use Module::Load "load","autoload";
			load("Data::Dumper", "Dumper");
			autoload("Carp");
	    croak( Dumper([$WORLD]) );');
    is_peace_in_world();

	_test('use Module::Load "load","autoload";
			load_remote("Data::Dumper");');
    isnt_def_sub();

	_test('use Module::Load "load","autoload";
			autoload_remote("Data::Dumper");');
    isnt_def_sub();

	_test('use Module::Load "load","autoload","none";
			autoload_remote("Data::Dumper");
	    Data::Dumper->Dump([$WORLD]);');
    isnt_def_sub();

	_test('use Module::Load "load","autoload","load_remote","autoload_remote";
			load("Carp");
			autoload("Data::Dumper");
			load_remote("XYZ::Module", "Carp");
			autoload_remote("XYZ::Module", "Carp");
	    Dumper([$WORLD]);');
    is_peace_in_world();

	_test('use Module::Load "all";
			load("Carp");
			autoload("Data::Dumper");
			load_remote("XYZ::Module" => "Carp");
			autoload_remote("XYZ::Module" => "Carp");
	    Dumper([$WORLD]);');
    is_peace_in_world();

	_test('use Module::Load "all","";
			load("Carp");');
    isnt_def_sub();

    done_testing();
};

subtest 'dumpxs' => sub{
    unless ( $Config::Config{usedl} ) {
      plan skip_all => 'Statically linked perl';
    }
    _test('use Module::Load;
	    load("Data::Dumper","Dumper","DumperX");
    	    Data::Dumper->Dump([$WORLD]);');
    is_peace_in_world();

	_test('use Module::Load "all";
	    load("Data::Dumper","Dumper","DumperX");
    	    DumperX([$WORLD]);');
    is_peace_in_world();

    _test('use Module::Load;
			Module::Load::autoload("Data::Dumper","Dumper","DumperX");
    	    DumperX($WORLD);');
    is_peace_in_world();

    _test('use Module::Load "all";
			autoload("Data::Dumper","Dumper","DumperX");
    	    DumperX($WORLD);');
    is_peace_in_world();

    _test('use Module::Load;
	    Module::Load::load_remote("XYZ::Module","Data::Dumper","Dumper","DumperX");
	    XYZ::Module::Dumper($WORLD);');
    is_peace_in_world();

    _test('use Module::Load "load_remote";
	    load_remote("XYZ::Module","Data::Dumper","Dumper","DumperX");
	    XYZ::Module::Dumper($WORLD);');
    is_peace_in_world();

    _test('use Module::Load "all";
	    load_remote("XYZ::Module","Data::Dumper","Dumper","DumperX");
	    XYZ::Module::Dumper($WORLD);');
    is_peace_in_world();

    _test('use Module::Load;
	    Module::Load::autoload_remote("XYZ::Module","Data::Dumper","Dumper","DumperX");
	    XYZ::Module::DumperX($WORLD);');
    is_peace_in_world();

    _test('use Module::Load "autoload_remote";
	    autoload_remote("XYZ::Module","Data::Dumper","Dumper","DumperX");
	    XYZ::Module::Dumper($WORLD);');
    is_peace_in_world();

    done_testing();
};

done_testing();

