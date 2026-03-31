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

#ifndef _Logger_h
#define _Logger_h

#include <fstream>
#include <string>


class Logger : public std::ofstream
{
public:

Logger();
~Logger();

std::string
currentTime();
};


#define require_noerr_with_log( LOG, MESSAGE, ERR, LABEL )  \
    do                      \
    {                       \
        int_least32_t localErr;         \
        localErr = (int_least32_t)( ERR );  \
        if( localErr != 0 )             \
        {                   \
            log << log.currentTime() << " [ERROR] " << MESSAGE << " returned " << ERR << std::endl;                     \
            log << log.currentTime() << " [WHERE] " << "\"" << __FILE__ << "\", \"" << __FUNCTION__ << "\", line " << __LINE__ << std::endl << std::endl;   \
            goto LABEL;         \
        }                   \
    } while( 0 )


#define require_action_with_log( LOG, X, LABEL, ACTION )    \
    do                      \
    {                       \
        if( !( X ) )                \
        {                   \
            log << log.currentTime() << " [ERROR] " << # X << std::endl; \
            log << log.currentTime() << " [WHERE] " << "\"" << __FILE__ << "\", \"" << __FUNCTION__ << "\", line " << __LINE__ << std::endl << std::endl;   \
            { ACTION; }         \
            goto LABEL;         \
        }                   \
    } while( 0 )

#endif
