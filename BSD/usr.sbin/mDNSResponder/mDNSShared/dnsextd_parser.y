/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2006-2010 Apple Inc. All rights reserved.
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

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mDNSEmbeddedAPI.h"
#include "DebugServices.h"
#include "dnsextd.h"

void yyerror( const char* error );
int  yylex(void);


typedef struct StringListElem
{
	char					*	string;
	struct StringListElem	*	next;
} StringListElem;


typedef struct OptionsInfo
{
	char	server_address[ 256 ];
	int		server_port;
	char	source_address[ 256 ];
	int		source_port;
	int		private_port;
	int		llq_port;
} OptionsInfo;


typedef struct ZoneInfo
{
	char	name[ 256 ];
	char	certificate_name[ 256 ];
	char	allow_clients_file[ 256 ];
	char	allow_clients[ 256 ];
	char	key[ 256 ];
} ZoneInfo;


typedef struct KeySpec
{
	char 				name[ 256 ];
	char				algorithm[ 256 ];
	char				secret[ 256 ];
	struct KeySpec	*	next;
} KeySpec;


typedef struct ZoneSpec
{
	char				name[ 256 ];
	DNSZoneSpecType		type;
	StringListElem	*	allowUpdate;
	StringListElem	*	allowQuery;
	char				key[ 256 ];
	struct ZoneSpec	*	next;
} ZoneSpec;


static StringListElem	*	g_stringList = NULL;
static StringListElem	*	g_addrList = NULL;
static KeySpec			*	g_keys;
static ZoneSpec			*	g_zones;
static ZoneSpec				g_zoneSpec;
static const char		*	g_filename;

#define YYPARSE_PARAM  context

void
SetupOptions
	(
	OptionsInfo	*	info,
	void		*	context
	);

%}

%union
{
	int			number;
	char	*	string;
}

%token	OPTIONS 
%token	LISTEN_ON 
%token	NAMESERVER
%token	PORT 
%token	ADDRESS 
%token	LLQ 
%token	PUBLIC
%token  PRIVATE
%token  ALLOWUPDATE
%token  ALLOWQUERY
%token	KEY 
%token  ALGORITHM
%token  SECRET
%token  ISSUER
%token  SERIAL
%token	ZONE
%token  TYPE
%token	ALLOW
%token	OBRACE 
%token	EBRACE 
%token	SEMICOLON
%token 	IN
%token	<string>	DOTTED_DECIMAL_ADDRESS 
%token	<string>	WILDCARD 
%token	<string>	DOMAINNAME 
%token	<string>	HOSTNAME 
%token	<string>	QUOTEDSTRING
%token	<number> 	NUMBER 

%type	<string>	addressstatement
%type	<string>	networkaddress

%%

commands:
        |        
        commands command SEMICOLON
        ;


command:
		options_set
		|
        zone_set 
		|
		key_set
        ;


options_set:
		OPTIONS optionscontent
		{
			// SetupOptions( &g_optionsInfo, context );
		}
		;

optionscontent:
		OBRACE optionsstatements EBRACE
		;

optionsstatements:
		|
		optionsstatements optionsstatement SEMICOLON
		;


optionsstatement:
		statements
		|
		LISTEN_ON addresscontent
		{
		}
		|
		LISTEN_ON PORT NUMBER addresscontent
		{
			mDNSIPPort listen_port = mDNSOpaque16fromIntVal( $3 );
			DaemonInfo* d = ( DaemonInfo* ) context;
			d->addr.sin_port = ( listen_port.NotAnInteger) ? listen_port.NotAnInteger : UnicastDNSPort.NotAnInteger;
			StringListElem* addr = g_addrList;
			while (addr != NULL)
			{
				StringListElem* next;
				// The first ipv4 address in {,} is used; the rest are ignored.
				if (inet_pton( AF_INET, addr->string, &d->addr.sin_addr ) == 0) {
					inet_pton( AF_INET, "127.0.0.1", &d->ns_addr.sin_addr );
					LogMsg("LISTEN_ON: An invalid ipv4 address, %s, detected.", addr->string);
				}
				next = addr->next;
				free(addr);
				addr = next;
			}
		}
		|
		NAMESERVER ADDRESS networkaddress
		{
		}
		|
		NAMESERVER ADDRESS networkaddress PORT NUMBER
		{
		}
		|
		PRIVATE PORT NUMBER
		{
			( ( DaemonInfo* ) context )->private_port = mDNSOpaque16fromIntVal( $3 );
		}
		|
		LLQ PORT NUMBER
		{
			( ( DaemonInfo* ) context )->llq_port = mDNSOpaque16fromIntVal( $3 );
		}
		;

key_set:
        KEY QUOTEDSTRING OBRACE SECRET QUOTEDSTRING SEMICOLON EBRACE
        {
			KeySpec	* keySpec;

			keySpec = ( KeySpec* ) malloc( sizeof( KeySpec ) );

			if ( !keySpec )
				{
				LogMsg("ERROR: memory allocation failure");
				YYABORT;
				}

			strncpy( keySpec->name, $2, sizeof( keySpec->name ) );
			strncpy( keySpec->secret, $5, sizeof( keySpec->secret ) );

			keySpec->next	= g_keys;
			g_keys			= keySpec;
        }
        ;

zone_set:
		ZONE QUOTEDSTRING zonecontent
		{
			ZoneSpec * zoneSpec;

			zoneSpec = ( ZoneSpec* ) malloc( sizeof( ZoneSpec ) );

			if ( !zoneSpec )
				{
				LogMsg("ERROR: memory allocation failure");
				YYABORT;
				}

			strncpy( zoneSpec->name, $2, sizeof( zoneSpec->name ) );
			zoneSpec->type = g_zoneSpec.type;
			strcpy( zoneSpec->key, g_zoneSpec.key );
			zoneSpec->allowUpdate = g_zoneSpec.allowUpdate;
			zoneSpec->allowQuery = g_zoneSpec.allowQuery;

			zoneSpec->next = g_zones;
			g_zones = zoneSpec;
		}
		|
		ZONE QUOTEDSTRING IN zonecontent
        {
			ZoneSpec * zoneSpec;

			zoneSpec = ( ZoneSpec* ) malloc( sizeof( ZoneSpec ) );

			if ( !zoneSpec )
				{
				LogMsg("ERROR: memory allocation failure");
				YYABORT;
				}

			strncpy( zoneSpec->name, $2, sizeof( zoneSpec->name ) );
			zoneSpec->type = g_zoneSpec.type;
			strcpy( zoneSpec->key, g_zoneSpec.key );
			zoneSpec->allowUpdate = g_zoneSpec.allowUpdate;
			zoneSpec->allowQuery = g_zoneSpec.allowQuery;

			zoneSpec->next = g_zones;
			g_zones = zoneSpec;
		}
        ;

zonecontent:
		OBRACE zonestatements EBRACE 

zonestatements:
        |
        zonestatements zonestatement SEMICOLON
        ;

zonestatement:
		TYPE PUBLIC
		{
			g_zoneSpec.type = kDNSZonePublic;
		}
		|
		TYPE PRIVATE
		{
			g_zoneSpec.type = kDNSZonePrivate;
		}
		|
		ALLOWUPDATE keycontent
		{
			g_zoneSpec.allowUpdate = g_stringList;
			g_stringList = NULL;
		}
		|
		ALLOWQUERY keycontent
		{
			g_zoneSpec.allowQuery = g_stringList;
			g_stringList = NULL;
		}
        ;

addresscontent:
		OBRACE addressstatements EBRACE
		{
		}

addressstatements:
		|
		addressstatements addressstatement SEMICOLON
		{
		}
		;

addressstatement:
		DOTTED_DECIMAL_ADDRESS
		{
			StringListElem * elem;

			elem = ( StringListElem* ) malloc( sizeof( StringListElem ) );

			if ( !elem )
			{
				LogMsg("ERROR: memory allocation failure");
				YYABORT;
			}

			elem->string = $1;

			elem->next		= g_addrList;
			g_addrList		= elem;
		}
		;


keycontent:
		OBRACE keystatements EBRACE
		{
		}

keystatements:
		|
		keystatements keystatement SEMICOLON
		{
		}
		;

keystatement:
		KEY DOMAINNAME
		{
			StringListElem * elem;

			elem = ( StringListElem* ) malloc( sizeof( StringListElem ) );

			if ( !elem )
				{
				LogMsg("ERROR: memory allocation failure");
				YYABORT;
				}

			elem->string = $2;

			elem->next		= g_stringList;
			g_stringList	= elem;
		}
		;


networkaddress:
		DOTTED_DECIMAL_ADDRESS
		|
		HOSTNAME
		|
		WILDCARD
		;

block: 
		OBRACE zonestatements EBRACE SEMICOLON
        ;

statements:
        |
		statements statement
        ;

statement:
		block
		{
			$<string>$ = NULL;
		}
		|
		QUOTEDSTRING
		{
			$<string>$ = $1;
		}
%%

int yywrap(void);

extern int yylineno;

void yyerror( const char *str )
{
        fprintf( stderr,"%s:%d: error: %s\n", g_filename, yylineno, str );
}
 
int yywrap()
{
        return 1;
} 


int
ParseConfig
	(
	DaemonInfo	*	d,
	const char	*	file
	)
	{
	extern FILE		*	yyin;
	DNSZone			*	zone;
	DomainAuthInfo	*	key;
	KeySpec			*	keySpec;
	ZoneSpec		*	zoneSpec;
	int					err = 0;

	g_filename = file;

	// Tear down the current zone specifiers

	zone = d->zones;

	while ( zone )
		{
		DNSZone * next = zone->next;

		key = zone->updateKeys;

		while ( key )
			{
			DomainAuthInfo * nextKey = key->next;

			free( key );

			key = nextKey;
			}

		key = zone->queryKeys;

		while ( key )
			{
			DomainAuthInfo * nextKey = key->next;

			free( key );

			key = nextKey;
			}

		free( zone );

		zone = next;
		}

	d->zones = NULL;
	
	yyin = fopen( file, "r" );
	require_action( yyin, exit, err = 0 );

	err = yyparse( ( void* ) d );
	require_action( !err, exit, err = 1 );

	for ( zoneSpec = g_zones; zoneSpec; zoneSpec = zoneSpec->next )
		{
		StringListElem  *   elem;
		mDNSu8			*	ok;

		zone = ( DNSZone* ) malloc( sizeof( DNSZone ) );
		require_action( zone, exit, err = 1 );
		memset( zone, 0, sizeof( DNSZone ) );

		zone->next	= d->zones;
		d->zones	= zone;

		// Fill in the domainname

		ok = MakeDomainNameFromDNSNameString( &zone->name, zoneSpec->name );
		require_action( ok, exit, err = 1 );

		// Fill in the type

		zone->type = zoneSpec->type;

		// Fill in the allow-update keys

		for ( elem = zoneSpec->allowUpdate; elem; elem = elem->next )
			{
			mDNSBool found = mDNSfalse;

			for ( keySpec = g_keys; keySpec; keySpec = keySpec->next )
				{
				if ( strcmp( elem->string, keySpec->name ) == 0 )
					{
					DomainAuthInfo	*	authInfo = malloc( sizeof( DomainAuthInfo ) );
					mDNSs32				keylen;
					require_action( authInfo, exit, err = 1 );
					memset( authInfo, 0, sizeof( DomainAuthInfo ) );

					ok = MakeDomainNameFromDNSNameString( &authInfo->keyname, keySpec->name );
					if (!ok) { free(authInfo); err = 1; goto exit; }

					keylen = DNSDigest_ConstructHMACKeyfromBase64( authInfo, keySpec->secret );
					if (keylen < 0) { free(authInfo); err = 1; goto exit; }

					authInfo->next = zone->updateKeys;
					zone->updateKeys = authInfo;

					found = mDNStrue;

					break;
					}
				}

			// Log this
			require_action( found, exit, err = 1 );
			}

		// Fill in the allow-query keys

		for ( elem = zoneSpec->allowQuery; elem; elem = elem->next )
			{
			mDNSBool found = mDNSfalse;

			for ( keySpec = g_keys; keySpec; keySpec = keySpec->next )
				{
				if ( strcmp( elem->string, keySpec->name ) == 0 )
					{
					DomainAuthInfo	*	authInfo = malloc( sizeof( DomainAuthInfo ) );
					mDNSs32				keylen;
					require_action( authInfo, exit, err = 1 );
					memset( authInfo, 0, sizeof( DomainAuthInfo ) );

					ok = MakeDomainNameFromDNSNameString( &authInfo->keyname, keySpec->name );
					if (!ok) { free(authInfo); err = 1; goto exit; }

					keylen = DNSDigest_ConstructHMACKeyfromBase64( authInfo, keySpec->secret );
					if (keylen < 0) { free(authInfo); err = 1; goto exit; }

					authInfo->next = zone->queryKeys;
					zone->queryKeys = authInfo;

					found = mDNStrue;

					break;
					}
				}

			// Log this
			require_action( found, exit, err = 1 );
			}
		}

exit:

	return err;
	}


void
SetupOptions
	(
	OptionsInfo	*	info,
	void		*	context
	)
	{
	DaemonInfo * d = ( DaemonInfo* ) context;

	if ( strlen( info->source_address ) )
		{
		inet_pton( AF_INET, info->source_address, &d->addr.sin_addr );
		}

	if ( info->source_port )
		{
		d->addr.sin_port = htons( ( mDNSu16 ) info->source_port );
		}
				
	if ( strlen( info->server_address ) )
		{
		inet_pton( AF_INET, info->server_address, &d->ns_addr.sin_addr );
		}

	if ( info->server_port )
		{
		d->ns_addr.sin_port = htons( ( mDNSu16 ) info->server_port );
		}

	if ( info->private_port )
		{
		d->private_port = mDNSOpaque16fromIntVal( info->private_port );
		}

	if ( info->llq_port )
		{
		d->llq_port = mDNSOpaque16fromIntVal( info->llq_port );
		}
	}
