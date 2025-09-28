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
using Mono.Unix;

namespace Avahi
{

    internal delegate void AddressResolverCallback (IntPtr resolver, int iface, Protocol proto,
                                                    ResolverEvent revent, IntPtr address,
                                                    IntPtr hostname, LookupResultFlags flags, IntPtr userdata);

    public delegate void HostAddressHandler (object o, HostAddressArgs args);

    public class HostAddressArgs : EventArgs
    {
        private string host;
        private IPAddress address;

        public string Host
        {
            get { return host; }
        }

        public IPAddress Address
        {
            get { return address; }
        }

        public HostAddressArgs (string host, IPAddress address)
        {
            this.host = host;
            this.address = address;
        }
    }

    public class AddressResolver : ResolverBase, IDisposable
    {
        private IntPtr handle;
        private Client client;
        private int iface;
        private Protocol proto;
        private IPAddress address;
        private LookupFlags flags;
        private AddressResolverCallback cb;

        private IPAddress currentAddress;
        private string currentHost;

        private ArrayList foundListeners = new ArrayList ();
        private ArrayList timeoutListeners = new ArrayList ();

        [DllImport ("avahi-client")]
        private static extern IntPtr avahi_address_resolver_new (IntPtr client, int iface, Protocol proto,
                                                                 IntPtr address, LookupFlags flags,
                                                                 AddressResolverCallback cb,
                                                                 IntPtr userdata);

        [DllImport ("avahi-client")]
        private static extern void avahi_address_resolver_free (IntPtr handle);

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

        public AddressResolver (Client client, IPAddress address) : this (client, -1, Protocol.Unspecified,
                                                                          address, LookupFlags.None)
        {
        }

        public AddressResolver (Client client, int iface, Protocol proto, IPAddress address, LookupFlags flags)
        {
            this.client = client;
            this.iface = iface;
            this.proto = proto;
            this.address = address;
            this.flags = flags;
            cb = OnAddressResolverCallback;
        }

        ~AddressResolver ()
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

            IntPtr addrPtr = Utility.AddressToPtr (address);

            lock (client) {
                handle = avahi_address_resolver_new (client.Handle, iface, proto, addrPtr, flags,
                                                     cb, IntPtr.Zero);

                if (handle == IntPtr.Zero)
                    client.ThrowError ();
            }

            Utility.Free (addrPtr);
        }

        private void Stop (bool force)
        {
            if (client.Handle != IntPtr.Zero && handle != IntPtr.Zero &&
                (force || (foundListeners.Count == 0 && timeoutListeners.Count == 0))) {

                lock (client) {
                    avahi_address_resolver_free (handle);
                    handle = IntPtr.Zero;
                }
            }
        }

        private void OnAddressResolverCallback (IntPtr resolver, int iface, Protocol proto,
                                                ResolverEvent revent, IntPtr address,
                                                IntPtr hostname, LookupResultFlags flags, IntPtr userdata)
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
