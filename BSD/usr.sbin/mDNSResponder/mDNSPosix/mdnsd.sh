#!/bin/sh
# Emacs settings: -*- tab-width: 4 -*-
#
# Copyright (c) 2002-2006 Apple Computer, Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Linux /etc/init.d script to start/stop the mdnsd daemon.
#
# The following lines are used by the *BSD rcorder system to decide
# the order it's going to run the rc.d scripts at startup time.
# PROVIDE: mdnsd
# REQUIRE: NETWORKING

if [ -r /usr/sbin/mdnsd ]; then
    DAEMON=/usr/sbin/mdnsd
else
    DAEMON=/usr/local/sbin/mdnsd
fi

test -r $DAEMON || exit 0

# Some systems have start-stop-daemon, some don't. 
if [ -r /sbin/start-stop-daemon ]; then
	START="start-stop-daemon --start --quiet --exec"
	# Suse Linux doesn't work with symbolic signal names, but we really don't need
	# to specify "-s TERM" since SIGTERM (15) is the default stop signal anway
	# STOP="start-stop-daemon --stop -s TERM --quiet --oknodo --exec"
	STOP="start-stop-daemon --stop --quiet --oknodo --retry 2 --exec"
else
	killmdnsd() {
		kill -TERM `cat /var/run/mdnsd.pid`
		sleep 1
	}
	START=
	STOP=killmdnsd
fi

case "$1" in
    start)
	echo -n "Starting Apple Darwin Multicast DNS / DNS Service Discovery daemon:"
	echo -n " mdnsd"
        $START $DAEMON
	echo "."
	;;
    stop)
        echo -n "Stopping Apple Darwin Multicast DNS / DNS Service Discovery daemon:"
        echo -n " mdnsd" ; $STOP $DAEMON
        echo "."
	;;
    reload|restart|force-reload)
		echo -n "Restarting Apple Darwin Multicast DNS / DNS Service Discovery daemon:"
		$STOP $DAEMON
		$START $DAEMON
		echo -n " mdnsd"
	;;
    *)
	echo "Usage: /etc/init.d/mDNS {start|stop|reload|restart}"
	exit 1
	;;
esac

exit 0
