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

using System;
using System.Net;
using System.Runtime.InteropServices;
using System.Text;
using Mono.Unix;

namespace Avahi
{

    [Flags]
    public enum PublishFlags {
        None = 0,
        Unique = 1,
        NoProbe = 2,
        NoAnnounce = 4,
        AllowMultiple = 8,
        NoReverse = 16,
        NoCookie = 32,
        Update = 64,
        UseWideArea = 128,
        UseMulticast = 256
    }

    public enum EntryGroupState {
        Uncommited,
        Registering,
        Established,
        Collision,
        Failure
    }

    public class EntryGroupStateArgs : EventArgs
    {
        private EntryGroupState state;

        public EntryGroupState State
        {
            get { return state; }
        }

        public EntryGroupStateArgs (EntryGroupState state)
        {
            this.state = state;
        }
    }

    internal delegate void EntryGroupCallback (IntPtr group, EntryGroupState state, IntPtr userdata);
    public delegate void EntryGroupStateHandler (object o, EntryGroupStateArgs args);

    public class EntryGroup : IDisposable
    {
        private Client client;
        private IntPtr handle;
        private EntryGroupCallback cb;

        [DllImport ("avahi-client")]
        private static extern IntPtr avahi_entry_group_new (IntPtr client, EntryGroupCallback cb, IntPtr userdata);

        [DllImport ("avahi-client")]
        private static extern int avahi_entry_group_commit (IntPtr group);

        [DllImport ("avahi-client")]
        private static extern int avahi_entry_group_reset (IntPtr group);

        [DllImport ("avahi-client")]
        private static extern EntryGroupState avahi_entry_group_get_state (IntPtr group);

        [DllImport ("avahi-client")]
        private static extern bool avahi_entry_group_is_empty (IntPtr group);

        [DllImport ("avahi-client")]
        private static extern int avahi_entry_group_add_service_strlst (IntPtr group, int iface, Protocol proto,
                                                                        PublishFlags flags, byte[] name, byte[] type,
                                                                        byte[] domain, byte[] host, UInt16 port,
                                                                        IntPtr strlst);

        [DllImport ("avahi-client")]
        private static extern int avahi_entry_group_update_service_strlst (IntPtr group, int iface, Protocol proto,
                                                                           PublishFlags flags, byte[] name,
                                                                           byte[] type, byte[] domain, IntPtr strlst);

        [DllImport ("avahi-client")]
        private static extern int avahi_entry_group_add_service_subtype (IntPtr group, int iface, Protocol proto,
                                                                         PublishFlags flags, byte[] name, byte[] type,
                                                                         byte[] domain, byte[] subtype);

        [DllImport ("avahi-client")]
        private static extern int avahi_entry_group_add_address (IntPtr group, int iface, Protocol proto,
                                                                 PublishFlags flags, byte[] name, IntPtr address);


        [DllImport ("avahi-client")]
        private static extern int avahi_entry_group_add_record (IntPtr group, int iface, Protocol proto,
                                                                PublishFlags flags, byte[] name, RecordClass clazz,
                                                                RecordType type, uint ttl, byte[] rdata, int size);

        [DllImport ("avahi-client")]
        private static extern void avahi_entry_group_free (IntPtr group);

        [DllImport ("avahi-common")]
        private static extern IntPtr avahi_string_list_new (IntPtr txt);

        [DllImport ("avahi-common")]
        private static extern IntPtr avahi_string_list_add (IntPtr list, byte[] txt);

        [DllImport ("avahi-common")]
        private static extern void avahi_string_list_free (IntPtr list);

        [DllImport ("avahi-common")]
        private static extern IntPtr avahi_alternative_service_name (byte[] name);

        public event EntryGroupStateHandler StateChanged;

        public EntryGroupState State
        {
            get {
                lock (client) {
                    return avahi_entry_group_get_state (handle);
                }
            }
        }

        public bool IsEmpty
        {
            get {
                lock (client) {
                    return avahi_entry_group_is_empty (handle);
                }
            }
        }

        public EntryGroup (Client client)
        {
            this.client = client;
            cb = OnEntryGroupCallback;

            lock (client) {
                handle = avahi_entry_group_new (client.Handle, cb, IntPtr.Zero);
                if (handle == IntPtr.Zero)
                    client.ThrowError ();
            }
        }

        ~EntryGroup ()
        {
            Dispose ();
        }

        public void Dispose ()
        {
            if (client.Handle != IntPtr.Zero && handle != IntPtr.Zero) {
                lock (client) {
                    avahi_entry_group_free (handle);
                    handle = IntPtr.Zero;
                }
            }
        }

        public void Commit ()
        {
            lock (client) {
                if (avahi_entry_group_commit (handle) < 0)
                    client.ThrowError ();
            }
        }

        public void Reset ()
        {
            lock (client) {
                if (avahi_entry_group_reset (handle) < 0)
                    client.ThrowError ();
            }
        }

        public void AddService (string name, string type, string domain,
                                UInt16 port, params string[] txt)
        {
            AddService (PublishFlags.None, name, type, domain, port, txt);
        }

        public void AddService (PublishFlags flags, string name, string type, string domain,
                                UInt16 port, params string[] txt)
        {
            AddService (-1, Protocol.Unspecified, flags, name, type, domain, null, port, txt);
        }

        public void AddService (int iface, Protocol proto, PublishFlags flags, string name, string type, string domain,
                                string host, UInt16 port, params string[] txt)
        {
            IntPtr list = avahi_string_list_new (IntPtr.Zero);

            if (txt != null) {
                foreach (string item in txt) {
                    list = avahi_string_list_add (list, Utility.StringToBytes (item));
                }
            }

            AddService (iface, proto, flags, name, type, domain, host, port, list);
        }

        public void AddService (int iface, Protocol proto, PublishFlags flags, string name, string type, string domain,
                                string host, UInt16 port, params byte[][] txt)
        {
            IntPtr list = avahi_string_list_new (IntPtr.Zero);

            if (txt != null) {
                foreach (byte[] item in txt) {
                    list = avahi_string_list_add (list, item);
                }
            }

            AddService (iface, proto, flags, name, type, domain, host, port, list);
        }

        private void AddService (int iface, Protocol proto, PublishFlags flags, string name, string type,
                                 string domain, string host, UInt16 port, IntPtr list)
        {
            int ret;

            lock (client) {
                ret = avahi_entry_group_add_service_strlst (handle, iface, proto, flags,
                                                            Utility.StringToBytes (name),
                                                            Utility.StringToBytes (type),
                                                            Utility.StringToBytes (domain),
                                                            Utility.StringToBytes (host), port, list);
            }

            avahi_string_list_free (list);

            if (ret < 0) {
                client.ThrowError ();
            }
        }

        public void UpdateService (string name, string type, string domain, params string[] txt)
        {
            UpdateService (-1, Protocol.Unspecified, PublishFlags.None, name, type, domain, txt);
        }

        public void UpdateService (int iface, Protocol proto, PublishFlags flags, string name, string type,
                                   string domain, params string[] txt)
        {
            IntPtr list = avahi_string_list_new (IntPtr.Zero);

            if (txt != null) {
                foreach (string item in txt) {
                    list = avahi_string_list_add (list, Utility.StringToBytes (item));
                }
            }

            UpdateService (iface, proto, flags, name, type, domain, list);
        }

        public void UpdateService (int iface, Protocol proto, PublishFlags flags, string name, string type,
                                   string domain, params byte[][] txt)
        {
            IntPtr list = avahi_string_list_new (IntPtr.Zero);

            if (txt != null) {
                foreach (byte[] item in txt) {
                    list = avahi_string_list_add (list, item);
                }
            }

            UpdateService (iface, proto, flags, name, type, domain, list);
        }

        private void UpdateService (int iface, Protocol proto, PublishFlags flags, string name, string type,
                                    string domain, IntPtr list)
        {
            lock (client) {
                int ret = avahi_entry_group_update_service_strlst (handle, iface, proto, flags,
                                                                   Utility.StringToBytes (name),
                                                                   Utility.StringToBytes (type),
                                                                   Utility.StringToBytes (domain),
                                                                   list);

                avahi_string_list_free (list);

                if (ret < 0) {
                    client.ThrowError ();
                }
            }
        }

        public void AddServiceSubtype (string name, string type, string domain, string subtype)
        {
            AddServiceSubtype (-1, Protocol.Unspecified, PublishFlags.None, name, type, domain, subtype);
        }

        public void AddServiceSubtype (int iface, Protocol proto, PublishFlags flags, string name,
                                       string type, string domain, string subtype)
        {
            lock (client) {
                int ret = avahi_entry_group_add_service_subtype (handle, iface, proto, flags,
                                                                 Utility.StringToBytes (name),
                                                                 Utility.StringToBytes (type),
                                                                 Utility.StringToBytes (domain),
                                                                 Utility.StringToBytes (subtype));

                if (ret < 0) {
                    client.ThrowError ();
                }
            }
        }

        public void AddAddress (string name, IPAddress address)
        {
            AddAddress (-1, Protocol.Unspecified, PublishFlags.None, name, address);
        }

        public void AddAddress (int iface, Protocol proto, PublishFlags flags, string name, IPAddress address)
        {
            IntPtr addressPtr = Utility.AddressToPtr (address);

            lock (client) {
                int ret = avahi_entry_group_add_address (handle, iface, proto, flags,
                                                         Utility.StringToBytes (name), addressPtr);

                Utility.Free (addressPtr);

                if (ret < 0) {
                    client.ThrowError ();
                }
            }
        }

        public void AddRecord (string name, RecordClass clazz, RecordType type, uint ttl, byte[] rdata, int length)
        {
            AddRecord (-1, Protocol.Unspecified, PublishFlags.None, name, clazz, type, ttl, rdata, length);
        }

        public void AddRecord (int iface, Protocol proto, PublishFlags flags, string name,
                               RecordClass clazz, RecordType type, uint ttl, byte[] rdata, int length)
        {
            lock (client) {
                int ret = avahi_entry_group_add_record (handle, iface, proto, flags,
                                                        Utility.StringToBytes (name),
                                                        clazz, type, ttl, rdata, length);

                if (ret < 0) {
                    client.ThrowError ();
                }
            }
        }

        public static string GetAlternativeServiceName (string name) {
            return Utility.PtrToStringFree (avahi_alternative_service_name (Utility.StringToBytes (name)));
        }

        private void OnEntryGroupCallback (IntPtr group, EntryGroupState state, IntPtr userdata)
        {
            if (StateChanged != null)
                StateChanged (this, new EntryGroupStateArgs (state));
        }
    }
}
