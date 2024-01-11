/*
 * Copyright 2003, 2004 Porchdog Software. All rights reserved.
 *
 *	Redistribution and use in source and binary forms, with or without modification,
 *	are permitted provided that the following conditions are met:
 *
 *		1. Redistributions of source code must retain the above copyright notice,
 *		   this list of conditions and the following disclaimer.
 *		2. Redistributions in binary form must reproduce the above copyright notice,
 *		   this list of conditions and the following disclaimer in the documentation
 *		   and/or other materials provided with the distribution.
 *
 *	THIS SOFTWARE IS PROVIDED BY PORCHDOG SOFTWARE ``AS IS'' AND ANY
 *	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *	IN NO EVENT SHALL THE HOWL PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *	INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *	BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 *	OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *	OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 *	OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	The views and conclusions contained in the software and documentation are those
 *	of the authors and should not be interpreted as representing official policies,
 *	either expressed or implied, of Porchdog Software.
 */

#include <howl.h>
#include <stdio.h>


static sw_result HOWL_API
my_service_reply(
				sw_discovery						discovery,
				sw_discovery_oid					oid,
				sw_discovery_publish_status	status,
				sw_opaque							extra)
{
	static sw_string
	status_text[] =
	{
		"Started",
		"Stopped",
		"Name Collision",
		"Invalid"
	};

	fprintf(stderr, "publish reply: %s\n", status_text[status]);
	return SW_OKAY;
}


#if defined(WIN32)
int __cdecl
#else
int
#endif
main(
	int		argc,
	char	**	argv)
{
	sw_discovery					discovery;
	sw_text_record					text_record;
	sw_result						result;
	sw_discovery_publish_id	id;
	int								i;

	if (sw_discovery_init(&discovery) != SW_OKAY)
	{
		fprintf(stderr, "sw_discovery_init() failed\n");
		return -1;
	}

	if (argc < 4)
	{
		fprintf(stderr, "usage: mDNSPublish <name> <type> <port> [service text 1]...[service text n]\n");
		return -1;
	}

	if (sw_text_record_init(&text_record) != SW_OKAY)
	{
		fprintf(stderr, "sw_text_record_init() failed\n");
		return -1;
	}

	for (i = 4; i < argc; i++)
	{
		if (sw_text_record_add_string(text_record, argv[i]) != SW_OKAY)
		{
			fprintf(stderr, "unable to add service text: %s\n", argv[i]);
			return -1;
		}
	}

	printf("%s %s %d\n", argv[1], argv[2], atoi(argv[3]));

	if ((result = sw_discovery_publish(discovery, 0, argv[1], argv[2], NULL, NULL, atoi(argv[3]), sw_text_record_bytes(text_record), sw_text_record_len(text_record), my_service_reply, NULL, &id)) != SW_OKAY)
	{
		fprintf(stderr, "publish failed: %d\n", result);
		sw_text_record_fina(text_record);
		return -1;
	}

	sw_text_record_fina(text_record);

	sw_discovery_run(discovery);

	return 0;
}
