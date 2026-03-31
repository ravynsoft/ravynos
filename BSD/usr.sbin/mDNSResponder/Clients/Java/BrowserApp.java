/* -*- Mode: Java; tab-width: 4 -*-
 *
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 * Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
 * ("Apple") in consideration of your agreement to the following terms, and your
 * use, installation, modification or redistribution of this Apple software
 * constitutes acceptance of these terms.  If you do not agree with these terms,
 * please do not use, install, modify or redistribute this Apple software.
 *
 * In consideration of your agreement to abide by the following terms, and subject
 * to these terms, Apple grants you a personal, non-exclusive license, under Apple's
 * copyrights in this original Apple software (the "Apple Software"), to use,
 * reproduce, modify and redistribute the Apple Software, with or without
 * modifications, in source and/or binary forms; provided that if you redistribute
 * the Apple Software in its entirety and without modifications, you must retain
 * this notice and the following text and disclaimers in all such redistributions of
 * the Apple Software.  Neither the name, trademarks, service marks or logos of
 * Apple Computer, Inc. may be used to endorse or promote products derived from the
 * Apple Software without specific prior written permission from Apple.  Except as
 * expressly stated in this notice, no other rights or licenses, express or implied,
 * are granted by Apple herein, including but not limited to any patent rights that
 * may be infringed by your derivative works or by other works in which the Apple
 * Software may be incorporated.
 *
 * The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
 * COMBINATION WITH YOUR PRODUCTS.
 *
 * IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
 * OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	BrowserApp demonstrates how to use DNS-SD to browse for and resolve services.

	To do:
	- display resolved TXTRecord
 */


import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.text.*;
import javax.swing.*;
import javax.swing.event.*;

import com.apple.dnssd.*;


class	BrowserApp implements ListSelectionListener, ResolveListener, Runnable
{
	static BrowserApp	app;
	JFrame				frame;
	DomainListModel		domainList;
	BrowserListModel	servicesList, serviceList;
	JList				domainPane, servicesPane, servicePane;
	DNSSDService		servicesBrowser, serviceBrowser, domainBrowser;
	JLabel				hostLabel, portLabel;
	String				hostNameForUpdate;
	int					portForUpdate;

	public		BrowserApp()
	{
		frame = new JFrame("DNS-SD Service Browser");
		frame.addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e) {System.exit(0);}
		});

		domainList = new DomainListModel();
		servicesList = new ServicesBrowserListModel();
		serviceList = new BrowserListModel();

		try {
			domainBrowser = DNSSD.enumerateDomains( DNSSD.BROWSE_DOMAINS, 0, domainList);

			servicesBrowser = DNSSD.browse( 0, 0, "_services._dns-sd._udp.", "", servicesList);
			serviceBrowser = null;
		}
		catch ( Exception ex) { terminateWithException( ex); }

		this.setupSubPanes( frame.getContentPane());
		frame.pack();
		frame.setVisible(true);
	}

	protected void	setupSubPanes( Container parent)
	{
		parent.setLayout( new BoxLayout( parent, BoxLayout.Y_AXIS));

		JPanel browserRow = new JPanel();
		browserRow.setLayout( new BoxLayout( browserRow, BoxLayout.X_AXIS));
		domainPane = new JList( domainList);
		domainPane.addListSelectionListener( this);
		JScrollPane domainScroller = new JScrollPane( domainPane, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
		browserRow.add( domainScroller);
		servicesPane = new JList( servicesList);
		servicesPane.addListSelectionListener( this);
		JScrollPane servicesScroller = new JScrollPane( servicesPane, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
		browserRow.add( servicesScroller);
		servicePane = new JList( serviceList);
		servicePane.addListSelectionListener( this);
		JScrollPane serviceScroller = new JScrollPane( servicePane, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
		browserRow.add( serviceScroller);

/*
		JPanel buttonRow = new JPanel();
		buttonRow.setLayout( new BoxLayout( buttonRow, BoxLayout.X_AXIS));
		buttonRow.add( Box.createHorizontalGlue());
		JButton connectButton = new JButton( "Don't Connect");
		buttonRow.add( connectButton);
		buttonRow.add( Box.createRigidArea( new Dimension( 16, 0)));
*/

		JPanel labelRow = new JPanel();
		labelRow.setLayout( new BoxLayout( labelRow, BoxLayout.X_AXIS));
		labelRow.add( new JLabel( "  Host: "));
		hostLabel = new JLabel();
		labelRow.add( hostLabel);
		labelRow.add( Box.createRigidArea( new Dimension( 32, 0)));
		labelRow.add( new JLabel( "Port: "));
		portLabel = new JLabel();
		labelRow.add( portLabel);
		labelRow.add( Box.createHorizontalGlue());

		parent.add( browserRow);
		parent.add( Box.createRigidArea( new Dimension( 0, 8)));
		parent.add( labelRow);
//		parent.add( buttonRow);
		parent.add( Box.createRigidArea( new Dimension( 0, 16)));
	}

	public void valueChanged( ListSelectionEvent e)
	{
		try {
			if ( e.getSource() == domainPane && !e.getValueIsAdjusting())
			{
				int		newSel = domainPane.getSelectedIndex();
				if ( -1 != newSel)
				{
					if ( serviceBrowser != null)
						serviceBrowser.stop();
					serviceList.removeAllElements();
					servicesBrowser = DNSSD.browse( 0, 0, "_services._dns-sd._udp.", "", servicesList);
				}
			}
			else if ( e.getSource() == servicesPane && !e.getValueIsAdjusting())
			{
				int		newSel = servicesPane.getSelectedIndex();
				if ( serviceBrowser != null)
					serviceBrowser.stop();
				serviceList.removeAllElements();
				if ( -1 != newSel)
					serviceBrowser = DNSSD.browse( 0, 0, servicesList.getNthRegType( newSel), "", serviceList);
			}
			else if ( e.getSource() == servicePane && !e.getValueIsAdjusting())
			{
				int		newSel = servicePane.getSelectedIndex();

				hostLabel.setText( "");
				portLabel.setText( "");

				if ( -1 != newSel)
				{
					DNSSD.resolve( 0, serviceList.getNthInterface( newSel), 
										serviceList.getNthServiceName( newSel), 
										serviceList.getNthRegType( newSel), 
										serviceList.getNthDomain( newSel), 
										this);
				}
			}
		}
		catch ( Exception ex) { terminateWithException( ex); }
	}

	public void run()
	{
		hostLabel.setText( hostNameForUpdate);
		portLabel.setText( String.valueOf( portForUpdate));		
	}

	public void	serviceResolved( DNSSDService resolver, int flags, int ifIndex, String fullName, 
								String hostName, int port, TXTRecord txtRecord)
	{
		// We want to update GUI on the AWT event dispatching thread, but we can't stop
		// the resolve from that thread, since stop() is synchronized with this callback.
		// So, we stop the resolve on this thread, then invokeAndWait on the AWT event thread.

		resolver.stop();

		hostNameForUpdate = hostName;
		portForUpdate = port;

		try {
			SwingUtilities.invokeAndWait(this);
		}
		catch ( Exception e)
		{
			e.printStackTrace();
		}
	}

	public void	operationFailed( DNSSDService service, int errorCode)
	{
		service.stop();
		// handle failure here
	}

	protected static void	terminateWithException( Exception e)
	{
		e.printStackTrace();
		System.exit( -1);
	}

    public static void main(String s[]) 
    {
		app = new BrowserApp();
    }
}


class	BrowserListModel extends DefaultListModel implements BrowseListener, Runnable
{
	public		BrowserListModel()
	{
		addCache = new Vector();
		removeCache = new Vector();
	}

	/* The Browser invokes this callback when a service is discovered. */
	public void	serviceFound( DNSSDService browser, int flags, int ifIndex, 
							String serviceName, String regType, String domain)
	{
		addCache.add( new BrowserListElem( serviceName, domain, regType, ifIndex));
		if ( ( flags & DNSSD.MORE_COMING) == 0)
			this.scheduleOnEventThread();
	}

	public void	serviceLost( DNSSDService browser, int flags, int ifIndex,
							String serviceName, String regType, String domain)
	{
		removeCache.add( serviceName);
		if ( ( flags & DNSSD.MORE_COMING) == 0)
			this.scheduleOnEventThread();
	}

	public void	run()
	{
		while ( removeCache.size() > 0)
		{
			String	serviceName = (String) removeCache.remove( removeCache.size() - 1);
			int		matchInd = this.findMatching( serviceName);	// probably doesn't handle near-duplicates well.
			if ( matchInd != -1)
				this.removeElementAt( matchInd);
		}
		while ( addCache.size() > 0)
		{
			BrowserListElem	elem = (BrowserListElem) addCache.remove( addCache.size() - 1);
			if ( -1 == this.findMatching( elem.fServiceName))	// probably doesn't handle near-duplicates well.
				this.addInSortOrder( elem);
		}
	}

	public void	operationFailed( DNSSDService service, int errorCode)
	{
		// handle failure here
	}

	/* The list contains BrowserListElem's */
	class	BrowserListElem
	{
		public	BrowserListElem( String serviceName, String domain, String type, int ifIndex)
		{ fServiceName = serviceName; fDomain = domain; fType = type; fInt = ifIndex; }
		
		public String	toString() { return fServiceName; }
		
		public String	fServiceName, fDomain, fType;
		public int		fInt;
	}

	public String	getNthServiceName( int n)
	{
		BrowserListElem sel = (BrowserListElem) this.get( n);
		return sel.fServiceName;
	}

	public String	getNthRegType( int n)
	{
		BrowserListElem sel = (BrowserListElem) this.get( n);
		return sel.fType;
	}

	public String	getNthDomain( int n)
	{
		BrowserListElem sel = (BrowserListElem) this.get( n);
		return sel.fDomain;
	}

	public int		getNthInterface( int n)
	{
		BrowserListElem sel = (BrowserListElem) this.get( n);
		return sel.fInt;
	}

	protected void	addInSortOrder( Object obj)
	{
		int	i;
		for ( i = 0; i < this.size(); i++)
			if ( sCollator.compare( obj.toString(), this.getElementAt( i).toString()) < 0)
				break;
		this.add( i, obj);
	}

	protected int	findMatching( String match)
	{
		for ( int i = 0; i < this.size(); i++)
			if ( match.equals( this.getElementAt( i).toString()))
				return i;
		return -1;
	}

	protected void	scheduleOnEventThread()
	{
		try {
			SwingUtilities.invokeAndWait( this);
		}
		catch ( Exception e)
		{
			e.printStackTrace();
		}
	}		

	protected Vector	removeCache;	// list of serviceNames to remove
	protected Vector	addCache;		// list of BrowserListElem's to add

	protected static Collator	sCollator;

	static	// Initialize our static variables
	{
		sCollator = Collator.getInstance();
		sCollator.setStrength( Collator.PRIMARY);
	}
}


class	ServicesBrowserListModel extends BrowserListModel
{
	/* The Browser invokes this callback when a service is discovered. */
	public void	serviceFound( DNSSDService browser, int flags, int ifIndex, 
							String serviceName, String regType, String domain)
	// Overridden to stuff serviceName into regType and make serviceName human-readable.
	{
		regType = serviceName + ( regType.startsWith( "_udp.") ? "._udp." : "._tcp.");
		super.serviceFound( browser, flags, ifIndex, this.mapTypeToName( serviceName), regType, domain);
	}

	public void	serviceLost( DNSSDService browser, int flags, int ifIndex, 
							String serviceName, String regType, String domain)
	// Overridden to make serviceName human-readable.
	{
		super.serviceLost( browser, flags, ifIndex, this.mapTypeToName( serviceName), regType, domain);
	}

	protected String	mapTypeToName( String type)
	// Convert a registration type into a human-readable string. Returns original string on no-match.
	{
		final String[]	namedServices = {
			"_afpovertcp",	"Apple File Sharing",
			"_http",		"World Wide Web servers",
			"_daap",		"Digital Audio Access",
			"_apple-sasl",	"Apple Password Servers",
			"_distcc",		"Distributed Compiler nodes",
			"_finger",		"Finger servers",
			"_ichat",		"iChat clients",
			"_presence",	"iChat AV clients",
			"_ssh",			"SSH servers",
			"_telnet",		"Telnet servers",
			"_workstation",	"Macintosh Manager clients",
			"_bootps",		"BootP servers",
			"_xserveraid",	"XServe RAID devices",
			"_eppc",		"Remote AppleEvents",
			"_ftp",			"FTP services",
			"_tftp",		"TFTP services"
		};

		for ( int i = 0; i < namedServices.length; i+=2)
			if ( namedServices[i].equals( type))
				return namedServices[i + 1];
		return type;
	}
}


class	DomainListModel extends DefaultListModel implements DomainListener
{
	/* Called when a domain is discovered. */
	public void	domainFound( DNSSDService domainEnum, int flags, int ifIndex, String domain)
	{
		if ( !this.contains( domain))
			this.addElement( domain);
	}
	
	public void	domainLost( DNSSDService domainEnum, int flags, int ifIndex, String domain)
	{
		if ( this.contains( domain))
			this.removeElement( domain);
	}

	public void	operationFailed( DNSSDService service, int errorCode)
	{
		// handle failure here
	}
}

