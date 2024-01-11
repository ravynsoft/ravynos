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

    internal delegate void HostNameResolverCallback (IntPtr resolver, int iface, Protocol proto,
                                                     ResolverEvent revent, IntPtr hostname, IntPtr address,
                                                     LookupResultFlags flags, IntPtr userdata);

    public class HostNameResolver : ResolverBase, IDisposable
    {
        private IntPtr handle;
        private Client client;
        private int iface;
        private Protocol proto;
        private string hostname;
        private Protocol aproto;
        private LookupFlags flags;
        private HostNameResolverCallback cb;

        private IPAddress currentAddress;
        private string currentHost;

        private ArrayList foundListeners = new ArrayList ();
        private ArrayList timeoutListeners = new ArrayList ();

        [DllImport ("avahi-client")]
        private static extern IntPtr avahi_host_name_resolver_new (IntPtr client, int iface, Protocol proto,
                                                                   byte[] hostname, Protocol aproto, LookupFlags flags,
                                                                   HostNameResolverCallback cb, IntPtr userdata);

        [DllImport ("avahi-client")]
        private static extern void avahi_host_name_resolver_free (IntPtr handle);

        public event HostAddressHandler Found
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

        public IPAddress Address
        {
            get { return currentAddress; }
        }

        public string HostName
        {
            get { return currentHost; }
        }

        public HostNameResolver (Client client, string hostname) : this (client, -1, Protocol.Unspecified,
                                                                         hostname, Protocol.Unspecified,
                                                                         LookupFlags.None)
        {
        }

        public HostNameResolver (Client client, int iface, Protocol proto, string hostname,
                                 Protocol aproto, LookupFlags flags)
        {
            this.client = client;
            this.iface = iface;
            this.proto = proto;
            this.hostname = hostname;
            this.aproto = aproto;
            this.flags = flags;
            cb = OnHostNameResolverCallback;
        }

        ~HostNameResolver ()
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
                handle = avahi_host_name_resolver_new (client.Handle, iface, proto,
                                                       Utility.StringToBytes (hostname), aproto, flags,
                                                       cb, IntPtr.Zero);

                if (handle == IntPtr.Zero)
                    client.ThrowError ();
            }
        }

        private void Stop (bool force)
        {
            if (client.Handle != IntPtr.Zero && handle != IntPtr.Zero &&
                (force || (foundListeners.Count == 0 && timeoutListeners.Count == 0))) {

                lock (client) {
                    avahi_host_name_resolver_free (handle);
                    handle = IntPtr.Zero;
                }
            }
        }

        private void OnHostNameResolverCallback (IntPtr resolver, int iface, Protocol proto,
                                                 ResolverEvent revent, IntPtr hostname, IntPtr address,
                                                 LookupResultFlags flags, IntPtr userdata)
        {
            switch (revent) {
            case ResolverEvent.Found:
                currentAddress = Utility.PtrToAddress (address);
                currentHost = Utility.PtrToString (hostname);

                foreach (HostAddressHandler handler in foundListeners)
                    handler (this, new HostAddressArgs (currentHost, currentAddress));
                break;
            case ResolverEvent.Failure:
                EmitFailure (client.LastError);
                break;
            }
        }
    }
}
