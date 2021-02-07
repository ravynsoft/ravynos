#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>
#import <Foundation/NSCharacterSet.h>

void testEncodeDecode(NSString* encoded, NSString* decoded, NSCharacterSet* charset, NSString* description){
    
    NSString* encodeTest = [decoded stringByAddingPercentEncodingWithAllowedCharacters:charset];
    NSString* decodeTest = [encoded stringByRemovingPercentEncoding];

    const char* encodeMsg  =  [[NSString stringWithFormat:@"Percent-Encode: %@", description] UTF8String];
    const char* decodeMsg  =  [[NSString stringWithFormat:@"Percent-Decode: %@", description] UTF8String];
     
    PASS_EQUAL(encodeTest, encoded, "%s", encodeMsg);
    PASS_EQUAL(decodeTest, decoded, "%s", decodeMsg);
}

int main (int argc, const char * argv[])
{

  NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];


  NSString *urlDecodedString = @"Only alphabetic characters should be allowed and not encoded. !@#$%^&*()_+-=";
  NSString *urlEncodedString = @"Only%20alphabetic%20characters%20should%20be%20allowed%20and%20not%20encoded%2E%20%21%40%23%24%25%5E%26%2A%28%29%5F%2B%2D%3D";
  testEncodeDecode(urlEncodedString, urlDecodedString, [NSCharacterSet alphanumericCharacterSet],
                     @"Skip alphanumericCharacterSet 01");


  urlDecodedString = @"https://www.microsoft.com/en-us/!@#$%^&*()_";
  urlEncodedString = @"https://www.microsoft.com/en-us/!@%23$%25%5E&*()_";
  testEncodeDecode(urlEncodedString, urlDecodedString, [NSCharacterSet URLFragmentAllowedCharacterSet],
                   @"Skip URLFragmentAllowedCharacterSet 02");


  urlDecodedString = @"All alphabetic characters should be encoded. Symbols should not be: !@#$%^&*()_+-=";
  urlEncodedString = @"%41%6C%6C %61%6C%70%68%61%62%65%74%69%63 %63%68%61%72%61%63%74%65%72%73 %73%68%6F%75%6C%64 %62%65 "
    @"%65%6E%63%6F%64%65%64. %53%79%6D%62%6F%6C%73 %73%68%6F%75%6C%64 %6E%6F%74 %62%65: !@#$%^&*()_+-=";
  testEncodeDecode(urlEncodedString, urlDecodedString, [[NSCharacterSet alphanumericCharacterSet] invertedSet],
                   @"Skip not alphanumericCharacterSet 03");


  urlDecodedString = @"Here are some Emojis: \U0001F601 \U0001F602 \U0001F638 Emojis done."; // Multibyte encoded characters
  urlEncodedString = @"Here%20are%20some%20Emojis:%20%F0%9F%98%81%20%F0%9F%98%82%20%F0%9F%98%B8%20Emojis%20done.";
  testEncodeDecode(urlEncodedString, urlDecodedString, [NSCharacterSet URLFragmentAllowedCharacterSet],
                   @"Skip URLFragmentAllowedCharacterSet 04");


  urlDecodedString = @"\1";
  urlEncodedString = @"%01";
  testEncodeDecode(urlEncodedString, urlDecodedString, [NSCharacterSet alphanumericCharacterSet],
                   @"Skip URLFragmentAllowedCharacterSet 05");


  urlDecodedString = @"£";
  urlEncodedString = @"%C2%A3";
  testEncodeDecode(urlEncodedString, urlDecodedString, [NSCharacterSet alphanumericCharacterSet],
                   @"Two-byte character 06");


  urlDecodedString = @"€";
  urlEncodedString = @"%E2%82%AC";
  testEncodeDecode(urlEncodedString, urlDecodedString, [NSCharacterSet alphanumericCharacterSet],
                   @"Three-byte character 07");


  urlDecodedString = @"𐍈";
  urlEncodedString = @"%F0%90%8D%88";
  testEncodeDecode(urlEncodedString, urlDecodedString, [NSCharacterSet alphanumericCharacterSet],
                   @"Four-byte character 08");


  //check full string encoding and decoding
  urlDecodedString = @"0123456789 AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz _-~`!#$%&'()*+,/:;=?@[]™…©®£ƒ‰¥§";
  urlEncodedString  = @"%30%31%32%33%34%35%36%37%38%39%20%41%61%42%62%43%63%44%64%45%65%46%66%47%67%48%68%49%69%4A%6A%4B%6B%4C%6C%4D%6D%4E%6E%4F%6F%50%70%51%71%52%72%53%73%54%74%55%75%56%76%57%77%58%78%59%79%5A%7A%20%5F%2D%7E%60%21%23%24%25%26%27%28%29%2A%2B%2C%2F%3A%3B%3D%3F%40%5B%5D%E2%84%A2%E2%80%A6%C2%A9%C2%AE%C2%A3%C6%92%E2%80%B0%C2%A5%C2%A7";
  testEncodeDecode(urlEncodedString, urlDecodedString, [[NSCharacterSet characterSetWithCharactersInString:urlDecodedString] invertedSet],
                   @"All characters in string 09");


  //check decoding of string with an unencoded part at the beginning
  NSString* asIsString = @"0123456789 AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz _-~`!#$&'()*+,/:;=?@[]™…©®£ƒ‰¥§";
  urlDecodedString = [asIsString stringByAppendingString:urlDecodedString];
  urlEncodedString = [asIsString stringByAppendingString:urlEncodedString];
  PASS_EQUAL([urlEncodedString stringByRemovingPercentEncoding], urlDecodedString, "Percent-encoded string decoding 10");


  //check decoding of string with the encoded part in the middle
  urlDecodedString = [urlDecodedString stringByAppendingString:asIsString];
  urlEncodedString = [urlEncodedString stringByAppendingString:asIsString];
  PASS_EQUAL([urlEncodedString stringByRemovingPercentEncoding], urlDecodedString, "Percent-encoded string decoding 11");


  [pool drain];
  return 0;
}
