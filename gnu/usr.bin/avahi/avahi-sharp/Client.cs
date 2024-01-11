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
using System.Threading;
using System.Collections;
using System.Runtime.InteropServices;
using Mono.Unix;
using Mono.Unix.Native;

using Stdlib = Mono.Unix.Native.Stdlib;

namespace Avahi
{
    internal enum ResolverEvent {
        Found,
        Failure
    }

    internal enum BrowserEvent {
        Added,
        Removed,
        CacheExhausted,
        AllForNow,
        Failure
    }

    internal delegate int PollCallback (IntPtr ufds, uint nfds, int timeout);
    internal delegate void ClientCallback (IntPtr client, ClientState state, IntPtr userData);

    public delegate void ClientStateHandler (object o, ClientStateArgs state);

    public class ClientStateArgs : EventArgs
    {
        private ClientState state;
        private ErrorCode error;

        public ClientState State
        {
            get { return state; }
        }

        public ErrorCode Error
        {
            get { return error; }
        }

        public ClientStateArgs (ClientState state, ErrorCode error)
        {
            this.state = state;
            this.error = error;
        }
    }

    public enum Protocol {
        Unspecified = -1,
        IPv4 = 0,
        IPv6 = 1
    }

    internal enum ServerState {
        Invalid,
        Registering,
        Running,
        Collision
    }

    public enum ClientState {
        Registering = ServerState.Registering,
        Running = ServerState.Running,
        Collision = ServerState.Collision,
        Failure = 100,
        Connecting = 101
    }

    [Flags]
    public enum LookupFlags {
        None = 0,
        UseWideArea = 1,
        UseMulticast = 2,
	NoTxt = 4,
        NoAddress = 8
    }

    [Flags]
    public enum LookupResultFlags {
        None = 0,
        Cached = 1,
        WideArea = 2,
        Multicast = 4,
        Local = 8,
        OurOwn = 16,
    }

    [Flags]
    public enum ClientFlags {
        None = 0,
        IgnoreUserConfig = 1,
        NoFail = 2
    }

    public class Client : IDisposable
    {
        private IntPtr handle;
        private ClientCallback cb;
        private PollCallback pollcb;
        private IntPtr spoll;

        private Thread thread;

        [DllImport ("avahi-client")]
        private static extern IntPtr avahi_client_new (IntPtr poll, ClientFlags flags, ClientCallback handler,
                                                       IntPtr userData, out int error);

        [DllImport ("avahi-client")]
        private static extern void avahi_client_free (IntPtr handle);

        [DllImport ("avahi-client")]
        private static extern IntPtr avahi_client_get_version_string (IntPtr handle);

        [DllImport ("avahi-client")]
        private static extern IntPtr avahi_client_get_host_name (IntPtr handle);

        [DllImport ("avahi-client")]
        private static extern IntPtr avahi_client_get_domain_name (IntPtr handle);

        [DllImport ("avahi-client")]
        private static extern IntPtr avahi_client_get_host_name_fqdn (IntPtr handle);

        [DllImport ("avahi-client")]
        private static extern ClientState avahi_client_get_state (IntPtr handle);

        [DllImport ("avahi-client")]
        private static extern int avahi_client_errno (IntPtr handle);

        [DllImport ("avahi-common")]
        private static extern IntPtr avahi_simple_poll_new ();

        [DllImport ("avahi-common")]
        private static extern IntPtr avahi_simple_poll_get (IntPtr spoll);

        [DllImport ("avahi-common")]
        private static extern void avahi_simple_poll_free (IntPtr spoll);

        [DllImport ("avahi-common")]
        private static extern int avahi_simple_poll_loop (IntPtr spoll);

        [DllImport ("avahi-common")]
        private static extern void avahi_simple_poll_set_func (IntPtr spoll, PollCallback cb);

        [DllImport ("avahi-common")]
        private static extern void avahi_simple_poll_quit (IntPtr spoll);

        [DllImport ("avahi-client")]
        private static extern uint avahi_client_get_local_service_cookie (IntPtr client);

        [DllImport ("avahi-common")]
        private static extern int avahi_service_name_join (IntPtr buf, int len, byte[] name, byte[] type,
                                                           byte[] domain);

        [DllImport ("avahi-common")]
        private static extern int avahi_service_name_split (byte[] service, IntPtr name, int name_len,
                                                            IntPtr type, int type_len,
                                                            IntPtr domain, int domain_len);


        [DllImport ("libc")]
        private static extern int poll(IntPtr ufds, uint nfds, int timeout);

        public event ClientStateHandler StateChanged;

        internal IntPtr Handle
        {
            get { return handle; }
        }

        public string Version
        {
            get {
                lock (this) {
                    return Utility.PtrToString (avahi_client_get_version_string (handle));
                }
            }
        }

        public string HostName
        {
            get {
                lock (this) {
                    return Utility.PtrToString (avahi_client_get_host_name (handle));
                }
            }
        }

        public string DomainName
        {
            get {
                lock (this) {
                    return Utility.PtrToString (avahi_client_get_domain_name (handle));
                }
            }
        }

        public string HostNameFqdn
        {
            get {
                lock (this) {
                    return Utility.PtrToString (avahi_client_get_host_name_fqdn (handle));
                }
            }
        }

        public ClientState State
        {
            get {
                lock (this) {
                    return (ClientState) avahi_client_get_state (handle);
                }
            }
        }

        public uint LocalServiceCookie
        {
            get {
                lock (this) {
                    return avahi_client_get_local_service_cookie (handle);
                }
            }
        }

        internal ErrorCode LastError
        {
            get {
                lock (this) {
                    return (ErrorCode) avahi_client_errno (handle);
                }
            }
        }

        public Client (ClientFlags flags)
        {
            spoll = avahi_simple_poll_new ();

            pollcb = OnPollCallback;
            avahi_simple_poll_set_func (spoll, pollcb);
            IntPtr poll = avahi_simple_poll_get (spoll);
            cb = OnClientCallback;

            int error;
            handle = avahi_client_new (poll, flags, cb, IntPtr.Zero, out error);
            if (error != 0)
                throw new ClientException (error);

            thread = new Thread (PollLoop);
            thread.IsBackground = true;
            thread.Start ();
        }

        public Client () : this (ClientFlags.None) {
        }

        ~Client ()
        {
            Dispose ();
        }

        public void Dispose ()
        {
            if (handle != IntPtr.Zero) {
                lock (this) {
                    avahi_client_free (handle);
                    handle = IntPtr.Zero;

                    avahi_simple_poll_quit (spoll);
                    Monitor.Wait (this);

                    avahi_simple_poll_free (spoll);
                }
            }
        }

        public static string JoinServiceName (string name, string type, string domain)
        {
            int len = 4 * (name.Length + type.Length + domain.Length) + 4;
            IntPtr buf = Stdlib.malloc ((ulong) len);

            int ret = avahi_service_name_join (buf, len,
                                               Utility.StringToBytes (name),
                                               Utility.StringToBytes (type),
                                               Utility.StringToBytes (domain));

            if (ret < 0) {
                Utility.Free (buf);
                return null; // FIXME, should throw exception
            }

            string service = Utility.PtrToString (buf);
            Utility.Free (buf);

            return service;
        }

        public static void SplitServiceName (string service, out string name, out string type, out string domain)
        {
            int len = 1024;

            IntPtr namePtr = Stdlib.malloc ((ulong) len);
            IntPtr typePtr = Stdlib.malloc ((ulong) len);
            IntPtr domainPtr = Stdlib.malloc ((ulong) len);

            int ret = avahi_service_name_split (Utility.StringToBytes (service), namePtr, len, typePtr, len,
                                                domainPtr, len);

            if (ret < 0) {
                Utility.Free (namePtr);
                Utility.Free (typePtr);
                Utility.Free (domainPtr);

                name = null;
                type = null;
                domain = null;
                return;
            }

            name = Utility.PtrToString (namePtr);
            type = Utility.PtrToString (typePtr);
            domain = Utility.PtrToString (domainPtr);

            Utility.Free (namePtr);
            Utility.Free (typePtr);
            Utility.Free (domainPtr);
        }

        internal void ThrowError ()
        {
            ErrorCode error = LastError;

            if (error != ErrorCode.Ok)
                throw new ClientException (error);
        }

        private void OnClientCallback (IntPtr client, ClientState state, IntPtr userData)
        {
            if (StateChanged != null)
                StateChanged (this, new ClientStateArgs (state, LastError));
        }

        private int OnPollCallback (IntPtr ufds, uint nfds, int timeout) {
            Monitor.Exit (this);
            int result = poll (ufds, nfds, timeout);
            Monitor.Enter (this);
            return result;
        }

        private void PollLoop () {
            try {
                lock (this) {
                    avahi_simple_poll_loop (spoll);
                    Monitor.Pulse (this);
                }
            } catch (Exception e) {
                Console.Error.WriteLine ("Error in avahi-sharp event loop: " + e);
            }
        }
    }
}
