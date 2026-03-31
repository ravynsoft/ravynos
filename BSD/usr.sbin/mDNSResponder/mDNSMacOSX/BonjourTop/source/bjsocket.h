//
//  bjsocket.h
//  TestTB
//
//  Created by Terrin Eager on 10/24/12.
//
//

#ifndef __TestTB__bjsocket__
#define __TestTB__bjsocket__

#include <iostream>

#include <sys/socket.h>

#include "bjtypes.h"
#include "bjIPAddr.h"
#include "bjstring.h"

#include "Frame.h"


class BJSocket
{
public:

    BJSocket();
    virtual ~BJSocket();

    bool Init();

    bool CreateListenerIPv4(BJString interfaceName);
    bool CreateListenerIPv6(BJString interfaceName);

    bool Close();

    int Read();

    Frame m_CurrentFrame;

    bool IsMulticastPacket();

    int GetSockectHandle();

    BJ_UINT8* GetBuffer();
    BJIPAddr* GetSrcAddr();
    BJIPAddr* GetDestAddr();

private:
    void JoinMulticastv4(BJString interfaceName);
    void JoinMulticastv6(BJString interfaceName);

    bool CheckInterface();

    BJ_UINT32 interfaceID;

    int socketHandle;
    BJ_UINT8* buffer;


    int IPVersion;
    BJIPAddr sourceAddr;
    BJIPAddr destAddr;

    struct msghdr socketMsghdr;
    sockaddr_storage peerAddr;
    struct iovec socketIovec[1];
    struct cmsghdr socketCmsghdr[10];

};

class BJSelect
{
public:
    BJSelect();

    bool Add(BJSocket& s);
    int Wait(int sec);

    bool IsReady(BJSocket& s);

private:
    fd_set socketSet;
    int maxSocket;

};

#endif /* defined(__TestTB__bjsocket__) */
