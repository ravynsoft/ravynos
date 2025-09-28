package Devel::nodb;
*DB::DB = sub { } if 0;
1;
