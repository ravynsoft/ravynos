tcp_send $* || return 1
tcp_read -d -t ${TCP_TIMEOUT:=0.3}
return 0
