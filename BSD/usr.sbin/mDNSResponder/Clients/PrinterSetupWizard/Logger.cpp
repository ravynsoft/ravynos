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

#include "stdafx.h"
#include "Logger.h"
#include "DebugServices.h"
#include <string>


Logger::Logger()
{
	std::string	tmp;
	char		path[ MAX_PATH ];
	HRESULT		err;
	BOOL		ok;

	err = SHGetFolderPathA( NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path );
	require_noerr( err, exit );

	tmp = path;

	// Create Logs subdir
	tmp += "\\Apple";
	ok = CreateDirectoryA( tmp.c_str(), NULL );
	require_action( ( ok || ( GetLastError() == ERROR_ALREADY_EXISTS ) ), exit, err = -1 );

	// Create Logs subdir
	tmp += "\\Bonjour";
	ok = CreateDirectoryA( tmp.c_str(), NULL );
	require_action( ( ok || ( GetLastError() == ERROR_ALREADY_EXISTS ) ), exit, err = -1 );

	// Create log file
	tmp += "\\PrinterSetupLog.txt";
	open( tmp.c_str());

	*this << currentTime() << " Log started" << std::endl;

exit:

	return;
}


Logger::~Logger()
{
	*this << currentTime() << " Log finished" << std::endl;
	flush();
}


std::string
Logger::currentTime()
{
	time_t					ltime;
	struct tm				now;
	int						err;
	std::string				ret;
	
	time( &ltime );
	err = localtime_s( &now, &ltime );

	if ( !err )
	{
		char temp[ 64 ];
		
		strftime( temp, sizeof( temp ), "%m/%d/%y %I:%M:%S %p", &now );
		ret = temp;
	}

	return ret;
}
