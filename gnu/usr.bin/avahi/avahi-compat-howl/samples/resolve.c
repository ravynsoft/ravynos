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
my_resolver(
				sw_discovery			discovery,
				sw_discovery_oid		oid,
				sw_uint32				interface_index,
				sw_const_string		name,
				sw_const_string		type,
				sw_const_string		domain,
				sw_ipv4_address		address,
				sw_port					port,
				sw_octets				text_record,
				sw_uint32				text_record_len,
				sw_opaque_t				extra)
{
	sw_text_record_iterator				it;
	sw_int8									name_buf[16];
	sw_int8									key[SW_TEXT_RECORD_MAX_LEN];
	sw_int8									sval[SW_TEXT_RECORD_MAX_LEN];
	sw_uint8									oval[SW_TEXT_RECORD_MAX_LEN];
	sw_uint32								oval_len;
	sw_result								err = SW_OKAY;

	sw_discovery_cancel(discovery, oid);

	fprintf(stderr, "resolve reply: 0x%x %s %s %s %s %d\n", interface_index, name, type, domain, sw_ipv4_address_name(address, name_buf, 16), port);

	if ((text_record_len > 0) && (text_record) && (*text_record != '\0'))
	{
		err = sw_text_record_iterator_init(&it, text_record, text_record_len);
		sw_check_okay(err, exit);

		while (sw_text_record_iterator_next(it, key, oval, &oval_len) == SW_OKAY)
		{
			fprintf(stderr, "key = %s, data is %d bytes\n", key, oval_len);
		}

		err = sw_text_record_iterator_fina(it);
		sw_check_okay(err, exit);
	}

exit:

	return err;
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

	if (argc != 3)
	{
		fprintf(stderr, "usage: mDNSResolve <name> <type>\n");
		return -1;
	}

	err = sw_discovery_resolve(discovery, 0, argv[1], argv[2], "local.", my_resolver, NULL, &oid);
	sw_check_okay(err, exit);

	err = sw_discovery_run(discovery);
	sw_check_okay(err, exit);

exit:

	return err;
}
