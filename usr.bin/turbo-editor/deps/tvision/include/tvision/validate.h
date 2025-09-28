/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   VALIDATE.H                                                            */
/*                                                                         */
/*   defines the classes TValidator, TPXPictureValidator, TRangeValidator, */
/*   TFilterValidator, TLookupValidator, and TStringLookupValidator.       */
/*                                                                         */
/* ------------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if defined(Uses_TValidator) && !defined(__TValidator)
#define __TValidator

// TValidator Status constants

static const int
  vsOk     =  0,
  vsSyntax =  1,      // Error in the syntax of either a TPXPictureValidator
                      // or a TDBPictureValidator

// Validator option flags
  voFill     =  0x0001,
  voTransfer =  0x0002,
  voReserved =  0x00FC;

// TVTransfer constants


enum TVTransfer {vtDataSize, vtSetData, vtGetData};

// Abstract TValidator object


class TValidator : public TObject, public TStreamable
{
public:
    TValidator() noexcept;
    virtual void error();
    virtual Boolean isValidInput(char* s, Boolean suppressFill);
    virtual Boolean isValid(const char* s);
    virtual ushort transfer(char *s, void* buffer, TVTransfer flag);
    Boolean validate(const char* s);

    ushort status;
    ushort options;

protected:
    TValidator( StreamableInit ) noexcept;
    virtual void write( opstream& os );
    virtual void* read( ipstream& is );

private:
    virtual const char *streamableName() const  {return name;};

public:
    static TStreamable *build();
    static const char * const _NEAR name;
};

#endif


#if defined(Uses_TPXPictureValidator) && !defined(__TPXPictureValidator)
#define __TPXPictureValidator

// TPXPictureValidator result type

enum TPicResult {prComplete, prIncomplete, prEmpty, prError, prSyntax,
    prAmbiguous, prIncompNoFill};

// TPXPictureValidator


class TPXPictureValidator : public TValidator
{

    static const char * _NEAR errorMsg;

public:
    TPXPictureValidator(TStringView aPic, Boolean autoFill);
    ~TPXPictureValidator();
    virtual void error();
    virtual Boolean isValidInput(char* s, Boolean suppressFill);
    virtual Boolean isValid(const char* s);
    virtual TPicResult picture(char* input, Boolean autoFill);


protected:
    TPXPictureValidator( StreamableInit ) noexcept;
    virtual void write( opstream& os );
    virtual void* read( ipstream& is );

    char* pic;

private:
    void consume(char ch, char* input);
    void toGroupEnd(int& i, int termCh);
    Boolean skipToComma(int termCh);
    int calcTerm(int);
    TPicResult iteration(char* input, int termCh);
    TPicResult group(char* input, int termCh);
    TPicResult checkComplete(TPicResult rslt, int termCh);
    TPicResult scan(char* input, int termCh);
    TPicResult process(char* input, int termCh);
    Boolean syntaxCheck();
    virtual const char *streamableName() const  {return name;};

    int index, jndex;

public:
    static TStreamable *build();
    static const char * const _NEAR name;
};

inline ipstream& operator >> ( ipstream& is, TValidator& v )
    { return is >> (TStreamable&)v; }
inline ipstream& operator >> ( ipstream& is, TValidator*& v )
    { return is >> (void *&)v; }

inline opstream& operator << ( opstream& os, TValidator& v )
    { return os << (TStreamable&)v; }
inline opstream& operator << ( opstream& os, TValidator* v )
    { return os << (TStreamable *)v; }

#endif


#if defined(Uses_TFilterValidator) && !defined(__TFilterValidator)
#define __TFilterValidator

// TFilterValidator

class TFilterValidator : public TValidator
{

    static const char * _NEAR errorMsg;

public:
    TFilterValidator(TStringView aValidChars) noexcept;
    ~TFilterValidator();
    virtual void error();
    virtual Boolean isValidInput(char* s, Boolean suppressFill);
    virtual Boolean isValid(const char* s);

protected:
    TFilterValidator( StreamableInit ) noexcept;
    virtual void write( opstream& os);
    virtual void* read( ipstream& is );

    char* validChars;

private:
    virtual const char *streamableName() const  {return name;};

public:
    static TStreamable *build();
    static const char * const _NEAR name;
};

inline ipstream& operator >> ( ipstream& is, TFilterValidator& v )
    { return is >> (TStreamable&)v; }
inline ipstream& operator >> ( ipstream& is, TFilterValidator*& v )
    { return is >> (void *&)v; }

inline opstream& operator << ( opstream& os, TFilterValidator& v )
    { return os << (TStreamable&)v; }
inline opstream& operator << ( opstream& os, TFilterValidator* v )
    { return os << (TStreamable *)v; }

#endif


#if defined(Uses_TRangeValidator) && !defined(__TRangeValidator)
#define __TRangeValidator

// TRangeValidator


class TRangeValidator : public TFilterValidator
{

    static const char * _NEAR validUnsignedChars;
    static const char * _NEAR validSignedChars;
    static const char * _NEAR errorMsg;

public:
    TRangeValidator(int32_t aMin, int32_t aMax) noexcept;
    virtual void error();
    virtual Boolean isValid(const char* s);
    virtual ushort transfer(char* s, void* buffer, TVTransfer flag);

protected:
    int32_t min;
    int32_t max;

    TRangeValidator( StreamableInit ) noexcept;
    virtual void write( opstream& os );
    virtual void* read( ipstream& is );

private:
    virtual const char *streamableName() const  {return name;};

public:
    static TStreamable *build();
    static const char * const _NEAR name;

};

inline ipstream& operator >> ( ipstream& is, TRangeValidator& v )
    { return is >> (TStreamable&)v; }
inline ipstream& operator >> ( ipstream& is, TRangeValidator*& v )
    { return is >> (void *&)v; }

inline opstream& operator << ( opstream& os, TRangeValidator& v )
    { return os << (TStreamable&)v; }
inline opstream& operator << ( opstream& os, TRangeValidator* v )
    { return os << (TStreamable *)v; }

#endif

#if defined(Uses_TLookupValidator) && !defined(__TLookupValidator)
#define __TLookupValidator

// TLookupValidator

class TLookupValidator : public TValidator
{
public:
    TLookupValidator() noexcept {};
    virtual Boolean isValid(const char* s);
    virtual Boolean lookup(const char* s);
    static TStreamable *build();
    static const char * const _NEAR name;
protected:
    TLookupValidator( StreamableInit ) noexcept;
private:
    virtual const char *streamableName() const  {return name;};
};

inline ipstream& operator >> ( ipstream& is, TLookupValidator& v )
    { return is >> (TStreamable&)v; }
inline ipstream& operator >> ( ipstream& is, TLookupValidator*& v )
    { return is >> (void *&)v; }

inline opstream& operator << ( opstream& os, TLookupValidator& v )
    { return os << (TStreamable&)v; }
inline opstream& operator << ( opstream& os, TLookupValidator* v )
    { return os << (TStreamable *)v; }

#endif


#if defined(Uses_TStringLookupValidator) && !defined(__TStringLookupValidator)
#define __TStringLookupValidator

// TStringLookupValidator

class TStringLookupValidator : public TLookupValidator
{

    static const char * _NEAR errorMsg;

public:
    TStringLookupValidator(TStringCollection* aStrings) noexcept;
    ~TStringLookupValidator();
    virtual void error();
    virtual Boolean lookup(const char* s);

protected:
    TStringLookupValidator( StreamableInit ) noexcept;
    virtual void write( opstream& os );
    virtual void* read( ipstream& is );

    TStringCollection* strings;

private:
    virtual const char *streamableName() const  {return name;};

public:
    void newStringList(TStringCollection* aStrings);
    static TStreamable *build();
    static const char * const _NEAR name;
};


inline ipstream& operator >> ( ipstream& is, TStringLookupValidator& v )
    { return is >> (TStreamable&)v; }
inline ipstream& operator >> ( ipstream& is, TStringLookupValidator*& v )
    { return is >> (void *&)v; }

inline opstream& operator << ( opstream& os, TStringLookupValidator& v )
    { return os << (TStreamable&)v; }
inline opstream& operator << ( opstream& os, TStringLookupValidator* v )
    { return os << (TStreamable *)v; }


#endif
