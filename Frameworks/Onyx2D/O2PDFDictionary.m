/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFDictionary.h>

#import <Onyx2D/O2PDFArray.h>
#import <Onyx2D/O2PDFStream.h>
#import <Onyx2D/O2PDFObject_Integer.h>
#import <Onyx2D/O2PDFObject_Name.h>
#import <Onyx2D/O2PDFObject_Real.h>
#import <Onyx2D/O2PDFObject_Boolean.h>
#import <Onyx2D/O2PDFContext.h>
#import <Foundation/NSString.h>
#import <stddef.h>
#import <string.h>

unsigned O2PDFHashCString(NSMapTable *table,const void *data){
   const char *s=data;

   if(s!=NULL){
    unsigned i,result=5381;

    for(i=0;s[i]!='\0';i++)
     result=((result<<5)+result)+(unsigned)(s[i]); // hash*33+c

    return result;
   }

   return 0;
}

BOOL O2PDFIsEqualCString(NSMapTable *table,const void *data1,const void *data2){
   if(data1 == data2)
    return YES;

   if(!data1)
    return !strlen((char *)data2);

   if (!data2)
    return !strlen((char *)data1);

   if(((char *)data1)[0]!=((char *)data2)[0])
    return NO;

   return (strcmp((char *)data1,(char *)data2))?NO:YES;
}

void O2PDFFreeCString(NSMapTable *table,void *data) {
   NSZoneFree(NULL,data);
}

NSMapTableKeyCallBacks O2PDFOwnedCStringKeyCallBacks={
  O2PDFHashCString,O2PDFIsEqualCString,NULL,O2PDFFreeCString,NULL,NULL
};

@implementation O2PDFDictionary

-init {
   _table=NSCreateMapTable(O2PDFOwnedCStringKeyCallBacks,NSObjectMapValueCallBacks,0);
   return self;
}

+(O2PDFDictionary *)pdfDictionary {
   return [[[self alloc] init] autorelease];
}

-(void)dealloc {
   NSFreeMapTable(_table);
   [super dealloc];
}

-(O2PDFObjectType)objectType { return kO2PDFObjectTypeDictionary; }

-(BOOL)checkForType:(O2PDFObjectType)type value:(void *)value {
   if(type!=kO2PDFObjectTypeDictionary)
    return NO;
   
   *((O2PDFDictionary **)value)=self;
   return YES;
}

-(void)setObjectForKey:(const char *)key value:(O2PDFObject *)object {
   char *keyCopy=NSZoneMalloc(NULL,strlen(key)+1);
   
   strcpy(keyCopy,key);
   
   NSMapInsert(_table,keyCopy,object);
}

-(void)setBooleanForKey:(const char *)key value:(O2PDFBoolean)value {
   [self setObjectForKey:key value:[O2PDFObject_Boolean pdfObjectWithBoolean:value]];
}

-(void)setIntegerForKey:(const char *)key value:(O2PDFInteger)value {
   [self setObjectForKey:key value:[O2PDFObject_Integer pdfObjectWithInteger:value]];
}

-(void)setNumberForKey:(const char *)key value:(O2PDFReal)value {
   [self setObjectForKey:key value:[O2PDFObject_Real pdfObjectWithReal:value]];
}

-(void)setNameForKey:(const char *)key value:(const char *)value {
   [self setObjectForKey:key value:[O2PDFObject_Name pdfObjectWithCString:value]];
}

-(O2PDFObject *)objectForCStringKey:(const char *)key {
   O2PDFObject *object=NSMapGet(_table,key);

   return [object realObject];
}

-(O2PDFObject *)inheritedForCStringKey:(const char *)cStringKey typecheck:(O2PDFObjectType)type {
   O2PDFDictionary *parent=self;
   O2PDFObject     *object;

   do{
    if((object=[parent objectForCStringKey:cStringKey])!=nil){
     if([object objectType]==type)
      return object;
    }
    
   }while([parent getDictionaryForKey:"Parent" value:&parent]);
   
   return nil;
}


-(BOOL)getObjectForKey:(const char *)key value:(O2PDFObject **)objectp {
   *objectp=[self objectForCStringKey:key];
   
   return (*objectp!=NULL)?YES:NO;
}

-(BOOL)getNullForKey:(const char *)key {
   O2PDFObject *object=[self objectForCStringKey:key];
   
   return ([object objectType]==kO2PDFObjectTypeNull)?YES:NO;
}

-(BOOL)getBooleanForKey:(const char *)key value:(O2PDFBoolean *)valuep {
   O2PDFObject *object=[self objectForCStringKey:key];
   
   return [object checkForType:kO2PDFObjectTypeBoolean value:valuep];
}

-(BOOL)getIntegerForKey:(const char *)key value:(O2PDFInteger *)valuep {
   O2PDFObject *object=[self objectForCStringKey:key];
   
   return [object checkForType:kO2PDFObjectTypeInteger value:valuep];
}

-(BOOL)getNumberForKey:(const char *)key value:(O2PDFReal *)valuep {
   O2PDFObject *object=[self objectForCStringKey:key];
   
   return [object checkForType:kO2PDFObjectTypeReal value:valuep];
}

-(BOOL)getNameForKey:(const char *)key value:(const char **)namep {
   O2PDFObject *object=[self objectForCStringKey:key];
   
   return [object checkForType:kO2PDFObjectTypeName value:namep];
}

-(BOOL)getStringForKey:(const char *)key value:(O2PDFString **)stringp {
   O2PDFObject *object=[self objectForCStringKey:key];
   
   return [object checkForType:kO2PDFObjectTypeString value:stringp];
}

-(BOOL)getArrayForKey:(const char *)key value:(O2PDFArray **)arrayp {
   O2PDFObject *object=[self objectForCStringKey:key];
   
   return [object checkForType:kO2PDFObjectTypeArray value:arrayp];
}

-(BOOL)getDictionaryForKey:(const char *)key value:(O2PDFDictionary **)dictionaryp {
   O2PDFObject *object=[self objectForCStringKey:key];
   
   return [object checkForType:kO2PDFObjectTypeDictionary value:dictionaryp];
}

-(BOOL)getStreamForKey:(const char *)key value:(O2PDFStream **)streamp {
   O2PDFObject *object=[self objectForCStringKey:key];
   
   return [object checkForType:kO2PDFObjectTypeStream value:streamp];
}

-(NSString *)description {
   NSMutableString *result=[NSMutableString string];
   NSMapEnumerator  state=NSEnumerateMapTable(_table);
   const char *key;
   id          value;
   
   [result appendString:@"<<\n"];
   while(NSNextMapEnumeratorPair(&state,(void **)&key,(void **)&value)){
    [result appendFormat:@"%s %@\n",key,value];
   }
   [result appendString:@">>\n"];

   return result;
}

-(void)encodeWithPDFContext:(O2PDFContext *)encoder {
   NSMapEnumerator  state=NSEnumerateMapTable(_table);
   const char      *key;
   id               value;
   
   [encoder appendString:@"<<\n"];
   while(NSNextMapEnumeratorPair(&state,(void **)&key,(void **)&value)){
    [encoder appendFormat:@"/%s ",key];
    [encoder encodePDFObject:value];
   }
   [encoder appendString:@">>\n"];
}

@end

