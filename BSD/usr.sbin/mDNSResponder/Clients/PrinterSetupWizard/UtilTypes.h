/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 1997-2004 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <dns_sd.h>
#include <string>
#include <list>
#include <DebugServices.h>

class CPrinterSetupWizardSheet;

#define kDefaultPriority    50
#define kDefaultQTotal      1

namespace PrinterSetupWizard
{
struct Printer;
struct Service;
struct Queue;
struct Manufacturer;
struct Model;

typedef std::list<Queue*>   Queues;
typedef std::list<Printer*> Printers;
typedef std::list<Service*> Services;
typedef std::list<Model*>   Models;

struct Printer
{
    Printer();

    ~Printer();

    Service*
    LookupService
    (
        const std::string   &   type
    );

    CPrinterSetupWizardSheet    *   window;
    HTREEITEM item;

    //
    // These are from the browse reply
    //
    std::string name;
    CString displayName;
    CString actualName;

    //
    // These keep track of the different services associated with this printer.
    // the services are ordered according to preference.
    //
    Services services;

    //
    // these are derived from the printer matching code
    //
    // if driverInstalled is false, then infFileName should
    // have an absolute path to the printers inf file.  this
    // is used to install the printer from printui.dll
    //
    // if driverInstalled is true, then model is the name
    // of the driver to use in AddPrinter
    //
    bool driverInstalled;
    CString infFileName;
    CString manufacturer;
    CString displayModelName;
    CString modelName;
    CString portName;
    bool deflt;

    // This let's us know that this printer was discovered via OSX Printer Sharing.
    // We use this knowledge to workaround a problem with OS X Printer sharing.

    bool isCUPSPrinter;

    //
    // state
    //
    unsigned resolving;
    bool installed;
};


struct Service
{
    Service();

    ~Service();

    Queue*
    SelectedQueue();

    void
    EmptyQueues();

    Printer     *   printer;
    uint32_t ifi;
    std::string type;
    std::string domain;

    //
    // these are from the resolve
    //
    DNSServiceRef serviceRef;
    CString hostname;
    unsigned short portNumber;
    CString protocol;
    unsigned short qtotal;

    //
    // There will usually one be one of these, however
    // this will handle printers that have multiple
    // queues.  These are ordered according to preference.
    //
    Queues queues;

    //
    // Reference count
    //
    unsigned refs;
};


struct Queue
{
    Queue();

    ~Queue();

    CString name;
    uint32_t priority;
    CString pdl;
    CString usb_MFG;
    CString usb_MDL;
    CString description;
    CString location;
    CString product;
};


struct Manufacturer
{
    CString name;
    Models models;

    Model*
    find( const CString & name );
};


struct Model
{
    bool driverInstalled;
    CString infFileName;
    CString displayName;
    CString name;
};


inline
Printer::Printer()
    :
    isCUPSPrinter( false )
{
}

inline
Printer::~Printer()
{
    while ( services.size() > 0 )
    {
        Service * service = services.front();
        services.pop_front();
        delete service;
    }
}

inline Service*
Printer::LookupService
(
    const std::string   &   type
)
{
    Services::iterator it;

    for ( it = services.begin(); it != services.end(); it++ )
    {
        Service * service = *it;

        if ( strcmp(service->type.c_str(), type.c_str()) == 0 )
        {
            return service;
        }
    }

    return NULL;
}

inline
Service::Service()
    :
    qtotal(kDefaultQTotal)
{
}

inline
Service::~Service()
{
    check( serviceRef == NULL );

    EmptyQueues();
}

inline Queue*
Service::SelectedQueue()
{
    return queues.front();
}

inline void
Service::EmptyQueues()
{
    while ( queues.size() > 0 )
    {
        Queue * q = queues.front();
        queues.pop_front();
        delete q;
    }
}

inline
Queue::Queue()
    :
    priority(kDefaultPriority)
{
}

inline
Queue::~Queue()
{
}

inline Model*
Manufacturer::find( const CString & name )
{
    Models::iterator it;

    for ( it = models.begin(); it != models.end(); it++ )
    {
        Model * model = *it;

        if ( model->name == name )
        {
            return model;
        }
    }

    return NULL;
}
}


