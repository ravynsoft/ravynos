/*
 * Copyright 2003, 2004, 2004 Porchdog Software. All rights reserved.
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
#include <salt/debug.h>
#include <stdio.h>


static sw_result HOWL_API
query_record_reply(
				sw_discovery								session,
				sw_discovery_oid							oid,
				sw_discovery_query_record_status		status,
				sw_uint32									interface_index,
				sw_const_string							fullname,
				sw_uint16									rrtype,
				sw_uint16									rrclass,
				sw_uint16									rrdatalen,
				sw_const_octets							rrdata,
				sw_uint32									ttl,
				sw_opaque									extra)
{
	sw_ipv4_address address;

	fprintf(stderr, "interface index = 0x%x, fullname is %s\n", interface_index, fullname);

	if ((rrtype == 1) && (rrclass == 1))
	{
		sw_ipv4_address	address;
		sw_char				name[16];

		sw_ipv4_address_init_from_saddr(&address, *(sw_saddr*) rrdata);

		fprintf(stderr, "address is %s\n", sw_ipv4_address_name(address, name, sizeof(name)));
	}

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
	sw_discovery		discovery;
	sw_discovery_oid	oid;
	sw_result			err;

	err = sw_discovery_init(&discovery);
	sw_check_okay(err, exit);

	if (argc != 4)
	{
		fprintf(stderr, "usage: mDNSBrowse <name> <rrtype> <rrclass>\n");
		return -1;
	}

	err = sw_discovery_query_record(discovery, 0, 0, argv[1], atoi(argv[2]), atoi(argv[3]), query_record_reply, NULL, &oid);
	sw_check_okay(err, exit);

	err = sw_discovery_run(discovery);
	sw_check_okay(err, exit);

exit:

	return err;
}
