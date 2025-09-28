#!perl -w
use strict;

BEGIN { warn "Running ".__FILE__."\n" };
BEGIN 
 {
  require "Config.pm";
  die "Config.pm:$@" if $@;
  Config->import;
 }
use File::Compare qw(compare);
use File::Copy qw(copy);
use File::Basename qw(fileparse);

my ($name, $dir) = fileparse($0);
$name =~ s#^(.*)\.PL$#../$1.SH#;
my %opt;
while (@ARGV && $ARGV[0] =~ /^([\w_]+)=(.*)$/)
 {
  $opt{$1}=$2;
  shift(@ARGV);
 }

$opt{CONFIG_H} ||= 'config.h';
$opt{CORE_DIR} ||= '../lib/CORE';

warn "Writing $opt{CONFIG_H}\n";

open(SH, "<", $name) || die "Cannot open $name:$!";
while (<SH>)
 {
  last if /^\s*sed/;
 }
my($term,$file,$pat) = /^\s*sed\s+<<(\S+)\s+>(\S+)\s+(.*)$/;

$file =~ s/^\$(\w+)$/$opt{$1}/g;

my $str = "sub munge\n{\n";

while ($pat =~ s/-e\s+'([^']*)'\s*//)
 {
  my $e = $1;
  $e =~ s/\\([\(\)])/$1/g;
  $e =~ s/\\(\d)/\$$1/g; 
  $str .= "$e;\n";
 }
$str .= "}\n";

eval $str;

die "$str:$@" if $@;

open(H, ">", "$file.new") || die "Cannot open $file.new:$!";
binmode(H);
while (<SH>)
 {
  last if /^$term$/o;
  s/\$([\w_]+)/Config($1)/eg;
  s/`([^\`]*)`/BackTick($1)/eg;
  munge();
  s/\\\$/\$/g;
  s#/[ *\*]*\*/#/**/#;
  s#(.)/\*\*/#$1/ **/# if(/^\/\*/); #avoid "/*" inside comments
  if (/^\s*#define\s+(PRIVLIB|SITELIB|VENDORLIB)_EXP/)
   {
     $_ = '#define '. $1 . '_EXP ('.(
       $1 eq 'PRIVLIB' ? 'PerlEnv_lib_path' :
       $1 eq 'SITELIB' ? 'PerlEnv_sitelib_path' :
       $1 eq 'VENDORLIB' ? 'PerlEnv_vendorlib_path' :
       die "unknown *LIB_EXP define \"$1\""
       ). "(PERL_VERSION_STRING, NULL))\t/**/\n";
   }
  # incpush() handles archlibs, so disable them
  elsif (/^\s*#define\s+(ARCHLIB|SITEARCH|VENDORARCH)_EXP/)
   {
     $_ = "/*#define ". $1 . "_EXP \"\"\t/ **/\n";
   }
  elsif (/^\s*#define\s+CPP(STDIN|RUN)\s+"gcc(.*)"\s*$/)
   {
     $_ = "#define CPP" . $1 . " \"" . $opt{ARCHPREFIX} . "gcc" . $2 . "\"\n";
   }
  print H;
 }
close(H);
close(SH);

my $core_config_h = "$opt{CORE_DIR}/$opt{CONFIG_H}";
if (compare("$file.new", $core_config_h)) {
    mkdir $opt{CORE_DIR} unless -d $opt{CORE_DIR};
    chmod(0666,$core_config_h);
    copy("$file.new",$core_config_h) || die "Cannot copy:$!";
    chmod(0444,$core_config_h);
}

if (compare("$file.new",$file))
 {
  warn "$file has changed\n";
  chmod(0666,$file);
  unlink($file);
  rename("$file.new",$file) || die "Cannot rename:$!";
 }
else
 {
  unlink ("$file.new");
 }

sub Config
{
 my $var = shift;
 my $val = $Config{$var};
 $val = 'undef' unless defined $val;
 $val =~ s/\\/\\\\/g;
 return $val;
}

sub BackTick
{
 my $cmd = shift;
 if ($cmd =~ /^echo\s+(.*?)\s*\|\s+sed\s+'(.*)'\s*$/)
  {
   my($data,$pat) = ($1,$2);
   $data =~ s/\s+/ /g;
   eval "\$data =~ $pat";
   return $data;
  }
 else
  {
   die "Cannot handle \`$cmd\`";
  }
 return $cmd;
}
