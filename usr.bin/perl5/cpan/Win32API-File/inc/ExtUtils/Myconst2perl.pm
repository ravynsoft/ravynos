# This should eventually become part of MakeMaker as ExtUtils::Mkconst2perl.
# Documentation for this is very skimpy at this point.  Full documentation
# will be added to ExtUtils::Mkconst2perl when it is created.
package # Hide from PAUSE
         ExtUtils::Myconst2perl;

use strict;
use Config;

use vars qw( @ISA @EXPORT @EXPORT_OK $VERSION );
BEGIN {
    require Exporter;
    push @ISA, 'Exporter';
    @EXPORT= qw( &Myconst2perl );
    @EXPORT_OK= qw( &ParseAttribs );
    $VERSION= 1.00;
}

use Carp;
use File::Basename;
use ExtUtils::MakeMaker qw( neatvalue );

# Return the extension to use for a file of C++ source code:
sub _cc
{
    # Some day, $Config{_cc} might be defined for us:
    return $Config{_cc}   if  $Config{_cc};
    return ".cxx";	# Seems to be the most widely accepted extension.
}

=item ParseAttribs

Parses user-firendly options into coder-firendly specifics.

=cut

sub ParseAttribs
{
    # Usage:  ParseAttribs( "Package::Name", \%opts, {opt=>\$var} );
    my( $pkg, $hvAttr, $hvRequests )= @_;
    my( $outfile, @perlfiles, %perlfilecodes, @cfiles, %cfilecodes );
    my @importlist= @{$hvAttr->{IMPORT_LIST}};
    my $perlcode= $hvAttr->{PERL_PE_CODE} ||
	'last if /^\s*(bootstrap|XSLoader::load)\b/';
    my $ccode= $hvAttr->{C_PE_CODE} ||
	'last if m#/[/*]\s*CONSTS_DEFINED\b|^\s*MODULE\b#';
    my $ifdef= $hvAttr->{IFDEF} || 0;
    my $writeperl= !! $hvAttr->{WRITE_PERL};
    my $export= !! $hvAttr->{DO_EXPORT};
    my $importto= $hvAttr->{IMPORT_TO} || "_constants";
    my $cplusplus= $hvAttr->{CPLUSPLUS};
    $cplusplus= ""   if  ! defined $cplusplus;
    my $object= "";
    my $binary= "";
    my $final= "";
    my $norebuild= "";
    my $subroutine= "";
    my $base;
    my %params= (
	PERL_PE_CODE => \$perlcode,
	PERL_FILE_LIST => \@perlfiles,
	PERL_FILE_CODES => \%perlfilecodes,
	PERL_FILES => sub { map {($_,$perlfilecodes{$_})} @perlfiles },
	C_PE_CODE => \$ccode,
	C_FILE_LIST => \@cfiles,
	C_FILE_CODES => \%cfilecodes,
	C_FILES => sub { map {($_,$cfilecodes{$_})} @cfiles },
	DO_EXPORT => \$export,
	IMPORT_TO => \$importto,
	IMPORT_LIST => \@importlist,
	SUBROUTINE => \$subroutine,
	IFDEF => \$ifdef,
	WRITE_PERL => \$writeperl,
	CPLUSPLUS => \$cplusplus,
	BASEFILENAME => \$base,
	OUTFILE => \$outfile,
	OBJECT => \$object,
	BINARY => \$binary,
	FINAL_PERL => \$final,
	NO_REBUILD => \$norebuild,
    );
    {   my @err= grep {! defined $params{$_}} keys %$hvAttr;
	carp "ExtUtils::Myconst2perl::ParseAttribs:  ",
	  "Unsupported option(s) (@err).\n"
	  if  @err;
    }
    $norebuild= $hvAttr->{NO_REBUILD}   if  exists $hvAttr->{NO_REBUILD};
    my $module= ( split /::/, $pkg )[-1];
    $base= "c".$module;
    $base= $hvAttr->{BASEFILENAME}   if  exists $hvAttr->{BASEFILENAME};
    my $ext=  ! $cplusplus  ?  ($Config{_c}||".c")
      :  $cplusplus =~ /^[.]/  ?  $cplusplus  :  _cc();
    if(  $writeperl  ) {
	$outfile= $base . "_pc" . $ext;
	$object= $base . "_pc" . ($Config{_o}||$Config{obj_ext});
	$object= $hvAttr->{OBJECT}   if  $hvAttr->{OBJECT};
	$binary= $base . "_pc" . ($Config{_exe}||$Config{exe_ext});
	$binary= $hvAttr->{BINARY}   if  $hvAttr->{BINARY};
	$final= $base . ".pc";
	$final= $hvAttr->{FINAL_PERL}   if  $hvAttr->{FINAL_PERL};
	$subroutine= "main";
    } elsif(  $cplusplus  ) {
	$outfile= $base . $ext;
	$object= $base . ($Config{_o}||$Config{obj_ext});
	$object= $hvAttr->{OBJECT}   if  $hvAttr->{OBJECT};
	$subroutine= "const2perl_" . $pkg;
	$subroutine =~ s/\W/_/g;
    } else {
	$outfile= $base . ".h";
    }
    $outfile= $hvAttr->{OUTFILE}   if  $hvAttr->{OUTFILE};
    if(  $hvAttr->{PERL_FILES}  ) {
	carp "ExtUtils::Myconst2perl:  PERL_FILES option not allowed ",
	  "with PERL_FILE_LIST nor PERL_FILE_CODES.\n"
	  if  $hvAttr->{PERL_FILE_LIST}  ||  $hvAttr->{PERL_FILE_CODES};
	%perlfilecodes= @{$hvAttr->{PERL_FILES}};
	my $odd= 0;
	@perlfiles= grep {$odd= !$odd} @{$hvAttr->{PERL_FILES}};
    } else {
	if(  $hvAttr->{PERL_FILE_LIST}  ) {
	    @perlfiles= @{$hvAttr->{PERL_FILE_LIST}};
	} elsif(  $hvAttr->{PERL_FILE_CODES}  ) {
	    @perlfiles= keys %{$hvAttr->{PERL_FILE_CODES}};
	} else {
	    @perlfiles= ( "$module.pm" );
	}
	%perlfilecodes= %{$hvAttr->{PERL_FILE_CODES}}
	  if  $hvAttr->{PERL_FILE_CODES};
    }
    for my $file (  @perlfiles  ) {
	$perlfilecodes{$file}= $perlcode  if  ! $perlfilecodes{$file};
    }
    if(  ! $subroutine  ) {
	; # Don't process any C source code files.
    } elsif(  $hvAttr->{C_FILES}  ) {
	carp "ExtUtils::Myconst2perl:  C_FILES option not allowed ",
	  "with C_FILE_LIST nor C_FILE_CODES.\n"
	  if  $hvAttr->{C_FILE_LIST}  ||  $hvAttr->{C_FILE_CODES};
	%cfilecodes= @{$hvAttr->{C_FILES}};
	my $odd= 0;
	@cfiles= grep {$odd= !$odd} @{$hvAttr->{C_FILES}};
    } else {
	if(  $hvAttr->{C_FILE_LIST}  ) {
	    @cfiles= @{$hvAttr->{C_FILE_LIST}};
	} elsif(  $hvAttr->{C_FILE_CODES}  ) {
	    @cfiles= keys %{$hvAttr->{C_FILE_CODES}};
	} elsif(  $writeperl  ||  $cplusplus  ) {
	    @cfiles= ( "$module.xs" );
	}
	%cfilecodes= %{$hvAttr->{C_FILE_CODES}}   if  $hvAttr->{C_FILE_CODES};
    }
    for my $file (  @cfiles  ) {
	$cfilecodes{$file}= $ccode  if  ! $cfilecodes{$file};
    }
    for my $key (  keys %$hvRequests  ) {
	if(  ! $params{$key}  ) {
	    carp "ExtUtils::Myconst2perl::ParseAttribs:  ",
	      "Unsupported output ($key).\n";
	} elsif(  "SCALAR" eq ref( $params{$key} )  ) {
	    ${$hvRequests->{$key}}= ${$params{$key}};
	} elsif(  "ARRAY" eq ref( $params{$key} )  ) {
	    @{$hvRequests->{$key}}= @{$params{$key}};
	} elsif(  "HASH" eq ref( $params{$key} )  ) {
	    %{$hvRequests->{$key}}= %{$params{$key}};
	} elsif(  "CODE" eq ref( $params{$key} )  ) {
	    @{$hvRequests->{$key}}=  &{$params{$key}};
	} else {
	    die "Impossible value in \$params{$key}";
	}
    }
}

=item Myconst2perl

Generates a file used to implement C constants as "constant subroutines" in
a Perl module.

Extracts a list of constants from a module's export list by C<eval>ing the
first part of the Module's F<*.pm> file and then requesting some groups of
symbols be exported/imported into a dummy package.  Then writes C or C++
code that can convert each C constant into a Perl "constant subroutine"
whose name is the constant's name and whose value is the constant's value.

=cut

sub Myconst2perl
{
    my( $pkg, %spec )= @_;
    my( $outfile, $writeperl, $ifdef, $export, $importto, @importlist,
        @perlfile, %perlcode, @cfile, %ccode, $routine );
    ParseAttribs( $pkg, \%spec, {
	DO_EXPORT => \$export,
	IMPORT_TO => \$importto,
	IMPORT_LIST => \@importlist,
	IFDEF => \$ifdef,
	WRITE_PERL => \$writeperl,
	OUTFILE => \$outfile,
	PERL_FILE_LIST => \@perlfile,
	PERL_FILE_CODES => \%perlcode,
	C_FILE_LIST => \@cfile,
	C_FILE_CODES => \%ccode,
	SUBROUTINE => \$routine,
    } );
    my $module= ( split /::/, $pkg )[-1];

    warn "Writing $outfile...\n";
    open( STDOUT, ">$outfile" )  or  die "Can't create $outfile: $!\n";

    my $code= "";
    my $file;
    foreach $file (  @perlfile  ) {
	warn "Reading Perl file, $file:  $perlcode{$file}\n";
	open( MODULE, "<$file" )  or  die "Can't read Perl file, $file: $!\n";
	eval qq[
	    while(  <MODULE>  ) {
		$perlcode{$file};
		\$code .= \$_;
	    }
	    1;
	]  or  die "$file eval: $@\n";
	close( MODULE );
    }

    print
      "/* $outfile - Generated by ExtUtils::Myconst2perl::Myconst2perl */\n";
    if(  $routine  ) {
	print "/* See start of $routine() for generation parameters used */\n";
	#print "#define main _main_proto"
	#  " /* Ignore Perl's main() prototype */\n\n";
	if(  $writeperl  ) {
	    # Here are more reasons why the WRITE_PERL option is discouraged.
	    if(  $Config{useperlio}  ) {
		print "#define PERLIO_IS_STDIO 1\n";
	    }
	    print "#define WIN32IO_IS_STDIO 1\n";	# May cause a warning
	    print "#define NO_XSLOCKS 1\n";	# What a hack!
	}
	foreach $file (  @cfile  ) {
	    warn "Reading C file, $file:  $ccode{$file}\n";
	    open( XS, "<$file" )  or  die "Can't read C file, $file: $!\n";
	    my $code= $ccode{$file};
	    $code =~ s#\\#\\\\#g;
	    $code =~ s#([^\s -~])#"\\x".sprintf "%02X",unpack "C",$1#ge;
	    $code =~ s#[*]/#*\\/#g;
	    print qq[\n/* Include $file:  $code */\n];
	    print qq[\n#line 1 "$file"\n];
	    eval qq[
		while(  <XS>  ) {
		    $ccode{$file};
		    print;
		}
		1;
	    ]  or  die "$file eval: $@\n";
	    close( XS );
	}
	#print qq[\n#undef main\n];
	print qq[\n#define CONST2WRITE_PERL\n];
	print qq[\n#include "const2perl.h"\n\n];
	if(  $writeperl  ) {
	    print "int\nmain( int argc, char *argv[], char *envp[] )\n";
	} else {
	    print "void\n$routine( void )\n";
	}
    }
    print "{\n";

    {
	@ExtUtils::Myconst2perl::importlist= @importlist;
	my $var= '@ExtUtils::Myconst2perl::importlist';
	my $port= $export ? "export" : "import";
	my $arg2= $export ? "q[$importto]," : "";
	local( $^W )= 0;
	eval $code . "{\n"
	  . "    {    package $importto;\n"
	  . "        warn qq[\u${port}ing to $importto: $var\\n];\n"
	  . "        \$pkg->$port( $arg2 $var );\n"
	  . "    }\n"
	  . "    {   no strict 'refs';\n"
	  . "        $var=  sort keys %{'_constants::'};   }\n"
	  . "    warn 0 + $var, qq[ symbols ${port}ed.\\n];\n"
	  . "}\n1;\n"
	  or  die "eval: $@\n";
    }
    my @syms= @ExtUtils::Myconst2perl::importlist;

    my $if;
    my $const;
    print qq[    START_CONSTS( "$pkg" )	/* No ";" */\n];
    {
	my( $head, $tail )= ( "/*", "\n" );
	if(  $writeperl  ) {
	    $head= '    printf( "#';
	    $tail= '\\n" );' . "\n";
	    print $head, " Generated by $outfile.", $tail;
	}
	print $head, " Package $pkg with options:", $tail;
	$head= " *"   if  ! $writeperl;
	my $key;
	foreach $key (  sort keys %spec  ) {
	    my $val= neatvalue($spec{$key});
	    $val =~ s/\\/\\\\/g   if  $writeperl;
	    print $head, "    $key => ", $val, $tail;
	}
	print $head, " Perl files eval'd:", $tail;
	foreach $key (  @perlfile  ) {
	    my $code= $perlcode{$key};
	    $code =~ s#\\#\\\\#g;
	    $code =~ s#([^ -~])#"\\".sprintf "%03o",unpack "C",$1#ge;
	    $code =~ s#"#\\"#g   if  $writeperl;
	    print $head, "    $key => ", $code, $tail;
	}
	if(  $writeperl  ) {
	    print $head, " C files included:", $tail;
	    foreach $key (  @cfile  ) {
		my $code= $ccode{$key};
		$code =~ s#\\#\\\\#g;
		$code =~ s#([^ -~])#"\\".sprintf "%03o",unpack "C",$1#ge;
		$code =~ s#"#\\"#g;
		print $head, "    $key => ", $code, $tail;
	    }
	} else {
	    print " */\n";
	}
    }
    if(  ! ref($ifdef)  &&  $ifdef =~ /[^\s\w]/  ) {
	my $sub= $ifdef;
	$sub= 'sub { local($_)= @_; ' . $sub . ' }'
	  unless  $sub =~ /^\s*sub\b/;
	$ifdef= eval $sub;
	die "$@:  $sub\n"   if  $@;
	if(  "CODE" ne ref($ifdef)  ) {
	    die "IFDEF didn't create subroutine reference:  eval $sub\n";
	}
    }
    foreach $const (  @syms  ) {
	$if=  "CODE" eq ref($ifdef)  ?  $ifdef->($const)  :  $ifdef;
	if(  ! $if  ) {
	    $if= "";
	} elsif(  "1" eq $if  ) {
	    $if= "#ifdef $const\n";
	} elsif(  $if !~ /^#/  ) {
	    $if= "#ifdef $if\n";
	} else {
	    $if= "$if\n";
	}
	print $if
	  . qq[    const2perl( $const );\n];
	if(  $if  ) {
	    print "#else\n"
	      . qq[    noconst( $const );\n]
	      . "#endif\n";
	}
    }
    if(  $writeperl  ) {
	print
	  qq[    printf( "1;\\n" );\n],
	  qq[    return( 0 );\n];
    }
    print "}\n";
}

1;
