ARCHS  = -arch i386 -arch x86_64
CFLAGS = -Wall -pipe -O0 -gdwarf-2

REACH_SERVER_C = ../../SystemConfiguration.fproj/reachability/SCNetworkReachabilityServer_server.c

REACH_SERVER_PLIST = /Library/LaunchDaemons/com.apple.SCNetworkReachability.plist
SYMROOT            = ${shell basename ${shell dirname ${shell dirname ${PWD}}}}

all: /var/tmp/reach_server ${REACH_SERVER_PLIST}

${REACH_SERVER_PLIST}: Makefile
	@sudo mkdir -p ${shell dirname ${REACH_SERVER_PLIST}}
	@sudo launchctl unload -w ${REACH_SERVER_PLIST}				> /dev/null 2>&1 || /usr/bin/true
	@sudo rm -f ${REACH_SERVER_PLIST}
	@sudo defaults write ${REACH_SERVER_PLIST} Label			\
			SCNetworkReachability-DEBUG
	@sudo defaults write ${REACH_SERVER_PLIST} MachServices			\
		-dict								\
			SCNetworkReachability-DEBUG				\
				-bool TRUE
	@sudo defaults write ${REACH_SERVER_PLIST} Program			\
			/var/tmp/reach_server
	@sudo defaults write ${REACH_SERVER_PLIST} ProgramArguments		\
		-array								\
			SCNetworkReachability-DEBUG				\
			verbose
	@sudo defaults write ${REACH_SERVER_PLIST} EnvironmentVariables		\
		-dict								\
			DYLD_FRAMEWORK_PATH					\
				${BUILDIT_DIR}/${SYMROOT}.roots/${SYMROOT}~sym	\
			REACH_SERVER						\
				SCNetworkReachability-DEBUG
	@sudo defaults write ${REACH_SERVER_PLIST} ThrottleInterval		\
			-int 5
	@sudo chown root:wheel ${REACH_SERVER_PLIST}
	@sudo chmod 644 ${REACH_SERVER_PLIST}
	sudo launchctl load -w ${REACH_SERVER_PLIST}

reach_server.o: ${REACH_SERVER_C}
	cc ${ARCHS} ${CFLAGS} -DMAIN -I ../../SystemConfiguration.fproj -F ${BUILDIT_DIR}/${SYMROOT}.roots/${SYMROOT}~sym -c -o reach_server.o ${REACH_SERVER_C}

reach_server: reach_server.o
	cc ${ARCHS} -o reach_server reach_server.o -framework CoreFoundation -F ${BUILDIT_DIR}/${SYMROOT}.roots/${SYMROOT}~sym -framework SystemConfiguration
	dsymutil reach_server -o reach_server.dSYM

/var/tmp/reach_server: reach_server
	@sudo rm -f /var/tmp/reach_server
	cp -p reach_server /var/tmp/reach_server
	@sudo chown root:wheel /var/tmp/reach_server
	@sudo chmod 755 /var/tmp/reach_server

clean:
	sudo launchctl unload -w ${REACH_SERVER_PLIST}				> /dev/null 2>&1 || /usr/bin/true
	@sudo rm -f ${REACH_SERVER_PLIST}
	@sudo rm -rf /var/tmp/reach_server
	rm -rf *.o reach_server reach_server.dSYM
