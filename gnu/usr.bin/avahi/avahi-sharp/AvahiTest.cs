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
using System.Text;
using System.Net;
using Avahi;

public class AvahiTest {
    private static Client client;
    private static ArrayList objects = new ArrayList ();

    public static void Main () {
        client = new Client ();

	Console.WriteLine ("joined service name: " + Client.JoinServiceName ("FooBar", "_foo", "local"));

        EntryGroup eg = new EntryGroup (client);
        eg.StateChanged += OnEntryGroupChanged;
        eg.AddService ("foobar2", "_dingdong._tcp", client.DomainName,
                       444, new string[] { "foo=stuff", "bar=stuff2", "baz=stuff3" });
        eg.Commit ();
        BrowseServiceTypes ("local");
        Console.WriteLine ("Press enter to quit");
        Console.ReadLine ();
        client.Dispose ();
    }

    private static void OnEntryGroupChanged (object o, EntryGroupStateArgs args)
    {
        Console.WriteLine ("Entry group status: " + args.State);
        if (args.State == EntryGroupState.Established) {
            DomainBrowser browser = new DomainBrowser (client);
            objects.Add (browser);

            browser.DomainAdded += OnDomainAdded;
        }
    }

    private static void OnDomainAdded (object o, DomainInfoArgs args)
    {
        Console.WriteLine ("Got domain added: " + args.Domain.Domain);
        // BrowseServiceTypes (args.Domain.Domain);
    }

    private static void BrowseServiceTypes (string domain)
    {
        ServiceTypeBrowser stb = new ServiceTypeBrowser (client, domain);
        objects.Add (stb);

        stb.CacheExhausted += OnCacheExhausted;
        stb.ServiceTypeAdded += OnServiceTypeAdded;
    }

    private static void OnCacheExhausted (object o, EventArgs args)
    {
        Console.WriteLine ("Cache is exhausted");
    }

    private static void OnServiceTypeAdded (object o, ServiceTypeInfoArgs args)
    {
        Console.WriteLine ("Got service type: " + args.ServiceType.ServiceType);
        ServiceBrowser sb = new ServiceBrowser (client, args.ServiceType.ServiceType, args.ServiceType.Domain);
        objects.Add (sb);

        sb.ServiceAdded += OnServiceAdded;
    }

    private static void OnServiceAdded (object o, ServiceInfoArgs args)
    {
        // Console.WriteLine ("Got service: " + info.Name);
        ServiceResolver resolver = new ServiceResolver (client, args.Service);
        objects.Add (resolver);
        resolver.Found += OnServiceResolved;
    }

    private static void OnServiceResolved (object o, ServiceInfoArgs args)
    {
        objects.Remove (o);

        Console.WriteLine ("Service '{0}' at {1}:{2}", args.Service.Name, args.Service.HostName, args.Service.Port);
        foreach (byte[] bytes in args.Service.Text) {
            Console.WriteLine ("Text: " + Encoding.UTF8.GetString (bytes));
        }

        AddressResolver ar = new AddressResolver (client, args.Service.Address);
        objects.Add (ar);

        ar.Found += OnAddressResolved;
        ar.Failed += OnAddressResolverFailed;
    }

    private static void OnAddressResolverFailed (object o, ErrorCodeArgs args)
    {
        Console.WriteLine ("Failed to resolve '{0}': {1}", (o as AddressResolver).Address, args.ErrorCode);
    }

    private static void OnAddressResolved (object o, HostAddressArgs args)
    {
        objects.Remove (o);

        Console.WriteLine ("Resolved {0} to {1}", args.Address, args.Host);
        HostNameResolver hr = new HostNameResolver (client, args.Host);
        objects.Add (hr);

        hr.Found += OnHostNameResolved;
    }

    private static void OnHostNameResolved (object o, HostAddressArgs args)
    {
        objects.Remove (o);
        Console.WriteLine ("Resolved {0} to {1}", args.Host, args.Address);
    }
}
