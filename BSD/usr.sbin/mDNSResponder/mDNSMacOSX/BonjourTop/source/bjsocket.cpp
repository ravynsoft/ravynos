//
//  bjsocket.cpp
//  TestTB
//
//  Created by Terrin Eager on 10/24/12.
//
//
#define __APPLE_USE_RFC_2292

#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <ifaddrs.h>
#include <net/if.h>

#include "bjsocket.h"
#include "bjstring.h"


////////////////////////////
/// BJSocket
///////////////////////////
const BJ_UINT16 BonjourPort = 5353;

BJSocket::BJSocket()
{
    socketHandle = 0;
    buffer = NULL;
    IPVersion = 0;
    interfaceID = 0;
}

BJSocket::~BJSocket()
{

}

bool BJSocket::Init()
{

    socketHandle = 0;
    buffer = (BJ_UINT8*)malloc(MAX_FRAME_SIZE);

    if (buffer == NULL)
        return false;

    //Setup msghdr;
    memset(&socketMsghdr, '\0', sizeof(socketMsghdr));
    socketMsghdr.msg_name = &peerAddr;
    socketMsghdr.msg_namelen = sizeof(peerAddr);
    socketMsghdr.msg_iov = socketIovec;
    socketMsghdr.msg_iovlen = 1;
    socketIovec[0].iov_base = (char *) buffer;
    socketIovec[0].iov_len = MAX_FRAME_SIZE;


    socketMsghdr.msg_control = socketCmsghdr;
    socketMsghdr.msg_controllen = sizeof(socketCmsghdr);

    return true;

}

bool BJSocket::CreateListenerIPv4(BJString interfaceName)
{
    bool bResult = true;
    const int onoptval = 1;

    if (socketHandle)
        Close();

    Init();


    if (interfaceName.GetLength() > 0)
        interfaceID  = if_nametoindex(interfaceName.GetBuffer());


    socketHandle = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
//     socketHandle = socket(PF_INET,SOCK_DGRAM,IPPROTO_RAW);

    if (-1 == setsockopt(socketHandle,SOL_SOCKET,SO_REUSEPORT,&onoptval,sizeof(onoptval)))
    {
        printf("setsockopt for SO_REUSEPORT failed");
        Close();
        return false;
    }

    JoinMulticastv4(interfaceName);

    // set PktInfo to get dest address

    if (-1 == setsockopt(socketHandle, IPPROTO_IP, IP_PKTINFO, &onoptval, sizeof(onoptval)))
    {
        printf("setsockopt for IP_PKTINFO failed");
        Close();
        return false;
    }

    // bind to socket

    struct sockaddr_in sa;
    memset(&sa,0,sizeof(sockaddr_in));
    sa.sin_len = sizeof(sockaddr_in);
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(BonjourPort);

    if (-1 == bind(socketHandle,(struct sockaddr*)&sa,sizeof(sa)))
    {
        printf("error in bind: %s\n",strerror(errno));
        Close();
        return false;
    }
    IPVersion = 4;

    return bResult;
}

bool BJSocket::CreateListenerIPv6(BJString interfaceName)
{
    bool bResult = true;
    const int onoptval=1;

    if (socketHandle)
        Close();

    Init();

    if (interfaceName.GetLength() > 0)
        interfaceID  = if_nametoindex(interfaceName.GetBuffer());

   socketHandle = socket(PF_INET6,SOCK_DGRAM,IPPROTO_UDP);

    if (-1 == setsockopt(socketHandle,SOL_SOCKET,SO_REUSEPORT,&onoptval,sizeof(onoptval)))
    {
        printf("setsockopt for SO_REUSEPORT failed");
        Close();
        return false;
    }

    JoinMulticastv6(interfaceName);

    // set PktInfo to get dest address
    if (-1 == setsockopt(socketHandle, IPPROTO_IPV6, IPV6_PKTINFO, &onoptval, sizeof(onoptval)))
    {
        printf("setsockopt for IP_PKTINFO failed");
        Close();
        return false;
    }

    // bind to socket
    struct sockaddr_in6 sa6;
    memset(&sa6,0,sizeof(sockaddr_in6));
    sa6.sin6_len = sizeof(sockaddr_in6);
    sa6.sin6_family = AF_INET6;
    sa6.sin6_addr = in6addr_any;
    sa6.sin6_port = htons(BonjourPort);

    if (-1 == bind(socketHandle,(struct sockaddr*)&sa6,sizeof(sa6)))
    {
        printf("error in bind: %s\n",strerror(errno));
        Close();
        return false;
    }
    IPVersion = 6;

    return bResult;
}

bool BJSocket::Close()
{
    bool bResult = true;

    if (socketHandle)
        close(socketHandle);

    socketHandle = 0;

    return bResult;
}

int BJSocket::Read()
{
    int nLength = (int) recvmsg(socketHandle, &socketMsghdr,0);
    if (!CheckInterface())
        return 0;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    m_CurrentFrame.Set(buffer-14-40-8,nLength,tv.tv_sec*1000000ll + tv.tv_usec);
    return nLength;
}

BJIPAddr* BJSocket::GetSrcAddr()
{
    sourceAddr.Set(&peerAddr);
    return &sourceAddr;
}

BJIPAddr* BJSocket::GetDestAddr()
{

    struct cmsghdr *cmsg;

    for(cmsg = CMSG_FIRSTHDR(&socketMsghdr); cmsg != NULL; cmsg = CMSG_NXTHDR(&socketMsghdr, cmsg))
    {

        if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_PKTINFO)
        {
            struct in_pktinfo* pPktInfo = (struct in_pktinfo*)CMSG_DATA(cmsg);

            destAddr.Set(&pPktInfo->ipi_addr);
        }
        if (cmsg->cmsg_level == IPPROTO_IPV6 && cmsg->cmsg_type == IPV6_PKTINFO)
        {
            struct in6_pktinfo* pPktInfo = (struct in6_pktinfo*)CMSG_DATA(cmsg);
            destAddr.Set(&pPktInfo->ipi6_addr);
        }
    }
    return &destAddr;
}

bool BJSocket::CheckInterface()
{
    if (interfaceID ==0)
        return true;
    struct cmsghdr *cmsg;

    bool bFound = false;

    for(cmsg = CMSG_FIRSTHDR(&socketMsghdr); cmsg != NULL; cmsg = CMSG_NXTHDR(&socketMsghdr, cmsg))
    {

        if ((cmsg->cmsg_level == IPPROTO_IP) && cmsg->cmsg_type == IP_PKTINFO)
        {
            bFound = true;
            struct in_pktinfo* pPktInfo = (struct in_pktinfo*)CMSG_DATA(cmsg);
            if (pPktInfo->ipi_ifindex == interfaceID)
                return true;
            else
            {
                if (pPktInfo->ipi_ifindex != 4)
                {
                    sourceAddr.Set(&peerAddr);
                    printf("address:%d %s \n",pPktInfo->ipi_ifindex,sourceAddr.GetString());
                }
            }
        }
        if (cmsg->cmsg_level == IPPROTO_IPV6 && cmsg->cmsg_type == IPV6_PKTINFO)
        {
            bFound = true;
            struct in6_pktinfo* pPktInfo = (struct in6_pktinfo*)CMSG_DATA(cmsg);
            if (pPktInfo->ipi6_ifindex == interfaceID)
                return true;
        }
    }
    if (!bFound)
         printf("PKTINFO not found \n");
    return false;
}

bool BJSocket::IsMulticastPacket()
{
    return GetDestAddr()->IsBonjourMulticast();

}

int BJSocket::GetSockectHandle()
{
    return socketHandle;
}

BJ_UINT8* BJSocket::GetBuffer()
{
    return buffer;
}

void BJSocket::JoinMulticastv4(BJString interfaceName)
{
   if (interfaceName.GetLength() == 0)
   {
       // join Multicast group
       struct ip_mreq imr;
       imr.imr_multiaddr.s_addr = inet_addr( "224.0.0.251");
       imr.imr_interface.s_addr = INADDR_ANY;
       if (-1 == setsockopt(socketHandle, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(imr)))
       {
           printf("setsockopt for IP_ADD_MEMBERSHIP failed");
       }
       return;
   }

    struct ifaddrs *ifa, *orig;

    getifaddrs(&ifa);

    orig = ifa;

    for ( ; ifa; ifa = ifa->ifa_next)
    {
        if (interfaceName == ifa->ifa_name  && ifa->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *ifa_addr = (struct sockaddr_in *)ifa->ifa_addr;
            struct ip_mreq imr;
            imr.imr_multiaddr.s_addr = inet_addr( "224.0.0.251");
            imr.imr_interface.s_addr = ifa_addr->sin_addr.s_addr;
            if (-1 == setsockopt(socketHandle, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(imr)))
            {
                printf("setsockopt for IP_ADD_MEMBERSHIP failed");
            }
        }
    }

    freeifaddrs(orig);

}

void BJSocket::JoinMulticastv6(BJString interfaceName)
{

    if (interfaceName.GetLength() == 0)
        return;

    // join Multicast group
    struct in6_addr BonjourMultiaddr = {{{ 0xFF,0x02,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0xFB }}};
    struct ipv6_mreq i6mr;
    memset(&i6mr,0,sizeof(i6mr));
    memcpy(&i6mr.ipv6mr_multiaddr, &BonjourMultiaddr, sizeof(BonjourMultiaddr));
    if (interfaceName.GetLength() > 0)
        i6mr.ipv6mr_interface = interfaceID;
    else
        i6mr.ipv6mr_interface = __IPV6_ADDR_SCOPE_SITELOCAL;
    int err = setsockopt(socketHandle, IPPROTO_IPV6, IPV6_JOIN_GROUP, &i6mr, sizeof(i6mr));
    if (err < 0 && (errno != EADDRINUSE))
    {
        printf("setsockopt for IPV6_JOIN_GROUP failed %d",errno);
    }
}

////////////////////////////////////////
// BJSelect
///////////////////////////////////////


BJSelect::BJSelect()
{
    FD_ZERO(&socketSet);
    maxSocket = 0;

}

bool BJSelect::Add(BJSocket& s)
{
    int sock = s.GetSockectHandle();
    FD_SET(sock, &socketSet);
    if (sock > maxSocket)
        maxSocket = sock;

    return true;

}

int BJSelect::Wait(int sec)
{
    struct timeval tv;
    memset(&tv, 0, sizeof(tv));
    tv.tv_sec = sec;

    int result = select(maxSocket+1, &socketSet, NULL, NULL, &tv);
    return result;

}

bool BJSelect::IsReady(BJSocket& Socket)
{
    int bIsSet = FD_ISSET(Socket.GetSockectHandle(), &socketSet);
    return (bIsSet != 0);
}




