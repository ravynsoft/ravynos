#import <Foundation/Foundation.h>
#import "Testing.h"
#import "ObjectTesting.h"


int main()
{
  START_SET("test1")
  NSURL *url;

  url = [NSURL URLWithString:
    @"scheme://user:password@host.domain.org:888/path/absfile.htm"];
  url = [NSURL URLWithString:
@"file%20name.htm;param1;param2?something=other&andmore=more#fragments"
    relativeToURL: url];

  PASS_EQUAL([url description],
    @"file%20name.htm;param1;param2?something=other&andmore=more#fragments -- scheme://user:password@host.domain.org:888/path/absfile.htm",
    "description ok");

  PASS_EQUAL([url absoluteString],
    @"scheme://user:password@host.domain.org:888/path/file%20name.htm;param1;param2?something=other&andmore=more#fragments",
    "absolute string ok");

  PASS_EQUAL([[url absoluteURL] description],
    @"scheme://user:password@host.domain.org:888/path/file%20name.htm;param1;param2?something=other&andmore=more#fragments",
    "absolute url description ok");

  PASS_EQUAL([[url baseURL] description],
    @"scheme://user:password@host.domain.org:888/path/absfile.htm",
    "base url description ok");

  PASS_EQUAL([url fragment], @"fragments", "fragment ok");

  PASS_EQUAL([url host], @"host.domain.org", "host ok");
  PASS (NO == [url isFileURL], "is not a file url");
  PASS_EQUAL([url parameterString], @"param1;param2", "parameter string ok");
  PASS_EQUAL([url password], @"password", "password ok");
  PASS_EQUAL([url path], @"/path/file name.htm", "path ok");
  PASS_EQUAL([url port], [NSNumber numberWithInt:888], "port ok");
  PASS_EQUAL([url query], @"something=other&andmore=more", "query ok");
  PASS_EQUAL([url relativePath], @"file name.htm", "relativePath ok");
  PASS_EQUAL([url relativeString],
    @"file%20name.htm;param1;param2?something=other&andmore=more#fragments",
   "relativeString ok");
  PASS_EQUAL([url resourceSpecifier],
    @"file%20name.htm;param1;param2?something=other&andmore=more#fragments",
    "resourceSpecifier ok");
  PASS_EQUAL([url scheme], @"scheme", "scheme ok");
  PASS_EQUAL([[url standardizedURL] absoluteString],
@"scheme://user:password@host.domain.org:888/path/file%20name.htm;param1;param2?something=other&andmore=more#fragments",
    "standardizedURL ok");
  PASS_EQUAL([url user], @"user", "user ok");
  END_SET("test1")


  START_SET("test2")
  NSURL *url = [NSURL URLWithString: @"data:,A%20brief%20note"];
  PASS_EQUAL([url scheme], @"data", "test data");
  PASS_EQUAL([url absoluteString], @"data:,A%20brief%20note",
    "data absolute string ok");
  END_SET("test2")

  START_SET("test3")
  NSURL *url = [NSURL URLWithString: @"data:image/gif;base64,R0lGODdhMAAwAPAAAAAAAP///ywAAAAAMAAwAAAC8IyPqcvt3wCcDkiLc7C0qwyGHhSWpjQu5yqmCYsapyuvUUlvONmOZtfzgFzByTB10QgxOR0TqBQejhRNzOfkVJ+5YiUqrXF5Y5lKh/DeuNcP5yLWGsEbtLiOSpa/TPg7JpJHxyendzWTBfX0cxOnKPjgBzi4diinWGdkF8kjdfnycQZXZeYGejmJlZeGl9i2icVqaNVailT6F5iJ90m6mvuTS4OK05M0vDk0Q4XUtwvKOzrcd3iq9uisF81M1OIcR7lEewwcLp7tuNNkM3uNna3F2JQFo97Vriy/Xl4/f1cf5VWzXyym7PHhhx4dbgYKAAA7"];
  PASS_EQUAL([url scheme], @"data", "longer base64 data url ok");
  END_SET("test3")

  START_SET("test4")
  NSURL *url = [NSURL URLWithString: @"data:,A%20brief%20note"
    relativeToURL: [NSURL URLWithString: @"data:other"]];
  PASS_EQUAL([url scheme], @"data", "relative data url ok");
  PASS_EQUAL([url absoluteString], @"data:,A%20brief%20note",
    "data absoluteString");
  END_SET("test4")

  START_SET("test4b")
  NSURL *url = [NSURL URLWithString: @"data:,A%20brief%20note"
    relativeToURL: [NSURL URLWithString: @"file://localhost/"]];
  PASS_EQUAL([url absoluteString], @"data:,A%20brief%20note",
    "data absoluteString");
  END_SET("test4b")

  START_SET("test5")
#ifdef	_WIN32
GSPathHandling("unix");
#endif
  NSURL *url = [NSURL fileURLWithPath: @"/this#is a Path with % < > ?"];
  PASS_EQUAL([url scheme], @"file", "scheme");
  PASS([url host] == nil, "host");
  PASS(nil == [url user], "user");
  PASS(nil == [url password], "password");
  PASS_EQUAL([url resourceSpecifier],
    @"/this%23is%20a%20Path%20with%20%25%20%3C%20%3E%20%3F",
    "resourceSpecifier");
  PASS_EQUAL([url path], @"/this#is a Path with % < > ?", "path");
  PASS(nil == [url query], "query");
  PASS(nil == [url parameterString], "parameterString");
  PASS(nil == [url fragment], "fragment");
  PASS_EQUAL([url absoluteString],
    @"file:///this%23is%20a%20Path%20with%20%25%20%3C%20%3E%20%3F",
    "absoluteString");
  PASS_EQUAL([url relativePath], @"/this#is a Path with % < > ?",
    "relativePath");
  PASS_EQUAL([url description],
    @"file:///this%23is%20a%20Path%20with%20%25%20%3C%20%3E%20%3F",
    "description");
  END_SET("test5")

  START_SET("test5b")
  NSURL *url = [NSURL URLWithString:
    @"file:///pathtofile;parameters?query#anchor"];
  PASS([url isFileURL], "file url");
  PASS_EQUAL([url scheme], @"file", "file scheme");
  PASS(nil == [url host], "host");
  PASS(nil == [url user], "user");
  PASS(nil == [url password], "password");
  PASS_EQUAL([url resourceSpecifier],
    @"/pathtofile;parameters?query#anchor",
    "resourceSpecifier");
  PASS_EQUAL([url path], @"/pathtofile", "path");
  PASS_EQUAL([url query], @"query", "query");
  PASS_EQUAL([url parameterString], @"parameters", "parameterString");
  PASS_EQUAL([url fragment], @"anchor", "fragment");
  PASS_EQUAL([url absoluteString],
    @"file:///pathtofile;parameters?query#anchor",
    "absoluteString");
  PASS_EQUAL([url relativePath], @"/pathtofile", "relativePath");
  PASS_EQUAL([url description], @"file:///pathtofile;parameters?query#anchor",
    "description");
  END_SET("test5b")

  START_SET("test5c")
  // can't initialize with spaces (must be %20)
  NSURL *url = [NSURL URLWithString:
    @"file:///pathtofile; parameters? query #anchor"];
  PASS(nil == url, "spaces not allowed in file url built with string");
  END_SET("test5c")

  START_SET("test5d")
  NSURL *url = [NSURL URLWithString:
    @"file:///pathtofile;%20parameters?%20query%20#anchor"];
  PASS(nil != url, "url");
  END_SET("test5d")

  START_SET("test6")
  // empty string is invalid - should return nils
  NSURL *url = [NSURL URLWithString: @""];
  PASS(nil == [url path], "empty string gives nil");
  END_SET("test6")

  return 0;
}
