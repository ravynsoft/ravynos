#ifndef foodbusprotocolhfoo
#define foodbusprotocolhfoo

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

int dbus_protocol_setup(const AvahiPoll *poll_api,
                        int _disable_user_service_publishing,
                        int _n_clients_max,
                        int _n_objects_per_client_max,
                        int _n_entries_per_entry_group_max,
                        int force);
void dbus_protocol_shutdown(void);
void dbus_protocol_server_state_changed(AvahiServerState state);

#endif
