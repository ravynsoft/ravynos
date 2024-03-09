/*-----------------------------------------------------------------------*/
/*                                                                       */
/*            Turbo Vision Help Compiler Source File                     */
/*                                                                       */
/*-----------------------------------------------------------------------*/
/* $Copyright: 1994 */

/*===== TVHC ============================================================*/
/*  Turbo Vision help file compiler documentation.                       */
/*=======================================================================*/
/*                                                                       */
/*    Refer to DEMOHELP.TXT for an example of a help source file.        */
/*                                                                       */
/*    This program takes a help script and produces a help file (.H16    */
/*    or .H32) and a help context file (.H).  The format for the help    */
/*    file is very simple.  Each context is given a symbolic name        */
/*    (i.e FileOpen) which is then put in the context file (i.e.         */
/*    hcFileOpen).  The text following the topic line is put into the    */
/*    help file.  Since the help file can be resized, some of the text   */
/*    will need to be wrapped to fit into the window.  If a line of      */
/*    text is flush left with no preceeding white space, the line will   */
/*    be wrapped.  All adjacent wrappable lines are wrapped as a         */
/*    paragraph.  If a line begins with a space it will not be           */
/*    wrapped. For example, the following is a help topic for a          */
/*    File|Open menu item.                                               */
/*                                                                       */
/*       |.topic FileOpen                                                */
/*       |  File|Open                                                    */
/*       |  ---------                                                    */
/*       |This menu item will bring up a dialog...                       */
/*                                                                       */
/*    The "File|Open" will not be wrapped with the "----" line since     */
/*    they both begin with a space, but the "This menu..." line will     */
/*    be wrapped.                                                        */
/*      The syntax for a ".topic" line is:                               */
/*                                                                       */
/*        .topic symbol[=number][, symbol[=number][...]]                 */
/*                                                                       */
/*    Note a topic can have multiple symbols that define it so that one  */
/*    topic can be used by multiple contexts.  The number is optional    */
/*    and will be the value of the hcXXX context in the context file     */
/*    Once a number is assigned all following topic symbols will be      */
/*    assigned numbers in sequence.  For example,                        */
/*                                                                       */
/*       .topic FileOpen=3, OpenFile, FFileOpen                          */
/*                                                                       */
/*    will produce the follwing help context number definitions,         */
/*                                                                       */
/*        hcFileOpen  = 3;                                               */
/*        hcOpenFile  = 4;                                               */
/*        hcFFileOpen = 5;                                               */
/*                                                                       */
/*    When building helpfiles for 16-bit programs, a topic should never  */
/*    be assigned a number higher than 16379.  In 32-bit programs there  */
/*    is no such limitation, but it is recommended that the numbers used */
/*    be kept low for efficiency reasons.                                */
/*                                                                       */
/*    Cross references can be imbedded in the text of a help topic which */
/*    allows the user to quickly access related topics.  The format for  */
/*    a cross reference is as follows,                                   */
/*                                                                       */
/*        {text[:alias]}                                                 */
/*                                                                       */
/*    The text in the brackets is highlighted by the help viewer.  This  */
/*    text can be selected by the user and will take the user to the     */
/*    topic by the name of the text.  Sometimes the text will not be     */
/*    the same as a topic symbol.  In this case you can use the optional */
/*    alias syntax.  The symbol you wish to use is placed after the text */
/*    after a ':'. The following is a paragraph of text using cross      */
/*    references,                                                        */
/*                                                                       */
/*      |The {file open dialog:FileOpen} allows you specify which        */
/*      |file you wish to view.  If it also allow you to navigate        */
/*      |directories.  To change to a given directory use the            */
/*      |{change directory dialog:ChDir}.                                */
/*                                                                       */
/*    To escape '{' characters outside cross references, type them       */
/*    twice. To escape ':' or '}' characters inside cross references,    */
/*    also type them twice.                                              */
/*                                                                       */
/*    The user can tab or use the mouse to select more information about */
/*    the "file open dialog" or the "change directory dialog". The help  */
/*    compiler handles forward references so a topic need not be defined */
/*    before it is referenced.  If a topic is referenced but not         */
/*    defined, the compiler will give a warning but will still create a  */
/*    useable help file.  If the undefined reference is used, a message  */
/*    ("No help available...") will appear in the help window.           */
/*                                                                       */
/*    Lines starting with ';' are skipped.                               */
/*=======================================================================*/

#define Uses_fpstream
#define Uses_TSortedCollection
#include <tvision/tv.h>

#if !defined( __TVHC_H )
#include "tvhc.h"
#endif  // __TVHC_H

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

#if !defined( __LIMITS_H )
#include <limits.h>
#endif  // __LIMITS_H

#if !defined __DOS_H
#include <dos.h>
#endif  // __DOS_H

#if !defined( __DIR_H )
#include <dir.h>
#endif  // __DIR_H

#if !defined( __IO_H )
#include <io.h>
#endif  // __IO_H

#if !defined( __CTYPE_H )
#include <ctype.h>
#endif  // __CTYPE_H

#if !defined( __STDLIB_H )
#include <stdlib.h>
#endif  // __STDLIB_H

#if !defined( __FSTREAM_H )
#include <fstream.h>
#endif  // __FSTREAM_H

#if !defined( __STRSTREA_H )
#include <strstrea.h>
#endif  // __STRSTREA_H

#if !defined( __ERRNO_H )
#include <errno.h>
#endif  // __ERRNO_H

#if !defined( __STDIO_H )
#include <stdio.h>
#endif // __STDIO_H

/*
 * Help compiler global variables.
 */
int           ofs;
int           bufferSize = 0;
char          textName[MAXPATH];
uchar         *buffer = 0;
TCrossRefNode *xRefs;
TRefTable     *refTable = 0;
char          line[MAXSTRSIZE] = "";
Boolean       lineInBuffer = False;
int           lineCount = 0;

#ifdef __FLAT__
#define HELPFILE_EXT ".h32"
#define TARGET "(32 bit)"
#else
#define HELPFILE_EXT ".h16"
#define TARGET "(16 bit)"
#endif


//======================= File Management ===============================//

TProtectedStream::TProtectedStream( const char *aFileName, openmode aMode ) :
    fstream( aFileName, aMode )
{
    strnzcpy(fileName, aFileName, sizeof(fileName));
    mode = aMode;
}

void error(const char *text);
void warning(const char *text);

//----- copyPath(dest, src, size) ---------------------------------------//
//  Safely copies a path or terminates on error.                         //
//-----------------------------------------------------------------------//

void copyPath( char *dest, const char *src, size_t size )
{
    if (strnzcpy(dest, src, size) < strlen(src))
        {
        cerr << "Path too long (" << strlen(src) << " > " << size << "): \""
             << src << "\"" << endl;
        exit(1);
        }
}

//----- replaceExt(fileName, nExt, force) -------------------------------//
//  Replace the extension of the given file with the given extension.    //
//  If an extension already exists Force indicates if it should be       //
//  replaced anyway.                                                     //
//-----------------------------------------------------------------------//

const char *replaceExt( const char *fileName, const char *nExt, Boolean force )
{
    char dir[MAXPATH];
    char name[MAXFILE];
    char ext[MAXEXT];
    char drive[MAXDRIVE];
    static char buffer[MAXPATH] = {0};
    ostrstream os(buffer, MAXPATH-1);

    fnsplit(fileName, drive, dir, name, ext);
    if (force || (strlen(ext) == 0))
        {
        os << dir << name << nExt << ends;
        return os.str();
        }
    else
        return fileName;
}

//----- fExist(fileName) ------------------------------------------------/
//  Returns true if the file exists false otherwise.                     /
//-----------------------------------------------------------------------/

Boolean fExists(const char *fileName)
{
    struct ffblk ffblk;

    if (findfirst(fileName,&ffblk,0))
        return(False);
    else
        return(True);
}

//----- isComment(line) -------------------------------------------------//
//  Checks if line contains a comment.                                   //
//-----------------------------------------------------------------------//

int isComment(const char *line)
{
    return line[0] == ';';
}

//----- getLine(s) ------------------------------------------------------//
//  Returns the next line out of the stream.                             //
//-----------------------------------------------------------------------//

char *getLine( fstream& s )
{
    if (s.eof())
        {
        strnzcpy(line, "\x1A", sizeof(line));
        return line;
        }
    do  {
        ++lineCount;
        if (!lineInBuffer)
            {
            s.getline(line, sizeof(line), '\n');
            // Remove carriage return at the end of line.
                {
                int len = strlen(line);
                if (len && line[len - 1] == '\r')
                    line[len - 1] = '\0';
                }
            // Detect truncation.
            if ((s.rdstate() & (ios::failbit | ios::eofbit)) == ios::failbit)
                {
                if (!isComment(line))
                    {
                    char buf[MAXSTRSIZE] = {0};
                    ostrstream os(buf, sizeof(buf)-1);
                    os << "Line longer than " << (sizeof(line)-1) << " characters.";
                    warning(os.str());
                    }
                // Read the rest of the line.
                do  {
                    char buf[MAXSTRSIZE] = {0};
                    s.clear(s.rdstate() & ~ios::failbit);
                    s.getline(buf, sizeof(buf), '\n');
                    } while ((s.rdstate() & (ios::failbit | ios::eofbit)) == ios::failbit);
                }
            }
        lineInBuffer = False;
        // Skip line if it is a comment.
        } while (isComment(line));
    return line;
}

//----- unGetLine(s) ----------------------------------------------------//
//  Return given line into the stream.                                   //
//-----------------------------------------------------------------------//

void unGetLine( const char *s )
{
    strnzcpy(line, s, sizeof(line));
    lineInBuffer = True;
    --lineCount;
}

//========================= Error routines ==============================//

//----- prntMsg(text) ---------------------------------------------------//
//  Used by Error and Warning to print the message.                      //
//-----------------------------------------------------------------------//

void prntMsg( const char *pref, const char *text )
{
    if (lineCount > 0)
        cerr << pref << ": " << textName << "("
             << lineCount << "): " << text << "\n";
    else
        cerr << pref << ": " << textName << " "
             << text << "\n";
}

//----- error(text) -----------------------------------------------------//
//  Used to indicate an error.  Terminates the program                   //
//-----------------------------------------------------------------------//

void error( const char *text )
{
    prntMsg("Error", text);
    exit(1);
}

//----- warning(text) ---------------------------------------------------//
//  Used to indicate an warning.                                         //
//-----------------------------------------------------------------------//

void warning( const char *text )
{
    prntMsg("Warning", text);
}

//====================== Topic Reference Management =====================//

void disposeFixUps( TFixUp *&p )
{
    TFixUp *q;

    while (p != 0)
        {
        q = p->next;
        delete p;
        p = q;
        }
}

//----- TRefTable -------------------------------------------------------//
//  TRefTable is a collection of TReference pointers used as a symbol    //
//  table. If the topic has not been seen, a forward reference is        //
//  inserted and a fix-up list is started.  When the topic is seen all   //
//  forward references are resolved.  If the topic has been seen already //
//  the value it has is used.                                            //
//-----------------------------------------------------------------------//

TRefTable::TRefTable( ccIndex aLimit, ccIndex aDelta ) :
     TSortedCollection( aLimit, aDelta )
{
}

int TRefTable::compare( void *key1, void *key2 )
{
    int compValue;


    compValue = strcmp( (const char *)key1, (const char *)key2 );
    if (compValue > 0)
        return 1;
    else if (compValue < 0)
        return (-1);
    else
        return(0);
}

void TRefTable::freeItem( void *item )
{
    TReference *ref;

    ref = (TReference *) item;
    if (ref->resolved == False)
        disposeFixUps(ref->val.fixUpList);
    delete ref->topic;
    delete ref;
}

TReference *TRefTable::getReference( const char *topic )
{
    TReference *ref;
    int i;

    if (search((void *) topic, i))
        ref = (TReference *) at(i);
    else
        {
        ref =  new TReference;
        ref->topic =  newStr(topic);
        ref->resolved = False;
        ref->val.fixUpList = 0;
        insert(ref);
        }
    return(ref);
}

void* TRefTable::keyOf( void *item )
{
    return(((TReference *)item)->topic);
}

//----- initRefTable ---------------------------------------------------//
//  Make sure the reference table is initialized.                       //
//----------------------------------------------------------------------//

void initRefTable()
{
    if (refTable == 0)
        refTable = new TRefTable(5,5);
}

//---- RecordReference -------------------------------------------------//
//  Record a reference to a topic to the given stream.  This routine    //
//  handles forward references.                                         //
//----------------------------------------------------------------------//

void recordReference( const char *topic, opstream& s )
{
    int i;
    TReference *ref;
    TFixUp *fixUp;

    initRefTable();
    ref = refTable->getReference(topic);
    if (ref->resolved == True)
        s << ref->val.value;
    else
        {
        fixUp =  new TFixUp;
        fixUp->pos = s.tellp();
        i = -1;
        s << i;
        fixUp->next = ref->val.fixUpList;
        ref->val.fixUpList = fixUp;
        }
}

void doFixUps( TFixUp *p, uint value, iopstream& s )
{
    streampos pos;

    for(pos = s.tellp(); (p != 0); p = p->next)
        {
        s.seekp(p->pos);
        s << value;
        }
    s.seekp(pos);
}

//----- resolveReference -----------------------------------------------//
//  Resolve a reference to a topic to the given stream.  This function  //
//  handles forward references.                                         //
//----------------------------------------------------------------------//

void resolveReference( const char *topic, uint value, iopstream& s )
{
    TReference *ref;

    initRefTable();
    ref = refTable->getReference(topic);
    if (ref->resolved)
        {
        char bufStr[MAXSTRSIZE] = "Redefinition of ";
        strncat(bufStr, ref->topic, MAXSTRSIZE-1);
        error(bufStr);
        }
    else
        {
        doFixUps(ref->val.fixUpList,value,s);
        disposeFixUps(ref->val.fixUpList);
        ref->resolved = True;
        ref->val.value = value;
        }
}

//======================= Help file parser =============================//

void skipWhite( const char *line, int& i )
{
    int len = strlen(line);
    while (i <= len && (line[i] == ' ' || line[i] == '\t'))
        ++i;
}

int checkForValidChar( char ch )
{
    if (isalnum(ch) || ch  == '_')
        return(0);
    return(-1);
}


void skipToNonWord( const char *line, int& i )
{
    int len = strlen(line);
    while (i <= len && !checkForValidChar(line[i]))
        ++i;
}

//----- getWord --------------------------------------------------------//
//   Extract the next word from the given line at offset i.             //
//----------------------------------------------------------------------//

const char *getWord( const char *line, int &i )
{
    int j;
    const char *strptr;
    static char getword[MAXSTRSIZE] = {0};

    skipWhite(line,i);
    j = i;
    if (j > (int) strlen(line))
        strncpy(getword, "", MAXSTRSIZE-1);
    else
        {
        ++i;
        if (!checkForValidChar(line[j]))
            skipToNonWord(line, i);
        strptr = line + j;
        strncpy(getword,strptr,i - j);
        getword[i-j] = '\0';
        }
    return getword;
}

//---- topicDefinition -------------------------------------------------//
//  Extracts the next topic definition from the given line at i.        //
//----------------------------------------------------------------------//

TTopicDefinition::TTopicDefinition( const char *aTopic, uint aValue )
{
    topic = newStr(aTopic);
    value = aValue;
    next = 0;
}

TTopicDefinition::~TTopicDefinition()
{
    delete[] topic;
    if (next != 0)
        delete next;
}

int is_numeric(const char *str)
{
    int len = strlen(str);

    for(int i = 0; i < len; ++i)
        if (!isdigit(str[i]))
            return 0;
    return 1;
}

TTopicDefinition *topicDefinition( const char *line, int& i )
{
    int j;
    char topic[MAXSTRSIZE], w[MAXSTRSIZE], *endptr;
    static unsigned helpCounter = 2; //1 is hcDragging

    strnzcpy(topic, getWord(line, i), sizeof(topic));
    if (strlen(topic) == 0)
        {
        error("Expected topic definition");
        return(0);
        }
    else
        {
        j = i;
        strnzcpy(w, getWord(line, j), sizeof(w));
        if (strcmp(w,"=") == 0)
            {
            i = j;
            strnzcpy(w, getWord(line, i), sizeof(w));
            if (!is_numeric(w))
                error("Expected numeric");
            else
                helpCounter = (int) strtol(w, &endptr,10);
            }
        else
            ++helpCounter;

        if (helpCounter > MAXHELPTOPICID)
            {
            char buf[MAXSTRSIZE] = {0};
            ostrstream os( buf, sizeof(buf)-1 );

            os << "Topic id for topic '" << topic
               << "' exceeds limit of " << MAXHELPTOPICID << ends;

            error(buf);
            return 0;
            }

        return(new TTopicDefinition(topic, helpCounter));
        }
}

//---- topicDefinitionList----------------------------------------------//
//  Extracts a list of topic definitions from the given line at i.      //
//----------------------------------------------------------------------//

TTopicDefinition *topicDefinitionList( const char *line, int &i )
{
    int j;
    char w[MAXSTRSIZE];
    TTopicDefinition *topicList, *p;

    j = i;
    topicList = 0;
    do  {
        i = j;
        p = topicDefinition(line, i);
        if (p == 0 )
            {
            if (topicList != 0)
                delete topicList;
            return(0);
            }
        p->next = topicList;
        topicList = p;
        j = i;
        strnzcpy(w, getWord(line, j), sizeof(w));
        } while ( strcmp(w,",") == 0);
    return(topicList);
}

//---- topicHeader -----------------------------------------------------//
//  Parse a Topic header                                                //
//----------------------------------------------------------------------//

TTopicDefinition *topicHeader( const char *line )
{
    int i;
    char w[75];

    i = 0;
    strnzcpy(w, getWord(line, i), sizeof(w));
    if (strcmp(w, commandChar) != 0)
        return(0);
    strnzcpy(w, getWord(line, i), sizeof(w));
    strupr(w);
    if (strcmp(w, "TOPIC") == 0)
        return topicDefinitionList(line, i);
    else
        {
        error("TOPIC expected");
        return(0);
        }
}

void growBuffer( int size )
{
    if (size <= bufferSize)
        return;
    do  {
        bufferSize = bufferSize ? 2*bufferSize : 4096;
        } while (size > bufferSize);
    uchar *ptr = (uchar *) realloc(buffer, bufferSize);
    if (ptr)
        buffer = ptr;
    else
        error("Text too long");
}

void addToBuffer( const char *line, Boolean wrapping )
{
    int len = strlen(line);
    int nOfs = ofs + len + 1;
    growBuffer(nOfs);
    memcpy(&buffer[ofs], line, len + 1);
    buffer[nOfs - 1] = wrapping ? ' ' : '\n';
    ofs = nOfs;
}


void addXRef( TStringView xRef, int offset, uchar length, TCrossRefNode *&xRefs )
{
    TCrossRefNode *p, *pp, *prev;

    p =  new TCrossRefNode;
    p->topic = newStr(xRef);
    p->offset = offset;
    p->length = length;
    p->next = 0;
    if (xRefs == 0)
        xRefs = p;
    else
        {
        pp = xRefs;
        prev = pp;
        while (pp != 0)
            {
            prev = pp;
            pp = pp->next;
            }
        prev->next = p;
        }
}

void replaceSpacesWithFF( char *line, int start, uchar length )
{
    int i;

    for(i = start; i <= (start + length); ++i)
        if (line[i] == ' ')
            line[i] = -1;
}

void strdel(char *string, int pos, int len)
{
    char *beg = string + pos;
    char *end = string + pos + len;
    memmove(beg, end, strlen(end) + 1);
}

char *strfnd( char *string, char **last, char ch )
{
    char *res;
    while ((res = strchr(string, ch)) != 0)
        {
        if ((!last || res < *last) && *(res + 1) == ch)
            {
            strdel(string, res - string, 1);
            string = res + 1;
            if (last) --*last;
            }
        else
            break;
        }
    return res;
}

void scanForCrossRefs( char *line, int& offset, TCrossRefNode *&xRefs )
{
    const char begXRef = '{';
    const char endXRef = '}';
    const char aliasCh = ':';

    size_t i = 0;
    do  {
        char *begPtr;
        if ((begPtr = strfnd(line+i, 0, begXRef)) == 0)
            i = 0;
        else
            {
            ++begPtr; // *begPtr == character after '{'.
            i = begPtr - line; // line[i] == *begPtr.
            char *endPtr;
            if ((endPtr = strfnd(begPtr, 0, endXRef)) == 0)
                error("Unterminated topic reference.");
            else // *endPtr == '}'.
                {
                char *aliasPtr = strfnd(begPtr, &endPtr, aliasCh);
                if ((aliasPtr == 0) || (aliasPtr > endPtr)) // No alias.
                    {
                    TStringView xRef(begPtr, endPtr - begPtr);
                    uchar len = uchar(xRef.size()); // Highlight length matches reference length.
                    addXRef(xRef, (offset + ofs + i), len, xRefs);
                    }
                else // *aliasPtr == ':'.
                    {
                    TStringView xRef(aliasPtr + 1, endPtr - (aliasPtr + 1));
                    uchar len = uchar(aliasPtr - begPtr);
                    addXRef(xRef, (offset + ofs + i), len, xRefs);
                    strdel(line, aliasPtr - line, endPtr - aliasPtr); // Remove ':'.
                    endPtr = aliasPtr; // *endPtr == '}'.
                    }
                uchar len = uchar(endPtr - begPtr);
                replaceSpacesWithFF(line, i, len);
                strdel(line, i + len, 1); // Remove '}'.
                strdel(line, i - 1, 1); // Remove '{'.
                i = endPtr - line - 2;
                }
            }

        } while (i != 0);
}


Boolean isEndParagraph( State state )
{
    int flag;
    int wrapping = 1;
    int notWrapping = 2;

    flag =
          ((line[0] == 0) ||
           (line[0] == commandChar[0]) ||
       (line[0] == 26) ||
           ((line[0] ==  ' ') && (state == wrapping)) ||
           ((line[0] != ' ') && (state == notWrapping)));
    if (flag)
        return(True);
    else
        return(False);
}

//---- readParagraph ----------------------------------------------------//
// Read a paragraph of the screen.  Returns the paragraph or 0 if the    //
// paragraph was not found in the given stream.  Searches for cross      //
// references and updates the xRefs variable.                            //
//-----------------------------------------------------------------------//

TParagraph *readParagraph( fstream& textFile, int& offset, TCrossRefNode *&xRefs )
{
    State state;
    Boolean flag;
    char line[MAXSTRSIZE];
    TParagraph *p;

    ofs = 0;
    state = undefined;
    strnzcpy(line, getLine(textFile), sizeof(line));
    while (strlen(line) == 0)
        {
        flag = (state == wrapping)? True: False;
        addToBuffer(line, flag);
        strnzcpy(line, getLine(textFile), sizeof(line));
        }

    if (isEndParagraph(state) == True)
        {
        unGetLine(line);
        return(0);
        }
    while (isEndParagraph(state) == False)
        {
        if (state == undefined)
            {
            if (line[0] == ' ')
                state = notWrapping;
            else
                state = wrapping;
            }
        scanForCrossRefs(line, offset, xRefs);
        flag = (state == wrapping)? True: False;
        addToBuffer(line, flag);
        strnzcpy(line, getLine(textFile), sizeof(line));
        }
    unGetLine(line);
    p = new TParagraph;
    p->size = ofs;
    p->wrap = (state == wrapping)? True: False;
    p->text = new char[ofs];
    memmove(p->text, buffer, ofs);
    p->next = 0;
    offset += ofs;
    return(p);
}

void _FAR handleCrossRefs( opstream& s, int xRefValue )
{
    TCrossRefNode *p;

    for (p = xRefs; xRefValue > 0; --xRefValue)
        {
        if (p != 0)
            p = p->next;
        }
    if (p != 0)
        recordReference( p->topic, s );
}

void skipBlankLines( fstream& s )
{
    char line[MAXSTRSIZE];

    line[0] = 0;
    while (line[0] == 0)
        strnzcpy(line, getLine(s), sizeof(line));
    unGetLine(line);
}

int xRefCount()
{
    int i;
    TCrossRefNode *p;

    i = 0;
    for (p=xRefs; (p != 0); p=p->next)
        ++i;
    return(i);
}

void disposeXRefs( TCrossRefNode  *p )
{
    TCrossRefNode *q;

    while (p != 0)
        {
        q = p;
        p = p->next;
        delete[] q->topic;
        delete q;
        }
}

void recordTopicDefinitions( TTopicDefinition *p, THelpFile& helpFile )
{
    while (p != 0)
        {
        resolveReference(p->topic, p->value, *(helpFile.stream));
        helpFile.recordPositionInIndex(p->value);
        p = p->next;
        }
}

//---- readTopic -------------------------------------------------------//
// Read a topic from the source file and write it to the help file      //
//----------------------------------------------------------------------//

void readTopic( fstream& textFile, THelpFile& helpFile )
{
    TParagraph *p;
    THelpTopic *topic;
    TTopicDefinition *topicDef;
    int i, j, offset;
    TCrossRef ref;
    TCrossRefNode  *refNode;

    // Get screen command
    skipBlankLines(textFile);
    strnzcpy(line, getLine(textFile), sizeof(line));

    topicDef = topicHeader(line);

    topic = new THelpTopic;

    // read paragraphs
    xRefs = 0;
    offset = 0;
    p = readParagraph(textFile, offset, xRefs);
    while (p != 0)
        {
        topic->addParagraph(p);
        p = readParagraph(textFile, offset, xRefs);
        }

    i = xRefCount();
    topic->setNumCrossRefs(i);
    refNode = xRefs;
    for( j = 0; j < i; ++j)
        {
        ref.offset = refNode->offset;
        ref.length = refNode->length;
        ref.ref = j;
        topic->setCrossRef(j, ref);
        refNode = refNode->next;
        }

    recordTopicDefinitions(topicDef, helpFile);

    crossRefHandler = handleCrossRefs;
    helpFile.putTopic(topic);


    if (topic != 0)
    delete topic;
    if (topicDef != 0)
    delete topicDef;
    disposeXRefs(xRefs);

    skipBlankLines(textFile);
}

void _FAR doWriteSymbol(void *p, void *p1)
{
    int numBlanks, i;
    ostrstream os(line, sizeof(line)-1);

    TProtectedStream *symbFile = (TProtectedStream *)p1;
    if (((TReference *)p)->resolved)
        {
        os << "\n  hc" << (char *)((TReference *)p)->topic;
        numBlanks = 20 - strlen((char *)((TReference *)p)->topic);
        for (i = 0; i < numBlanks; ++i)
            os << ' ';
        os << " = " << ((TReference *)p)->val.value << ","<< ends;
        *symbFile << os.str();
        }
    else
        {
        os << "Unresolved forward reference \""
           << ((TReference *)p)->topic << "\"" << ends;
        warning(os.str());
        }
}

//---- writeSymbFile ---------------------------------------------------//
// Write the .H file containing all screen titles as constants.         //
//----------------------------------------------------------------------//

void writeSymbFile( TProtectedStream *symbFile )
{
    char header1[] = "const int";

    *symbFile << header1;
    refTable->forEach(doWriteSymbol, symbFile);
    symbFile->seekp(-1L, ios::end);
    *symbFile << ";\n";

}

//---- processtext -----------------------------------------------------//
// Compile the given stream, and output a help file.                    //
//----------------------------------------------------------------------//

void processText( TProtectedStream& textFile,
                  iopstream& helpFile,
                  TProtectedStream& symbFile )
{
    THelpFile *helpRez;

    helpRez =  new THelpFile(helpFile);

    while (!textFile.eof())
        readTopic(textFile, *helpRez);
    writeSymbFile(&symbFile);
    delete helpRez;
}

//---- checkOverwrite --------------------------------------------------//
// Check whether the output file name exists.  If it does, ask whether  //
// it's ok to overwrite it.                                             //
//----------------------------------------------------------------------//

void checkOverwrite( const char *fName )
{
    if (fExists(fName))
        {
        cerr << "File already exists: " << fName << ".  Overwrite? (y/n) ";
        char ch;
        if( scanf(" %c", &ch) != 1 || toupper(ch) != 'Y' )
            exit(1);
        }
}

//========================== Program Block ==========================//

int main(int argc, char **argv)
{
    char helpName[MAXPATH];
    char symbName[MAXPATH];
    fpstream* helpStrm;

    // Banner messages
    char initialText[] = "Help Compiler " TARGET "  Version 2.0  Copyright (c) 1994"
                         " Borland International.\n";
    char helpText[] =
       "\n  Syntax  TVHC <Help text>[.txt] [<Help file>[" HELPFILE_EXT "] [<Symbol file>[.h]]\n"
       "\n"
       "     Help text   = Help file source\n"
       "     Help file   = Compiled help file\n"
       "     Symbol file = An include file containing all the screen names as const's\n";

    cout << initialText;
    if (argc < 2)
        {
        cout << helpText;
        exit(1);
        }

    //  Calculate file names
    copyPath(textName, replaceExt(argv[1], ".txt", False), sizeof(textName));
    if (!fExists(textName))
        {
        cerr << "Error: File '" << textName << "' not found." << endl;
        exit(1);
        }

    if (argc >= 3)
        copyPath(helpName, replaceExt(argv[2], HELPFILE_EXT, False), sizeof(helpName));
    else
        copyPath(helpName, replaceExt(textName, HELPFILE_EXT, True), sizeof(helpName));

    checkOverwrite( helpName );

    if (argc >= 4)
        copyPath(symbName, replaceExt(argv[3], ".h", False), sizeof(symbName));
    else
        copyPath(symbName, replaceExt(helpName, ".h", True), sizeof(symbName));

    checkOverwrite( symbName );

    TProtectedStream textStrm(textName, ios::in);
    TProtectedStream symbStrm(symbName, ios::out);

    helpStrm =  new fpstream(helpName, ios::out|ios::binary);
    processText(textStrm, *helpStrm, symbStrm);
    return 0;
}
