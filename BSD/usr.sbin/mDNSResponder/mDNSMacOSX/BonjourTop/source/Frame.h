//
//  Frame.h
//  TestTB
//
//  Created by Terrin Eager on 1/19/13.
//
//

#ifndef __TestTB__Frame__
#define __TestTB__Frame__

#include "bjtypes.h"
#include "bjIPAddr.h"
#include "bjMACAddr.h"

class Frame
{
public:
    void Set(BJ_UINT8* data,BJ_UINT32 len,BJ_UINT64 t);
    BJ_UINT8* GetEthernetStart();
    BJ_UINT8* GetIPStart();
    BJ_UINT8* GetUDPStart();
    BJ_UINT8* GetBonjourStart();

    BJIPAddr* GetSrcIPAddr();
    BJIPAddr* GetDestIPAddr();

    BJMACAddr* GetSrcMACAddr();
    BJMACAddr* GetDestMACAddr();

    int m_bCurrentFrameIPversion;

    BJ_UINT64 GetTime(){ return frameTime; };

    enum BJ_DATALINKTYPE {
        BJ_DLT_EN10MB = 1,
        BJ_DLT_IEEE802_11=105
    };

    void SetDatalinkType (BJ_DATALINKTYPE datalinkType);
private:

    BJ_UINT32 GetLinklayerHeaderLength();

    //Get the header length of the current 802.11 frame.
    BJ_UINT32 Get80211HeaderLength();

    BJ_UINT8* frameData;
    BJ_UINT32 length;

    BJIPAddr sourceIPAddr;
    BJIPAddr destIPAddr;

    BJMACAddr sourceMACAddr;
    BJMACAddr destMACAddr;

    BJ_UINT64 frameTime; // in microseconds


    BJ_DATALINKTYPE m_datalinkType = BJ_DLT_EN10MB;


};


#endif /* defined(__TestTB__Frame__) */
