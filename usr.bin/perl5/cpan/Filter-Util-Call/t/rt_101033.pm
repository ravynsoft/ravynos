package rt_101033;

use strict;
use Filter::Util::Call;

sub import
{
  filter_add({});
  1;
}
    
sub unimport
{	
  filter_del()
}
      
sub filter
{
  my($self) = @_ ;
  my $status = 1;
  $status = filter_read(1_000_000);
  #print "code: !$_!\n\n";
  return $status;
}

1;
                
