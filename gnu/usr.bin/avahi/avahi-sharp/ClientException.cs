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
using System.Runtime.InteropServices;

namespace Avahi
{
    public enum ErrorCode {
        Ok = 0,
        Failure = -1,
        BadState = -2,
        InvalidHostName = - 3,
        InvalidDomainName = -4,
        NoNetwork = -5,
        InvalidTTL = -6,
        IsPattern = -7,
        Collision = -8,
        InvalidRecord = -9,
        InvalidServiceName = -10,
        InvalidServiceType = -11,
        InvalidPort = -12,
        InvalidKey = -13,
        InvalidAddress = -14,
        Timeout = -15,
        TooManyClients = -16,
        TooManyObjects = -17,
        TooManyEntries = -18,
        OS = -19,
        AccessDenied = -20,
        InvalidOperation = -21,
        DBusError = -22,
        Disconnected = -23,
        NoMemory = -24,
        InvalidObject = -25,
        NoDaemon = -26,
        InvalidInterface = -27,
        InvalidProtocol = -28,
        InvalidFlags = -29,
        NotFound = -30,
        InvalidConfig = -31,
        VersionMismatch = -32,
        InvalidServiceSubtype = -33,
        InvalidPacket = -34,
        InvalidDnsError = -35,
        DnsFormErr = -36,
        DnsServFail = -37,
        DnsNxDomain = -38,
        DnsNoTimp = -39,
        DnsRefused = -40,
        DnsYxDomain = -41,
        DnsYxRrSet = -42,
        DnsNxRrSet = -43,
        DnsNotAuth = -44,
        DnsNotZone = -45,
        InvalidRData = -46,
        InvalidDnsClass = -47,
        InvalidDnsType = -48,
        NotSupported = -49,
        NotPermitted = -50
    }

    public delegate void ErrorCodeHandler (object o, ErrorCodeArgs args);

    public class ErrorCodeArgs : EventArgs
    {
        private ErrorCode code;

        public ErrorCode ErrorCode
        {
            get { return code; }
        }

        public ErrorCodeArgs (ErrorCode code)
        {
            this.code = code;
        }
    }

    public class ClientException : ApplicationException
    {
        private ErrorCode code;

        [DllImport ("avahi-common")]
        private static extern IntPtr avahi_strerror (ErrorCode code);

        public ErrorCode ErrorCode
        {
            get { return code; }
        }

        internal ClientException (int code) : this ((ErrorCode) code) {
        }

        internal ClientException (ErrorCode code) : base (GetErrorString (code))
        {
            this.code = code;
        }

        private static string GetErrorString (ErrorCode code)
        {
            IntPtr str = avahi_strerror (code);
            return Utility.PtrToString (str);
        }
    }
}
