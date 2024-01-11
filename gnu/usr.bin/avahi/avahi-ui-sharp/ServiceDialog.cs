using System;
using System.Net;
using System.Collections;
using System.Runtime.InteropServices;
using Gtk;
using Mono.Unix;
using Mono.Unix.Native;

namespace Avahi.UI
{
    public class ServiceDialog : Dialog
    {
        [DllImport ("avahi-ui")]
        private static extern IntPtr aui_service_dialog_new (string title, IntPtr parent, IntPtr dummy);

        [DllImport ("avahi-ui")]
        private static extern void aui_service_dialog_set_browse_service_typesv (IntPtr dialog, IntPtr[] types);

        [DllImport ("avahi-ui")]
        private static extern IntPtr aui_service_dialog_get_browse_service_types (IntPtr dialog);

        [DllImport ("avahi-ui")]
        private static extern IntPtr aui_service_dialog_get_domain (IntPtr dialog);

        [DllImport ("avahi-ui")]
        private static extern void aui_service_dialog_set_domain (IntPtr dialog, IntPtr domain);

        [DllImport ("avahi-ui")]
        private static extern IntPtr aui_service_dialog_get_service_type (IntPtr dialog);

        [DllImport ("avahi-ui")]
        private static extern void aui_service_dialog_set_service_type (IntPtr dialog, IntPtr type);

        [DllImport ("avahi-ui")]
        private static extern IntPtr aui_service_dialog_get_service_name (IntPtr dialog);

        [DllImport ("avahi-ui")]
        private static extern void aui_service_dialog_set_service_name (IntPtr dialog, IntPtr type);

        [DllImport ("avahi-ui")]
        private static extern IntPtr aui_service_dialog_get_address (IntPtr dialog);

        [DllImport ("avahi-ui")]
        private static extern UInt16 aui_service_dialog_get_port (IntPtr dialog);

        [DllImport ("avahi-ui")]
        private static extern IntPtr aui_service_dialog_get_host_name (IntPtr dialog);

        [DllImport ("avahi-ui")]
        private static extern IntPtr aui_service_dialog_get_txt_data (IntPtr dialog);

        [DllImport ("avahi-ui")]
        private static extern bool aui_service_dialog_get_resolve_service (IntPtr dialog);

        [DllImport ("avahi-ui")]
        private static extern void aui_service_dialog_set_resolve_service (IntPtr dialog, bool val);

        [DllImport ("avahi-ui")]
        private static extern bool aui_service_dialog_get_resolve_host_name (IntPtr dialog);

        [DllImport ("avahi-ui")]
        private static extern void aui_service_dialog_set_resolve_host_name (IntPtr dialog, bool val);

        [DllImport ("avahi-ui")]
        private static extern Protocol aui_service_dialog_get_address_family (IntPtr dialog);

        [DllImport ("avahi-ui")]
        private static extern void aui_service_dialog_set_address_family (IntPtr dialog, Protocol proto);

        [DllImport ("avahi-common")]
        private static extern IntPtr avahi_address_snprint (IntPtr buf, int size, IntPtr address);

        [DllImport ("avahi-common")]
        private static extern IntPtr avahi_string_list_get_next (IntPtr list);

        [DllImport ("avahi-common")]
        private static extern IntPtr avahi_string_list_get_text (IntPtr list);

        [DllImport ("avahi-common")]
        private static extern int avahi_string_list_get_size (IntPtr list);

        public string[] BrowseServiceTypes {
            get {
                IntPtr arr = aui_service_dialog_get_browse_service_types (Raw);

                ArrayList values = new ArrayList ();

                for (int i = 0;;i++) {
                    IntPtr ptr = Marshal.ReadIntPtr (arr, i * Marshal.SizeOf (typeof (IntPtr)));

                    if (ptr == IntPtr.Zero)
                        break;

                    values.Add (GLib.Marshaller.Utf8PtrToString (ptr));
                }

                return (string[]) values.ToArray (typeof (string));
            } set {
                IntPtr[] types;
                if (value == null) {
                    types = new IntPtr[] { IntPtr.Zero };
                } else {
                    types = new IntPtr[value.Length + 1];

                    for (int i = 0; i < value.Length; i++) {
                        types[i] = GLib.Marshaller.StringToPtrGStrdup (value[i]);
                    }

                    types[value.Length] = IntPtr.Zero;
                }

                aui_service_dialog_set_browse_service_typesv (Raw, types);

                for (int i = 0;;i++) {
                    if (types[i] != IntPtr.Zero)
                        break;

                    GLib.Marshaller.Free (types[i]);
                }
            }
        }

        public string ServiceType {
            get {
                return GLib.Marshaller.Utf8PtrToString (aui_service_dialog_get_service_type (Raw));
            } set {
                IntPtr type = GLib.Marshaller.StringToPtrGStrdup (value);
                aui_service_dialog_set_service_type (Raw, type);
                GLib.Marshaller.Free (type);
            }
        }

        public string ServiceName {
            get {
                return GLib.Marshaller.Utf8PtrToString (aui_service_dialog_get_service_name (Raw));
            } set {
                IntPtr name = GLib.Marshaller.StringToPtrGStrdup (value);
                aui_service_dialog_set_service_name (Raw, name);
                GLib.Marshaller.Free (name);
            }
        }

        public IPAddress Address {
            get {
                return PtrToAddress (aui_service_dialog_get_address (Raw));
            }
        }

        public UInt16 Port {
            get {
                return aui_service_dialog_get_port (Raw);
            }
        }

        public string HostName {
            get {
                return GLib.Marshaller.Utf8PtrToString (aui_service_dialog_get_host_name (Raw));
            }
        }

        public string Domain {
            get {
                return GLib.Marshaller.Utf8PtrToString (aui_service_dialog_get_domain (Raw));
            } set {
                IntPtr domain = GLib.Marshaller.StringToPtrGStrdup (value);
                aui_service_dialog_set_domain (Raw, domain);
                GLib.Marshaller.Free (domain);
            }
        }

        public byte[][] TxtData {
            get {
                ArrayList txtlist = new ArrayList ();
                IntPtr txt = aui_service_dialog_get_txt_data (Raw);

                for (IntPtr l = txt; l != IntPtr.Zero; l = avahi_string_list_get_next (l)) {
                    IntPtr buf = avahi_string_list_get_text (l);
                    int len = avahi_string_list_get_size (l);

                    byte[] txtbuf = new byte[len];
                    Marshal.Copy (buf, txtbuf, 0, len);
                    txtlist.Add (txtbuf);
                }

                return (byte[][]) txtlist.ToArray (typeof (byte[]));
            }
        }

        public bool ResolveServiceEnabled {
            get {
                return aui_service_dialog_get_resolve_service (Raw);
            } set {
                aui_service_dialog_set_resolve_service (Raw, value);
            }
        }

        public bool ResolveHostNameEnabled {
            get {
                return aui_service_dialog_get_resolve_host_name (Raw);
            } set {
                aui_service_dialog_set_resolve_host_name (Raw, value);
            }
        }

        public Protocol AddressFamily {
            get {
                return aui_service_dialog_get_address_family (Raw);
            } set {
                aui_service_dialog_set_address_family (Raw, value);
            }
        }

        public ServiceDialog (string title, Window parent, params object[] buttonData)
        {
            Raw = aui_service_dialog_new (title, parent == null ? IntPtr.Zero : parent.Handle,
                                          IntPtr.Zero);

            for (int i = 0; i < buttonData.Length - 1; i += 2) {
                AddButton ((string) buttonData[i], (int) buttonData[i + 1]);
            }
        }

        private static IPAddress PtrToAddress (IntPtr ptr)
        {
            IPAddress address = null;

            if (ptr != IntPtr.Zero) {
                IntPtr buf = Stdlib.malloc (256);
                IntPtr addrPtr = avahi_address_snprint (buf, 256, ptr);
                address = IPAddress.Parse (GLib.Marshaller.Utf8PtrToString (addrPtr));

                Stdlib.free (addrPtr);
            }

            return address;
        }
    }
}
