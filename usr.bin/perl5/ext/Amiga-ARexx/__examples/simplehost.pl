#!perl

# Simple ARExx Host

use strict;
use Amiga::ARexx;
use feature "switch";

my $host = Amiga::ARexx->new('HostName' => "TESTSCRIPT");

my $alive = 1;

while ($alive)
{
	$host->wait();
    my $msg = $host->getmsg();
	while($msg)
	{
		my $rc = 0;
		my $rc2 = 0;
		my $result = "";

		print $msg->message . "\n";
		given($msg->message)
		{
			when ("QUIT")
			{
				$alive = 0;
				$result = "quitting!";
			}
			when ("SHOUT")
			{
				$result = "HEEELLLLOOOO!";
			}
			default {
				$rc = 10;
				$rc2 = 22;
			}
		}
		$msg->reply($rc,$rc2,$result);

		$msg = $host->getmsg();
	}

}

