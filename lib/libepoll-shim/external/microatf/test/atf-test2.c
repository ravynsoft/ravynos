#include <atf-c.h>

ATF_TC_WITHOUT_HEAD(atf__success);
ATF_TC_BODY(atf__success, tc)
{
}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, atf__success);

	return atf_no_error();
}
