/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2006-2011 Apple Inc. All rights reserved.
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
#include <string.h>
#include <stdio.h>
#include "dnsextd_parser.h"


extern YYSTYPE yylval;

#define YY_NO_INPUT 1
int  yylex(void);

static char*
StripQuotes
	(
	const char * string
	)
{
	char * dup;

	dup = strdup( string + 1);

	dup[ strlen( dup ) - 1 ] = '\0';

	return dup;
}


%}

%option nounput
%%

options								return OPTIONS;
listen-on							return LISTEN_ON;
nameserver							return NAMESERVER;
port								return PORT;
address								return ADDRESS;
llq									return LLQ;
public								return PUBLIC;
private								return PRIVATE;
key									return KEY;
allow-update						return ALLOWUPDATE;
allow-query							return ALLOWQUERY;
algorithm							return ALGORITHM;
secret								return SECRET;
zone                    			return ZONE;
type                    			return TYPE;
allow								return ALLOW;
\{                      			return OBRACE;
\}                      			return EBRACE;
;                       			return SEMICOLON;
IN									return IN;
\*									yylval.string = strdup(yytext);	return WILDCARD;
[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+		yylval.string = strdup(yytext);	return DOTTED_DECIMAL_ADDRESS;
[0123456789]+						yylval.number = atoi(yytext);	return NUMBER;
[a-zA-Z0-9]+(\.[a-zA-Z0-9]+)*		yylval.string = strdup(yytext);	return HOSTNAME;
[a-zA-Z0-9\.]+([a-zA-Z0-9\.]+)*		yylval.string = strdup(yytext);	return DOMAINNAME;
\"([^"\\\r\n]*(\\.[^"\\\r\n]*)*)\"	yylval.string = StripQuotes(yytext);	return QUOTEDSTRING;
[\/][\/].*							/* ignore C++ style comments */;
\n                      			yylineno++; /* ignore EOL */;
[ \t]+                  			/* ignore whitespace */;
%%
