#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <darwintest.h>

T_DECL(link_ntoa_basic, "link_ntoa converts to proper string")
{
//	u_char	sdl_len;	/* Total length of sockaddr */
//	u_char	sdl_family;	/* AF_LINK */
//	u_short	sdl_index;	/* if != 0, system given index for interface */
//	u_char	sdl_type;	/* interface type */
//	u_char	sdl_nlen;	/* interface name length, no trailing 0 reqd. */
//	u_char	sdl_alen;	/* link level address length */
//	u_char	sdl_slen;	/* link layer selector length */
//	char	sdl_data[12];	/* minimum work area, can be larger;
//				   contains both if name and ll address */

	struct sockaddr_dl sad;
	bzero(&sad, sizeof(sad));
	sad.sdl_nlen = 3;
	sad.sdl_len = 0;
	sad.sdl_data[0] = 'l';
	sad.sdl_data[1] = 'e';
	sad.sdl_data[2] = '0';
	sad.sdl_data[3] = 0x01;
	sad.sdl_data[4] = 0x80;
	sad.sdl_data[5] = 0xc2;
	sad.sdl_data[6] = 0x00;
	sad.sdl_data[7] = 0x00;
	sad.sdl_data[8] = 0x02;
	sad.sdl_data[9] = 0xaa;
	sad.sdl_data[10] = 0xbb;
	sad.sdl_data[11] = 0xcc;
	sad.sdl_alen = 6;

	char *foo = link_ntoa(&sad);

	T_EXPECT_EQ_STR("le0:1.80.c2.0.0.2", foo, NULL);
}

T_DECL(link_ntoa_overflow, "link_ntoa try to overflow")
{
	char sockraw[64];
	struct sockaddr_dl *sad;
	sad = (struct sockaddr_dl *)&sockraw;
	bzero(sad, sizeof(*sad));
	sad->sdl_nlen = 3;
	sad->sdl_len = 0;
	sad->sdl_data[0] = 'l';
	sad->sdl_data[1] = 'e';
	sad->sdl_data[2] = '0';
	sad->sdl_data[3] = 0x11;
	sad->sdl_data[4] = 0x80;
	sad->sdl_data[5] = 0xc2;
	sad->sdl_data[6] = 0x11;
	sad->sdl_data[7] = 0x11;
	sad->sdl_data[8] = 0xa2;
	sad->sdl_data[9] = 0xaa;
	sad->sdl_data[10] = 0xbb;
	sad->sdl_data[11] = 0xcc;
	sockraw[20] = 0xdd;
	sockraw[21] = 0xee;
	sockraw[22] = 0xff;
	sockraw[23] = 0x1a;
	sockraw[24] = 0x1b;
	sockraw[25] = 0x1c;
	sockraw[26] = 0x1d;
	sockraw[27] = 0x1e;
	sockraw[28] = 0x1f;
	sockraw[29] = 0x2a;
	sockraw[30] = 0x2b;
	sockraw[31] = 0x2c;

	/* set the length to something that will fit in the buffer */
	sad->sdl_alen = 20;

	char *foo = link_ntoa(sad);

	char over = foo[64];
	char over2 = foo[65];

	/* this string should be 66 bytes long and exceed the buffer */
	sad->sdl_alen = 21;

	foo = link_ntoa(sad);

	T_EXPECT_EQ_STR("", foo, NULL);

	T_EXPECT_EQ(over, foo[64], "did not overflow");
	T_EXPECT_EQ(over2, foo[65], "did not overflow either");
}

T_DECL(inet_ntop, "inet_ntop")
{
	char *addresses4[] = { "1.2.3.4", "10.0.0.1", "2.2.2.2" };
	char *addresses6[] = { "2001:db8:85a3::8a2e:370:7334", "::1", "::" };
	for (int i = 0; i < sizeof(addresses4)/sizeof(addresses4[0]); i++){
		struct in_addr addr4;
		char buf[64];
		T_EXPECT_EQ(inet_pton(AF_INET, addresses4[i], &addr4), 1, "inet_pton(AF_INET, %s)", addresses4[i]);
		char *str = inet_ntop(AF_INET, &addr4, buf, sizeof(buf));
		T_EXPECT_NOTNULL(str, "inet_ntop(AF_INET) of %s", addresses4[i]);
		T_EXPECT_EQ_STR(str, addresses4[i], "round-trip of %s", addresses4[i]);
	}
	for (int i = 0; i < sizeof(addresses6)/sizeof(addresses6[0]); i++){
		struct in6_addr addr6;
		char buf[64];
		T_EXPECT_EQ(inet_pton(AF_INET6, addresses6[i], &addr6), 1, "inet_pton(AF_INET6, %s)", addresses6[i]);
		char *str = inet_ntop(AF_INET6, &addr6, buf, sizeof(buf));
		T_EXPECT_NOTNULL(str, "inet_ntop(AF_INET6) of %s", addresses6[i]);
		T_EXPECT_EQ_STR(str, addresses6[i], "round-trip of %s", addresses6[i]);
	}
}

struct testcase {
	const char *in_addr;
	const char *expected_out_addr;
};

static const struct testcase test_addrs[] = {
	{ "1:2:3:4:5::1.2.3.4", "1:2:3:4:5:0:102:304" },
	{ "1:0:3:0:5:0:7:8", "1:0:3:0:5:0:7:8" },
	{ "0:0:3:0:0:0:7:8", "0:0:3::7:8" },
	{ "0:0:3:0:5:0:7:8", "::3:0:5:0:7:8" },
	{ "0:0:0:0:0:0:0:0", "::" },
	{ "0:0:0:0:0:1:0:0", "::1:0:0" },
	{ "1:0:0:0:0:0:0:0", "1::" },
	{ "0:0:0:1:0:0:0:0", "0:0:0:1::" },
	{ "1:0:0:0:0:0:0:1", "1::1" },
	{ "1:2:3:4:5:6:0:0", "1:2:3:4:5:6::" },
	{ "1:2:3:4:5:0:0:0", "1:2:3:4:5::" },
};

T_DECL(inet_ntop_resolve_zeroes, "Check for proper behavior when shortening zeroes w/ inet_ntop")
{
	// Take ip addrs as text, convert to binary and back.
	// Upon converting back, they should adhere to the IPv6 guidelines.
	for (int i = 0; i < sizeof(test_addrs)/sizeof(struct testcase); ++i) {
		struct in6_addr addr6;
		char buf[64];
		char *in_addr = test_addrs[i].in_addr;
		char *expected_out_addr = test_addrs[i].expected_out_addr;
		T_EXPECT_EQ(inet_pton(AF_INET6, in_addr, &addr6), 1, "inet_pton(AF_INET6, %s)", in_addr);
		char *str = inet_ntop(AF_INET6, &addr6, buf, sizeof(buf));
		T_EXPECT_NOTNULL(str, "inet_ntop(AF_INET6) of %s", in_addr);
		T_EXPECT_EQ_STR(str, expected_out_addr, NULL);
	}

	// Same test, but step through the possible range of ipv6 values.
	for (int i = 0x0; i < 0x10000; ++i) {
		struct in6_addr addr6;
		char buf[64];
		char in_addr[64];
		sprintf(in_addr, "1:1:1:1:1:1:1:%x", i);
		char *expected_out_addr = in_addr;
		T_QUIET;
		T_EXPECT_EQ(inet_pton(AF_INET6, in_addr, &addr6), 1, "inet_pton(AF_INET6, %s)", in_addr);
		char *str = inet_ntop(AF_INET6, &addr6, buf, sizeof(buf));
		T_QUIET;
		T_EXPECT_NOTNULL(str, "inet_ntop(AF_INET6) of %s", in_addr);
		T_QUIET;
		T_EXPECT_EQ_STR(str, expected_out_addr, NULL);
	}
	T_PASS("Passed ipv6 value testing");

}

static void
conv(const char *addr)
{
	int ret;
	void *retp;

	struct in6_addr addr6;
	memset(&addr6, 0, sizeof addr6);
	ret = inet_pton(AF_INET6, addr, &addr6);
	T_ASSERT_EQ(ret, 1, "inet_pton");

	T_LOG("%s: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n", addr,
			((unsigned char *) &addr6)[0], ((unsigned char *) &addr6)[1], ((unsigned char *) &addr6)[2], ((unsigned char *) &addr6)[3],
			((unsigned char *) &addr6)[4], ((unsigned char *) &addr6)[5], ((unsigned char *) &addr6)[6], ((unsigned char *) &addr6)[7],
			((unsigned char *) &addr6)[8], ((unsigned char *) &addr6)[9], ((unsigned char *) &addr6)[10], ((unsigned char *) &addr6)[11],
			((unsigned char *) &addr6)[12], ((unsigned char *) &addr6)[13], ((unsigned char *) &addr6)[14], ((unsigned char *) &addr6)[15]);

	char buf6[INET6_ADDRSTRLEN];
	memset(buf6, 0, sizeof buf6);
	retp = inet_ntop(AF_INET6, &addr6, buf6, (socklen_t) sizeof buf6);
	T_ASSERT_NOTNULL(retp, "inet_ntop");

	T_LOG("%s: %s\n", addr, buf6);

	T_EXPECT_EQ_STR(addr, buf6, NULL);
}

T_DECL(inet_ntop_PR46867324, "Regression test for PR46867324")
{
	conv("2001:db8::1");
	conv("::192.168.1.2");
	conv("::ffff:10.11.12.13");
}
