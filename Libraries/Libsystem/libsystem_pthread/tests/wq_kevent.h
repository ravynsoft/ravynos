#define REQUESTS_LEN 4
static struct workq_reqthreads_req_s {unsigned long priority; int count;} requests[REQUESTS_LEN];

#define QOS_STR(q) (q == QOS_CLASS_USER_INTERACTIVE ? "UInter" : (q == QOS_CLASS_USER_INITIATED ? "UInit" : (q == QOS_CLASS_DEFAULT ? "Dflt" : (q == QOS_CLASS_UTILITY ? "Util" : (q == QOS_CLASS_BACKGROUND ? "BG" : "Unkn" ) ) ) ) )
static char* describe_pri(pthread_priority_t pri){
	qos_class_t qos;
	unsigned long flags;

	qos = _pthread_qos_class_decode(pri, NULL, &flags);

	static char desc[32];
	if (flags & _PTHREAD_PRIORITY_EVENT_MANAGER_FLAG){
		sprintf(desc, "E:%s", QOS_STR(qos));
	} else if (flags & _PTHREAD_PRIORITY_OVERCOMMIT_FLAG){
		sprintf(desc, "O:%s", QOS_STR(qos));
	} else {
		sprintf(desc, "%s", QOS_STR(qos));
	}

	return desc;
}

static char* describe_req(void){
	static char desc[256];
	char *c = desc;

	*c++ = '[';
	for (int i = 0; i < REQUESTS_LEN; i++){
		if (i) *c++ = ',';
		c += sprintf(c, "{%s,%d}", describe_pri(requests[i].priority), requests[i].count);
	}
	*c++ = ']';
	*c++ = '\0';

	return desc;
}

static char *dummy_text = "Four score and seven years ago our fathers brought forth on this continent a new nation, conceived in liberty, and dedicated to the";
// takes about 1us on my machine
static void burn_cpu(void){
	char key[64]; char txt[64];
	strncpy(txt, dummy_text, 64);
	for (int i = 0; i < 64; i++)
		key[i] = rand() % 1;
	setkey(key);
	encrypt(txt, 0);
	encrypt(txt, 1);
}

static int do_req(void){
	int ret = sysctlbyname("debug.wq_kevent_test", NULL, NULL, requests, sizeof(requests));
	if (ret >= 0){
		fprintf(stderr, "wq_kevent_test(%s) -> %d\n", describe_req(), ret);
	} else {
		perror("debug.wk_kevent_test");
		return errno;
	}
	return ret;
}
