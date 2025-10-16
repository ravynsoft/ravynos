/* !checksrc! disable COPYRIGHT all */

#include "first.h"

#include "memptr.c"
#include "getpart.c"
#include "util.c"
#include "../../lib/curlx/base64.c"
#include "../../lib/curlx/inet_pton.c"
#include "../../lib/curlx/inet_ntop.c"
#include "../../lib/curlx/multibyte.c"
#include "../../lib/curlx/nonblock.c"
#include "../../lib/curlx/strparse.c"
#include "../../lib/curlx/timediff.c"
#include "../../lib/curlx/timeval.c"
#include "../../lib/curlx/version_win32.c"
#include "../../lib/curlx/wait.c"
#include "../../lib/curlx/warnless.c"
#include "../../lib/curlx/winapi.c"
#include "dnsd.c"
#include "mqttd.c"
#include "resolve.c"
#include "rtspd.c"
#include "sockfilt.c"
#include "socksd.c"
#include "sws.c"
#include "tftpd.c"

const struct entry_s s_entries[] = {
  {"dnsd", test_dnsd},
  {"mqttd", test_mqttd},
  {"resolve", test_resolve},
  {"rtspd", test_rtspd},
  {"sockfilt", test_sockfilt},
  {"socksd", test_socksd},
  {"sws", test_sws},
  {"tftpd", test_tftpd},
  {NULL, NULL}
};

#include "first.c"
