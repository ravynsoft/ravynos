using System;
using System.Diagnostics;
using Gtk;
using Avahi.UI;

public class EntryPoint {
    public static void Main () {
        Application.Init ();

        ServiceDialog dialog = new ServiceDialog ("Choose SSH Server", null,
                                                  Stock.Cancel, ResponseType.Cancel,
                                                  Stock.Connect, ResponseType.Accept);
	dialog.BrowseServiceTypes = new string[] { "_ssh._tcp" };
        dialog.ResolveServiceEnabled = true;

        if (dialog.Run () == (int) ResponseType.Accept) {
            Console.WriteLine ("Connecting to {0}:{1}", dialog.Address, dialog.Port);

            string user = Environment.UserName;

            foreach (byte[] txtBytes in dialog.TxtData) {
                string txt = System.Text.Encoding.UTF8.GetString (txtBytes);
                string[] splitTxt = txt.Split(new char[] { '=' }, 2);

                if (splitTxt.Length != 2)
                    continue;

                if (splitTxt[0] == "u") {
                    user = splitTxt[1];
                }

                string args = String.Format ("gnome-terminal -t {0} -x ssh -p {1} -l {2} {3}",
                                             dialog.HostName, dialog.Port, user, dialog.Address.ToString ());
                Console.WriteLine ("Launching: " + args);
                Process.Start (args);
            }
        }
    }
}
