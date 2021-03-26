/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "O2PDFFunction_Type4.h"
#import <Onyx2D/O2PDFArray.h>
#import <Onyx2D/O2PDFDictionary.h>
#import <Onyx2D/O2PDFContext.h>
#import <Onyx2D/O2PDFObject_Integer.h>
#import <Onyx2D/O2PDFObject_Real.h>
#import <Onyx2D/O2PDFObject_Boolean.h>
#import <Onyx2D/O2PDFObject_const.h>
#import <Onyx2D/O2PDFObject_identifier.h>
#import "O2PDFBlock.h"

#import <Foundation/NSArray.h>
#import <stddef.h>

@implementation O2PDFFunction_Type4

static void runBlock(NSMutableArray *stack,O2PDFBlock *block){
   NSArray  *operators=[block objects];
   NSInteger i,count=[operators count];
   
   for(i=0;i<count;i++){
    O2PDFObject *check=[operators objectAtIndex:i];
    
    if([check objectType]!=O2PDFObjectType_identifier)
     [stack addObject:check];
    else {
     NSLog(@"execute %@",check);
     switch([(O2PDFObject_identifier *)check identifier]){

      default:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;
       
      case O2PDFIdentifier_abs:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_add:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_atan:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_ceiling:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_cos:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_cvi:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_cvr:{
        O2PDFInteger value;
       
        if([[stack lastObject] checkForType:kO2PDFObjectTypeInteger value:&value]){
         [stack removeLastObject];
         [stack addObject:[O2PDFObject_Real pdfObjectWithReal:value]];
        }
       }
       break;

      case O2PDFIdentifier_div:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_exp:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_floor:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_idiv:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_ln:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_log:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_mod:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_mul:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_neg:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_round:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_sin:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_sqrt:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_sub:{
        O2PDFReal num2,num1;
        
        if(![[stack lastObject] checkForType:kO2PDFObjectTypeReal value:&num2]){
         O2PDFFix(__FILE__,__LINE__,@"sub operator typecheck fail");
         return;
        }
        [stack removeLastObject];
        
        if(![[stack lastObject] checkForType:kO2PDFObjectTypeReal value:&num1]){
         O2PDFFix(__FILE__,__LINE__,@"sub operator typecheck fail");
         return;
        }
        [stack removeLastObject];
        
        [stack addObject:[O2PDFObject_Real pdfObjectWithReal:num1-num2]];
       }
       break;

      case O2PDFIdentifier_truncate:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;
 
      case O2PDFIdentifier_and:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_bitshift:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_eq:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_ge:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_gt:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_le:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_lt:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_ne:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_not:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_or:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_xor:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;
 
      case O2PDFIdentifier_if:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_ifelse:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;
 
      case O2PDFIdentifier_copy:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_dup:
       O2PDFFix(__FILE__,__LINE__,@"PostScript calculator operator %@",check);
       break;

      case O2PDFIdentifier_exch:{
        NSInteger count=[stack count];
        
        [stack addObject:[stack objectAtIndex:count-2]];
        [stack removeObjectAtIndex:count-3];
       }
       break;

      case O2PDFIdentifier_index:;
       O2PDFInteger index;
       
       if(![[stack lastObject] checkForType:kO2PDFObjectTypeInteger value:&index]){
        O2PDFFix(__FILE__,__LINE__,@"roll operator typecheck fail");
        return;
       }
       [stack removeLastObject];
       
       NSInteger count=[stack count];
       [stack addObject:[stack objectAtIndex:(count-1)-index]];
       break;

      case O2PDFIdentifier_pop:
       [stack removeLastObject];
       break;

      case O2PDFIdentifier_roll:;
       O2PDFInteger rollCount,rollSize;
       
       if(![[stack lastObject] checkForType:kO2PDFObjectTypeInteger value:&rollCount]){
        O2PDFFix(__FILE__,__LINE__,@"roll operator typecheck fail");
        return;
       }
       
       [stack removeLastObject];
       
       if(![[stack lastObject] checkForType:kO2PDFObjectTypeInteger value:&rollSize]){
        O2PDFFix(__FILE__,__LINE__,@"roll operator typecheck fail");
        return;
       }
       [stack removeLastObject];
       
       NSInteger stackSize=[stack count];
       
       while(--rollCount>=0){
        id object=[stack objectAtIndex:stackSize-rollSize];
        [stack addObject:object];
        [stack removeObjectAtIndex:stackSize-(rollSize+1)];
       }
       
       break;
     }
     
    }
   }
}

static void evaluate(void *info,const float *input,float *output) {
   O2PDFFunction_Type4 *self=info;
   float                x=input[0];
   NSMutableArray      *stack=[NSMutableArray array];
   
   [stack addObject:[O2PDFObject_Real pdfObjectWithReal:x]];
   runBlock(stack,self->_calculator);
   
   NSLog(@"stack=%@",stack);

   int count=self->_rangeCount/2;
   while(--count>=0){
    O2PDFReal value=0.0;
    
    [[stack lastObject] checkForType:kO2PDFObjectTypeReal value:&value];

    output[count]=value;
   }
}

#define LF 10
#define FF 12
#define CR 13

static O2PDFIdentifier O2PostScriptClassifyIdentifier(const char *bytes,unsigned length) {
   char name[length+1];
   
   strncpy(name,bytes,length);
   name[length]='\0';

   
   if(strcmp(name,"true")==0)
    return O2PDFIdentifier_true;
   if(strcmp(name,"false")==0)
    return O2PDFIdentifier_false;

   if(strcmp(name,"abs")==0)
    return O2PDFIdentifier_abs;
   if(strcmp(name,"add")==0)
    return O2PDFIdentifier_add;
   if(strcmp(name,"atan")==0)
    return O2PDFIdentifier_atan;
   if(strcmp(name,"ceiling")==0)
    return O2PDFIdentifier_ceiling;
   if(strcmp(name,"cos")==0)
    return O2PDFIdentifier_cos;
   if(strcmp(name,"cvi")==0)
    return O2PDFIdentifier_cvi;
   if(strcmp(name,"cvr")==0)
    return O2PDFIdentifier_cvr;
   if(strcmp(name,"div")==0)
    return O2PDFIdentifier_div;
   if(strcmp(name,"exp")==0)
    return O2PDFIdentifier_exp;
   if(strcmp(name,"floor")==0)
    return O2PDFIdentifier_floor;
   if(strcmp(name,"idiv")==0)
    return O2PDFIdentifier_idiv;
   if(strcmp(name,"ln")==0)
    return O2PDFIdentifier_ln;
   if(strcmp(name,"log")==0)
    return O2PDFIdentifier_log;
   if(strcmp(name,"mod")==0)
    return O2PDFIdentifier_mod;
   if(strcmp(name,"mul")==0)
    return O2PDFIdentifier_mul;
   if(strcmp(name,"neg")==0)
    return O2PDFIdentifier_neg;
   if(strcmp(name,"round")==0)
    return O2PDFIdentifier_round;
   if(strcmp(name,"sin")==0)
    return O2PDFIdentifier_sin;
   if(strcmp(name,"sqrt")==0)
    return O2PDFIdentifier_sqrt;
   if(strcmp(name,"sub")==0)
    return O2PDFIdentifier_sub;
   if(strcmp(name,"truncate")==0)
    return O2PDFIdentifier_truncate;

   if(strcmp(name,"and")==0)
    return O2PDFIdentifier_and;
   if(strcmp(name,"bitshift")==0)
    return O2PDFIdentifier_bitshift;
   if(strcmp(name,"eq")==0)
    return O2PDFIdentifier_eq;
   if(strcmp(name,"ge")==0)
    return O2PDFIdentifier_ge;
   if(strcmp(name,"le")==0)
    return O2PDFIdentifier_le;
   if(strcmp(name,"lt")==0)
    return O2PDFIdentifier_lt;
   if(strcmp(name,"ne")==0)
    return O2PDFIdentifier_ne;
   if(strcmp(name,"not")==0)
    return O2PDFIdentifier_not;
   if(strcmp(name,"or")==0)
    return O2PDFIdentifier_or;
   if(strcmp(name,"xor")==0)
    return O2PDFIdentifier_xor;

   if(strcmp(name,"if")==0)
    return O2PDFIdentifier_if;
   if(strcmp(name,"ifelse")==0)
    return O2PDFIdentifier_ifelse;

   if(strcmp(name,"copy")==0)
    return O2PDFIdentifier_copy;
   if(strcmp(name,"dup")==0)
    return O2PDFIdentifier_dup;
   if(strcmp(name,"exch")==0)
    return O2PDFIdentifier_exch;
   if(strcmp(name,"index")==0)
    return O2PDFIdentifier_index;
   if(strcmp(name,"pop")==0)
    return O2PDFIdentifier_pop;
   if(strcmp(name,"roll")==0)
    return O2PDFIdentifier_roll;

       
   return O2PDFIdentifierUnknown;
}

// Returns YES and *objectp==NULL on end of stream
static BOOL O2PDFScanCalculator(const char *bytes,unsigned length,O2PDFInteger position,O2PDFInteger *lastPosition,O2PDFObject **objectp) {
   O2PDFInteger     currentSign=1,currentInt=0;
   O2PDFReal        currentReal=0,currentFraction=0;
   int              inlineLocation=0;
   
   enum {
    STATE_SCANNING,
    STATE_COMMENT,
    STATE_INTEGER,
    STATE_REAL,
    STATE_IDENTIFIER,
   } state=STATE_SCANNING;
   
   *objectp=NULL;
   
   for(;position<length;position++){
    unsigned char code=bytes[position];
	
	      //NSLog(@"state=%d,code=%c",state,code);
	switch(state){
          
	 case STATE_SCANNING:
	  switch(code){
	  
	   case ' ':
	   case  CR:
	   case  FF:
	   case  LF:
	   case '\t':
	    break;
		
       case '%':
        state=STATE_COMMENT;
        break;

       case '-':
        state=STATE_INTEGER;
	    currentSign=-1;
	    currentInt=0;
        break;
        
       case '+':
        state=STATE_INTEGER;
	    currentSign=1;
	    currentInt=0;
        break;

       case '0': case '1': case '2': case '3': case '4':
       case '5': case '6': case '7': case '8': case '9':
        state=STATE_INTEGER;
	    currentSign=1;
	    currentInt=code-'0';
        break;

       case '.':
        state=STATE_REAL;
	    currentSign=1;
	    currentReal=0;
        currentFraction=0.1;
        break;
        
       case '{':
        *objectp=[O2PDFObject_const pdfObjectProcMark];
        *lastPosition=position+1;
        return YES;

       case '}':
        *objectp=[O2PDFObject_const pdfObjectProcMarkEnd];
        *lastPosition=position+1;
        return YES;

       default:
        state=STATE_IDENTIFIER;
        inlineLocation=position;
        break;
	  }
	  break;
	  
     case STATE_COMMENT:
      if(code==CR || code==LF || code==FF)
       state=STATE_SCANNING;
      break;

     case STATE_INTEGER:
      if(code=='.'){
       state=STATE_REAL;
       currentReal=currentInt;
       currentFraction=0.1;
      }
      else if(code>='0' && code<='9')
       currentInt=currentInt*10+code-'0';
      else {
       *objectp=[O2PDFObject_Integer pdfObjectWithInteger:currentSign*currentInt];
       *lastPosition=position;
       return YES;
      }
      break;

     case STATE_REAL:
      if(code>='0' && code<='9'){
       currentReal+=currentFraction*(code-'0');
       currentFraction*=0.1;
      }
      else {
       *objectp=[O2PDFObject_Real pdfObjectWithReal:currentSign*currentReal];
       *lastPosition=position;
       return YES;
      }
      break;

     case STATE_IDENTIFIER:
      if(code==' ' || code==CR || code==FF || code==LF || code=='\t' || code=='\0' ||
         code=='%' || code=='(' || code==')' || code=='<' || code=='>' || code==']' ||
         code=='[' || code=='{' || code=='}' || code=='/'){
       const char     *name=bytes+inlineLocation;
       unsigned        length=position-inlineLocation;
       O2PDFIdentifier identifier=O2PostScriptClassifyIdentifier(name,length);
       
       if(identifier==O2PDFIdentifier_true)
        *objectp=[O2PDFObject_Boolean pdfObjectWithTrue];
       else if(identifier==O2PDFIdentifier_false)
        *objectp=[O2PDFObject_Boolean pdfObjectWithFalse];
       else
        *objectp=[O2PDFObject_identifier pdfObjectWithIdentifier:identifier name:name length:length];

       *lastPosition=position;
       return YES;
      }
      break;
	}
   }
       
   *lastPosition=position;
    
   return (state==STATE_SCANNING)?YES:NO;
}

static O2PDFBlock *O2PDFParseCalculator(const char *bytes,unsigned length,O2PDFInteger position,O2PDFInteger *lastPosition) {
   NSMutableArray *stack=[NSMutableArray array];
   O2PDFObject    *check;
         
   while(YES) {

    if(!O2PDFScanCalculator(bytes,length,position,&position,&check))
     return NO;

NSLog(@"check=%@",check);

    if(check==NULL){
     *lastPosition=position;
     return nil;
    }
            
    switch([check objectType]){
     O2PDFBlock *block;
     
     case kO2PDFObjectTypeBoolean:
     case kO2PDFObjectTypeInteger:
     case kO2PDFObjectTypeReal:
     case O2PDFObjectType_identifier:
      block=[stack lastObject];
      
      if(block==nil){
       O2PDFError(__FILE__,__LINE__,@"PS calculator parse error");
       return NO;
      }
      
      [block addObject:check];
      break;
           
     case O2PDFObjectTypeMark_proc_open:
       [stack addObject:[O2PDFBlock pdfBlock]];
       break;

     case O2PDFObjectTypeMark_proc_close:;
       block=[stack lastObject];
       
       if(block==nil){
        O2PDFError(__FILE__,__LINE__,@"PS calculator parse error");
        return NO;
       }
       
       [[block retain] autorelease];
       [stack removeLastObject];
       
       O2PDFBlock *superBlock=[stack lastObject];
       
       if(superBlock==nil)
        return block;
       
       [superBlock addObject:block];
       break;
       
            
      [stack addObject:check];
      break;
                  
     default:
      return NO;
    }
   }
   
   return NO;
}

-initWithDomain:(O2PDFArray *)domain range:(O2PDFArray *)range calculator:(NSData *)data {
NSLog(@"INITIALIZE TYPE 4");
   if([super initWithDomain:domain range:range]==nil)
    return nil;

   _info=self;
   _callbacks.evaluate=evaluate;

   int lastPosition=0;
   _calculator=[O2PDFParseCalculator([data bytes],[data length],0,&lastPosition) retain];
   NSLog(@"calculator=%@",_calculator);
         
   return self;
}

-(void)dealloc {
   [_calculator release];
   [super dealloc];
}

-(BOOL)isLinear {
   return NO;
}

-(O2PDFObject *)encodeReferenceWithContext:(O2PDFContext *)context {
   O2PDFDictionary *result=[O2PDFDictionary pdfDictionary];
   int              i;
   
   [result setIntegerForKey:"FunctionType" value:4];
   [result setObjectForKey:"Domain" value:[O2PDFArray pdfArrayWithNumbers:_domain count:_domainCount]];
   [result setObjectForKey:"Range" value:[O2PDFArray pdfArrayWithNumbers:_range count:_rangeCount]];
   // FIXME  stream
   
   return [context encodeIndirectPDFObject:result];
}

-(NSString *)description {
   NSMutableString *result=[NSMutableString string];
   [result appendFormat:@"<%@ %p:",isa,self];
   [result appendFormat:@"calculator=%@",_calculator];
   [result appendFormat:@">"];
   
   return result;
}

@end
