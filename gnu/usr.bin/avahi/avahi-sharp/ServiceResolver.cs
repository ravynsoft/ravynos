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
using System.Collections;
using System.Net;
using System.Runtime.InteropServices;
using System.Text;
using Mono.Unix;

namespace Avahi
{

    internal delegate void ServiceResolverCallback (IntPtr resolver, int iface, Protocol proto,
                                                    ResolverEvent revent, IntPtr name, IntPtr type,
                                                    IntPtr domain, IntPtr host, IntPtr address,
                                                    UInt16 port, IntPtr txt, LookupResultFlags flags,
                                                    IntPtr userdata);

    public class ServiceResolver : ResolverBase, IDisposable
    {
        private IntPtr handle;
        private ServiceInfo currentInfo;
        private Client client;
        private int iface;
        private Protocol proto;
        private string name;
        private string type;
        private string domain;
        private Protocol aproto;
        private LookupFlags flags;
        private ServiceResolverCallback cb;

        private ArrayList foundListeners = new ArrayList ();
        private ArrayList timeoutListeners = new ArrayList ();

        [DllImport ("avahi-client")]
        private static extern IntPtr avahi_service_resolver_new (IntPtr client, int iface, Protocol proto,
                                                                 byte[] name, byte[] type, byte[] domain,
                                                                 Protocol aproto, LookupFlags flags,
                                                                 ServiceResolverCallback cb,
                                                                 IntPtr userdata);

        [DllImport ("avahi-common")]
        private static extern IntPtr avahi_string_list_get_next (IntPtr list);

        [DllImport ("avahi-common")]
        private static extern IntPtr avahi_string_list_get_text (IntPtr list);

        [DllImport ("avahi-common")]
        private static extern int avahi_string_list_get_size (IntPtr list);

        [DllImport ("avahi-client")]
        private static extern void avahi_service_resolver_free (IntPtr handle);

        public event ServiceInfoHandler Found
        {
            add {
                foundListeners.Add (value);
                Start ();
            }
            remove {
                foundListeners.Remove (value);
                Stop (false);
            }
        }

        public event EventHandler Timeout
        {
            add {
                timeoutListeners.Add (value);
                Start ();
            }
            remove {
                timeoutListeners.Remove (value);
                Stop (false);
            }
        }

        public ServiceInfo Service
        {
            get { return currentInfo; }
        }

        public ServiceResolver (Client client, string name, string type, string domain) : this (client, -1,
                                                                                                Protocol.Unspecified,
                                                                                                name, type, domain,
                                                                                                Protocol.Unspecified,
                                                                                                LookupFlags.None)
        {
        }

        public ServiceResolver (Client client, ServiceInfo service) : this (client, service.NetworkInterface,
                                                                            service.Protocol, service.Name,
                                                                            service.ServiceType, service.Domain,
                                                                            Protocol.Unspecified,
                                                                            GetLookupFlags (service.Flags))
        {
        }

        public ServiceResolver (Client client, int iface, Protocol proto, string name,
                                string type, string domain, Protocol aproto, LookupFlags flags)
        {
            this.client = client;
            this.iface = iface;
            this.proto = proto;
            this.name = name;
            this.type = type;
            this.domain = domain;
            this.aproto = aproto;
            this.flags = flags;
            cb = OnServiceResolverCallback;
        }

        ~ServiceResolver ()
        {
            Dispose ();
        }

        public void Dispose ()
        {
            Stop (true);
        }

        private void Start ()
        {
            if (client.Handle == IntPtr.Zero || handle != IntPtr.Zero ||
                (foundListeners.Count == 0 && timeoutListeners.Count == 0))
                return;

            lock (client) {
                handle = avahi_service_resolver_new (client.Handle, iface, proto,
                                                     Utility.StringToBytes (name), Utility.StringToBytes (type),
                                                     Utility.StringToBytes (domain), aproto, flags, cb, IntPtr.Zero);

                if (handle == IntPtr.Zero)
                    client.ThrowError ();
            }
        }

        private void Stop (bool force)
        {
            if (client.Handle != IntPtr.Zero && handle != IntPtr.Zero &&
                (force || (foundListeners.Count == 0 && timeoutListeners.Count == 0))) {

                lock (client) {
                    avahi_service_resolver_free (handle);
                    handle = IntPtr.Zero;
                }
            }
        }

        private void OnServiceResolverCallback (IntPtr resolver, int iface, Protocol proto,
                                                ResolverEvent revent, IntPtr name, IntPtr type,
                                                IntPtr domain, IntPtr host, IntPtr address,
                                                UInt16 port, IntPtr txt, LookupResultFlags flags,
                                                IntPtr userdata)
        {
            ServiceInfo info;
            info.NetworkInterface = iface;
            info.Protocol = proto;
            info.Domain = Utility.PtrToString (domain);
            info.ServiceType = Utility.PtrToString (type);
            info.Name = Utility.PtrToString (name);
            info.HostName = Utility.PtrToString (host);
            info.Address = Utility.PtrToAddress (address);
            info.Port = port;

            if (proto == Protocol.IPv6)
              info.Address.ScopeId = iface;

            ArrayList txtlist = new ArrayList ();
            for (IntPtr l = txt; l != IntPtr.Zero; l = avahi_string_list_get_next (l)) {
                IntPtr buf = avahi_string_list_get_text (l);
                int len = avahi_string_list_get_size (l);

                byte[] txtbuf = new byte[len];
                Marshal.Copy (buf, txtbuf, 0, len);
                txtlist.Add (txtbuf);
            }

            info.Text = (byte[][]) txtlist.ToArray (typeof (byte[]));
            info.Flags = flags;

            switch (revent) {
            case ResolverEvent.Found:
                currentInfo = info;

                foreach (ServiceInfoHandler handler in foundListeners)
                    handler (this, new ServiceInfoArgs (info));
                break;
            case ResolverEvent.Failure:
                EmitFailure (client.LastError);
                break;
            }
        }

        private static LookupFlags GetLookupFlags (LookupResultFlags rflags) {
            LookupFlags ret = LookupFlags.None;

            if ((rflags & LookupResultFlags.Multicast) > 0)
                ret |= LookupFlags.UseMulticast;
            if ((rflags & LookupResultFlags.WideArea) > 0)
                ret |= LookupFlags.UseWideArea;

            return ret;
        }
    }
}
