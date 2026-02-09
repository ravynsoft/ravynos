/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 *  CFNetDiagnosticPing.c
 *  CFNetwork
 *
 *  Created by Louis Gerbarg on 8/30/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

//Copied from Network Diagnostic, from sample code. Should be migrated to CFHost based, and made API

#include <CoreFoundation/CoreFoundation.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>

struct PingICMPPacket 
{
    struct icmp 	icmpHeader; //required icmp header
    struct timeval 	packetTimeStamp; //our optional data will be the packet send time.
};

static int in_cksum(u_short *addr, int len);
static int WaitAndPrintICMPs(int socketConnectionToHost, int TimeoutInSeconds, int ReturnImmediatlyAfterReply, int* GotResponse);
static int CreateAndSendICMPPacket(struct sockaddr_in HostAddress, int SequenceNumber, int SocketConnectionToHost);
static int CreateSocketForCommunicationWithHost(const struct sockaddr_in HostAddress, int* SocketToReturn);
static int LookupHostAddress(const char* HostToPing, struct sockaddr_in* HostAddress); 

/*****************************************************
 * SimplePing
 *****************************************************
 * Purpose:  This function uses the above functions to ping a given remote host with
 * a given number of packets and with a given timeout to wait for responses on each packet.
 * Note this function does not require root permissions since on MacOSX 10.2.x pings are
 * possible using non-privaldged datagram sockets.
 *
 * Parameters:
 *	HostToPing 		A C-String describing the remote host.  On calling
 * SimplePing this variable will hold a description of the host as a C-String.  The string
 * can either be a hostname (e.g. www.google.com) or an IP address (e.g. 17.203.23.111)
 *
 * 	NumberOfPacketsToSend	An integer.  On calling SimplePing this variable holds
 * the number of ping packets to send to the host before stopping.
 *
 * 	PingTimeoutInSeconds	An integer.  On calling Simple Ping this variable holds
 * the amount of time SimplePing will wait for reponses from the ping before going onto the next packet.
 *
 * 	ReturnImmediatlyAfterReply An integer.  On calling Simple Ping if this variable is zero then 
 * we will wait for the timeout to occur even if we get the appropriate ping reply (waiting for any other packets).
 * If this variable is non-zero then simple ping will return immediatly after getting the appropriate ping reply to
 * the request.
 *
 * 	*Function Result* 	A UNIX error integer return value as described in:
 *				/usr/include/sys/errno.h on a BSD system.
 * 				Note: This value will be zero on success.
 *****************************************************/

int _CFNetDiagnosticPing(CFStringRef HostToPing, const int NumberOfPacketsToSend, const int PingTimeoutInSeconds);

int _CFNetDiagnosticPing(CFStringRef HostToPing, const int NumberOfPacketsToSend, const int PingTimeoutInSeconds)
{
    struct sockaddr_in hostAddress;
    int packetSequenceNumber;
    int socketConnectionToHost;
    int gotResponse = 0;
    int numberPacketsSent = 0;
    int numberPacketsReceived = 0;
	char HostToPingStr[256];
							
    if ((HostToPing == NULL) || (NumberOfPacketsToSend < 1) || (PingTimeoutInSeconds < 1))
    {
        //printf("Invalid arguments to SimplePing\n");
        return(EINVAL); //invalid arguments error
    }
	
	if(!CFStringGetCString((CFStringRef)HostToPing, HostToPingStr, sizeof(HostToPingStr), kCFStringEncodingASCII)) {
		return(EINVAL);
	}

    int error = LookupHostAddress(HostToPingStr, &hostAddress);
    
    if (error != 0)
    {
        //printf("Unable to lookup host address.  UNIX Error: %d", error);
        return(error);
    }
    
    //printf("PING %s (%s)\n", HostToPing, inet_ntoa(hostAddress.sin_addr));
    //fflush(stdout);
    
    error = CreateSocketForCommunicationWithHost(hostAddress, &socketConnectionToHost);
    
    if (error != 0)
    {
        //printf("Unable to create socket. UNIX Error: %d", error);
        return(error);
    }

    packetSequenceNumber = 0;
    while ((packetSequenceNumber < NumberOfPacketsToSend) && (error == 0)) 
    {
        //printf("*Pinging host %s: icmp_seq=%u\n", 
        //        inet_ntoa(hostAddress.sin_addr), packetSequenceNumber);
        //fflush(stdout);

        error = CreateAndSendICMPPacket(hostAddress, 
                             packetSequenceNumber, socketConnectionToHost);
    
        if (error == 0) //successfully sent packet
        {
            numberPacketsSent = numberPacketsSent + 1;

            //Here we wait for the return on the packet until we get it or until the timeout fires.
            error = WaitAndPrintICMPs(socketConnectionToHost, PingTimeoutInSeconds, 1, &gotResponse);
            
            if (error == 0) //was able to wait for reply successfully (whether we got a reply or not)
            {
                if (gotResponse == 1)
                    {numberPacketsReceived = numberPacketsReceived + 1;}
            }
        }
        
        packetSequenceNumber = packetSequenceNumber + 1;
    }
    
    close(socketConnectionToHost); //closing the socket connection since we are finished with it.
    
    //printf("\n--- %s ping statistics ---\n", HostToPing);
    //printf("%d packets transmitted, %d packets received, %.0f%% packet loss", 
    //            numberPacketsSent, numberPacketsReceived, 
    //            100*((double)numberPacketsSent - 
    //            (double)numberPacketsReceived)/(double)numberPacketsSent);
    //fflush(stdout);
    
    return(error);
}

/*****************************************************
 * LookupHostAddress
 *****************************************************
 * Purpose:  This function provides a mechanism to get a host address
 * as a internet address structure given only a string describing the
 * remote host.  The string passed will be either a host name
 * (e.g. www.apple.com) or an IP address (e.g. 17.203.23.111)
 *
 * Parameters:
 * 	HostToPing		A constant C-string.  On calling
 * LookupHostAddress this variable holds a CString describing the
 * remote host.  The string can either be a host name (e.g. www.google.com)
 * or an IP address (e.g. 17.203.23.112)
 *
 * 	HostAddress	A pointer to a pre-allocated array of sockaddr_in structure.
 * On calling LookupHostAddress this variable must be a pointer to a
 * pre-allocated sockaddr_in structure.  On successful return from LookupHostAddress
 * this variable will hold the host address expressed as an sockaddr_in structure.
 * On failed return the value of this variable is undefined
 *
 * 	*Function Result* 	A UNIX error integer return value as described in:
 *				/usr/include/sys/errno.h on a BSD system.
 * 				Note: This value will be zero on success.
 *****************************************************/
 
static int LookupHostAddress(const char* HostToPing, struct sockaddr_in* HostAddress)
{
    struct hostent * 	hostInformation;
    
    //Checking input argument for validity
    if ((HostToPing == NULL) || (HostAddress == NULL))
    {	
        return(EINVAL); //invalid argument error return
    }

    //Initalizing host address
    memset(HostAddress, 0, sizeof(struct sockaddr_in));

    /* Calling inet_addr to try to interpret the input address as a 
     * IP string "xx.xx.xx.xx.xx.  If this doesn't work then we will
     * try gethostbyname which will resolve any domain names (i.e. www.google.com)
     * First Argument: The IP address expressed as a character string.  Assuming
     *    the string passed to us is an IP string we try to interpret the string.
     *    if this doesn't work then we will try gethostbyname.
     * Return Value: If properly formed input then what is returned is an internet
     *    address structure.  We will use this to set the internet address structure
     *    in our socket address.  If the request is malformed then the constant
     *    INADDR_NONE will be returned for the internet address.
     */
    HostAddress->sin_addr.s_addr = inet_addr(HostToPing);
    
    //Checking to see if IP address was got correctly from character string.
    if (HostAddress->sin_addr.s_addr != INADDR_NONE) //Success no error returned!
    {	
        //Setting socket family to internet (TCP/IP) since know it was IP address interpreted
        HostAddress->sin_family = AF_INET;
    }
    else //failure!  now try with gethostbyname
    {
        /* The inet_addr call failed because the string isn't an IP address.  Instead
         * we will try to interpret it as a host name (e.g. www.google.com).  
         * First Argument: The character string we are trying to interpret of as a
         *    host name.  
         * Return Value: A host information structure on success which will contain the
         *    information we need for connecting to the host.  On failure a NULL pointer
         *    will be returned
         */
        hostInformation = gethostbyname(HostToPing);

        if (hostInformation == NULL)  //Failure!
        {
            //Failure unable to interpret character string as host name.  
            //We give up here by returning can't find host (unreachable host host).
            return(EHOSTUNREACH);
        }
        
        //Setting the socket family for connecting to the host.  We get this information from
        //the hostInformation structure directly
        HostAddress->sin_family = hostInformation->h_addrtype;
        
        //Now getting the internet address structure from our host information structure. 
        //We copy the structure into ours using a memmove 
        memmove(&HostAddress->sin_addr, hostInformation->h_addr, hostInformation->h_length);
    }
    
    /* Now for the host we have the address and connection family for the host we can return this
     * information
     */
    return(0); //return zero indicating success.
}

/*****************************************************
 * CreateSocketForCommunicationWithHost
 *****************************************************
 * Purpose:  This function sets up a socket for communicating
 * with the given remote host and also sets the parameters of 
 * that socket to our required specifications (timeout of 1 second
 * and larger receive buffer).
 *
 * Parameters:
 * 	HostAddress		A sockaddr_in structure.  On calling 
 * CreateSocketForCommunicationWithHost this variable has an address
 * of the host the socket will be established with expressed in sockaddr_in format.
 *
 * 	SocketToReturn	A pointer to a pre-allocated integer.  On successful 
 * return from CreateSocketForCommunicationWithHost this variable will hold
 * a socket descriptor used to communicate with the socket in future calls.  On
 * failed result this variable will be undefined.  Note on successful result its
 * the caller's responsibility to close the socket.
 *
 * 	*Function Result* 	A UNIX error integer return value as described in:
 *				/usr/include/sys/errno.h on a BSD system.
 * 				Note: This value will be zero on success.
 *****************************************************/
 
static int CreateSocketForCommunicationWithHost(const struct sockaddr_in HostAddress, int* SocketToReturn)
{
    struct protoent* 	protocalInformation;
    const int kRecieveSocketBufferSize = 50 * 1024; //here we want 50K for size of receive buffer
    struct timeval pingTimeout;
    int error = 0;

    /* Getting the protocal information we need to create the socket.  This socket will use the
     * icmp protocal (protocal used for pings).  We first need to get the procal information with
     * a getprotobyname call
     * First Argument: The name of the protocal we will be using
     * Return Value: A pointer to a protocal structure which provides us with the protocal
     * information used to create the socket.  On failure the pointer will be NULL.
     */
    protocalInformation = getprotobyname("icmp");
    
    //Check to see if got protocal information successfully.
    if (protocalInformation == NULL)
    {
        //here the protocal we want isn't supported or isn't found.  Need to fail here
        return(EPFNOSUPPORT); //return protocal not supported UNIX error
    }

    /* Now creating the socket which will be used for pinging the remote host.  Note
     * here we need to create a socket to send the ping.  Note previous to MacOSX 10.2
     * this required a raw socket which required root permissions.  However, now in 10.2.x
     * you can simply create a datagram socket and can use that to ping remote hosts.
     * First Argument: The communication domain to use.  In this case the ARPA internet 
     *    protocal which is used to do pings.
     * Second Argument: The type of socket to create.  In this case we need a raw socket
     *    to ping the remote host.
     * Third Argument: The partucular protocal to use over the socket.  Here we use the
     *    icmp protocal so pass the protocal number from the protocal information found earlier.  
     * Return Value:  On success the created socket will be returned.  On failure a value
     *    less then zero will be returned.
     */
    *SocketToReturn = socket(AF_INET, SOCK_DGRAM, protocalInformation->p_proto);
    
    if (*SocketToReturn < 0)
    {
        //error creating socket give up here.  Return operation not permitted error indicating 
        //we can't create socket.
        return(EPERM);
    }
    
    /* Now that we have the socket created there is one extra step we would like to do.  
     * That is increase the receive buffer size.  The reason we do this is when we send
     * an echo request there can be many replies. This means that a regular sized buffer
     * might get overwelmed.  Thus we increase the receive buffer size to 50K which
     * should be large enough to hold the ICMP replies.
     * First Argument: The socket we are editing the receive buffer size on.  In this case
     *    the socket we just created.
     * Second Argument: UNIX constant SOL_SOCKET indicating we are editing a socket level
     *    attribute (see man page for setsockopt for more details).
     * Third Argument: UNIX constant SO_RCVBUF indicating we are editing a receive buffer on
     *    the socket (see man page for setsockopt for more details).
     * Forth Argument: The new size of the buffer we desire.  In this case the arbitrarily
     *    chosen 50K buffer.
     * Fifth Argument: The size of the forth argument (i.e. sizeof())
     * Return Value: There is actually a return value of zero on success and -1 on failure.
     *    However, whether we get an error or not we will continue.  We would *like* to increase
     *    the buffer size but its not required.
     */
    (void) setsockopt(*SocketToReturn, SOL_SOCKET, SO_RCVBUF, 
                        &kRecieveSocketBufferSize, sizeof(kRecieveSocketBufferSize));
    

    /* Now that we have the socket created we want to set the timeout on the socket.  This is
     * the time a ping request will wait before assuming failure.  Here we use the value which 
     * is passed to us as the timeout value.  We set the timeout by setting the timeout for
     * recieve requests on the socket using setsockopt.
     * First Argument: The socket we are editing the receive timeout.  In this case
     *    the socket we just created.  This timeout will end up being the time ping waits for a response.
     * Second Argument: UNIX constant SOL_SOCKET indicating we are editing a socket level
     *    attribute (see man page for setsockopt for more details).
     * Third Argument: UNIX constant SO_RCVTIMEO indicating we are editing the receive timeout on
     *    the socket (see man page for setsockopt for more details).
     * Forth Argument: The value that the timeout should be as a timeval structure.  In this case
     *    we use the timeout passed to us as the timeout for the requests.
     * Fifth Argument: The size of the forth argument (i.e. sizeof())
     * Return Value: There is actually a return value of zero on success and -1 on failure.
     */

    //Setting the ping timeout to one second.  This way a recieve call will timeout after 1 
    //second later on.  This saves us from having to use UNIX signal alarms for timeouts
    pingTimeout.tv_sec = 1;
    pingTimeout.tv_usec = 0;
    
    error = setsockopt(*SocketToReturn, SOL_SOCKET, SO_RCVTIMEO, 
                        &pingTimeout, sizeof(pingTimeout));
                        
    if (error != 0)
    {
        close(*SocketToReturn); //before returning error close the socket.
        //were unable to set timeout on the socket.  Here we will return an error.
        return(errno);
    }
                    
    return(0); //if got this far were successful.  Return zero for success.
}

/*****************************************************
 * CreateAndSendICMPPacket
 *****************************************************
 * Purpose:  This function creates an ICMP packet with the neccessary information
 * inside of it to make it a ping packet.  This function also sends that ping packet to
 * the remote host.
 *
 * Parameters:
 * 	HostAddress		A sockaddr_in structure.  On calling 
 * CreateAndSendICMPPacket this variable has an address
 * of the host where the ping will be sent.
 *
 * 	SequenceNumber	A integer.  when calling CreateAndSendICMPPacket this variable 
 * will hold the sequence number of the ping packet being sent.  This information is
 * then put in the ping packet sent to the host so when they reply we know which
 * send the reply is accociated with.
 *
 *	SocketConnectionToHost A socket descriptor expressed as an integer.  On calling
 * CreateAndSendICMPPacket this variable will be a valid socket which is already setup
 * for communicating with the remote host.  
 *
 * 	*Function Result* 	A UNIX error integer return value as described in:
 *				/usr/include/sys/errno.h on a BSD system.
 * 				Note: This value will be zero on success.
 *****************************************************/
 
 
static int CreateAndSendICMPPacket(struct sockaddr_in HostAddress, int SequenceNumber, int SocketConnectionToHost)
{	
    struct PingICMPPacket PacketToSend;
    int error;
    ssize_t sizeOfDataSent;
    // --- Setting up the packet we are going to send. --- //
    
    //ICMP type is an echo packet which is the packet type for a ping.
    PacketToSend.icmpHeader.icmp_type = ICMP_ECHO;
    
    //Zero code for ping packet.
    PacketToSend.icmpHeader.icmp_code = 0;

    //Sequence number of the packet is whatever is passed to us
    PacketToSend.icmpHeader.icmp_seq = SequenceNumber;
    
    //add our PID as the identifyer so we know later the ping originated from us.
    PacketToSend.icmpHeader.icmp_id = getpid(); 

    /* Now we will get time of day so we can add it to the "extra" data on the ICMP packet.
     * The time of day will allow us to calculate the round trip time on the ping opon
     * recieveing the echo packet
     */
    error = gettimeofday(&PacketToSend.packetTimeStamp, NULL);
    
    if (error != 0) //if not equal to zero then couldn't get time of day
    {
        if (errno != 0)
        {
            return(errno); //errno was set by gettimeofday.  Return that value as error.
        }
    
        return(EPERM); //could not perform operation return UNIX error code operation not permitted.
    }

    /* Now that we have the filled out packet we will calculate the checksum for the packet.
     * We actually will calculate checksum but only after we have everything filled out and
     * set the checksum value to zero (for calculation) 
     */
    PacketToSend.icmpHeader.icmp_cksum = 0;

    //The in_cksum function will calcluate the checksum for us given the packet and its length.
    PacketToSend.icmpHeader.icmp_cksum = in_cksum((u_short *)&PacketToSend, sizeof(PacketToSend));

    /* Now that the packet is finished we will go ahead and send the packet to the host
     * We use the sendto API to send the packet to the host.
     * First Argument: The Socket used for sending the packet to the host.  In this case the
     *    ICMP socket we already created.
     * Second Argument: The actual packet itself we already created
     * Third Argument: The size of the packet we are sending in this case the size of the packet
     *    structure
     * Forth Argument: This is for additional flags which we won't use.  Pass zero to ignore.
     * Fifth Argument: The host address to send the packet to.
     * Sixth Argument: The size of the host address passed in.
     * Return Value: This call returns the size of the data sent (hopefully it will be the
     *     same as PacketToSend).  On failure this function returns -1.
     */
    sizeOfDataSent = sendto(SocketConnectionToHost, &PacketToSend, 
                            sizeof(PacketToSend), 0, 
                            (struct sockaddr*) &HostAddress, sizeof(HostAddress));
              
    //if size of data sent is -1 (indicating error) or not size we wanted then we have a problem
    if ((sizeOfDataSent < 0) || (sizeOfDataSent != sizeof(PacketToSend)))
    {
        if (errno != 0)
        {
            //packet wasn't sent properly return error.
            return(errno); //return UNIX created by sendto
        }
        
        return(EPERM); //return more generic code since error code since errno wasn't set.
    }

    return(0); //If got this far were successful.  Return zero indicating success.
}

/*****************************************************
 * WaitAndPrintICMPs
 *****************************************************
 * Purpose:  This function creates an ICMP packet with the neccessary information
 * inside of it to make it a ping packet.  This function also sends that ping packet to
 * the remote host.
 *
 * Parameters:
 *	SocketConnectionToHost A socket descriptor expressed as an integer.  On calling
 * WaitAndPrintICMPs this variable will be a valid socket which is already setup
 * for communicating with the remote host.  This will be used in getting the reply
 * packets from the host.
 *
 * 	TimeoutInSeconds	A timeout expressed as an integer.  On calling 
 * WaitAndPrintICMPs this variable will have the number of seconds to wait for
 * ICMP replies.  Note even if a valid ICMP reply comes that matches what we expect
 * we will still wait for additional ICMP packets until the timeout has occurred.
 *
 * 	ReturnImmediatlyAfterReply An integer.  On calling WaitAndPrintICMPs if this variable is zero then 
 * we will wait for the timeout to occur even if we get the appropriate ping reply (waiting for any other packets).
 * If this variable is non-zero then simple ping will return immediatly after getting the appropriate ping reply to
 * the request.
 *
 * 	GotResponse		A pre-allocated integer.  when calling WaitAndPrintICMPs
 * this variable will be a pre-allocated integer.  On successful return from WaitAndPrintICMPs this
 * variable will hold a value of 1 if a ICMP packet response to our ping was recieved.
 * And a zero otherwise.  On failed return the value of this variable will be undefined.
 *
 * 	*Function Result* 	A UNIX error integer return value as described in:
 *				/usr/include/sys/errno.h on a BSD system.
 * 				Note: This value will be zero on success.
 *****************************************************/
 
static int WaitAndPrintICMPs(int socketConnectionToHost, int TimeoutInSeconds, int ReturnImmediatlyAfterReply, int* GotResponse)
{
    const int kBufferSize = 2048; //size we allocate for the reply buffer.
    char pingReplyBuffer[kBufferSize];
    struct sockaddr_in remoteHost;
    socklen_t sizeOfRemoteHost = (int) sizeof(remoteHost);
    ssize_t numberOfBytesReceived; 

    struct ip* packetInterpetedAsIPPacket;
    int ipHeaderLength;
    struct PingICMPPacket* icmpPacket;
    unsigned int icmpPacketSize;
    int error = 0;
    int gotStartTime = 0;
    
    struct timeval currentTime, timeSinceStartedWaiting;
    struct timeval startTime, roundTripTimeOnPacket;
    double roundTripTimeInMS;

    //Checking input argument for validity
    if ((GotResponse == NULL) || (TimeoutInSeconds < 1))
    {	
        return(EINVAL); //invalid argument error return
    }

    *GotResponse = 0;

    do
    {
        /* Now that he have sent the ping to the host we need to wait for a reply.  We do this
        * by calling recvfrom which will wait for a response for the timeout specified earlier
        * when creating the socket
        * First Argument: The socket we are waiting for data upon
        * Second Argument: The buffer to use for storing the reply from the remote host.
        * Third Argument: The size of the buffer in argument two
        * Forth Argument: Additional flags which can be used for special options.  We ignore this 
        *    by passing zero
        * Fifth Argument: A internet address structure used to hold the address information from where the
        *    data came from
        * Sixth Argument: Size of the buffer/structure in argument five.
        * Return Value: The size of the data returned.  On error this value will be less than zero.
        */
        
        numberOfBytesReceived = recvfrom(socketConnectionToHost, pingReplyBuffer, 
                                            sizeof(pingReplyBuffer), 0,
                                        (struct sockaddr *) &remoteHost, &sizeOfRemoteHost);
                                        
        if (numberOfBytesReceived < 0) //error reciving data, return error
        {
            error = errno; //return the UNIX errno variable which would be set by recvfrom.
        }
        
        if (gotStartTime == 0)
        {
            //Getting the start time for this function if we dont already have it.
            if (gettimeofday(&startTime, NULL) != 0)
            {
                if (errno != 0)
                {
                    //if got error getting time of day return UNIX error and give up.
                    return(errno);
                }
                
                return(EPERM); //return more generic error since wasn't set by gettimeofday
            }
            gotStartTime = 1;
        }
    
        if (error == 0)
        {
            /* Now that we have a reply we will test it to see if its the reply we expect.
            * First step is to discard the IP header from the raw packet we got back.  We can do this
            * by determining the size of the IP header and then moving our pointer beyond that.
            */
            
            //Interpret packet as IP packet to remove header
            packetInterpetedAsIPPacket = (struct ip*)pingReplyBuffer;
        
            //The ip_hl item within the IP packet has the length of the IP header expressed as bytes 
            //(shifted right twice, thus need to shift left to compensate.
            ipHeaderLength = packetInterpetedAsIPPacket->ip_hl << 2;
        
            //Now we know the IP header length we can get a pointer to the ICMP section of the packet
            icmpPacket = (struct PingICMPPacket*)(pingReplyBuffer + ipHeaderLength);
            
            icmpPacketSize = numberOfBytesReceived - ipHeaderLength;
        
            //The echo request must be at least as large as the one we sent to have all echo information.
            if (icmpPacketSize >= sizeof(struct PingICMPPacket))
            {
                //This packet also has to be an ICMP echo reply packet to the one.
                if (icmpPacket->icmpHeader.icmp_type == ICMP_ECHOREPLY)
                {
                    //To be our packet the id on the packet has to match our PID.  
                    //This is because in the echo
                    //the id wouldn't change and we sent pid as the id.
                    if (icmpPacket->icmpHeader.icmp_id == getpid())
                    {
                        //If we got this far then this is a reply to our ping request!
                        *GotResponse = 1;

                        //--- Print out ICMP packet information ---//
                    
                        //Get the current time for determining round trip time
                        if (gettimeofday(&currentTime, NULL) != 0)
                        {
                            if (errno != 0)
                            {
                                //if error getting time of day return UNIX error.
                                return(errno);
                            }
                            
                            return(EPERM);  //return more generic EPERM since gettimeofday 
                                            //didn't set error
                        }

                        //Subtracting the timeval structures to get roundTripTimeOnPacket
                        timersub(&currentTime, &(icmpPacket->packetTimeStamp), &roundTripTimeOnPacket);
                        
                        //getting round trip time in milliseconds
                        roundTripTimeInMS = (double) (roundTripTimeOnPacket.tv_sec*1000 + 
                                                    (((double) roundTripTimeOnPacket.tv_usec) / 1000));
                        
                        //printf("Response from %s: icmp_seq=%u ttl=%d time=%.3f ms\n", 
                        //        inet_ntoa(remoteHost.sin_addr),
                        //        icmpPacket->icmpHeader.icmp_seq,
                        //        packetInterpetedAsIPPacket->ip_ttl,
                        //        roundTripTimeInMS);
                        //fflush(stdout);
                    }//endif
                }//endif
            }//endif
        }//endif no error on getting packet
        else if (error == EAGAIN) 
        {
            //we ignore EAGAIN errors since they are fired by the 1 second timeout on the socket
            //On these occasions we just want to see if our timeout for the function has passed.
            error = 0;
        }
        
        //no errors up to this point then calculate the time for timeout
        if (error == 0)
        {
            //Get the current time for looking for determining timeout for function
            if (gettimeofday(&currentTime, NULL) != 0)
            {
                if (errno != 0)
                {
                    //if error getting time of day return UNIX error.
                    error = errno;
                }
                else 
                    {error = EPERM;} //get more generic error since gettimeofday didn't set value
            }
            
            //Subtract two timeval structures to get the time since we started this function
            timersub(&currentTime, &startTime, &timeSinceStartedWaiting);
        }

    }//end do while
    while ((error == 0) && (timeSinceStartedWaiting.tv_sec < TimeoutInSeconds) && ((ReturnImmediatlyAfterReply == 0) || (GotResponse == 0)));
    
    if (*GotResponse == 0)
    {
        //printf("No Response From Host\n"); fflush(stdout);
    }

    return(0);
}

//Here stealing the in_cksum function which computes checksum for our packets from original ping.
static int in_cksum(u_short *addr, int len)
{
	register int nleft = len;
	register u_short *w = addr;
	register int sum = 0;
	u_short answer = 0;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(u_char *)(&answer) = *(u_char *)w ;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return(answer);
}




