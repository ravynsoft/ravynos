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
using System.Collections;
using System.Runtime.InteropServices;
using System.Text;

namespace Avahi
{

    public delegate void RecordInfoHandler (object o, RecordInfoArgs args);

    internal delegate void RecordBrowserCallback (IntPtr browser, int iface, Protocol proto, BrowserEvent bevent,
                                                  IntPtr name, ushort clazz, ushort type, IntPtr rdata, int size,
                                                  LookupResultFlags flags, IntPtr userdata);

    public enum RecordClass {
        In = 1
    }

    public enum RecordType {
        A = 1,
        Ns = 2,
        Cname = 5,
        Soa = 6,
        Ptr = 12,
        Hinfo = 13,
        Mx = 15,
        Txt = 16,
        Aaa = 28,
        Srv = 33
    }

    public struct RecordInfo
    {
        public int NetworkInterface;
        public Protocol Protocol;
        public string Name;
        public RecordClass Class;
        public RecordType Type;
        public byte[] Data;
        public LookupResultFlags Flags;
    }

    public class RecordInfoArgs : EventArgs
    {
        private RecordInfo record;

        public RecordInfo Record {
            get { return record; }
        }

        public RecordInfoArgs (RecordInfo record)
        {
            this.record = record;
        }
    }

    public class RecordBrowser : BrowserBase, IDisposable
    {
        private IntPtr handle;
        private ArrayList infos = new ArrayList ();
        private Client client;
        private int iface;
        private Protocol proto;
        private string name;
        private RecordClass clazz;
        private RecordType type;
        private LookupFlags flags;
        private RecordBrowserCallback cb;

        private ArrayList addListeners = new ArrayList ();
        private ArrayList removeListeners = new ArrayList ();

        [DllImport ("avahi-client")]
        private static extern IntPtr avahi_record_browser_new (IntPtr client, int iface, Protocol proto,
                                                               byte[] name, ushort clazz, ushort type,
                                                               LookupFlags flags, RecordBrowserCallback cb,
                                                               IntPtr userdata);


        [DllImport ("avahi-client")]
        private static extern void avahi_record_browser_free (IntPtr handle);

        public event RecordInfoHandler RecordAdded
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

        public event RecordInfoHandler RecordRemoved
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

        public RecordInfo[] Records
        {
            get { return (RecordInfo[]) infos.ToArray (typeof (RecordInfo)); }
        }

        public RecordBrowser (Client client, string name, RecordType type) :
            this (client, -1, Protocol.Unspecified, name, RecordClass.In, type, LookupFlags.None)
        {
        }

        public RecordBrowser (Client client, int iface, Protocol proto, string name, RecordClass clazz,
                              RecordType type, LookupFlags flags)
        {
            this.client = client;
            this.iface = iface;
            this.proto = proto;
            this.name = name;
            this.clazz = clazz;
            this.type = type;
            this.flags = flags;
            cb = OnRecordBrowserCallback;
        }

        ~RecordBrowser ()
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
                handle = avahi_record_browser_new (client.Handle, iface, proto, Utility.StringToBytes (name),
                                                   (ushort) clazz, (ushort) type, flags, cb, IntPtr.Zero);

                if (handle == IntPtr.Zero)
                    client.ThrowError ();
            }
        }

        private void Stop (bool force)
        {
            if (client.Handle != IntPtr.Zero && handle != IntPtr.Zero &&
                (force || (addListeners.Count == 0 && removeListeners.Count == 0))) {

                lock (client) {
                    avahi_record_browser_free (handle);
                    handle = IntPtr.Zero;
                }
            }
        }

        private void OnRecordBrowserCallback (IntPtr browser, int iface, Protocol proto, BrowserEvent bevent,
                                              IntPtr name, ushort clazz, ushort type, IntPtr rdata, int size,
                                              LookupResultFlags flags, IntPtr userdata)
        {
            RecordInfo info;
            info.NetworkInterface = iface;
            info.Protocol = proto;
            info.Name = Utility.PtrToString (name);
            info.Class = (RecordClass) clazz;
            info.Type = (RecordType) type;
            info.Flags = flags;
            info.Data = new byte[size];

            if (rdata != IntPtr.Zero) {
                Marshal.Copy (rdata, info.Data, 0, size);
            }

            switch (bevent) {
            case BrowserEvent.Added:
                infos.Add (info);

                foreach (RecordInfoHandler handler in addListeners)
                    handler (this, new RecordInfoArgs (info));

                break;
            case BrowserEvent.Removed:
                infos.Remove (info);

                foreach (RecordInfoHandler handler in removeListeners)
                    handler (this, new RecordInfoArgs (info));

                break;
            default:
                EmitBrowserEvent (bevent);
                break;
            }
        }
    }
}
