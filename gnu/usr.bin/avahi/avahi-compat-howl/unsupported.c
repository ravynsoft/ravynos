/***
  This file is part of avahi.

  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <avahi-common/gccmacro.h>

#include "howl.h"
#include "warn.h"

AVAHI_GCC_NORETURN
sw_string sw_strdup(AVAHI_GCC_UNUSED sw_const_string str) {
    AVAHI_WARN_UNSUPPORTED_ABORT;
}

AVAHI_GCC_NORETURN
sw_opaque _sw_debug_malloc(
    AVAHI_GCC_UNUSED sw_size_t size,
    AVAHI_GCC_UNUSED sw_const_string function,
    AVAHI_GCC_UNUSED sw_const_string file,
    AVAHI_GCC_UNUSED sw_uint32 line) {
    AVAHI_WARN_UNSUPPORTED_ABORT;
}

AVAHI_GCC_NORETURN
sw_opaque _sw_debug_realloc(
    AVAHI_GCC_UNUSED sw_opaque_t mem,
    AVAHI_GCC_UNUSED sw_size_t size,
    AVAHI_GCC_UNUSED sw_const_string function,
    AVAHI_GCC_UNUSED sw_const_string file,
    AVAHI_GCC_UNUSED sw_uint32 line) {
    AVAHI_WARN_UNSUPPORTED_ABORT;
}

void _sw_debug_free(
    AVAHI_GCC_UNUSED sw_opaque_t mem,
    AVAHI_GCC_UNUSED sw_const_string function,
    AVAHI_GCC_UNUSED sw_const_string file,
    AVAHI_GCC_UNUSED sw_uint32 line) {
    AVAHI_WARN_UNSUPPORTED;
}

AVAHI_GCC_NORETURN
sw_const_string sw_strerror(/* howl sucks */) {
    AVAHI_WARN_UNSUPPORTED_ABORT;
}

sw_result sw_timer_init(AVAHI_GCC_UNUSED sw_timer * self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_timer_fina(AVAHI_GCC_UNUSED sw_timer self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_time_init(AVAHI_GCC_UNUSED sw_time * self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_time_init_now(AVAHI_GCC_UNUSED sw_time * self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_time_fina(AVAHI_GCC_UNUSED sw_time self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

AVAHI_GCC_NORETURN
sw_time sw_time_add(
    AVAHI_GCC_UNUSED sw_time self,
    AVAHI_GCC_UNUSED sw_time y) {
    AVAHI_WARN_UNSUPPORTED_ABORT;
}

AVAHI_GCC_NORETURN
sw_time sw_time_sub(
    AVAHI_GCC_UNUSED sw_time self,
    AVAHI_GCC_UNUSED sw_time y) {
    AVAHI_WARN_UNSUPPORTED_ABORT;
}

AVAHI_GCC_NORETURN
sw_int32 sw_time_cmp(
    AVAHI_GCC_UNUSED sw_time self,
    AVAHI_GCC_UNUSED sw_time y) {
    AVAHI_WARN_UNSUPPORTED_ABORT;
}

sw_result sw_salt_init(
    AVAHI_GCC_UNUSED sw_salt * self,
    AVAHI_GCC_UNUSED int argc,
    AVAHI_GCC_UNUSED char ** argv) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_salt_fina(AVAHI_GCC_UNUSED sw_salt self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_salt_register_socket(
    AVAHI_GCC_UNUSED sw_salt self,
    AVAHI_GCC_UNUSED struct _sw_socket * _socket,
    AVAHI_GCC_UNUSED sw_socket_event events,
    AVAHI_GCC_UNUSED sw_socket_handler handler,
    AVAHI_GCC_UNUSED sw_socket_handler_func func,
    AVAHI_GCC_UNUSED sw_opaque extra) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_salt_unregister_socket(
    AVAHI_GCC_UNUSED sw_salt self,
    AVAHI_GCC_UNUSED struct _sw_socket * _socket) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}


sw_result sw_salt_register_timer(
    AVAHI_GCC_UNUSED sw_salt self,
    AVAHI_GCC_UNUSED struct _sw_timer * timer,
    AVAHI_GCC_UNUSED sw_time timeout,
    AVAHI_GCC_UNUSED sw_timer_handler handler,
    AVAHI_GCC_UNUSED sw_timer_handler_func func,
    AVAHI_GCC_UNUSED sw_opaque extra) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_salt_unregister_timer(
    AVAHI_GCC_UNUSED sw_salt self,
    AVAHI_GCC_UNUSED struct _sw_timer * timer) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_salt_register_network_interface(
    AVAHI_GCC_UNUSED sw_salt self,
    AVAHI_GCC_UNUSED struct _sw_network_interface * netif,
    AVAHI_GCC_UNUSED sw_network_interface_handler handler,
    AVAHI_GCC_UNUSED sw_network_interface_handler_func func,
    AVAHI_GCC_UNUSED sw_opaque extra) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_salt_unregister_network_interface_handler(AVAHI_GCC_UNUSED sw_salt self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_salt_register_signal(
    AVAHI_GCC_UNUSED sw_salt self,
    AVAHI_GCC_UNUSED struct _sw_signal * _signal,
    AVAHI_GCC_UNUSED sw_signal_handler handler,
    AVAHI_GCC_UNUSED sw_signal_handler_func func,
    AVAHI_GCC_UNUSED sw_opaque extra) {

    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_salt_unregister_signal(
    AVAHI_GCC_UNUSED sw_salt self,
    AVAHI_GCC_UNUSED struct _sw_signal * _signal) {

    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

void sw_print_assert(
    AVAHI_GCC_UNUSED int code,
    AVAHI_GCC_UNUSED sw_const_string assert_string,
    AVAHI_GCC_UNUSED sw_const_string file,
    AVAHI_GCC_UNUSED sw_const_string func,
    AVAHI_GCC_UNUSED int line) {
    AVAHI_WARN_UNSUPPORTED;
}

void sw_print_debug(
    AVAHI_GCC_UNUSED int level,
    AVAHI_GCC_UNUSED sw_const_string format,
    ...) {
    AVAHI_WARN_UNSUPPORTED;
}

sw_result sw_tcp_socket_init(AVAHI_GCC_UNUSED sw_socket * self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_tcp_socket_init_with_desc(
    AVAHI_GCC_UNUSED sw_socket * self,
    AVAHI_GCC_UNUSED sw_sockdesc_t desc) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_udp_socket_init(AVAHI_GCC_UNUSED sw_socket * self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_multicast_socket_init(AVAHI_GCC_UNUSED sw_socket * self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_fina(AVAHI_GCC_UNUSED sw_socket self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_bind(
    AVAHI_GCC_UNUSED sw_socket self,
    AVAHI_GCC_UNUSED sw_ipv4_address address,
    AVAHI_GCC_UNUSED sw_port port) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_join_multicast_group(
    AVAHI_GCC_UNUSED sw_socket self,
    AVAHI_GCC_UNUSED sw_ipv4_address local_address,
    AVAHI_GCC_UNUSED sw_ipv4_address multicast_address,
    AVAHI_GCC_UNUSED sw_uint32 ttl) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_leave_multicast_group(AVAHI_GCC_UNUSED sw_socket self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_listen(
    AVAHI_GCC_UNUSED sw_socket self,
    AVAHI_GCC_UNUSED int qsize) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_connect(
    AVAHI_GCC_UNUSED sw_socket self,
    AVAHI_GCC_UNUSED sw_ipv4_address address,
    AVAHI_GCC_UNUSED sw_port port) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_accept(
    AVAHI_GCC_UNUSED sw_socket self,
    AVAHI_GCC_UNUSED sw_socket * _socket) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_send(
    AVAHI_GCC_UNUSED sw_socket self,
    AVAHI_GCC_UNUSED sw_octets buffer,
    AVAHI_GCC_UNUSED sw_size_t len,
    AVAHI_GCC_UNUSED sw_size_t * bytesWritten) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_sendto(
    AVAHI_GCC_UNUSED sw_socket self,
    AVAHI_GCC_UNUSED sw_octets buffer,
    AVAHI_GCC_UNUSED sw_size_t len,
    AVAHI_GCC_UNUSED sw_size_t * bytesWritten,
    AVAHI_GCC_UNUSED sw_ipv4_address to,
    AVAHI_GCC_UNUSED sw_port port) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_recv(
    AVAHI_GCC_UNUSED sw_socket self,
    AVAHI_GCC_UNUSED sw_octets buffer,
    AVAHI_GCC_UNUSED sw_size_t max,
    AVAHI_GCC_UNUSED sw_size_t * len) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_recvfrom(
    AVAHI_GCC_UNUSED sw_socket self,
    AVAHI_GCC_UNUSED sw_octets buffer,
    AVAHI_GCC_UNUSED sw_size_t max,
    AVAHI_GCC_UNUSED sw_size_t * len,
    AVAHI_GCC_UNUSED sw_ipv4_address * from,
    AVAHI_GCC_UNUSED sw_port * port,
    AVAHI_GCC_UNUSED sw_ipv4_address * dest,
    AVAHI_GCC_UNUSED sw_uint32 * interface_index) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_set_blocking_mode(
    AVAHI_GCC_UNUSED sw_socket self,
    AVAHI_GCC_UNUSED sw_bool blocking_mode) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_set_options(
    AVAHI_GCC_UNUSED sw_socket self,
    AVAHI_GCC_UNUSED sw_socket_options options) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

AVAHI_GCC_NORETURN
sw_ipv4_address sw_socket_ipv4_address(AVAHI_GCC_UNUSED sw_socket self) {
    AVAHI_WARN_UNSUPPORTED_ABORT;
}

AVAHI_GCC_NORETURN
sw_port sw_socket_port(AVAHI_GCC_UNUSED sw_socket self) {
    AVAHI_WARN_UNSUPPORTED_ABORT;
}

AVAHI_GCC_NORETURN
sw_sockdesc_t sw_socket_desc(AVAHI_GCC_UNUSED sw_socket self) {
    AVAHI_WARN_UNSUPPORTED_ABORT;
}

sw_result sw_socket_close(AVAHI_GCC_UNUSED sw_socket self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_options_init(AVAHI_GCC_UNUSED sw_socket_options * self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_options_fina(AVAHI_GCC_UNUSED sw_socket_options self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_options_set_debug(
    AVAHI_GCC_UNUSED sw_socket_options self,
    AVAHI_GCC_UNUSED sw_bool val) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_options_set_nodelay(
    AVAHI_GCC_UNUSED sw_socket_options self,
    AVAHI_GCC_UNUSED sw_bool val) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_options_set_dontroute(
    AVAHI_GCC_UNUSED sw_socket_options self,
    AVAHI_GCC_UNUSED sw_bool val) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_options_set_keepalive(
    AVAHI_GCC_UNUSED sw_socket_options self,
    AVAHI_GCC_UNUSED sw_bool val) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_options_set_linger(
    AVAHI_GCC_UNUSED sw_socket_options self,
    AVAHI_GCC_UNUSED sw_bool onoff,
    AVAHI_GCC_UNUSED sw_uint32 linger) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_options_set_reuseaddr(
    AVAHI_GCC_UNUSED sw_socket_options self,
    AVAHI_GCC_UNUSED sw_bool val) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_options_set_rcvbuf(
    AVAHI_GCC_UNUSED sw_socket_options self,
    AVAHI_GCC_UNUSED sw_uint32 val) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_socket_options_set_sndbuf(
    AVAHI_GCC_UNUSED sw_socket_options self,
    AVAHI_GCC_UNUSED sw_uint32 val) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

AVAHI_GCC_NORETURN
int sw_socket_error_code(void) {
    AVAHI_WARN_UNSUPPORTED_ABORT;
}

sw_result sw_corby_orb_init(
    AVAHI_GCC_UNUSED sw_corby_orb * self,
    AVAHI_GCC_UNUSED sw_salt salt,
    AVAHI_GCC_UNUSED const sw_corby_orb_config * config,
    AVAHI_GCC_UNUSED sw_corby_orb_observer observer,
    AVAHI_GCC_UNUSED sw_corby_orb_observer_func func,
    AVAHI_GCC_UNUSED sw_opaque_t extra) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_orb_fina(AVAHI_GCC_UNUSED sw_corby_orb self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_orb_register_servant(
    AVAHI_GCC_UNUSED sw_corby_orb self,
    AVAHI_GCC_UNUSED sw_corby_servant servant,
    AVAHI_GCC_UNUSED sw_corby_servant_cb cb,
    AVAHI_GCC_UNUSED sw_const_string oid,
    AVAHI_GCC_UNUSED struct _sw_corby_object ** object,
    AVAHI_GCC_UNUSED sw_const_string protocol_name) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_orb_unregister_servant(
    AVAHI_GCC_UNUSED sw_corby_orb self,
    AVAHI_GCC_UNUSED sw_const_string oid) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_orb_register_bidirectional_object(
    AVAHI_GCC_UNUSED sw_corby_orb self,
    AVAHI_GCC_UNUSED struct _sw_corby_object * object) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_orb_register_channel(
    AVAHI_GCC_UNUSED sw_corby_orb self,
    AVAHI_GCC_UNUSED struct _sw_corby_channel * channel) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

AVAHI_GCC_NORETURN
sw_corby_orb_delegate sw_corby_orb_get_delegate(AVAHI_GCC_UNUSED sw_corby_orb self) {
    AVAHI_WARN_UNSUPPORTED_ABORT;
}

sw_result sw_corby_orb_set_delegate(
    AVAHI_GCC_UNUSED sw_corby_orb self,
    AVAHI_GCC_UNUSED sw_corby_orb_delegate delegate) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_orb_set_observer(
    AVAHI_GCC_UNUSED sw_corby_orb self,
    AVAHI_GCC_UNUSED sw_corby_orb_observer observer,
    AVAHI_GCC_UNUSED sw_corby_orb_observer_func func,
    AVAHI_GCC_UNUSED sw_opaque_t extra) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_orb_protocol_to_address(
    AVAHI_GCC_UNUSED sw_corby_orb self,
    AVAHI_GCC_UNUSED sw_const_string tag,
    AVAHI_GCC_UNUSED sw_string addr,
    AVAHI_GCC_UNUSED sw_port * port) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_orb_protocol_to_url(
    AVAHI_GCC_UNUSED sw_corby_orb self,
    AVAHI_GCC_UNUSED sw_const_string tag,
    AVAHI_GCC_UNUSED sw_const_string name,
    AVAHI_GCC_UNUSED sw_string url,
    AVAHI_GCC_UNUSED sw_size_t url_len) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_orb_read_channel(
    AVAHI_GCC_UNUSED sw_corby_orb self,
    AVAHI_GCC_UNUSED struct _sw_corby_channel * channel) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_orb_dispatch_message(
    AVAHI_GCC_UNUSED sw_corby_orb self,
    AVAHI_GCC_UNUSED struct _sw_corby_channel * channel,
    AVAHI_GCC_UNUSED struct _sw_corby_message * message,
    AVAHI_GCC_UNUSED struct _sw_corby_buffer * buffer,
    AVAHI_GCC_UNUSED sw_uint8 endian) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_message_init(AVAHI_GCC_UNUSED sw_corby_message * self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_message_fina(AVAHI_GCC_UNUSED sw_corby_message self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_init(AVAHI_GCC_UNUSED sw_corby_buffer * self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_init_with_size(
    AVAHI_GCC_UNUSED sw_corby_buffer * self,
    AVAHI_GCC_UNUSED sw_size_t size) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_init_with_delegate(
    AVAHI_GCC_UNUSED sw_corby_buffer * self,
    AVAHI_GCC_UNUSED sw_corby_buffer_delegate delegate,
    AVAHI_GCC_UNUSED sw_corby_buffer_overflow_func overflow,
    AVAHI_GCC_UNUSED sw_corby_buffer_underflow_func underflow,
    AVAHI_GCC_UNUSED sw_opaque_t extra) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_init_with_size_and_delegate(
    AVAHI_GCC_UNUSED sw_corby_buffer * self,
    AVAHI_GCC_UNUSED sw_size_t size,
    AVAHI_GCC_UNUSED sw_corby_buffer_delegate delegate,
    AVAHI_GCC_UNUSED sw_corby_buffer_overflow_func overflow,
    AVAHI_GCC_UNUSED sw_corby_buffer_underflow_func underflow,
    AVAHI_GCC_UNUSED sw_opaque_t extra) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_fina(AVAHI_GCC_UNUSED sw_corby_buffer self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

void sw_corby_buffer_reset(AVAHI_GCC_UNUSED sw_corby_buffer self) {
    AVAHI_WARN_UNSUPPORTED;
}

sw_result sw_corby_buffer_set_octets(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_octets octets,
    AVAHI_GCC_UNUSED sw_size_t size) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_octets sw_corby_buffer_octets(AVAHI_GCC_UNUSED sw_corby_buffer self) {
    AVAHI_WARN_UNSUPPORTED;
    return NULL;
}

sw_size_t sw_corby_buffer_bytes_used(AVAHI_GCC_UNUSED sw_corby_buffer self) {
    AVAHI_WARN_UNSUPPORTED;
    return 0;
}

sw_size_t sw_corby_buffer_size(AVAHI_GCC_UNUSED sw_corby_buffer self) {
    AVAHI_WARN_UNSUPPORTED;
    return 0;
}

sw_result sw_corby_buffer_put_int8(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_int8 val) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_put_uint8(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_uint8 val) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_put_int16(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_int16 val) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_put_uint16(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_uint16 val) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_put_int32(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_int32 val) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_put_uint32(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_uint32 val) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_put_octets(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_const_octets val,
    AVAHI_GCC_UNUSED sw_size_t size) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_put_sized_octets(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_const_octets val,
    AVAHI_GCC_UNUSED sw_uint32 len) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_put_cstring(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_const_string val) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_put_object(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED const struct _sw_corby_object * object) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_put_pad(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_corby_buffer_pad pad) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_get_int8(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_int8 * val) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_get_uint8(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_uint8 * val) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_get_int16(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_int16 * val,
    AVAHI_GCC_UNUSED sw_uint8 endian) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_get_uint16(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_uint16 * val,
    AVAHI_GCC_UNUSED sw_uint8 endian) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_get_int32(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_int32 * val,
    AVAHI_GCC_UNUSED sw_uint8 endian) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_get_uint32(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_uint32 * val,
    AVAHI_GCC_UNUSED sw_uint8 endian) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_get_octets(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_octets octets,
    AVAHI_GCC_UNUSED sw_size_t size) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_allocate_and_get_sized_octets(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_octets * val,
    AVAHI_GCC_UNUSED sw_uint32 * size,
    AVAHI_GCC_UNUSED sw_uint8 endian) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_get_zerocopy_sized_octets(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_octets * val,
    AVAHI_GCC_UNUSED sw_uint32 * size,
    AVAHI_GCC_UNUSED sw_uint8 endian) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_get_sized_octets(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_octets val,
    AVAHI_GCC_UNUSED sw_uint32 * len,
    AVAHI_GCC_UNUSED sw_uint8 endian) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_allocate_and_get_cstring(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_string * val,
    AVAHI_GCC_UNUSED sw_uint32 * len,
    AVAHI_GCC_UNUSED sw_uint8 endian) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_get_zerocopy_cstring(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_string * val,
    AVAHI_GCC_UNUSED sw_uint32 * len,
    AVAHI_GCC_UNUSED sw_uint8 endian) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_get_cstring(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED sw_string val,
    AVAHI_GCC_UNUSED sw_uint32 * len,
    AVAHI_GCC_UNUSED sw_uint8 endian) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_buffer_get_object(
    AVAHI_GCC_UNUSED sw_corby_buffer self,
    AVAHI_GCC_UNUSED struct _sw_corby_object ** object,
    AVAHI_GCC_UNUSED sw_uint8 endian) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_channel_start_request(
    AVAHI_GCC_UNUSED sw_corby_channel self,
    AVAHI_GCC_UNUSED sw_const_corby_profile profile,
    AVAHI_GCC_UNUSED struct _sw_corby_buffer ** buffer,
    AVAHI_GCC_UNUSED sw_const_string op,
    AVAHI_GCC_UNUSED sw_uint32 oplen,
    AVAHI_GCC_UNUSED sw_bool reply_expected) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_channel_start_reply(
    AVAHI_GCC_UNUSED sw_corby_channel self,
    AVAHI_GCC_UNUSED struct _sw_corby_buffer ** buffer,
    AVAHI_GCC_UNUSED sw_uint32 request_id,
    AVAHI_GCC_UNUSED sw_corby_reply_status status) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_channel_send(
    AVAHI_GCC_UNUSED sw_corby_channel self,
    AVAHI_GCC_UNUSED struct _sw_corby_buffer * buffer,
    AVAHI_GCC_UNUSED sw_corby_buffer_observer observer,
    AVAHI_GCC_UNUSED sw_corby_buffer_written_func func,
    AVAHI_GCC_UNUSED sw_opaque_t extra) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_channel_recv(
    AVAHI_GCC_UNUSED sw_corby_channel self,
    AVAHI_GCC_UNUSED sw_salt * salt,
    AVAHI_GCC_UNUSED struct _sw_corby_message ** message,
    AVAHI_GCC_UNUSED sw_uint32 * request_id,
    AVAHI_GCC_UNUSED sw_string * op,
    AVAHI_GCC_UNUSED sw_uint32 * op_len,
    AVAHI_GCC_UNUSED struct _sw_corby_buffer ** buffer,
    AVAHI_GCC_UNUSED sw_uint8 * endian,
    AVAHI_GCC_UNUSED sw_bool block) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_channel_last_recv_from(
    AVAHI_GCC_UNUSED sw_corby_channel self,
    AVAHI_GCC_UNUSED sw_ipv4_address * from,
    AVAHI_GCC_UNUSED sw_port * from_port) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_channel_ff(
    AVAHI_GCC_UNUSED sw_corby_channel self,
    AVAHI_GCC_UNUSED struct _sw_corby_buffer * buffer) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

AVAHI_GCC_NORETURN
sw_socket sw_corby_channel_socket(AVAHI_GCC_UNUSED sw_corby_channel self) {
    AVAHI_WARN_UNSUPPORTED_ABORT;
}

sw_result sw_corby_channel_retain(AVAHI_GCC_UNUSED sw_corby_channel self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_channel_set_delegate(
    AVAHI_GCC_UNUSED sw_corby_channel self,
    AVAHI_GCC_UNUSED sw_corby_channel_delegate delegate) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

AVAHI_GCC_NORETURN
sw_corby_channel_delegate sw_corby_channel_get_delegate(
    AVAHI_GCC_UNUSED sw_corby_channel self) {
    AVAHI_WARN_UNSUPPORTED_ABORT;
}

void sw_corby_channel_set_app_data(
    AVAHI_GCC_UNUSED sw_corby_channel self,
    AVAHI_GCC_UNUSED sw_opaque app_data) {
    AVAHI_WARN_UNSUPPORTED;
}

AVAHI_GCC_NORETURN
sw_opaque sw_corby_channel_get_app_data(AVAHI_GCC_UNUSED sw_corby_channel self) {
    AVAHI_WARN_UNSUPPORTED_ABORT;
}

sw_result sw_corby_channel_fina(AVAHI_GCC_UNUSED sw_corby_channel self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_object_init_from_url(
    AVAHI_GCC_UNUSED sw_corby_object * self,
    AVAHI_GCC_UNUSED struct _sw_corby_orb * orb,
    AVAHI_GCC_UNUSED sw_const_string url,
    AVAHI_GCC_UNUSED sw_socket_options options,
    AVAHI_GCC_UNUSED sw_uint32 bufsize) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_object_fina(
    AVAHI_GCC_UNUSED sw_corby_object self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_object_start_request(
    AVAHI_GCC_UNUSED sw_corby_object self,
    AVAHI_GCC_UNUSED sw_const_string op,
    AVAHI_GCC_UNUSED sw_uint32 op_len,
    AVAHI_GCC_UNUSED sw_bool reply_expected,
    AVAHI_GCC_UNUSED sw_corby_buffer * buffer) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_object_send(
    AVAHI_GCC_UNUSED sw_corby_object self,
    AVAHI_GCC_UNUSED sw_corby_buffer buffer,
    AVAHI_GCC_UNUSED sw_corby_buffer_observer observer,
    AVAHI_GCC_UNUSED sw_corby_buffer_written_func func,
    AVAHI_GCC_UNUSED sw_opaque extra) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_object_recv(
    AVAHI_GCC_UNUSED sw_corby_object self,
    AVAHI_GCC_UNUSED sw_corby_message * message,
    AVAHI_GCC_UNUSED sw_corby_buffer * buffer,
    AVAHI_GCC_UNUSED sw_uint8 * endian,
    AVAHI_GCC_UNUSED sw_bool block) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_object_channel(
    AVAHI_GCC_UNUSED sw_corby_object self,
    AVAHI_GCC_UNUSED sw_corby_channel * channel) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_corby_object_set_channel(
    AVAHI_GCC_UNUSED sw_corby_object self,
    AVAHI_GCC_UNUSED sw_corby_channel channel) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_discovery_publish_host(
    AVAHI_GCC_UNUSED sw_discovery self,
    AVAHI_GCC_UNUSED sw_uint32 interface_index,
    AVAHI_GCC_UNUSED sw_const_string name,
    AVAHI_GCC_UNUSED sw_const_string domain,
    AVAHI_GCC_UNUSED sw_ipv4_address address,
    AVAHI_GCC_UNUSED sw_discovery_publish_reply reply,
    AVAHI_GCC_UNUSED sw_opaque extra,
    AVAHI_GCC_UNUSED sw_discovery_oid * oid) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_discovery_publish_update(
    AVAHI_GCC_UNUSED sw_discovery self,
    AVAHI_GCC_UNUSED sw_discovery_oid oid,
    AVAHI_GCC_UNUSED sw_octets text_record,
    AVAHI_GCC_UNUSED sw_uint32 text_record_len) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_discovery_query_record(
    AVAHI_GCC_UNUSED sw_discovery self,
    AVAHI_GCC_UNUSED sw_uint32 interface_index,
    AVAHI_GCC_UNUSED sw_uint32 flags,
    AVAHI_GCC_UNUSED sw_const_string fullname,
    AVAHI_GCC_UNUSED sw_uint16 rrtype,
    AVAHI_GCC_UNUSED sw_uint16 rrclass,
    AVAHI_GCC_UNUSED sw_discovery_query_record_reply reply,
    AVAHI_GCC_UNUSED sw_opaque extra,
    AVAHI_GCC_UNUSED sw_discovery_oid * oid) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_text_record_string_iterator_init(
    AVAHI_GCC_UNUSED sw_text_record_string_iterator * self,
    AVAHI_GCC_UNUSED sw_const_string text_record_string) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_text_record_string_iterator_fina(
    AVAHI_GCC_UNUSED sw_text_record_string_iterator self) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}

sw_result sw_text_record_string_iterator_next(
    AVAHI_GCC_UNUSED sw_text_record_string_iterator self,
    AVAHI_GCC_UNUSED char key[255],
    AVAHI_GCC_UNUSED char val[255]) {
    AVAHI_WARN_UNSUPPORTED;
    return SW_E_NO_IMPL;
}
