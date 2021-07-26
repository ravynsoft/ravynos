/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSPredicate.h>
#import <Foundation/NSCompoundPredicate.h>
#import <Foundation/NSComparisonPredicate.h>
#import <Foundation/NSExpression.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSNull.h>
#import <Foundation/NSRaise.h>
#import "NSPredicate_BOOL.h"
#import "NSExpression_operator.h"
#import "NSExpression_array.h"
#import "NSExpression_assignment.h"
#include <math.h>

#define LF 10
#define FF 12
#define CR 13

// FIX, need to add a selector token type for function calls which can be selector or identifier

enum {
 predTokenEOF=-1,

 predTokenLeftParen='(',
 predTokenRightParen=')',
 predTokenLeftBracket='[',
 predTokenRightBracket=']',
 predTokenLeftBrace='{',
 predTokenRightBrace='}',

 predTokenLessThan='<',
 predTokenGreaterThan='>',


 predTokenEqual='=',
 predTokenPercent='%',
 predTokenDollar='$',
 predTokenAtSign='@',
 predTokenPeriod='.',
 predTokenComma=',',
 predTokenPlus='+',
 predTokenMinus='-',
 predTokenAsterisk='*',
 predTokenSlash='/',
 predTokenExclamation='!',

 predToken_AND=128,
 predToken_OR,
 predToken_IN,
 predToken_NOT,
 predToken_ALL,
 predToken_ANY,
 predToken_NONE,
 predToken_LIKE,
 predToken_CASEINSENSITIVE,
 predToken_CI,
 predToken_MATCHES,
 predToken_CONTAINS,
 predToken_BEGINSWITH,
 predToken_ENDSWITH,
 predToken_BETWEEN,
 predToken_NULL,
 predToken_SELF,
 predToken_TRUE,
 predToken_FALSE,
 predToken_FIRST,
 predToken_LAST,
 predToken_SIZE,
 predToken_ANYKEY,
 predToken_SUBQUERY,
 predToken_CAST,
 predToken_TRUEPREDICATE,
 predToken_FALSEPREDICATE,

 predTokenIdentifier,
 predTokenString,
 predTokenReservedWord,
 predTokenNumeric,

 predTokenNotEqual,
 predTokenLessThanOrEqual,
 predTokenGreaterThanOrEqual,
 predTokenColonEqual,
 predTokenAsteriskAsterisk,
};

typedef struct {
   NSString *original;
   unichar *unicode;
   NSInteger      length;
   NSInteger      position;
   NSInteger      nextArgument;
   union {
    va_list  arguments;
    NSArray *argumentArray;
   };
} predicateScanner;

static void raiseError(predicateScanner *scanner,NSString *format,...){
   NSString *reason;
   va_list   arguments;

   va_start(arguments,format);

   reason=[[[NSString alloc] initWithFormat:format arguments:arguments] autorelease];
   va_end(arguments);

   [NSException raise:NSInvalidArgumentException format:@"Unable to parse the format string \"%@\", reason = %@",scanner->original,reason];
}

static int classifyToken(NSString *token){
   struct {
    NSString *name;
    int       type;
   } table[]={
    { @"AND", predToken_AND },
    { @"OR", predToken_OR },
    { @"IN", predToken_IN },
    { @"NOT", predToken_NOT },
    { @"ALL", predToken_ALL },
    { @"ANY", predToken_ANY },
    { @"SOME", predToken_ANY },
    { @"NONE", predToken_NONE },
    { @"LIKE", predToken_LIKE },
    { @"CASEINSENSITIVE", predToken_CASEINSENSITIVE },
    { @"CI", predToken_CI },
    { @"MATCHES", predToken_MATCHES },
    { @"CONTAINS", predToken_CONTAINS },
    { @"BEGINSWITH", predToken_BEGINSWITH },
    { @"ENDSWITH", predToken_ENDSWITH },
    { @"BETWEEN", predToken_BETWEEN },
    { @"NULL", predToken_NULL },
    { @"NIL", predToken_NULL },
    { @"SELF", predToken_SELF },
    { @"TRUE", predToken_TRUE },
    { @"YES", predToken_TRUE },
    { @"FALSE", predToken_FALSE },
    { @"NO", predToken_FALSE },
    { @"FIRST", predToken_FIRST },
    { @"LAST", predToken_LAST },
    { @"SIZE", predToken_SIZE },
    { @"ANYKEY", predToken_ANYKEY },
    { @"SUBQUERY", predToken_SUBQUERY },
    { @"CAST", predToken_CAST },
    { @"TRUEPREDICATE", predToken_TRUEPREDICATE },
    { @"FALSEPREDICATE", predToken_FALSEPREDICATE },
    { nil,0 }
   };
   int i;

   token=[token uppercaseString];

   for(i=0;table[i].name!=nil;i++)
    if([table[i].name isEqualToString:token])
     return table[i].type;

   return predTokenIdentifier;
}

/*
 BNF mentions octal, hex and unicode escapes for identifiers(??) doesn't appear to be present
 */

static BOOL codeIsHex(unichar code,unichar *hexChar) {
   if(code>='0' && code<='9'){
    *hexChar=*hexChar*16+(code-'0');
    return YES;
   }
   else if(code>='a' && code<='f'){
    *hexChar=*hexChar*16+(10+code-'a');
    return YES;
   }
   else if(code>='A' && code<='F'){
    *hexChar=*hexChar*16+(10+code-'A');
    return YES;
   }
   return NO;
}

static int scanToken(predicateScanner *scanner,id *token){
   int   currentSign=1,currentInt=0;
   double currentReal=0,currentFraction=0,exponentSign=1,currentExponent=0;
   BOOL  identifyReservedWords=YES;
   NSInteger   tokenLocation=0;
   NSMutableString *buffer=nil;
   unichar hexChar=0;
   auto enum {
    STATE_SCANNING,
    STATE_IDENTIFIER,
    STATE_ESCAPED_IDENTIFIER,
    STATE_ZERO,
    STATE_HEX,
    STATE_INTEGER,
    STATE_REAL,
    STATE_EXPONENT,
    STATE_HEX_SEQUENCE,
    STATE_OCTAL_SEQUENCE,
    STATE_BINARY_SEQUENCE,

    STATE_STRING_DOUBLE,
    STATE_STRING_DOUBLE_BUFFERED,
    STATE_STRING_DOUBLE_ESCAPE,
    STATE_STRING_DOUBLE_HEX,
    STATE_STRING_DOUBLE_NIBBLE,

    STATE_STRING_SINGLE,
    STATE_STRING_SINGLE_BUFFERED,
    STATE_STRING_SINGLE_ESCAPE,
    STATE_STRING_SINGLE_HEX,
    STATE_STRING_SINGLE_NIBBLE,

    STATE_EQUALS,
    STATE_EXCLAMATION,

    STATE_LESSTHAN,
    STATE_GREATERTHAN,
    STATE_COLON,
    STATE_AMPERSAND,
    STATE_BAR,
    STATE_ASTERISK,
   } state=STATE_SCANNING;

   *token=nil;

   for(;scanner->position<=scanner->length;scanner->position++){
    unichar code=(scanner->position<scanner->length)?scanner->unicode[scanner->position]:0xFFFF;

    switch(state){

	 case STATE_SCANNING:
      switch(code){

	   case ' ':
	   case  CR:
	   case  FF:
	   case  LF:
	   case '\t':
	    break;
// clang-format off
       case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I':
       case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
       case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
       case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i':
       case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
       case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
       case '_':
// clang-format on
        state=STATE_IDENTIFIER;
        tokenLocation=scanner->position;
        break;

       case '0':
        state=STATE_ZERO;
        break;

// clang-format off
       case '1': case '2': case '3': case '4':
       case '5': case '6': case '7': case '8': case '9':
// clang-format on
        state=STATE_INTEGER;
	    currentSign=1;
	    currentInt=code-'0';
        break;

       case '(':
       case ')':
       case '[':
       case ']':
       case '{':
       case '}':
       case '%':
       case '$':
       case '@':
       case '.':
       case '+':
       case '-':
       case '/':
        scanner->position++;
        return code;

       case '=':
        state=STATE_EQUALS;
        break;

       case '!':
        state=STATE_EXCLAMATION;
        break;

       case '<':
        state=STATE_LESSTHAN;
        break;

       case '>':
        state=STATE_GREATERTHAN;
        break;

       case ':':
        state=STATE_COLON;
        break;

       case '&':
        state=STATE_AMPERSAND;
        break;

       case '|':
        state=STATE_BAR;
        break;

       case '*':
        state=STATE_ASTERISK;
        break;

       case '#':
        state=STATE_ESCAPED_IDENTIFIER;
        break;

       case '\"':
        state=STATE_STRING_DOUBLE;
        tokenLocation=scanner->position+1;
        break;

       case '\'':
        state=STATE_STRING_SINGLE;
        tokenLocation=scanner->position+1;
        break;
      }
      break;

     case STATE_IDENTIFIER:
      switch(code){
// clang-format off
       case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I':
       case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
       case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
       case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i':
       case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
       case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
       case '_':
       case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
// clang-format on
        state=STATE_IDENTIFIER;
        break;

       default:
        *token=[NSString stringWithCharacters:scanner->unicode+tokenLocation length:(scanner->position-tokenLocation)];
        return identifyReservedWords?classifyToken(*token):predTokenIdentifier;
      }
      break;

     case STATE_ESCAPED_IDENTIFIER:
      switch(code){
       case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I':
       case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
       case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
       case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i':
       case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
       case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
       case '_':
        state=STATE_IDENTIFIER;
        identifyReservedWords=NO;
        tokenLocation=scanner->position;
        break;

       default:
        raiseError(scanner,@"Expecting identifier after #");
        break;
      }
      break;

     case STATE_ZERO:
      if(code=='x'){
       state=STATE_HEX;
	   currentInt=0;
      }
      else {
       scanner->position--;
       state=STATE_INTEGER;
	   currentSign=1;
	   currentInt=0;
      }
      break;

     case STATE_HEX:
      if(code>='0' && code<='9')
       currentInt=currentInt*16+(code-'0');
      else if(code>='A' && code<='F')
       currentInt=currentInt*16+(10+code-'A');
      else if(code>='a' && code<='a')
       currentInt=currentInt*16+(10+code-'a');
      else {
       *token=[NSNumber numberWithInt:currentInt];
       return predTokenNumeric;
      }
      break;

     case STATE_INTEGER:
      if(code=='.'){
       state=STATE_REAL;
       currentReal=currentInt;
       currentFraction=0.1;
      }
      else if(code=='e' || code=='E')
       state=STATE_EXPONENT;
      else if(code>='0' && code<='9')
       currentInt=currentInt*10+code-'0';
      else if(code=='x')
       state=STATE_HEX_SEQUENCE;
      else if(code=='o')
       state=STATE_OCTAL_SEQUENCE;
      else if(code=='b')
       state=STATE_BINARY_SEQUENCE;
      else {
       *token=[NSNumber numberWithInt:currentSign*currentInt];
       return predTokenNumeric;
      }
      break;

     case STATE_REAL:
      if(code>='0' && code<='9'){
       currentReal+=currentFraction*(code-'0');
       currentFraction*=0.1;
      }
      else if(code=='e' || code=='E'){
       state=STATE_EXPONENT;
      }
      else {
       *token=[NSNumber numberWithDouble:currentSign*currentReal];
       return predTokenNumeric;
      }
      break;

     case STATE_EXPONENT:
      if(code=='+')
       break;
      if(code=='-')
       exponentSign=-1;
      else if(code>='0' && code<='9')
       currentExponent=currentExponent*10+(code-'0');
      else {
       *token=[NSNumber numberWithDouble:currentSign*currentReal*pow(10,exponentSign*currentExponent)];
       return predTokenNumeric;
      }
      break;

     case STATE_HEX_SEQUENCE:
      NSUnimplementedFunction();
      break;

     case STATE_OCTAL_SEQUENCE:
      NSUnimplementedFunction();
      break;

     case STATE_BINARY_SEQUENCE:
      NSUnimplementedFunction();
      break;

     case STATE_STRING_DOUBLE:
      if(code=='\\'){
       state=STATE_STRING_DOUBLE_ESCAPE;
       buffer=[NSMutableString stringWithCharacters:scanner->unicode+tokenLocation length:(scanner->position-tokenLocation)];
      }
      else if(code=='\"'){
       *token=[NSString stringWithCharacters:scanner->unicode+tokenLocation length:(scanner->position-tokenLocation)];
       scanner->position++;
       return predTokenString;
      }
      break;

     case STATE_STRING_DOUBLE_BUFFERED:
      if(code=='\\')
       state=STATE_STRING_DOUBLE_ESCAPE;
      else if(code=='\"'){
       *token=buffer;
       scanner->position++;
       return predTokenString;
      }
      else
       [buffer appendFormat:@"%C",code];
      break;

     case STATE_STRING_DOUBLE_ESCAPE:
      if(code=='\"'){
       [buffer appendFormat:@"%C",code];
       state=STATE_STRING_DOUBLE_BUFFERED;
      }
      else if(code=='x' || code=='X'){
       state=STATE_STRING_DOUBLE_HEX;
       hexChar=0;
      }
      break;

     case STATE_STRING_DOUBLE_HEX:
      if(codeIsHex(code,&hexChar))
       state=STATE_STRING_DOUBLE_NIBBLE;
      else {
       scanner->position--;
       [buffer appendFormat:@"x"];
       state=STATE_STRING_DOUBLE_BUFFERED;
      }
      break;

     case STATE_STRING_DOUBLE_NIBBLE:
      if(codeIsHex(code,&hexChar)){
       [buffer appendFormat:@"%C",hexChar];
       state=STATE_STRING_DOUBLE_BUFFERED;
      }
      else {
       scanner->position--;
       [buffer appendFormat:@"x%C",scanner->unicode[scanner->position]];
       state=STATE_STRING_DOUBLE_BUFFERED;
      }
      break;

     case STATE_STRING_SINGLE:
      if(code=='\\'){
       state=STATE_STRING_SINGLE_ESCAPE;
       buffer=[NSMutableString stringWithCharacters:scanner->unicode+tokenLocation length:(scanner->position-tokenLocation)];
      }
      else if(code=='\''){
       *token=[NSString stringWithCharacters:scanner->unicode+tokenLocation length:(scanner->position-tokenLocation)];
       scanner->position++;
       return predTokenString;
      }
      break;

     case STATE_STRING_SINGLE_BUFFERED:
      if(code=='\\')
       state=STATE_STRING_SINGLE_ESCAPE;
      else if(code=='\''){
       *token=buffer;
       scanner->position++;
       return predTokenString;
      }
      else
       [buffer appendFormat:@"%C",code];
      break;

     case STATE_STRING_SINGLE_ESCAPE:
      if(code=='\"'){
       [buffer appendFormat:@"%C",code];
       state=STATE_STRING_SINGLE_BUFFERED;
      }
      else if(code=='x' || code=='X'){
       state=STATE_STRING_SINGLE_HEX;
       hexChar=0;
      }
      break;

     case STATE_STRING_SINGLE_HEX:
      if(codeIsHex(code,&hexChar))
       state=STATE_STRING_SINGLE_NIBBLE;
      else {
       [buffer appendFormat:@"x"];
       scanner->position--;
       state=STATE_STRING_SINGLE_BUFFERED;
      }
      break;

     case STATE_STRING_SINGLE_NIBBLE:
      if(codeIsHex(code,&hexChar)){
       [buffer appendFormat:@"%C",hexChar];
       state=STATE_STRING_SINGLE_BUFFERED;
      }
      else {
       scanner->position--;
       [buffer appendFormat:@"x%C",scanner->unicode[scanner->position]];
       state=STATE_STRING_SINGLE_BUFFERED;
      }
      break;

     case STATE_EQUALS:
      if(code=='='){
       scanner->position++;
       return  predTokenEqual;
      }
      if(code=='<'){
       scanner->position++;
       return  predTokenGreaterThanOrEqual;
      }
      if(code=='>'){
       scanner->position++;
       return  predTokenLessThanOrEqual;
      }
      return predTokenEqual;

     case STATE_EXCLAMATION:
      if(code=='='){
       scanner->position++;
       return  predTokenNotEqual;
      }
      return predTokenExclamation;

     case STATE_LESSTHAN:
      if(code=='='){
       scanner->position++;
       return  predTokenLessThanOrEqual;
      }
      if(code=='>'){
       scanner->position++;
       return  predTokenNotEqual;
      }
      return predTokenLessThan;

     case STATE_GREATERTHAN:
      if(code=='='){
       scanner->position++;
       return  predTokenGreaterThanOrEqual;
      }
      return predTokenGreaterThan;

     case STATE_COLON:
      if(code=='='){
       scanner->position++;
       return  predTokenColonEqual;
      }
      raiseError(scanner,@"Expecting = after :");
      break;

     case STATE_AMPERSAND:
      if(code=='&'){
       scanner->position++;
       return  predToken_AND;
      }
      raiseError(scanner,@"Expecting & after &");
      break;

     case STATE_BAR:
      if(code=='|'){
       scanner->position++;
       return  predToken_OR;
      }
      raiseError(scanner,@"Expecting | after |");
      break;

     case STATE_ASTERISK:
      if(code=='*'){
       scanner->position++;
       return  predTokenAsteriskAsterisk;
      }
      return predTokenAsterisk;
    }

   }
   return predTokenEOF;
}

static int peekTokenType(predicateScanner *scanner){
   NSInteger save=scanner->position;
   id  token;
   int tokenType;

   tokenType=scanToken(scanner,&token);
   scanner->position=save;

   return tokenType;
}

static void skipToken(predicateScanner *scanner){
   id token;

   scanToken(scanner,&token);
}

static void expectTokenType(predicateScanner *scanner,int expect){
   id  token;
   int tokenType;

   if((tokenType=scanToken(scanner,&token))!=expect)
    raiseError(scanner,@"Expecting token type %d, got %d",expect,tokenType);
}

static NSExpression *nextExpression(predicateScanner *scanner);
static NSPredicate *nextPredicate(predicateScanner *scanner);

static NSExpression *nextFunctionExpression(predicateScanner *scanner,NSString *name){
   NSMutableArray *arguments=[NSMutableArray array];

   while(peekTokenType(scanner)!=predTokenRightParen){
    if([arguments count]>0){
     if(peekTokenType(scanner)==predTokenComma)
      skipToken(scanner);
    }

    [arguments addObject:nextExpression(scanner)];
   }
   skipToken(scanner);

   return [NSExpression expressionForFunction:name arguments:arguments];
}

static id nextArgumentFromArray(predicateScanner *scanner){
   if(scanner->nextArgument>=[scanner->argumentArray count])
    raiseError(scanner,@"Insufficient arguments for conversion characters specified in format string"); // FIX, the string is actually the reason

   return [scanner->argumentArray objectAtIndex:scanner->nextArgument++];
}

static id objectArgument(predicateScanner *scanner){
   if(scanner->nextArgument<0)
    return va_arg(scanner->arguments,id);
   else
    return nextArgumentFromArray(scanner);
}

static id cStringArgument(predicateScanner *scanner){
   if(scanner->nextArgument<0)
    return [NSString stringWithCString:va_arg(scanner->arguments,char *)];
   else
    return nextArgumentFromArray(scanner);
}

static id charArgument(predicateScanner *scanner){
   if(scanner->nextArgument<0)
    return [NSNumber numberWithChar:va_arg(scanner->arguments,int)];
   else
    return nextArgumentFromArray(scanner);
}

static id shortArgument(predicateScanner *scanner){
   if(scanner->nextArgument<0)
    return [NSNumber numberWithShort:va_arg(scanner->arguments,int)];
   else
    return nextArgumentFromArray(scanner);
}

static id unsignedShortArgument(predicateScanner *scanner){
   if(scanner->nextArgument<0)
    return [NSNumber numberWithUnsignedShort:va_arg(scanner->arguments,int)];
   else
    return nextArgumentFromArray(scanner);
}

static id unicharArgument(predicateScanner *scanner){
   return unsignedShortArgument(scanner);
}

static id intArgument(predicateScanner *scanner){
   if(scanner->nextArgument<0)
    return [NSNumber numberWithInt:va_arg(scanner->arguments,int)];
   else
    return nextArgumentFromArray(scanner);
}

static id longArgument(predicateScanner *scanner){
   if(scanner->nextArgument<0)
    return [NSNumber numberWithLong:va_arg(scanner->arguments,long)];
   else
    return nextArgumentFromArray(scanner);
}

static id longLongArgument(predicateScanner *scanner){
   if(scanner->nextArgument<0)
    return [NSNumber numberWithLongLong:va_arg(scanner->arguments,long long)];
   else
    return nextArgumentFromArray(scanner);
}

static id unsignedIntArgument(predicateScanner *scanner){
   if(scanner->nextArgument<0)
    return [NSNumber numberWithUnsignedInt:va_arg(scanner->arguments,unsigned int)];
   else
    return nextArgumentFromArray(scanner);
}

static id unsignedLongArgument(predicateScanner *scanner){
   if(scanner->nextArgument<0)
    return [NSNumber numberWithUnsignedLong:va_arg(scanner->arguments,unsigned long)];
   else
    return nextArgumentFromArray(scanner);
}

static id unsignedLongLongArgument(predicateScanner *scanner){
   if(scanner->nextArgument<0)
    return [NSNumber numberWithUnsignedLongLong:va_arg(scanner->arguments,unsigned long long)];
   else
    return nextArgumentFromArray(scanner);
}

static id floatArgument(predicateScanner *scanner){
   if(scanner->nextArgument<0)
    return [NSNumber numberWithDouble:va_arg(scanner->arguments,double)];
   else
    return nextArgumentFromArray(scanner);
}

static id doubleArgument(predicateScanner *scanner){
   if(scanner->nextArgument<0)
    return [NSNumber numberWithDouble:va_arg(scanner->arguments,double)];
   else
    return nextArgumentFromArray(scanner);
}

static id longFloatArgument(predicateScanner *scanner){
   return doubleArgument(scanner);
}

static id longDoubleArgument(predicateScanner *scanner){
   return doubleArgument(scanner);
}

static NSExpression *nextFormatCharacter(predicateScanner *scanner){
   enum {
    STATE_MODIFIER,
    STATE_CONVERSION,
   } state=STATE_MODIFIER;
   unichar modifier='\0';

   for(;scanner->position<=scanner->length;){
    unichar unicode=(scanner->position<scanner->length)?scanner->unicode[scanner->position++]:0xFFFF;

    switch(state){

     case STATE_MODIFIER:
      switch(unicode){

       case 'h': case 'l': case 'q':
        modifier=unicode;
        break;

       default:
        scanner->position--;
        state=STATE_CONVERSION;
        break;
      }
      break;

     case STATE_CONVERSION:

      switch(unicode){

       case 'd': case 'i':{
         id value;

         if(modifier=='h')
          value=shortArgument(scanner);
         else if(modifier=='l')
          value=longArgument(scanner);
         else if(modifier=='q')
          value=longLongArgument(scanner);
         else
          value=intArgument(scanner);

         return [NSExpression expressionForConstantValue:value];
        }
        break;

       case 'o':
       case 'x':
       case 'X':
       case 'u':{
         id value;

         if(modifier=='h')
          value=unsignedShortArgument(scanner);
         else if(modifier=='l')
          value=unsignedLongArgument(scanner);
         else if(modifier=='q')
          value=unsignedLongLongArgument(scanner);
         else
          value=unsignedIntArgument(scanner);

         return [NSExpression expressionForConstantValue:value];
        }
        break;

       case 'c':
        return [NSExpression expressionForConstantValue:charArgument(scanner)];

       case 'C':
        return [NSExpression expressionForConstantValue:unicharArgument(scanner)];

       case 's':
        return [NSExpression expressionForConstantValue:cStringArgument(scanner)];
        break;

       case 'f':{
         id value;

         if(modifier=='l')
          value=longFloatArgument(scanner);
         else
          value=floatArgument(scanner);

         return [NSExpression expressionForConstantValue:value];
        }
        break;

       case 'e': case 'E':
       case 'g': case 'G':{
         id value;

         if(modifier=='l')
          value=longDoubleArgument(scanner);
         else
          value=doubleArgument(scanner);

         return [NSExpression expressionForConstantValue:value];
        }
        break;

       case 'p':
        return [NSExpression expressionForConstantValue:objectArgument(scanner)];

       case '@':
        return [NSExpression expressionForConstantValue:objectArgument(scanner)];

       case '%':
        return [NSExpression expressionForConstantValue:@"%"];

       case 'K':
        return [NSExpression expressionForKeyPath:objectArgument(scanner)];

       default:
        raiseError(scanner,@"Invalid format character %C",unicode);
        break;
     }
    }
   }
   return nil;
}

static NSExpression *nextPrimaryExpression(predicateScanner *scanner){
   id token;

   switch(peekTokenType(scanner)){

    case predTokenEOF:
     raiseError(scanner,@"Encountered EOF while parsing expression");
     break;

    case predTokenLeftParen:{
      auto NSExpression *result;

      skipToken(scanner);

      result=nextExpression(scanner);

      expectTokenType(scanner,predTokenRightParen);

      return result;
     }

    case predTokenIdentifier:
     scanToken(scanner,&token);

     if(peekTokenType(scanner)!=predTokenLeftParen)
      return [NSExpression expressionForKeyPath:token];

     return nextFunctionExpression(scanner,token);

    case predTokenAtSign:
     skipToken(scanner);
     if(scanToken(scanner,&token)!=predTokenIdentifier)
      raiseError(scanner,@"Expecting identifer after @ for keypath expression");

     return [NSExpression expressionForKeyPath:token];

    case predTokenString:
     scanToken(scanner,&token);
     return [NSExpression expressionForConstantValue:token];

    case predTokenNumeric:
     scanToken(scanner,&token);
     return [NSExpression expressionForConstantValue:token];

    case predTokenPercent:
     skipToken(scanner);
     return nextFormatCharacter(scanner);

    case predTokenDollar:{
      id  identifier;
      int identifierType;

      skipToken(scanner);

      if((identifierType=scanToken(scanner,&identifier))!=predTokenIdentifier)
       raiseError(scanner,@"Expecting identifier, got %@",identifier);

      return [NSExpression expressionForVariable:identifier];
     }
     break;

    case predToken_NULL:
     skipToken(scanner);
     return [NSExpression expressionForConstantValue:[NSNull null]];

    case predToken_TRUE:
     skipToken(scanner);
     return [NSExpression expressionForConstantValue:[NSNumber numberWithBool:YES]];

    case predToken_FALSE:
      skipToken(scanner);
      return [NSExpression expressionForConstantValue:[NSNumber numberWithBool:NO]];

    case predToken_SELF:
      skipToken(scanner);
      return [NSExpression expressionForEvaluatedObject];

    case predTokenLeftBrace:{
      NSMutableArray *aggregate=[NSMutableArray array];

      skipToken(scanner);

      while(peekTokenType(scanner)!=predTokenRightBrace){
       if([aggregate count]>0){
        if(peekTokenType(scanner)==predTokenComma)
         skipToken(scanner);
       }

       [aggregate addObject:nextExpression(scanner)];
      }
      skipToken(scanner);

      return [NSExpression_array expressionForArray:aggregate];
     }
   }

   return nil;
}

static NSExpression *nextKeypathExpression(predicateScanner *scanner){
   NSExpression *left=nextPrimaryExpression(scanner),*right;

   do{
    switch(peekTokenType(scanner)){

     case predTokenPeriod:
      skipToken(scanner);
      if((right=nextPrimaryExpression(scanner))==nil)
       raiseError(scanner,@"Expecting expression after .");

      left=[NSExpression_operator expressionForOperator:NSExpressionOperatorKeypath arguments:[NSArray arrayWithObjects:left,right,nil]];
      break;

     case predTokenLeftBracket:
      skipToken(scanner);

      switch(peekTokenType(scanner)){

       case predToken_FIRST:
        left=[NSExpression_operator expressionForOperator:NSExpressionOperatorIndexFirst arguments:[NSArray arrayWithObject:left]];
        break;

       case predToken_LAST:
        left=[NSExpression_operator expressionForOperator:NSExpressionOperatorIndexLast arguments:[NSArray arrayWithObject:left]];
        break;

       case predToken_SIZE:
        left=[NSExpression_operator expressionForOperator:NSExpressionOperatorIndexSize arguments:[NSArray arrayWithObject:left]];
        break;

       default:
        if((right=nextExpression(scanner))==nil)
         raiseError(scanner,@"Expecting expression after [");
        left=[NSExpression_operator expressionForOperator:NSExpressionOperatorIndex arguments:[NSArray arrayWithObjects:left,right,nil]];
        break;
      }
      expectTokenType(scanner,predTokenRightBracket);
      break;

     default:
      return left;
    }

   }while(YES);
}

static NSExpression *nextUnaryExpression(predicateScanner *scanner){
   if(peekTokenType(scanner)==predTokenMinus){
    NSExpression *right;

     skipToken(scanner);
     if((right=nextUnaryExpression(scanner))==nil)
      raiseError(scanner,@"Expecting expression after -");

     // coalesce -'s
     if([right isKindOfClass:[NSExpression_operator class]]){
      if([(NSExpression_operator *)right expressionType]==(int)NSExpressionOperatorNegate)
       return [[(NSExpression_operator *)right arguments] objectAtIndex:0];
     }
     return [NSExpression_operator expressionForOperator:NSExpressionOperatorNegate arguments:[NSArray arrayWithObject:right]];
   }

   return nextKeypathExpression(scanner);
}

static NSExpression *nextExponentiationExpression(predicateScanner *scanner){
   NSExpression *left=nextUnaryExpression(scanner),*right;

   do{
    switch(peekTokenType(scanner)){

     case predTokenAsteriskAsterisk:
      skipToken(scanner);
     if((right=nextUnaryExpression(scanner))==nil)
      raiseError(scanner,@"Expecting expression after **");
      left=[NSExpression_operator expressionForOperator:NSExpressionOperatorExp arguments:[NSArray arrayWithObjects:left,right,nil]];
      break;

     default:
      return left;
    }

   }while(YES);
}

static NSExpression *nextMultiplicativeExpression(predicateScanner *scanner){
   NSExpression *left=nextExponentiationExpression(scanner),*right;

   do{
    switch(peekTokenType(scanner)){

     case predTokenAsterisk:
      skipToken(scanner);
      if((right=nextExponentiationExpression(scanner))==nil)
       raiseError(scanner,@"Expecting expression after *");
      left=[NSExpression_operator expressionForOperator:NSExpressionOperatorMultiply arguments:[NSArray arrayWithObjects:left,right,nil]];
      break;

     case predTokenSlash:
      skipToken(scanner);
      if((right=nextExponentiationExpression(scanner))==nil)
       raiseError(scanner,@"Expecting expression after /");
      left=[NSExpression_operator expressionForOperator:NSExpressionOperatorDivide arguments:[NSArray arrayWithObjects:left,right,nil]];
      break;

     default:
      return left;
    }

   }while(YES);
}

static NSExpression *nextAdditiveExpression(predicateScanner *scanner){
   NSExpression *left=nextMultiplicativeExpression(scanner),*right;

   do{
    switch(peekTokenType(scanner)){

     case predTokenPlus:
      skipToken(scanner);
      if((right=nextMultiplicativeExpression(scanner))==nil)
       raiseError(scanner,@"Expecting expression after +");
      left=[NSExpression_operator expressionForOperator:NSExpressionOperatorAdd arguments:[NSArray arrayWithObjects:left,right,nil]];
      break;

     case predTokenMinus:
      skipToken(scanner);
      if((right=nextMultiplicativeExpression(scanner))==nil)
       raiseError(scanner,@"Expecting expression after -");
      left=[NSExpression_operator expressionForOperator:NSExpressionOperatorSubtract arguments:[NSArray arrayWithObjects:left,right,nil]];
      break;

     default:
      return left;
    }

   }while(YES);
}

static NSExpression *nextAssignmentExpression(predicateScanner *scanner){
   NSExpression *left=nextAdditiveExpression(scanner),*right;

   do{
    switch(peekTokenType(scanner)){

     case predTokenColonEqual:
      skipToken(scanner);
      if((right=nextAdditiveExpression(scanner))==nil)
       raiseError(scanner,@"Expecting expression after :=");
      // FIX, verify left expression is just a variable

      return [NSExpression_assignment expressionWithVariable:left expression:right];

     default:
      return left;
    }

   }while(YES);
}

static NSExpression *nextExpression(predicateScanner *scanner){
   return nextAssignmentExpression(scanner);
}

static void nextOperationOption(predicateScanner *scanner,unsigned *options){
   id  token;
   int tokenType;

   if(peekTokenType(scanner)!=predTokenLeftBracket)
    return;
   skipToken(scanner);

   if((tokenType=scanToken(scanner,&token))!=predTokenIdentifier)
    raiseError(scanner,@"Expecting identifier in options");

   if([token isEqualToString:@"c"])
    *options=NSCaseInsensitivePredicateOption;
   else if([token isEqualToString:@"d"])
    *options=NSDiacriticInsensitivePredicateOption;
   else if([token isEqualToString:@"cd"])
    *options=NSCaseInsensitivePredicateOption|NSDiacriticInsensitivePredicateOption;
   else {
    raiseError(scanner,@"Expecting c, d, or cd in string options");
   }

   expectTokenType(scanner,predTokenRightBracket);
}

static BOOL nextOperation(predicateScanner *scanner,NSPredicateOperatorType *type,unsigned *options){
   *options=0;

   switch(peekTokenType(scanner)){

    case predTokenEqual:
     skipToken(scanner);
     *type=NSEqualToPredicateOperatorType;
     return YES;

    case predTokenNotEqual:
     skipToken(scanner);
     *type=NSNotEqualToPredicateOperatorType;
     return YES;

    case predTokenLessThan:
     skipToken(scanner);
     *type=NSLessThanPredicateOperatorType;
     return YES;

    case predTokenGreaterThan:
     skipToken(scanner);
     *type=NSGreaterThanPredicateOperatorType;
     return YES;

    case predTokenLessThanOrEqual:
     skipToken(scanner);
     *type=NSLessThanOrEqualToPredicateOperatorType;
     return YES;

    case predTokenGreaterThanOrEqual:
     skipToken(scanner);
     *type=NSGreaterThanOrEqualToPredicateOperatorType;
     return YES;

    case predToken_IN:
     skipToken(scanner);
     *type=NSInPredicateOperatorType;
     nextOperationOption(scanner,options);
     return YES;

    case predToken_BEGINSWITH:
     skipToken(scanner);
     *type=NSBeginsWithPredicateOperatorType;
     nextOperationOption(scanner,options);
     return YES;

    case predToken_ENDSWITH:
     skipToken(scanner);
     *type=NSEndsWithPredicateOperatorType;
     nextOperationOption(scanner,options);
     return YES;

    case predToken_LIKE:
     skipToken(scanner);
     *type=NSLikePredicateOperatorType;
     nextOperationOption(scanner,options);
     return YES;

    case predToken_MATCHES:
     skipToken(scanner);
     *type=NSMatchesPredicateOperatorType;
     nextOperationOption(scanner,options);
     return YES;

    default:
     return NO;
   }

}

static NSPredicate *nextComparisonPredicate(predicateScanner *scanner){
   NSComparisonPredicateModifier modifier=NSDirectPredicateModifier;
   BOOL negate=NO;

   switch(peekTokenType(scanner)){

    case predToken_ANY:
     skipToken(scanner);
     modifier=NSAnyPredicateModifier;
     break;

    case predToken_ALL:
     skipToken(scanner);
     modifier=NSAllPredicateModifier;
     break;

    case predToken_NONE:
     skipToken(scanner);
     modifier=NSAnyPredicateModifier;
     negate=YES;
     break;
  }

  {
   NSExpression *left=nextExpression(scanner);
   NSExpression *right;
   NSPredicate  *result;
   NSPredicateOperatorType type = 0;
   unsigned options;

   switch(peekTokenType(scanner)){

    case predToken_CONTAINS:
     skipToken(scanner);
     nextOperationOption(scanner,&options);
     right=nextExpression(scanner);
     result=[NSComparisonPredicate predicateWithLeftExpression:left rightExpression:right modifier:modifier type:NSInPredicateOperatorType options:options];
     break;

    case predToken_BETWEEN:
     skipToken(scanner);
     right=nextExpression(scanner);
     {
      NSExpression *rightFirst=[NSExpression_operator expressionForOperator:NSExpressionOperatorIndexFirst arguments:[NSArray arrayWithObject:right]];
      NSPredicate  *greaterOrEqual=[NSComparisonPredicate predicateWithLeftExpression:left rightExpression:rightFirst modifier:NSDirectPredicateModifier type:NSGreaterThanOrEqualToPredicateOperatorType options:0];

      NSExpression *rightLast=[NSExpression_operator expressionForOperator:NSExpressionOperatorIndexLast arguments:[NSArray arrayWithObject:right]];
      NSPredicate  *lessOrEqual=[NSComparisonPredicate predicateWithLeftExpression:left rightExpression:rightLast modifier:NSDirectPredicateModifier type:NSLessThanOrEqualToPredicateOperatorType options:0];

      result=[NSCompoundPredicate andPredicateWithSubpredicates:[NSArray arrayWithObjects:greaterOrEqual,lessOrEqual,nil]];
     }
     break;

    default:
     if(!nextOperation(scanner,&type,&options)){
      raiseError(scanner,@"Expecting comparison operator");
     }

     right=nextExpression(scanner);
     result=[NSComparisonPredicate predicateWithLeftExpression:left rightExpression:right modifier:modifier type:type options:options];
     break;
   }

   if(negate)
    result=[NSCompoundPredicate notPredicateWithSubpredicate:result];

   return result;
  }
}

static NSPredicate *nextPrimaryPredicate(predicateScanner *scanner){

   switch(peekTokenType(scanner)){

    case predToken_TRUEPREDICATE:
     skipToken(scanner);
     return [NSPredicate predicateWithValue:YES];

    case predToken_FALSEPREDICATE:
     skipToken(scanner);
     return [NSPredicate predicateWithValue:NO];

    case predTokenLeftParen:{
      NSPredicate *result;

      skipToken(scanner);

      result=nextPredicate(scanner);

      expectTokenType(scanner,predTokenRightParen);

      return result;
     }
   }

   return nextComparisonPredicate(scanner);
}

static NSPredicate *nextUnaryPredicate(predicateScanner *scanner){
   NSPredicate *right;

   if(peekTokenType(scanner)==predToken_NOT){
     skipToken(scanner);
     right=nextUnaryPredicate(scanner);

     // coalesce NOT's
     if([right isKindOfClass:[NSCompoundPredicate class]]){
      if([(NSCompoundPredicate *)right compoundPredicateType]==NSNotPredicateType)
       return [[(NSCompoundPredicate *)right subpredicates] objectAtIndex:0];
     }

     return [NSCompoundPredicate notPredicateWithSubpredicate:right];
   }

   right=nextPrimaryPredicate(scanner);
   return right;
}

static NSPredicate *nextConditionalAndPredicate(predicateScanner *scanner){
   NSPredicate *left=nextUnaryPredicate(scanner),*right;

   do{
    switch(peekTokenType(scanner)){

     case predToken_AND:
      skipToken(scanner);
      right=nextUnaryPredicate(scanner);
      left=[NSCompoundPredicate andPredicateWithSubpredicates:[NSArray arrayWithObjects:left,right,nil]];
      break;

     default:
      return left;
    }
   }while(YES);
}

static NSPredicate *nextConditionalOrPredicate(predicateScanner *scanner){
   NSPredicate *left=nextConditionalAndPredicate(scanner),*right;

   do{
    switch(peekTokenType(scanner)){

     case predToken_OR:
      skipToken(scanner);
      right=nextConditionalAndPredicate(scanner);
      left=[NSCompoundPredicate orPredicateWithSubpredicates:[NSArray arrayWithObjects:left,right,nil]];
      break;

     default:
      return left;
    }
   }while(YES);
}

static NSPredicate *nextPredicate(predicateScanner *scanner){
   return nextConditionalOrPredicate(scanner);
}

static NSPredicate *nextTopLevelPredicate(predicateScanner *scanner){
   NSPredicate *result=nextPredicate(scanner);

#if 1
// broken?
   if(peekTokenType(scanner)!=predTokenEOF)
    raiseError(scanner,@"Extraneous tokens at end of string");
#endif

   return result;
}

@implementation NSPredicate

-initWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
   return self;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}

+(NSPredicate *)predicateWithFormat:(NSString *)format arguments:(va_list)arguments {
   predicateScanner scanner;
   NSUInteger         length=[format length];
   unichar          buffer[length];

   [format getCharacters:buffer];

   scanner.original=format;
   scanner.unicode=buffer;
   scanner.length=length;
   scanner.position=0;
   scanner.nextArgument=-1;
   va_copy(scanner.arguments,arguments);

   NSPredicate *result=nextTopLevelPredicate(&scanner);

   va_end(scanner.arguments);

   return result;
}

+(NSPredicate *)predicateWithFormat:(NSString *)format,... {
   va_list arguments;

   va_start(arguments,format);

   NSPredicate *result=[self predicateWithFormat:format arguments:arguments];

   va_end(arguments);

   return result;
}

+(NSPredicate *)predicateWithFormat:(NSString *)format argumentArray:(NSArray *)arguments {
   predicateScanner scanner;
   NSUInteger         length=[format length];
   unichar          buffer[length];

   [format getCharacters:buffer];

   scanner.original=format;
   scanner.unicode=buffer;
   scanner.length=length;
   scanner.position=0;
   scanner.nextArgument=0;
   scanner.argumentArray=arguments;

   return nextTopLevelPredicate(&scanner);
}

+(NSPredicate *)predicateWithValue:(BOOL)value {
   return [[[NSPredicate_BOOL allocWithZone:NULL] initWithBool:value] autorelease];
}

-(NSString *)predicateFormat {
   return nil;
}

-(NSPredicate *)predicateWithSubstitutionVariables:(NSDictionary *)variables {
   return self;
}

-(BOOL)evaluateWithObject:object {
   return NO;
}

-(NSString *)description {
   return [self predicateFormat];
}

@end
