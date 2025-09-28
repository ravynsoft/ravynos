#!@PYTHON@
# -*-python-*-
# This file is part of avahi.
#
# avahi is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# avahi is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with avahi; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.

from __future__ import print_function

import os, sys

try:
    import avahi, gettext, dbus, avahi.ServiceTypeDatabase
    gettext.bindtextdomain(@GETTEXT_PACKAGE@, @LOCALEDIR@)
    gettext.textdomain(@GETTEXT_PACKAGE@)
    import gi
    gi.require_version('Gtk', '3.0')
    from gi.repository import Gtk, GObject
    _ = gettext.gettext
except ImportError as e:
    print("Sorry, to use this tool you need to install Avahi, pygtk and python-dbus.\n Error: %s" % e)
    sys.exit(1)
except Exception as e:
    print("Failed to initialize: %s" % e)
    sys.exit(1)


## !!NOTE!! ##
# It's really important to do this, else you won't see any events
##
try:
    from dbus import DBusException
    from dbus.mainloop.glib import DBusGMainLoop
    DBusGMainLoop(set_as_default=True)
except ImportError as e:
    pass

service_type_browsers = {}
service_browsers = {}

def error_msg(msg):
    d = Gtk.MessageDialog(parent=None, flags=Gtk.DialogFlags.MODAL,
                          type=Gtk.MessageType.ERROR, buttons=Gtk.ButtonsType.OK)
    d.set_markup(msg)
    d.show_all()
    d.run()
    d.destroy()

ui_dir = "@interfacesdir@"

service_type_db = avahi.ServiceTypeDatabase.ServiceTypeDatabase()

class Main_window:
    def __init__(self, path="avahi-discover.ui", root="main_window", domain=None, **kwargs):
        path = os.path.join(ui_dir, path)
        Gtk.Window.set_default_icon_name("network-wired")
        self.ui = Gtk.Builder()
        self.ui.set_translation_domain("avahi")
        self.ui.add_from_file(path)
        self.ui.connect_signals(self)
        self.tree_view = self.ui.get_object("tree_view")
        self.info_label = self.ui.get_object("info_label")
        self.new()

    def on_tree_view_cursor_changed(self, widget, *args):
        (model, iter) = widget.get_selection().get_selected()
        stype = None
        if iter is not None:
            (name,interface,protocol,stype,domain) = self.treemodel.get(iter,1,2,3,4,5)
        if stype == None:
            self.info_label.set_markup(_("<i>No service currently selected.</i>"))
            return
        #Asynchronous resolving
        self.server.ResolveService( int(interface), int(protocol), name, stype, domain, avahi.PROTO_UNSPEC, dbus.UInt32(0), reply_handler=self.service_resolved, error_handler=self.print_error)

    def protoname(self,protocol):
        if protocol == avahi.PROTO_INET:
            return "IPv4"
        if protocol == avahi.PROTO_INET6:
            return "IPv6"
        return "n/a"

    def siocgifname(self, interface):
        if interface <= 0:
            return "n/a"
        else:
            return self.server.GetNetworkInterfaceNameByIndex(interface)

    def get_interface_name(self, interface, protocol):
        if interface == avahi.IF_UNSPEC and protocol == avahi.PROTO_UNSPEC:
            return "Wide Area"
        else:
            return str(self.siocgifname(interface)) + " " + str(self.protoname(protocol))

    def service_resolved(self, interface, protocol, name, stype, domain, host, aprotocol, address, port, txt, flags):
        print("Service data for service '%s' of type '%s' in domain '%s' on %i.%i:"
                % (name, stype, domain, interface, protocol))

        print("\tHost %s (%s), port %i, TXT data: %s"
                % (host, address, port, str(avahi.txt_array_to_string_array(txt))))

        self.update_label(interface, protocol, name, stype, domain, host, aprotocol, address, port, avahi.txt_array_to_string_array(txt))

    def print_error(self, err):
        error_label = "<b>Error:</b> %s" % (err)
        self.info_label.set_markup(error_label)
        print("Error:", str(err))

    def lookup_type(self, stype):
        global service_type_db

        try:
            return service_type_db[stype].decode('utf-8', 'replace')
        except KeyError:
            return stype

    def new_service(self, interface, protocol, name, stype, domain, flags):
        print("Found service '%s' of type '%s' in domain '%s' on %i.%i."
                % (name, stype, domain, interface, protocol))

        if (interface,protocol) not in self.zc_ifaces:
            ifn = self.get_interface_name(interface, protocol)
            self.zc_ifaces[(interface,protocol)] = self.insert_row(self.treemodel, None, ifn, None,interface,protocol,None,domain)

        if (interface,protocol,domain) not in self.zc_domains:
            self.zc_domains[(interface,protocol,domain)] = self.insert_row(self.treemodel, self.zc_ifaces[(interface,protocol)], domain,None,interface,protocol,None,domain)

        if (interface,protocol,stype,domain) not in self.zc_types:
            thisDomain = self.zc_domains[(interface,protocol,domain)]
            self.zc_types[(interface,protocol,stype,domain)] = self.insert_row(self.treemodel, thisDomain, self.lookup_type(stype), name, interface,None,None,None)

        treeiter = self.insert_row(self.treemodel,self.zc_types[(interface,protocol,stype,domain)], name, name, interface,protocol,stype,domain)
        self.services_browsed[(interface, protocol, name, stype, domain)] = treeiter
        # expand the tree of this path
        self.tree_view.expand_to_path(self.treemodel.get_path(treeiter))

    def remove_service(self, interface, protocol, name, stype, domain, flags):
        print("Service '%s' of type '%s' in domain '%s' on %i.%i disappeared."
                % (name, stype, domain, interface, protocol))
        self.info_label.set_markup("")
        treeiter=self.services_browsed[(interface, protocol, name, stype, domain)]
        parent = self.treemodel.iter_parent(treeiter)
        self.treemodel.remove(treeiter)
        del self.services_browsed[(interface, protocol, name, stype, domain)]
        if self.treemodel.iter_has_child(parent) == False:
            treeiter=self.zc_types[(interface,protocol,stype,domain)]
            parent = self.treemodel.iter_parent(treeiter)
            self.treemodel.remove(treeiter)
            del self.zc_types[(interface,protocol,stype,domain)]
            if self.treemodel.iter_has_child(parent) == False:
                treeiter=self.zc_domains[(interface,protocol,domain)]
                parent = self.treemodel.iter_parent(treeiter)
                self.treemodel.remove(treeiter)
                del self.zc_domains[(interface,protocol,domain)]
                if self.treemodel.iter_has_child(parent) == False:
                    treeiter=self.zc_ifaces[(interface,protocol)]
                    parent = self.treemodel.iter_parent(treeiter)
                    self.treemodel.remove(treeiter)
                    del self.zc_ifaces[(interface,protocol)]

    def new_service_type(self, interface, protocol, stype, domain, flags):
        global service_browsers

        # Are we already browsing this domain for this type?
        if (interface, protocol, stype, domain) in service_browsers:
            return

        print("Browsing for services of type '%s' in domain '%s' on %i.%i ..."
                % (stype, domain, interface, protocol))

        b = dbus.Interface(self.bus.get_object(avahi.DBUS_NAME, self.server.ServiceBrowserNew(interface, protocol, stype, domain, dbus.UInt32(0))),  avahi.DBUS_INTERFACE_SERVICE_BROWSER)
        b.connect_to_signal('ItemNew', self.new_service)
        b.connect_to_signal('ItemRemove', self.remove_service)

        service_browsers[(interface, protocol, stype, domain)] = b

    def browse_domain(self, interface, protocol, domain):
        global service_type_browsers

        # Are we already browsing this domain?
        if (interface, protocol, domain) in service_type_browsers:
            return

        if self.stype is None:
            print("Browsing domain '%s' on %i.%i ..." % (domain, interface, protocol))

            try:
                b = dbus.Interface(self.bus.get_object(avahi.DBUS_NAME, self.server.ServiceTypeBrowserNew(interface, protocol, domain, dbus.UInt32(0))),  avahi.DBUS_INTERFACE_SERVICE_TYPE_BROWSER)
            except DBusException as e:
                print(e)
                error_msg("You should check that the avahi daemon is running.\n\nError : %s" % e)
                sys.exit(0)

            b.connect_to_signal('ItemNew', self.new_service_type)

            service_type_browsers[(interface, protocol, domain)] = b
        else:
            self.new_service_type(interface, protocol, self.stype, domain)

    def new_domain(self,interface, protocol, domain, flags):
        if (interface,protocol) not in self.zc_ifaces:
            ifn = self.get_interface_name(interface, protocol)
            self.zc_ifaces[(interface,protocol)] = self.insert_row(self.treemodel, None, ifn,None,interface,protocol,None,domain)
        if (interface,protocol,domain) not in self.zc_domains:
            self.zc_domains[(interface,protocol,domain)] = self.insert_row(self.treemodel, self.zc_ifaces[(interface,protocol)], domain,None,interface,protocol,None,domain)
        if domain != "local":
            self.browse_domain(interface, protocol, domain)

    def pair_to_dict(self, l):
        res = dict()
        for el in l:
            if "=" not in el:
                res[el]=''
            else:
                tmp = el.split('=',1)
                if len(tmp[0]) > 0:
                    res[tmp[0]] = tmp[1]
        return res


    def update_label(self,interface, protocol, name, stype, domain, host, aprotocol, address, port, txt):
        if len(txt) != 0:
            txts = ""
            txtd = self.pair_to_dict(txt)
            for k,v in txtd.items():
                txts+="<b>" + _("TXT") + " <i>%s</i></b> = %s\n" % (k,v)
        else:
            txts = "<b>" + _("TXT Data:") + "</b> <i>" + _("empty") + "</i>"

        infos = "<b>" + _("Service Type:") + "</b> %s\n"
        infos += "<b>" + _("Service Name:") + "</b> %s\n"
        infos += "<b>" + _("Domain Name:") + "</b> %s\n"
        infos += "<b>" + _("Interface:") + "</b> %s %s\n"
        infos += "<b>" + _("Address:") + "</b> %s/%s:%i\n%s"
        infos = infos % (stype, name, domain, self.siocgifname(interface), self.protoname(protocol), host, address, port, txts.strip())
        self.info_label.set_markup(infos)

    def insert_row(self, model,parent,
                   content, name, interface,protocol,stype,domain):
        myiter=model.insert_after(parent,None)
        model.set(myiter,0,content,1,name,2,str(interface),3,str(protocol),4,stype,5,domain)
        return myiter

    def new(self):
        self.treemodel=Gtk.TreeStore(GObject.TYPE_STRING, GObject.TYPE_STRING, GObject.TYPE_STRING, GObject.TYPE_STRING, GObject.TYPE_STRING, GObject.TYPE_STRING)
        self.tree_view.set_model(self.treemodel)

        #creating the columns headers
        self.tree_view.set_headers_visible(False)
        renderer=Gtk.CellRendererText()
        column=Gtk.TreeViewColumn("",renderer, text=0)
        column.set_resizable(True)
        column.set_sizing(Gtk.TreeViewColumnSizing.GROW_ONLY)
        column.set_expand(True)
        self.tree_view.append_column(column)

        self.domain = None
        self.stype = None
        self.zc_ifaces = {}
        self.zc_domains = {}
        self.zc_types = {}
        self.services_browsed = {}

        try:
            self.bus = dbus.SystemBus()
            self.server = dbus.Interface(self.bus.get_object(avahi.DBUS_NAME, avahi.DBUS_PATH_SERVER), avahi.DBUS_INTERFACE_SERVER)
        except Exception as e:
            print("Failed to connect to Avahi Server (Is it running?): %s" % e)
            sys.exit(1)

        if self.domain is None:
            # Explicitly browse .local
            self.browse_domain(avahi.IF_UNSPEC, avahi.PROTO_UNSPEC, "local")

            # Browse for other browsable domains
            db = dbus.Interface(self.bus.get_object(avahi.DBUS_NAME, self.server.DomainBrowserNew(avahi.IF_UNSPEC, avahi.PROTO_UNSPEC, "", avahi.DOMAIN_BROWSER_BROWSE, dbus.UInt32(0))), avahi.DBUS_INTERFACE_DOMAIN_BROWSER)
            db.connect_to_signal('ItemNew', self.new_domain)
        else:
            # Just browse the domain the user wants us to browse
            self.browse_domain(avahi.IF_UNSPEC, avahi.PROTO_UNSPEC, self.domain)

    def gtk_main_quit(self, *args):
        Gtk.main_quit()

def main():
    main_window = Main_window()
    # http://stackoverflow.com/a/16486080/434217
    import signal
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    Gtk.main()

if __name__ == "__main__":
    main()
