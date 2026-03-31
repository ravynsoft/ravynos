//
//  main.cpp
//  BonjourTop
//
//  Created by Terrin Eager on 4/24/13.
//  Copyright (c) 2013-2014 Apple Inc. All rights reserved.
//

#include <stdio.h>
#include <curses.h>

#include "bjtypes.h"
#include "BonjourTop.h"

#include <signal.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>


#define BJ_VERSION_MAJOR 0
#define BJ_VERSION_MINOR 23
CBonjourTop BjTop;

static void
usage()
{
    printf("bonjourtop usage: bonjourTop (Version: %d.%d)\n",BJ_VERSION_MAJOR,BJ_VERSION_MINOR);
    printf("\t\t\t [-t tcptrace_filename ]\n");
    printf("\t\t\t [-i interfaceName]\n");
    printf("\t\t\t [-m ipaddress/subnetmask]  ie 17.255.45.12/17\n");
    printf("\t\t\t [-e export_filename]  \n");
    printf("\t\t\t [-x seconds]  'Snapshot export every x seconds'\n");
    printf("\t\t\t [-s] 'service information'\n");
    printf("\t\t\t [-v] 'report the version number'  \n");
    printf("\t\t\t [-d] filename 'export device map. Adds timestamp and csv extension to the filename'  \n");
    printf("\t\t\t [-f application] 'filter application for device map (only available with -t -d options)'  \n");
    printf("While running the follow keys may be used:\n");
    printf("\t b - sort by Bytes\n");
    printf("\t p - sort by Packets (default)\n");
    printf("\t n - sort by Name\n");
    printf("\t a - Display Application Names (default) \n");
    printf("\t s - Display Services Names  \n");
    printf("\t t - Display 24 hour packet per min  \n");

    printf("\t o - flip sort order\n");
    printf("\t e - export to BonjourTop.csv\n");
    printf("\t q - quit\n\n");
}

static void
handle_window_change(int signal) {
    switch (signal) {
        case SIGWINCH:
            BjTop.WindowSizeChanged();
            break;
        default:
            break;
    }
}

int main(int argc, char * const *argv)
{

    sigset_t sset, oldsset;
    int c;

    static struct option longopts[] = {
        {   "trace", required_argument, NULL, 't' },
        {   "interface", required_argument, NULL, 'i' },
        {   "ipaddr_subnet", required_argument, NULL, 'm' },
        {   "export", required_argument, NULL, 'e' },
        {   "snapshot", required_argument, NULL, 'x' },
        {   "service", no_argument, NULL, 's' },
        {   "version", no_argument, NULL, 'v' },
        {   "devicemap", required_argument, NULL, 'd' },
        {   "filter", required_argument, NULL, 'f' },
        {   NULL, 0, NULL, 0 }
    };

	sigemptyset(&sset);

    /* Block SIGWINCH signals while we are in a relayout. */
    if(-1 == sigprocmask(SIG_BLOCK, &sset, &oldsset)) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    BJ_COLLECTBY_TYPE TypeList[] = {CBT_SERVICE,CBT_REQUEST_RESPONDS,CBT_SAME_DIFF_SUBNET,CBT_IP_ADDRESS_TYPE,CBT_PACKET};
    BJString sTemp;

    bool bLiveCapture = true;
    bool bExport = false;

    while ((c = getopt_long(argc, argv, "t:i:m:e:x:svd:f:phb", longopts, NULL)) != -1) {
        switch (c) {
            case 't':
                BjTop.m_pTcpDumpFileName = optarg; // TCP Dump Filename
                bLiveCapture = false;
                BjTop.m_bCursers = false;
                break;
            case 'p':
                BjTop.m_bCursers = false;
                break;
            case 'e':
                bExport = true;
                BjTop.m_pExportFileName = optarg;   // Export filename
                break;
            case 'i':
                BjTop.interfaceName = optarg;       // Interface name
                break;
            case 'm':
                BjTop.SetIPAddr(optarg);            // TODO: verify that the argument is an ip address
                break;
            case 'x':
                sTemp = optarg;                     // time in seconds for snapshots
                BjTop.m_SnapshotSeconds = sTemp.GetUINT32();
                break;
            case 'd':
                BjTop.m_bImportExportDeviceMap = true;
                BjTop.m_DeviceFileName = optarg;
                break;
            case 'f':
                BjTop.filterApplicationName = optarg;
                break;
            case 's':
                BjTop.m_CurrentDisplay = CBonjourTop::BJ_DISPLAY_SERVICE;
                break;
            case 'v':
                printf("\nbonjourtop Version: %d.%d\n\n",BJ_VERSION_MAJOR,BJ_VERSION_MINOR);
                exit(0);
                break;
            case 'b':
                BjTop.m_Collection.Init(TypeList);
                bExport = true;
                break;
            case 'h':
                usage();
                exit(0);
                break;
            default:
                usage();
                break;
        }
    }

    if (BjTop.m_bCursers)
    {
        signal(SIGWINCH, handle_window_change);
        initscr();
        timeout(0);
        BjTop.PrintResults(1,false);
    }

    if (bLiveCapture)
        BjTop.LiveCapture();
    else
        BjTop.CaptureFile();


    if (bExport)
    {
        BjTop.ExportResults();
        return 0;
    }

    if (BjTop.m_bCursers)
        endwin();

    if (BjTop.m_bImportExportDeviceMap)
    {
        BjTop.WriteDeviceFile();
        BjTop.WriteVendorFile();
    }

    return 0;
}


