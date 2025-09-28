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
using System.Runtime.InteropServices;
using System.Text;

namespace Avahi
{
    internal delegate void ServiceTypeBrowserCallback (IntPtr browser, int iface, Protocol proto, BrowserEvent bevent,
                                                       IntPtr type, IntPtr domain, LookupResultFlags flags,
                                                       IntPtr userdata);

    public struct ServiceTypeInfo
    {
        public int NetworkInterface;
        public Protocol Protocol;
        public string Domain;
        public string ServiceType;
        public LookupResultFlags Flags;
    }

    public class ServiceTypeInfoArgs : EventArgs
    {
        private ServiceTypeInfo type;

        public ServiceTypeInfo ServiceType
        {
            get { return type; }
        }

        public ServiceTypeInfoArgs (ServiceTypeInfo type)
        {
            this.type = type;
        }
    }

    public delegate void ServiceTypeInfoHandler (object o, ServiceTypeInfoArgs args);

    public class ServiceTypeBrowser : BrowserBase, IDisposable
    {
        private IntPtr handle;
        private ArrayList infos = new ArrayList ();
        private Client client;
        private int iface;
        private Protocol proto;
        private string domain;
        private LookupFlags flags;
        private ServiceTypeBrowserCallback cb;

        private ArrayList addListeners = new ArrayList ();
        private ArrayList removeListeners = new ArrayList ();

        [DllImport ("avahi-client")]
        private static extern IntPtr avahi_service_type_browser_new (IntPtr client, int iface, int proto,
                                                                     byte[] domain, LookupFlags flags,
                                                                     ServiceTypeBrowserCallback cb,
                                                                     IntPtr userdata);

        [DllImport ("avahi-client")]
        private static extern void avahi_service_type_browser_free (IntPtr handle);

        public event ServiceTypeInfoHandler ServiceTypeAdded
        {
            add {
                addListeners.Add (value);
                Start ();
            }
            remove {
                addListeners.Remove (value);
                Stop (false);
            }
        }

        public event ServiceTypeInfoHandler ServiceTypeRemoved
        {
            add {
                removeListeners.Add (value);
                Start ();
            }
            remove {
                removeListeners.Remove (value);
                Stop (false);
            }
        }

        public ServiceTypeInfo[] ServiceTypes
        {
            get { return (ServiceTypeInfo[]) infos.ToArray (typeof (ServiceTypeInfo)); }
        }

        public ServiceTypeBrowser (Client client) : this (client, client.DomainName)
        {
        }

        public ServiceTypeBrowser (Client client, string domain) : this (client, -1, Protocol.Unspecified,
                                                                         domain, LookupFlags.None)
        {
        }

        public ServiceTypeBrowser (Client client, int iface, Protocol proto, string domain, LookupFlags flags)
        {
            this.client = client;
            this.iface = iface;
            this.proto = proto;
            this.domain = domain;
            this.flags = flags;
            cb = OnServiceTypeBrowserCallback;
        }

        ~ServiceTypeBrowser ()
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
                (addListeners.Count == 0 && removeListeners.Count == 0))
                return;

            lock (client) {
                handle = avahi_service_type_browser_new (client.Handle, iface, (int) proto,
                                                         Utility.StringToBytes (domain), flags,
                                                         cb, IntPtr.Zero);

                if (handle == IntPtr.Zero)
                    client.ThrowError ();
            }
        }

        private void Stop (bool force)
        {
            if (client.Handle != IntPtr.Zero && handle != IntPtr.Zero &&
                (force || (addListeners.Count == 0 && removeListeners.Count == 0))) {

                lock (client) {
                    avahi_service_type_browser_free (handle);
                    handle = IntPtr.Zero;
                }
            }
        }

        private void OnServiceTypeBrowserCallback (IntPtr browser, int iface, Protocol proto, BrowserEvent bevent,
                                                   IntPtr type, IntPtr domain, LookupResultFlags flags,
                                                   IntPtr userdata)
        {

            ServiceTypeInfo info;
            info.NetworkInterface = iface;
            info.Protocol = proto;
            info.Domain = Utility.PtrToString (domain);
            info.ServiceType = Utility.PtrToString (type);
            info.Flags = flags;

            switch (bevent) {
            case BrowserEvent.Added:
                infos.Add (info);

                foreach (ServiceTypeInfoHandler handler in addListeners)
                    handler (this, new ServiceTypeInfoArgs (info));
                break;
            case BrowserEvent.Removed:
                infos.Remove (info);

                foreach (ServiceTypeInfoHandler handler in removeListeners)
                    handler (this, new ServiceTypeInfoArgs (info));
                break;
            default:
                EmitBrowserEvent (bevent);
                break;
            }
        }
    }
}
