//
//  CaptureFile.cpp
//  TestTB
//
//  Created by Terrin Eager on 9/14/12.
//
//

#include "CaptureFile.h"
#include <stdio.h>
#include <pcap.h>
#include <sys/types.h>

#define BJ_MAX_PACKET (1024*20)

struct packetheader
{
    __uint32_t sec;
    __uint32_t usec;
    __uint32_t captureLen;
    __uint32_t origLen;

};


CCaptureFile::CCaptureFile()
{
    m_pFileHeader = NULL;
    m_pFrameData = NULL;
    m_pFrameHeader = NULL;
    m_hFile = NULL;

    m_nFirstFrameTime = 0;

    if (!Init())
        Clear();
}
CCaptureFile::~CCaptureFile()
{
    Clear();
}

bool CCaptureFile::Init()
{
    m_pFileHeader = new BJ_UINT8[sizeof(pcap_file_header)];
    m_pFrameHeader = new BJ_UINT8[sizeof(packetheader)];
    m_pFrameData = new BJ_UINT8[BJ_MAX_PACKET];

    return (m_pFrameHeader && m_pFrameData && m_pFileHeader);
}

bool CCaptureFile::Clear()
{
    delete m_pFileHeader; m_pFileHeader = NULL;
    delete m_pFrameData; m_pFrameData = NULL;
    delete m_pFrameHeader; m_pFrameHeader = NULL;

    fclose(m_hFile); m_hFile = NULL;
    return true;
}

bool CCaptureFile::Open(const char* pFileName)
{
    m_hFile = fopen(pFileName, "r");

    if (!m_hFile)
    {
        printf("Failed to open %s\n",pFileName);
        return false;
    }


    fread(m_pFileHeader, sizeof(pcap_file_header), 1,m_hFile);

   //  pcap_file_header* pHeader = (pcap_file_header*)m_pFileHeader;
   // int magic = pHeader->magic;
   // int nType = pHeader->linktype;

    pcap_file_header* pHeader = (pcap_file_header*)m_pFileHeader;
    m_datalinkType = (Frame::BJ_DATALINKTYPE) pHeader->linktype;
    m_CurrentFrame.SetDatalinkType(m_datalinkType);
    return true;
}

bool CCaptureFile::NextFrame()
{
    packetheader* pFrameHeader = NULL;

    if(!m_hFile)
        return false;

    if (fread(m_pFrameHeader,1,sizeof(packetheader),m_hFile)< sizeof(packetheader))
        return false;

    pFrameHeader = (packetheader*) m_pFrameHeader;

    m_nWireLen = pFrameHeader->origLen;
    m_TimeSec = pFrameHeader->sec;
    if (m_nFirstFrameTime == 0)
        m_nFirstFrameTime = m_TimeSec;
    m_nCaptureLen = pFrameHeader->captureLen; // to do handle frames bigger than buffer

    long nSkip = 0;
    if (m_nCaptureLen >  BJ_MAX_PACKET)
    {   // force truncate the packet ...
        nSkip = m_nCaptureLen - BJ_MAX_PACKET;
        m_nCaptureLen = BJ_MAX_PACKET;
    }

    if (fread(m_pFrameData,1,m_nCaptureLen,m_hFile) < m_nCaptureLen)
        return false;

    if (nSkip)
        fseek(m_hFile, nSkip, SEEK_CUR);

    m_CurrentFrame.Set(m_pFrameData, m_nCaptureLen,pFrameHeader->sec*1000000ll + pFrameHeader->usec);


    return true;
}

bool CCaptureFile::Close()
{

    return true;
}

__uint32_t CCaptureFile::GetDeltaTime()
{
    return m_TimeSec-m_nFirstFrameTime;
}

__uint32_t CCaptureFile::GetBufferLen(BJ_UINT8* pStart)
{
    return m_nCaptureLen -  (__uint32_t) (pStart - m_pFrameData);
}





