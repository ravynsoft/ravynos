/*
    Copyright (c) Microsoft Corporation

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
    associated documentation files (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial
    portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
    NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "tessellator.hpp"
#include "util/macros.h"
#if defined(_MSC_VER)
#include <math.h> // ceil
#else
#include <cmath>
#endif
#define min(x,y) (x < y ? x : y)
#define max(x,y) (x > y ? x : y)

//=================================================================================================================================
// Some D3D Compliant Float Math (reference rasterizer implements these in RefALU class)
//=================================================================================================================================
//
//---------------------------------------------------------------------------------------------------------------------------------
// isNaN
//---------------------------------------------------------------------------------------------------------------------------------

union fiu {
    float f;
    int i;
};

static bool tess_isNaN( float a )
{
    static const int exponentMask = 0x7f800000;
    static const int mantissaMask = 0x007fffff;
    union fiu fiu;
    fiu.f = a;
    return ( ( ( fiu.i & exponentMask ) == exponentMask ) && ( fiu.i & mantissaMask ) ); // NaN
}

//---------------------------------------------------------------------------------------------------------------------------------
// flush (denorm)
//---------------------------------------------------------------------------------------------------------------------------------
static float tess_flush( float a )
{
    static const int minNormalizedFloat = 0x00800000;
    static const int signBit = 0x80000000;
    static const int signBitComplement = 0x7fffffff;
    union fiu fiu, uif;
    fiu.f = a;
    int b = fiu.i & signBitComplement; // fabs()
    if( b < minNormalizedFloat ) // UINT comparison. NaN/INF do test false here
    {
        b = signBit & (fiu.i);
        uif.i = b;
        return uif.f;
    }
    return a;
}

//---------------------------------------------------------------------------------------------------------------------------------
// IEEE754R min
//---------------------------------------------------------------------------------------------------------------------------------
static float tess_fmin( float a, float b )
{
    float _a = tess_flush( a );
    float _b = tess_flush( b );
    if( tess_isNaN( _b ) )
    {
        return a;
    }
    else if( ( _a == 0 ) && ( _b == 0 ) )
    {
        union fiu fiu;
        fiu.f = _a;
        return ( fiu.i & 0x80000000 ) ? a : b;
    }
    return _a < _b ? a : b;
}

//---------------------------------------------------------------------------------------------------------------------------------
// IEEE754R max
//---------------------------------------------------------------------------------------------------------------------------------
static float tess_fmax( float a, float b )
{
    float _a = tess_flush( a );
    float _b = tess_flush( b );

    if( tess_isNaN( _b ) )
    {
        return a;
    }
    else if( ( _a == 0 ) && ( _b == 0 ) )
    {
        union fiu fiu;
        fiu.f = _b;
        return ( fiu.i & 0x80000000 ) ? a : b;
    }
    return _a >= _b ? a : b;
}

//=================================================================================================================================
// Fixed Point Math
//=================================================================================================================================

//-----------------------------------------------------------------------------------------------------------------------------
// floatToFixedPoint
//
// Convert 32-bit float to 32-bit fixed point integer, using only
// integer arithmetic + bitwise operations.
//
// c_uIBits:  UINT8     : Width of i (aka. integer bits)
// c_uFBits:  UINT8     : Width of f (aka. fractional bits)
// c_bSigned: bool      : Whether the integer bits are a 2's complement signed value
// input:     float     : All values valid.
// output:    INT32     : At most 24 bits from LSB are meaningful, depending
//                        on the fixed point bit representation chosen (see
//                        below).  Extra bits are sign extended from the most
//                        meaningful bit.
//
//-----------------------------------------------------------------------------------------------------------------------------

typedef unsigned char UINT8;
typedef int INT32;
template< const UINT8 c_uIBits, const UINT8 c_uFBits, const bool c_bSigned >
INT32 floatToIDotF( const float& input )
{
    // ------------------------------------------------------------------------
    //                                                output fixed point format
    // 32-bit result:
    //
    //      [sign-extend]i.f
    //      |              |
    //      MSB(31)...LSB(0)
    //
    //      f               fractional part of the number, an unsigned
    //                      value with _fxpFracBitCount bits (defined below)
    //
    //      .               implied decimal
    //
    //      i               integer part of the number, a 2's complement
    //                      value with _fxpIntBitCount bits (defined below)
    //
    //      [sign-extend]   MSB of i conditionally replicated
    //
    // ------------------------------------------------------------------------
    // Define fixed point bit counts
    //

    // Commenting out C_ASSERT below to minimise #includes:
    // C_ASSERT( 2 <= c_uIBits && c_uIBits <= 32 && c_uFBits <= 32 && c_uIBits + c_uFBits <= 32 );

    // Define most negative and most positive fixed point values
    const INT32 c_iMinResult = (c_bSigned ? INT32( -1 ) << (c_uIBits + c_uFBits - 1) : 0);
    const INT32 c_iMaxResult = ~c_iMinResult;

    // ------------------------------------------------------------------------
    //                                                constant float properties
    // ------------------------------------------------------------------------
    const UINT8 _fltMantissaBitCount = 23;
    const UINT8 _fltExponentBitCount = 8;
    const INT32 _fltExponentBias     = (INT32( 1 ) << (_fltExponentBitCount - 1)) - 1;
    const INT32 _fltHiddenBit        = INT32( 1 ) << _fltMantissaBitCount;
    const INT32 _fltMantissaMask     = _fltHiddenBit - 1;
    const INT32 _fltExponentMask     = ((INT32( 1 ) << _fltExponentBitCount) - 1) << _fltMantissaBitCount;
    const INT32 _fltSignBit          = INT32( 1 ) << (_fltExponentBitCount + _fltMantissaBitCount);

    // ------------------------------------------------------------------------
    //              define min and max values as floats (clamp to these bounds)
    // ------------------------------------------------------------------------
    INT32 _fxpMaxPosValueFloat;
    INT32 _fxpMaxNegValueFloat;

    if (c_bSigned)
    {
        // The maximum positive fixed point value is 2^(i-1) - 2^(-f).
        // The following constructs the floating point bit pattern for this value,
        // as long as i >= 2.
        _fxpMaxPosValueFloat = (_fltExponentBias + c_uIBits - 1) <<_fltMantissaBitCount;
        const INT32 iShift = _fltMantissaBitCount + 2 - c_uIBits - c_uFBits;
        if (iShift >= 0)
        {
//            assert( iShift < 32 );
#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4293 26452 )
#endif
            _fxpMaxPosValueFloat -= INT32( 1 ) << iShift;
#if defined(_MSC_VER)
#pragma warning( pop )
#endif
        }

        // The maximum negative fixed point value is -2^(i-1).
        // The following constructs the floating point bit pattern for this value,
        // as long as i >= 2.
        // We need this number without the sign bit
        _fxpMaxNegValueFloat = (_fltExponentBias + c_uIBits - 1) << _fltMantissaBitCount;
    }
    else
    {
        // The maximum positive fixed point value is 2^(i) - 2^(-f).
        // The following constructs the floating point bit pattern for this value,
        // as long as i >= 2.
        _fxpMaxPosValueFloat = (_fltExponentBias + c_uIBits) <<_fltMantissaBitCount;
        const INT32 iShift = _fltMantissaBitCount + 1 - c_uIBits - c_uFBits;
        if (iShift >= 0)
        {
//            assert( iShift < 32 );
#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4293 26452 )
#endif
            _fxpMaxPosValueFloat -= INT32( 1 ) << iShift;
#if defined(_MSC_VER)
#pragma warning( pop )
#endif
        }

        // The maximum negative fixed point value is 0.
        _fxpMaxNegValueFloat = 0;
    }

    // ------------------------------------------------------------------------
    //                                                float -> fixed conversion
    // ------------------------------------------------------------------------

    // ------------------------------------------------------------------------
    //                                                      examine input float
    // ------------------------------------------------------------------------
    INT32 output              = *(INT32*)&input;
    INT32 unbiasedExponent    = ((output & _fltExponentMask) >> _fltMantissaBitCount) - _fltExponentBias;
    INT32 isNegative          = output & _fltSignBit;

    // ------------------------------------------------------------------------
    //                                                                      nan
    // ------------------------------------------------------------------------
    if (unbiasedExponent == (_fltExponentBias + 1) && (output & _fltMantissaMask))
    {
        // nan converts to 0
        output = 0;
    }
    // ------------------------------------------------------------------------
    //                                                       too large positive
    // ------------------------------------------------------------------------
    else if (!isNegative && output >= _fxpMaxPosValueFloat) // integer compare
    {
        output = c_iMaxResult;
    }
    // ------------------------------------------------------------------------
    //                                                       too large negative
    // ------------------------------------------------------------------------
                                            // integer compare
    else if (isNegative && (output & ~_fltSignBit) >= _fxpMaxNegValueFloat)
    {
        output = c_iMinResult;
    }
    // ------------------------------------------------------------------------
    //                                                                too small
    // ------------------------------------------------------------------------
    else if (unbiasedExponent < -c_uFBits - 1)
    {
        // clamp to 0
        output = 0;
    }
    // ------------------------------------------------------------------------
    //                                                             within range
    // ------------------------------------------------------------------------
    else
    {
        // copy mantissa, add hidden bit
        output = (output & _fltMantissaMask) | _fltHiddenBit;

        INT32 extraBits = _fltMantissaBitCount - c_uFBits - unbiasedExponent;
        if (extraBits >= 0)
        {
            // 2's complement if negative
            if (isNegative)
            {
                output = ~output + 1;
            }

            // From the range checks that led here, it is known that
            // unbiasedExponent < c_uIBits.  So, at most:
            // (a) unbiasedExponent == c_uIBits - 1.
            //
            // From compile validation above, it is known that
            // c_uIBits + c_uFBits <= _fltMantissaBitCount + 1).
            // So, at minimum:
            // (b) _fltMantissaBitCount == _fxtIntBitCount + c_uFBits - 1
            //
            // Substituting (a) and (b) into extraBits calculation above:
            // extraBits >= (_fxtIntBitCount + c_uFBits - 1)
            //              - c_uFBits - (c_uIBits - 1)
            // extraBits >= 0
            //
            // Thus we only have to worry about shifting right by 0 or more
            // bits to get the decimal to the right place, and never have
            // to shift left.

            INT32 LSB             = 1 << extraBits; // last bit being kept
            INT32 extraBitsMask   = LSB - 1;
            INT32 half            = LSB >> 1; // round bias

            // round to nearest-even at LSB
            if ((output & LSB) || (output & extraBitsMask) > half)
            {
                output += half;
            }

            // shift off the extra bits (sign extending)
            output >>= extraBits;
        }
        else
        {
            output <<= -extraBits;

            // 2's complement if negative
            if (isNegative)
            {
                output = ~output + 1;
            }
        }
    }
    return output;
}
//-----------------------------------------------------------------------------------------------------------------------------

#define FXP_INTEGER_BITS 15
#define FXP_FRACTION_BITS 16
#define FXP_FRACTION_MASK 0x0000ffff
#define FXP_INTEGER_MASK 0x7fff0000
#define FXP_THREE (3<<FXP_FRACTION_BITS)
#define FXP_ONE (1<<FXP_FRACTION_BITS)
#define FXP_ONE_THIRD 0x00005555
#define FXP_TWO_THIRDS 0x0000aaaa
#define FXP_ONE_HALF   0x00008000

#define FXP_MAX_INPUT_TESS_FACTOR_BEFORE_TRIPLE_AVERAGE 0x55540000 // 1/3 of max fixed point number - 1.  Numbers less than
                                                    // or equal to this allows avg. reduction on a tri patch
                                                    // including rounding.

#define FXP_MAX_INPUT_TESS_FACTOR_BEFORE_PAIR_AVERAGE 0x7FFF0000 // 1/2 of max fixed point number - 1.  Numbers less than
                                                    // or equal to this allows avg. reduction on a quad patch
                                                    // including rounding.

static const FXP s_fixedReciprocal[PIPE_TESSELLATOR_MAX_TESSELLATION_FACTOR+1] =
{
    0xffffffff, // 1/0 is the first entry (unused)
    0x10000, 0x8000, 0x5555, 0x4000,
    0x3333, 0x2aab, 0x2492, 0x2000,
    0x1c72, 0x199a, 0x1746, 0x1555,
    0x13b1, 0x1249, 0x1111, 0x1000,
    0xf0f, 0xe39, 0xd79, 0xccd,
    0xc31, 0xba3, 0xb21, 0xaab,
    0xa3d, 0x9d9, 0x97b, 0x925,
    0x8d4, 0x889, 0x842, 0x800,
    0x7c2, 0x788, 0x750, 0x71c,
    0x6eb, 0x6bd, 0x690, 0x666,
    0x63e, 0x618, 0x5f4, 0x5d1,
    0x5b0, 0x591, 0x572, 0x555,
    0x539, 0x51f, 0x505, 0x4ec,
    0x4d5, 0x4be, 0x4a8, 0x492,
    0x47e, 0x46a, 0x457, 0x444,
    0x432, 0x421, 0x410, 0x400, // 1/64 is the last entry
};

#define FLOAT_THREE 3.0f
#define FLOAT_ONE 1.0f

//---------------------------------------------------------------------------------------------------------------------------------
// floatToFixed
//---------------------------------------------------------------------------------------------------------------------------------
FXP floatToFixed(const float& input)
{
    return floatToIDotF< FXP_INTEGER_BITS, FXP_FRACTION_BITS, /*bSigned*/false >( input );
}

//---------------------------------------------------------------------------------------------------------------------------------
// fixedToFloat
//---------------------------------------------------------------------------------------------------------------------------------
float fixedToFloat(const FXP& input)
{
    // not worrying about denorm flushing the float operations (the DX spec behavior for div), since the numbers will not be that small during tessellation.
    return ((float)(input>>FXP_FRACTION_BITS) + (float)(input&FXP_FRACTION_MASK)/(1<<FXP_FRACTION_BITS));
}

//---------------------------------------------------------------------------------------------------------------------------------
// isEven
//---------------------------------------------------------------------------------------------------------------------------------
bool isEven(const float& input)
{
    return (((int)input) & 1) ? false : true;
}

//---------------------------------------------------------------------------------------------------------------------------------
// fxpCeil
//---------------------------------------------------------------------------------------------------------------------------------
FXP fxpCeil(const FXP& input)
{
    if( input & FXP_FRACTION_MASK )
    {
        return (input & FXP_INTEGER_MASK) + FXP_ONE;
    }
    return input;
}

//---------------------------------------------------------------------------------------------------------------------------------
// fxpFloor
//---------------------------------------------------------------------------------------------------------------------------------
FXP fxpFloor(const FXP& input)
{
    return (input & FXP_INTEGER_MASK);
}

//=================================================================================================================================
// CHWTessellator
//=================================================================================================================================

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::CHWTessellator
//---------------------------------------------------------------------------------------------------------------------------------
CHWTessellator::CHWTessellator()
{
    m_Point = 0;
    m_Index = 0;
    m_NumPoints = 0;
    m_NumIndices = 0;
    m_bUsingPatchedIndices = false;
    m_bUsingPatchedIndices2 = false;
}
//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::~CHWTessellator
//---------------------------------------------------------------------------------------------------------------------------------
CHWTessellator::~CHWTessellator()
{
    delete [] m_Point;
    delete [] m_Index;
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::Init
// User calls this.
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::Init(
    PIPE_TESSELLATOR_PARTITIONING       partitioning,
    PIPE_TESSELLATOR_OUTPUT_PRIMITIVE   outputPrimitive)
{
    if( 0 == m_Point )
    {
        m_Point = new DOMAIN_POINT[MAX_POINT_COUNT];
    }
    if( 0 == m_Index )
    {
        m_Index = new int[MAX_INDEX_COUNT];
    }
    m_partitioning = partitioning;
    m_originalPartitioning = partitioning;
    switch( partitioning )
    {
    case PIPE_TESSELLATOR_PARTITIONING_INTEGER:
    default:
        break;
    case PIPE_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD:
        m_parity = TESSELLATOR_PARITY_ODD;
        break;
    case PIPE_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN:
        m_parity = TESSELLATOR_PARITY_EVEN;
        break;
    }
    m_originalParity = m_parity;
    m_outputPrimitive = outputPrimitive;
    m_NumPoints = 0;
    m_NumIndices = 0;
}
//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::TessellateQuadDomain
// User calls this
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::TessellateQuadDomain( float tessFactor_Ueq0, float tessFactor_Veq0, float tessFactor_Ueq1, float tessFactor_Veq1,
                                         float insideTessFactor_U, float insideTessFactor_V )
{
    PROCESSED_TESS_FACTORS_QUAD processedTessFactors;
    QuadProcessTessFactors(tessFactor_Ueq0,tessFactor_Veq0,tessFactor_Ueq1,tessFactor_Veq1,insideTessFactor_U,insideTessFactor_V,processedTessFactors);

    if( processedTessFactors.bPatchCulled )
    {
        m_NumPoints = 0;
        m_NumIndices = 0;
        return;
    }
    else if( processedTessFactors.bJustDoMinimumTessFactor )
    {
        DefinePoint(/*U*/0,/*V*/0,/*pointStorageOffset*/0);
        DefinePoint(/*U*/FXP_ONE,/*V*/0,/*pointStorageOffset*/1);
        DefinePoint(/*U*/FXP_ONE,/*V*/FXP_ONE,/*pointStorageOffset*/2);
        DefinePoint(/*U*/0,/*V*/FXP_ONE,/*pointStorageOffset*/3);
        m_NumPoints = 4;

        switch(m_outputPrimitive)
        {
        case PIPE_TESSELLATOR_OUTPUT_TRIANGLE_CW:
        case PIPE_TESSELLATOR_OUTPUT_TRIANGLE_CCW:
            // function orients them CCW if needed
            DefineClockwiseTriangle(0,1,3,/*indexStorageOffset*/0);
            DefineClockwiseTriangle(1,2,3,/*indexStorageOffset*/3);
            m_NumIndices = 6;
            break;
        case PIPE_TESSELLATOR_OUTPUT_POINT:
            DumpAllPoints();
            break;
        case PIPE_TESSELLATOR_OUTPUT_LINE:
            DumpAllPointsAsInOrderLineList();
            break;
        }
        return;
    }

    QuadGeneratePoints(processedTessFactors);

    if( m_outputPrimitive == PIPE_TESSELLATOR_OUTPUT_POINT )
    {
        DumpAllPoints();
        return;
    }
    if( m_outputPrimitive == PIPE_TESSELLATOR_OUTPUT_LINE )
    {
        DumpAllPointsAsInOrderLineList();
        return;
    }

    QuadGenerateConnectivity(processedTessFactors); // can be done in parallel to QuadGeneratePoints()
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::QuadProcessTessFactors
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::QuadProcessTessFactors( float tessFactor_Ueq0, float tessFactor_Veq0, float tessFactor_Ueq1, float tessFactor_Veq1,
                      float insideTessFactor_U, float insideTessFactor_V, PROCESSED_TESS_FACTORS_QUAD& processedTessFactors )
{
    // Is the patch culled?
    if( !(tessFactor_Ueq0 > 0) || // NaN will pass
        !(tessFactor_Veq0 > 0) ||
        !(tessFactor_Ueq1 > 0) ||
        !(tessFactor_Veq1 > 0) )
    {
        processedTessFactors.bPatchCulled = true;
        return;
    }
    else
    {
        processedTessFactors.bPatchCulled = false;
    }

    // Clamp edge TessFactors
    float lowerBound = 0.0, upperBound = 0.0;
    switch(m_originalPartitioning)
    {
        case PIPE_TESSELLATOR_PARTITIONING_INTEGER:
        case PIPE_TESSELLATOR_PARTITIONING_POW2: // don�t care about pow2 distinction for validation, just treat as integer
            lowerBound = PIPE_TESSELLATOR_MIN_ODD_TESSELLATION_FACTOR;
            upperBound = PIPE_TESSELLATOR_MAX_EVEN_TESSELLATION_FACTOR;
            break;

        case PIPE_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN:
            lowerBound = PIPE_TESSELLATOR_MIN_EVEN_TESSELLATION_FACTOR;
            upperBound = PIPE_TESSELLATOR_MAX_EVEN_TESSELLATION_FACTOR;
            break;

        case PIPE_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD:
            lowerBound = PIPE_TESSELLATOR_MIN_ODD_TESSELLATION_FACTOR;
            upperBound = PIPE_TESSELLATOR_MAX_ODD_TESSELLATION_FACTOR;
            break;
    }

    tessFactor_Ueq0 = tess_fmin( upperBound, tess_fmax( lowerBound, tessFactor_Ueq0 ) );
    tessFactor_Veq0 = tess_fmin( upperBound, tess_fmax( lowerBound, tessFactor_Veq0 ) );
    tessFactor_Ueq1 = tess_fmin( upperBound, tess_fmax( lowerBound, tessFactor_Ueq1 ) );
    tessFactor_Veq1 = tess_fmin( upperBound, tess_fmax( lowerBound, tessFactor_Veq1 ) );

    if( HWIntegerPartitioning()) // pow2 or integer, round to next int (hw doesn't care about pow2 distinction)
    {
        tessFactor_Ueq0 = ceil(tessFactor_Ueq0);
        tessFactor_Veq0 = ceil(tessFactor_Veq0);
        tessFactor_Ueq1 = ceil(tessFactor_Ueq1);
        tessFactor_Veq1 = ceil(tessFactor_Veq1);
    }

    // Clamp inside TessFactors
    if(PIPE_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD == m_originalPartitioning)
    {
#define EPSILON 0.0000152587890625f // 2^(-16), min positive fixed point fraction
#define MIN_ODD_TESSFACTOR_PLUS_HALF_EPSILON (PIPE_TESSELLATOR_MIN_ODD_TESSELLATION_FACTOR + EPSILON/2)
        // If any TessFactor will end up > 1 after floatToFixed conversion later,
        // then force the inside TessFactors to be > 1 so there is a picture frame.
        if( (tessFactor_Ueq0 > MIN_ODD_TESSFACTOR_PLUS_HALF_EPSILON) ||
            (tessFactor_Veq0 > MIN_ODD_TESSFACTOR_PLUS_HALF_EPSILON) ||
            (tessFactor_Ueq1 > MIN_ODD_TESSFACTOR_PLUS_HALF_EPSILON) ||
            (tessFactor_Veq1 > MIN_ODD_TESSFACTOR_PLUS_HALF_EPSILON) ||
            (insideTessFactor_U > MIN_ODD_TESSFACTOR_PLUS_HALF_EPSILON) ||
            (insideTessFactor_V > MIN_ODD_TESSFACTOR_PLUS_HALF_EPSILON) )
        {
            // Force picture frame
            lowerBound = PIPE_TESSELLATOR_MIN_ODD_TESSELLATION_FACTOR + EPSILON;
        }
    }

    insideTessFactor_U = tess_fmin( upperBound, tess_fmax( lowerBound, insideTessFactor_U ) );
    insideTessFactor_V = tess_fmin( upperBound, tess_fmax( lowerBound, insideTessFactor_V ) );
    // Note the above clamps map NaN to lowerBound


    if( HWIntegerPartitioning()) // pow2 or integer, round to next int (hw doesn't care about pow2 distinction)
    {
        insideTessFactor_U = ceil(insideTessFactor_U);
        insideTessFactor_V = ceil(insideTessFactor_V);
    }

    // Reset our vertex and index buffers.  We have enough storage for the max tessFactor.
    m_NumPoints = 0;
    m_NumIndices = 0;

    // Process tessFactors
    float outsideTessFactor[QUAD_EDGES] = {tessFactor_Ueq0, tessFactor_Veq0, tessFactor_Ueq1, tessFactor_Veq1};
    float insideTessFactor[QUAD_AXES] = {insideTessFactor_U,insideTessFactor_V};
    int edge, axis;
    if( HWIntegerPartitioning() )
    {
        for( edge = 0; edge < QUAD_EDGES; edge++ )
        {
            int edgeEven = isEven(outsideTessFactor[edge]);
            processedTessFactors.outsideTessFactorParity[edge] = edgeEven ? TESSELLATOR_PARITY_EVEN : TESSELLATOR_PARITY_ODD;
        }
        for( axis = 0; axis < QUAD_AXES; axis++ )
        {
            processedTessFactors.insideTessFactorParity[axis] =
                (isEven(insideTessFactor[axis]) || (FLOAT_ONE == insideTessFactor[axis]) )
                ? TESSELLATOR_PARITY_EVEN : TESSELLATOR_PARITY_ODD;
        }
    }
    else
    {
        for( edge = 0; edge < QUAD_EDGES; edge++ )
        {
            processedTessFactors.outsideTessFactorParity[edge] = m_originalParity;
        }
        processedTessFactors.insideTessFactorParity[U] = processedTessFactors.insideTessFactorParity[V] = m_originalParity;
    }

    // Save fixed point TessFactors
    for( edge = 0; edge < QUAD_EDGES; edge++ )
    {
        processedTessFactors.outsideTessFactor[edge] = floatToFixed(outsideTessFactor[edge]);
    }
    for( axis = 0; axis < QUAD_AXES; axis++ )
    {
        processedTessFactors.insideTessFactor[axis] = floatToFixed(insideTessFactor[axis]);
    }

    if( HWIntegerPartitioning() || Odd() )
    {
        // Special case if all TessFactors are 1
        if( (FXP_ONE == processedTessFactors.insideTessFactor[U]) &&
            (FXP_ONE == processedTessFactors.insideTessFactor[V]) &&
            (FXP_ONE == processedTessFactors.outsideTessFactor[Ueq0]) &&
            (FXP_ONE == processedTessFactors.outsideTessFactor[Veq0]) &&
            (FXP_ONE == processedTessFactors.outsideTessFactor[Ueq1]) &&
            (FXP_ONE == processedTessFactors.outsideTessFactor[Veq1]) )
        {
            processedTessFactors.bJustDoMinimumTessFactor = true;
            return;
        }
    }
    processedTessFactors.bJustDoMinimumTessFactor = false;

    // Compute TessFactor-specific metadata
    for(int edge = 0; edge < QUAD_EDGES; edge++ )
    {
        SetTessellationParity(processedTessFactors.outsideTessFactorParity[edge]);
        ComputeTessFactorContext(processedTessFactors.outsideTessFactor[edge], processedTessFactors.outsideTessFactorCtx[edge]);
    }

    for(int axis = 0; axis < QUAD_AXES; axis++)
    {
        SetTessellationParity(processedTessFactors.insideTessFactorParity[axis]);
        ComputeTessFactorContext(processedTessFactors.insideTessFactor[axis], processedTessFactors.insideTessFactorCtx[axis]);
    }

    // Compute some initial data.

    // outside edge offsets and storage
    for(int edge = 0; edge < QUAD_EDGES; edge++ )
    {
        SetTessellationParity(processedTessFactors.outsideTessFactorParity[edge]);
        processedTessFactors.numPointsForOutsideEdge[edge] = NumPointsForTessFactor(processedTessFactors.outsideTessFactor[edge]);
        m_NumPoints += processedTessFactors.numPointsForOutsideEdge[edge];
    }
    m_NumPoints -= 4;

    // inside edge offsets
    for(int axis = 0; axis < QUAD_AXES; axis++)
    {
        SetTessellationParity(processedTessFactors.insideTessFactorParity[axis]);
        processedTessFactors.numPointsForInsideTessFactor[axis] = NumPointsForTessFactor(processedTessFactors.insideTessFactor[axis]);
        int pointCountMin = ( TESSELLATOR_PARITY_ODD == processedTessFactors.insideTessFactorParity[axis] ) ? 4 : 3;
        // max() allows degenerate transition regions when inside TessFactor == 1
        processedTessFactors.numPointsForInsideTessFactor[axis] = max(pointCountMin,processedTessFactors.numPointsForInsideTessFactor[axis]);
    }

    processedTessFactors.insideEdgePointBaseOffset = m_NumPoints;

    // inside storage, including interior edges above
    int numInteriorPoints = (processedTessFactors.numPointsForInsideTessFactor[U] - 2)*(processedTessFactors.numPointsForInsideTessFactor[V]-2);
    m_NumPoints += numInteriorPoints;
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::QuadGeneratePoints
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::QuadGeneratePoints( const PROCESSED_TESS_FACTORS_QUAD& processedTessFactors )
{
    // Generate exterior ring edge points, clockwise from top-left
    int pointOffset = 0;
    int edge;
    for(edge = 0; edge < QUAD_EDGES; edge++ )
    {
        int parity = edge&0x1;
        int startPoint = 0;
        int endPoint = processedTessFactors.numPointsForOutsideEdge[edge] - 1;
        for(int p = startPoint; p < endPoint; p++,pointOffset++) // don't include end, since next edge starts with it.
        {
            FXP fxpParam;
            int q = ((edge==1)||(edge==2)) ? p : endPoint - p; // reverse order
            SetTessellationParity(processedTessFactors.outsideTessFactorParity[edge]);
            PlacePointIn1D(processedTessFactors.outsideTessFactorCtx[edge],q,fxpParam);
            if( parity )
            {
                DefinePoint(/*U*/fxpParam,
                            /*V*/(edge == 3) ? FXP_ONE : 0,
                            /*pointStorageOffset*/pointOffset);
            }
            else
            {
                DefinePoint(/*U*/(edge == 2) ? FXP_ONE : 0,
                            /*V*/fxpParam,
                            /*pointStorageOffset*/pointOffset);
            }
        }
    }

    // Generate interior ring points, clockwise from (U==0,V==1) (bottom-left) spiralling toward center
    static const int startRing = 1;
    int minNumPointsForTessFactor = min(processedTessFactors.numPointsForInsideTessFactor[U],processedTessFactors.numPointsForInsideTessFactor[V]);
    int numRings = (minNumPointsForTessFactor >> 1);  // note for even tess we aren't counting center point here.
    for(int ring = startRing; ring < numRings; ring++)
    {
        int startPoint = ring;
        int endPoint[QUAD_AXES] = {processedTessFactors.numPointsForInsideTessFactor[U] - 1 - startPoint,
                                   processedTessFactors.numPointsForInsideTessFactor[V] - 1 - startPoint};

        for(edge = 0; edge < QUAD_EDGES; edge++ )
        {
            int parity[QUAD_AXES] = {edge&0x1,((edge+1)&0x1)};
            int perpendicularAxisPoint = (edge < 2) ? startPoint : endPoint[parity[0]];
            FXP fxpPerpParam;
            SetTessellationParity(processedTessFactors.insideTessFactorParity[parity[0]]);
            PlacePointIn1D(processedTessFactors.insideTessFactorCtx[parity[0]],perpendicularAxisPoint,fxpPerpParam);
            SetTessellationParity(processedTessFactors.insideTessFactorParity[parity[1]]);
            for(int p = startPoint; p < endPoint[parity[1]]; p++, pointOffset++) // don't include end: next edge starts with it.
            {
                FXP fxpParam;
                int q = ((edge == 1)||(edge==2)) ? p : endPoint[parity[1]] - (p - startPoint);
                PlacePointIn1D(processedTessFactors.insideTessFactorCtx[parity[1]],q,fxpParam);
                if( parity[1] )
                {
                    DefinePoint(/*U*/fxpPerpParam,
                                /*V*/fxpParam,
                                /*pointStorageOffset*/pointOffset);
                }
                else
                {
                    DefinePoint(/*U*/fxpParam,
                                /*V*/fxpPerpParam,
                                /*pointStorageOffset*/pointOffset);
                }
            }
        }
    }
    // For even tessellation, the inner "ring" is degenerate - a row of points
    if( (processedTessFactors.numPointsForInsideTessFactor[U] > processedTessFactors.numPointsForInsideTessFactor[V]) &&
        (TESSELLATOR_PARITY_EVEN == processedTessFactors.insideTessFactorParity[V]) )
    {
        int startPoint = numRings;
        int endPoint = processedTessFactors.numPointsForInsideTessFactor[U] - 1 - startPoint;
        SetTessellationParity(processedTessFactors.insideTessFactorParity[U]);
        for( int p = startPoint; p <= endPoint; p++, pointOffset++ )
        {
            FXP fxpParam;
            PlacePointIn1D(processedTessFactors.insideTessFactorCtx[U],p,fxpParam);
            DefinePoint(/*U*/fxpParam,
                        /*V*/FXP_ONE_HALF, // middle
                        /*pointStorageOffset*/pointOffset);
        }
    }
    else if( (processedTessFactors.numPointsForInsideTessFactor[V] >= processedTessFactors.numPointsForInsideTessFactor[U]) &&
             (TESSELLATOR_PARITY_EVEN == processedTessFactors.insideTessFactorParity[U]) )
    {
        int startPoint = numRings;
        int endPoint;
        FXP fxpParam;
        endPoint = processedTessFactors.numPointsForInsideTessFactor[V] - 1 - startPoint;
        SetTessellationParity(processedTessFactors.insideTessFactorParity[V]);
        for( int p = endPoint; p >= startPoint; p--, pointOffset++ )
        {
            PlacePointIn1D(processedTessFactors.insideTessFactorCtx[V],p,fxpParam);
            DefinePoint(/*U*/FXP_ONE_HALF, // middle
                        /*V*/fxpParam,
                        /*pointStorageOffset*/pointOffset);
        }
    }
}
//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::QuadGenerateConnectivity
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::QuadGenerateConnectivity( const PROCESSED_TESS_FACTORS_QUAD& processedTessFactors )
{
    // Generate primitives for all the concentric rings, one side at a time for each ring
    static const int startRing = 1;
    int numPointRowsToCenter[QUAD_AXES] = {((processedTessFactors.numPointsForInsideTessFactor[U]+1) >> 1),
                                            ((processedTessFactors.numPointsForInsideTessFactor[V]+1) >> 1)}; // +1 is so even tess includes the center point
    int numRings = min(numPointRowsToCenter[U],numPointRowsToCenter[V]);
    int degeneratePointRing[QUAD_AXES] = { // Even partitioning causes degenerate row of points,
                                           // which results in exceptions to the point ordering conventions
                                           // when travelling around the rings counterclockwise.
        (TESSELLATOR_PARITY_EVEN == processedTessFactors.insideTessFactorParity[V]) ? numPointRowsToCenter[V] - 1 : -1,
        (TESSELLATOR_PARITY_EVEN == processedTessFactors.insideTessFactorParity[U]) ? numPointRowsToCenter[U] - 1 : -1 };

    const TESS_FACTOR_CONTEXT* outsideTessFactorCtx[QUAD_EDGES] = {&processedTessFactors.outsideTessFactorCtx[Ueq0],
                                                    &processedTessFactors.outsideTessFactorCtx[Veq0],
                                                    &processedTessFactors.outsideTessFactorCtx[Ueq1],
                                                    &processedTessFactors.outsideTessFactorCtx[Veq1]};
    TESSELLATOR_PARITY outsideTessFactorParity[QUAD_EDGES] = {processedTessFactors.outsideTessFactorParity[Ueq0],
                                                        processedTessFactors.outsideTessFactorParity[Veq0],
                                                        processedTessFactors.outsideTessFactorParity[Ueq1],
                                                        processedTessFactors.outsideTessFactorParity[Veq1]};
    int numPointsForOutsideEdge[QUAD_EDGES] = {processedTessFactors.numPointsForOutsideEdge[Ueq0],
                                              processedTessFactors.numPointsForOutsideEdge[Veq0],
                                              processedTessFactors.numPointsForOutsideEdge[Ueq1],
                                              processedTessFactors.numPointsForOutsideEdge[Veq1]};

    int insideEdgePointBaseOffset = processedTessFactors.insideEdgePointBaseOffset;
    int outsideEdgePointBaseOffset = 0;
    int edge;
    for(int ring = startRing; ring < numRings; ring++)
    {
        int numPointsForInsideEdge[QUAD_AXES] = {processedTessFactors.numPointsForInsideTessFactor[U] - 2*ring,
                                                 processedTessFactors.numPointsForInsideTessFactor[V] - 2*ring};

        int edge0InsidePointBaseOffset = insideEdgePointBaseOffset;
        int edge0OutsidePointBaseOffset = outsideEdgePointBaseOffset;

        for(edge = 0; edge < QUAD_EDGES; edge++ )
        {
            int parity = (edge+1)&0x1;

            int numTriangles = numPointsForInsideEdge[parity] + numPointsForOutsideEdge[edge] - 2;
            int insideBaseOffset;
            int outsideBaseOffset;
            if( edge == 3 ) // We need to patch the indexing so Stitch() can think it sees
                            // 2 sequentially increasing rows of points, even though we have wrapped around
                            // to the end of the inner and outer ring's points, so the last point is really
                            // the first point for the ring.
                            // We make it so that when Stitch() calls AddIndex(), that function
                            // will do any necessary index adjustment.
            {
                if( ring == degeneratePointRing[parity] )
                {
                    m_IndexPatchContext2.baseIndexToInvert = insideEdgePointBaseOffset + 1;
                    m_IndexPatchContext2.cornerCaseBadValue = outsideEdgePointBaseOffset + numPointsForOutsideEdge[edge] - 1;
                    m_IndexPatchContext2.cornerCaseReplacementValue = edge0OutsidePointBaseOffset;
                    m_IndexPatchContext2.indexInversionEndPoint = (m_IndexPatchContext2.baseIndexToInvert << 1) - 1;
                    insideBaseOffset = m_IndexPatchContext2.baseIndexToInvert;
                    outsideBaseOffset = outsideEdgePointBaseOffset;
                    SetUsingPatchedIndices2(true);
                }
                else
                {
                    m_IndexPatchContext.insidePointIndexDeltaToRealValue    = insideEdgePointBaseOffset;
                    m_IndexPatchContext.insidePointIndexBadValue            = numPointsForInsideEdge[parity] - 1;
                    m_IndexPatchContext.insidePointIndexReplacementValue    = edge0InsidePointBaseOffset;
                    m_IndexPatchContext.outsidePointIndexPatchBase          = m_IndexPatchContext.insidePointIndexBadValue+1; // past inside patched index range
                    m_IndexPatchContext.outsidePointIndexDeltaToRealValue   = outsideEdgePointBaseOffset
                                                                                - m_IndexPatchContext.outsidePointIndexPatchBase;
                    m_IndexPatchContext.outsidePointIndexBadValue           = m_IndexPatchContext.outsidePointIndexPatchBase
                                                                                + numPointsForOutsideEdge[edge] - 1;
                    m_IndexPatchContext.outsidePointIndexReplacementValue   = edge0OutsidePointBaseOffset;

                    insideBaseOffset = 0;
                    outsideBaseOffset = m_IndexPatchContext.outsidePointIndexPatchBase;
                    SetUsingPatchedIndices(true);
                }
            }
            else if( (edge == 2) && (ring == degeneratePointRing[parity]) )
            {
                m_IndexPatchContext2.baseIndexToInvert = insideEdgePointBaseOffset;
                m_IndexPatchContext2.cornerCaseBadValue = -1; // unused
                m_IndexPatchContext2.cornerCaseReplacementValue = -1; // unused
                m_IndexPatchContext2.indexInversionEndPoint = m_IndexPatchContext2.baseIndexToInvert << 1;
                insideBaseOffset = m_IndexPatchContext2.baseIndexToInvert;
                outsideBaseOffset = outsideEdgePointBaseOffset;
                SetUsingPatchedIndices2(true);
            }
            else
            {
                insideBaseOffset = insideEdgePointBaseOffset;
                outsideBaseOffset = outsideEdgePointBaseOffset;
            }
            if( ring == startRing )
            {
                StitchTransition(/*baseIndexOffset: */m_NumIndices,
                               insideBaseOffset,processedTessFactors.insideTessFactorCtx[parity].numHalfTessFactorPoints,processedTessFactors.insideTessFactorParity[parity],
                               outsideBaseOffset,outsideTessFactorCtx[edge]->numHalfTessFactorPoints,outsideTessFactorParity[edge]);
            }
            else
            {
                StitchRegular(/*bTrapezoid*/true, DIAGONALS_MIRRORED,
                              /*baseIndexOffset: */m_NumIndices,
                              numPointsForInsideEdge[parity],
                              insideBaseOffset,outsideBaseOffset);
            }
            SetUsingPatchedIndices(false);
            SetUsingPatchedIndices2(false);
            m_NumIndices += numTriangles*3;
            outsideEdgePointBaseOffset += numPointsForOutsideEdge[edge] - 1;
            if( (edge == 2) && (ring == degeneratePointRing[parity]) )
            {
                insideEdgePointBaseOffset -= numPointsForInsideEdge[parity] - 1;
            }
            else
            {
                insideEdgePointBaseOffset += numPointsForInsideEdge[parity] - 1;
            }
            numPointsForOutsideEdge[edge] = numPointsForInsideEdge[parity];
        }
        if( startRing == ring )
        {
            for(edge = 0; edge < QUAD_EDGES; edge++ )
            {
                outsideTessFactorCtx[edge] = &processedTessFactors.insideTessFactorCtx[edge&1];
                outsideTessFactorParity[edge] = processedTessFactors.insideTessFactorParity[edge&1];
            }
        }
    }

    // Triangulate center - a row of quads if odd
    // This triangulation may be producing diagonals that are asymmetric about
    // the center of the patch in this region.
    if( (processedTessFactors.numPointsForInsideTessFactor[U] > processedTessFactors.numPointsForInsideTessFactor[V]) &&
        (TESSELLATOR_PARITY_ODD == processedTessFactors.insideTessFactorParity[V] ) )
    {
        SetUsingPatchedIndices2(true);
        int stripNumQuads = (((processedTessFactors.numPointsForInsideTessFactor[U]>>1) - (processedTessFactors.numPointsForInsideTessFactor[V]>>1))<<1)+
                            ((TESSELLATOR_PARITY_EVEN == processedTessFactors.insideTessFactorParity[U] ) ? 2 : 1);
        m_IndexPatchContext2.baseIndexToInvert = outsideEdgePointBaseOffset + stripNumQuads + 2;
        m_IndexPatchContext2.cornerCaseBadValue = m_IndexPatchContext2.baseIndexToInvert;
        m_IndexPatchContext2.cornerCaseReplacementValue = outsideEdgePointBaseOffset;
        m_IndexPatchContext2.indexInversionEndPoint = m_IndexPatchContext2.baseIndexToInvert +
                                                      m_IndexPatchContext2.baseIndexToInvert + stripNumQuads;
        StitchRegular(/*bTrapezoid*/false,DIAGONALS_INSIDE_TO_OUTSIDE,
                       /*baseIndexOffset: */m_NumIndices, /*numInsideEdgePoints:*/stripNumQuads+1,
                       /*insideEdgePointBaseOffset*/m_IndexPatchContext2.baseIndexToInvert,
                       outsideEdgePointBaseOffset+1);
        SetUsingPatchedIndices2(false);
        m_NumIndices += stripNumQuads*6;
    }
    else if((processedTessFactors.numPointsForInsideTessFactor[V] >= processedTessFactors.numPointsForInsideTessFactor[U]) &&
            (TESSELLATOR_PARITY_ODD == processedTessFactors.insideTessFactorParity[U]) )
    {
        SetUsingPatchedIndices2(true);
        int stripNumQuads = (((processedTessFactors.numPointsForInsideTessFactor[V]>>1) - (processedTessFactors.numPointsForInsideTessFactor[U]>>1))<<1)+
                            ((TESSELLATOR_PARITY_EVEN == processedTessFactors.insideTessFactorParity[V] ) ? 2 : 1);
        m_IndexPatchContext2.baseIndexToInvert = outsideEdgePointBaseOffset + stripNumQuads + 1;
        m_IndexPatchContext2.cornerCaseBadValue = -1; // unused
        m_IndexPatchContext2.indexInversionEndPoint = m_IndexPatchContext2.baseIndexToInvert +
                                                      m_IndexPatchContext2.baseIndexToInvert + stripNumQuads;
		DIAGONALS diag = (TESSELLATOR_PARITY_EVEN == processedTessFactors.insideTessFactorParity[V]) ?
							DIAGONALS_INSIDE_TO_OUTSIDE : DIAGONALS_INSIDE_TO_OUTSIDE_EXCEPT_MIDDLE;
        StitchRegular(/*bTrapezoid*/false,diag,
                       /*baseIndexOffset: */m_NumIndices, /*numInsideEdgePoints:*/stripNumQuads+1,
                       /*insideEdgePointBaseOffset*/m_IndexPatchContext2.baseIndexToInvert,
                       outsideEdgePointBaseOffset);
        SetUsingPatchedIndices2(false);
        m_NumIndices += stripNumQuads*6;
    }
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::TessellateTriDomain
// User calls this
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::TessellateTriDomain( float tessFactor_Ueq0, float tessFactor_Veq0, float tessFactor_Weq0,
                                        float insideTessFactor )
{
    PROCESSED_TESS_FACTORS_TRI processedTessFactors;
    TriProcessTessFactors(tessFactor_Ueq0,tessFactor_Veq0,tessFactor_Weq0,insideTessFactor,processedTessFactors);

    if( processedTessFactors.bPatchCulled )
    {
        m_NumPoints = 0;
        m_NumIndices = 0;
        return;
    }
    else if( processedTessFactors.bJustDoMinimumTessFactor )
    {
        DefinePoint(/*U*/0,/*V*/FXP_ONE,/*pointStorageOffset*/0); //V=1 (beginning of Ueq0 edge VW)
        DefinePoint(/*U*/0,/*V*/0,/*pointStorageOffset*/1); //W=1 (beginning of Veq0 edge WU)
        DefinePoint(/*U*/FXP_ONE,/*V*/0,/*pointStorageOffset*/2); //U=1 (beginning of Weq0 edge UV)
        m_NumPoints = 3;

        switch(m_outputPrimitive)
        {
        case PIPE_TESSELLATOR_OUTPUT_TRIANGLE_CW:
        case PIPE_TESSELLATOR_OUTPUT_TRIANGLE_CCW:
            // function orients them CCW if needed
            DefineClockwiseTriangle(0,1,2,/*indexStorageBaseOffset*/m_NumIndices);
            m_NumIndices = 3;
            break;
        case PIPE_TESSELLATOR_OUTPUT_POINT:
            DumpAllPoints();
            break;
        case PIPE_TESSELLATOR_OUTPUT_LINE:
            DumpAllPointsAsInOrderLineList();
            break;
        }
        return;
    }

    TriGeneratePoints(processedTessFactors);

    if( m_outputPrimitive == PIPE_TESSELLATOR_OUTPUT_POINT )
    {
        DumpAllPoints();
        return;
    }
    if( m_outputPrimitive == PIPE_TESSELLATOR_OUTPUT_LINE )
    {
        DumpAllPointsAsInOrderLineList();
        return;
    }

    TriGenerateConnectivity(processedTessFactors); // can be done in parallel to TriGeneratePoints()
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::TriProcessTessFactors
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::TriProcessTessFactors( float tessFactor_Ueq0, float tessFactor_Veq0, float tessFactor_Weq0,
                                            float insideTessFactor, PROCESSED_TESS_FACTORS_TRI& processedTessFactors )
{
    // Is the patch culled?
    if( !(tessFactor_Ueq0 > 0) || // NaN will pass
        !(tessFactor_Veq0 > 0) ||
        !(tessFactor_Weq0 > 0) )
    {
        processedTessFactors.bPatchCulled = true;
        return;
    }
    else
    {
        processedTessFactors.bPatchCulled = false;
    }

    // Clamp edge TessFactors
    float lowerBound = 0.0, upperBound = 0.0;
    switch(m_originalPartitioning)
    {
        case PIPE_TESSELLATOR_PARTITIONING_INTEGER:
        case PIPE_TESSELLATOR_PARTITIONING_POW2: // don�t care about pow2 distinction for validation, just treat as integer
            lowerBound = PIPE_TESSELLATOR_MIN_ODD_TESSELLATION_FACTOR;
            upperBound = PIPE_TESSELLATOR_MAX_EVEN_TESSELLATION_FACTOR;
            break;

        case PIPE_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN:
            lowerBound = PIPE_TESSELLATOR_MIN_EVEN_TESSELLATION_FACTOR;
            upperBound = PIPE_TESSELLATOR_MAX_EVEN_TESSELLATION_FACTOR;
            break;

        case PIPE_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD:
            lowerBound = PIPE_TESSELLATOR_MIN_ODD_TESSELLATION_FACTOR;
            upperBound = PIPE_TESSELLATOR_MAX_ODD_TESSELLATION_FACTOR;
            break;
    }

    tessFactor_Ueq0 = tess_fmin( upperBound, tess_fmax( lowerBound, tessFactor_Ueq0 ) );
    tessFactor_Veq0 = tess_fmin( upperBound, tess_fmax( lowerBound, tessFactor_Veq0 ) );
    tessFactor_Weq0 = tess_fmin( upperBound, tess_fmax( lowerBound, tessFactor_Weq0 ) );

    if( HWIntegerPartitioning()) // pow2 or integer, round to next int (hw doesn't care about pow2 distinction)
    {
        tessFactor_Ueq0 = ceil(tessFactor_Ueq0);
        tessFactor_Veq0 = ceil(tessFactor_Veq0);
        tessFactor_Weq0 = ceil(tessFactor_Weq0);
    }

    // Clamp inside TessFactors
    if(PIPE_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD == m_originalPartitioning)
    {
        if( (tessFactor_Ueq0 > MIN_ODD_TESSFACTOR_PLUS_HALF_EPSILON) ||
            (tessFactor_Veq0 > MIN_ODD_TESSFACTOR_PLUS_HALF_EPSILON) ||
            (tessFactor_Weq0 > MIN_ODD_TESSFACTOR_PLUS_HALF_EPSILON))
            // Don't need the same check for insideTessFactor for tri patches,
            // since there is only one insideTessFactor, as opposed to quad
            // patches which have 2 insideTessFactors.
        {
            // Force picture frame
            lowerBound = PIPE_TESSELLATOR_MIN_ODD_TESSELLATION_FACTOR + EPSILON;
        }
    }

    insideTessFactor = tess_fmin( upperBound, tess_fmax( lowerBound, insideTessFactor ) );
    // Note the above clamps map NaN to lowerBound

    if( HWIntegerPartitioning()) // pow2 or integer, round to next int (hw doesn't care about pow2 distinction)
    {
        insideTessFactor = ceil(insideTessFactor);
    }

    // Reset our vertex and index buffers.  We have enough storage for the max tessFactor.
    m_NumPoints = 0;
    m_NumIndices = 0;

    // Process tessFactors
    float outsideTessFactor[TRI_EDGES] = {tessFactor_Ueq0, tessFactor_Veq0, tessFactor_Weq0};
    int edge;
    if( HWIntegerPartitioning() )
    {
        for( edge = 0; edge < TRI_EDGES; edge++ )
        {
            int edgeEven = isEven(outsideTessFactor[edge]);
            processedTessFactors.outsideTessFactorParity[edge] = edgeEven ? TESSELLATOR_PARITY_EVEN : TESSELLATOR_PARITY_ODD;
        }
        processedTessFactors.insideTessFactorParity = (isEven(insideTessFactor) || (FLOAT_ONE == insideTessFactor))
                                        ? TESSELLATOR_PARITY_EVEN : TESSELLATOR_PARITY_ODD;
    }
    else
    {
        for( edge = 0; edge < TRI_EDGES; edge++ )
        {
            processedTessFactors.outsideTessFactorParity[edge] = m_originalParity;
        }
        processedTessFactors.insideTessFactorParity = m_originalParity;
    }

    // Save fixed point TessFactors
    for( edge = 0; edge < TRI_EDGES; edge++ )
    {
        processedTessFactors.outsideTessFactor[edge] = floatToFixed(outsideTessFactor[edge]);
    }
    processedTessFactors.insideTessFactor = floatToFixed(insideTessFactor);

    if( HWIntegerPartitioning() || Odd() )
    {
        // Special case if all TessFactors are 1
        if( (FXP_ONE == processedTessFactors.insideTessFactor) &&
            (FXP_ONE == processedTessFactors.outsideTessFactor[Ueq0]) &&
            (FXP_ONE == processedTessFactors.outsideTessFactor[Veq0]) &&
            (FXP_ONE == processedTessFactors.outsideTessFactor[Weq0]) )
        {
            processedTessFactors.bJustDoMinimumTessFactor = true;
            return;
        }
    }
    processedTessFactors.bJustDoMinimumTessFactor = false;

    // Compute per-TessFactor metadata
    for(edge = 0; edge < TRI_EDGES; edge++ )
    {
        SetTessellationParity(processedTessFactors.outsideTessFactorParity[edge]);
        ComputeTessFactorContext(processedTessFactors.outsideTessFactor[edge], processedTessFactors.outsideTessFactorCtx[edge]);
    }
    SetTessellationParity(processedTessFactors.insideTessFactorParity);
    ComputeTessFactorContext(processedTessFactors.insideTessFactor, processedTessFactors.insideTessFactorCtx);

    // Compute some initial data.

    // outside edge offsets and storage
    for(edge = 0; edge < TRI_EDGES; edge++ )
    {
        SetTessellationParity(processedTessFactors.outsideTessFactorParity[edge]);
        processedTessFactors.numPointsForOutsideEdge[edge] = NumPointsForTessFactor(processedTessFactors.outsideTessFactor[edge]);
        m_NumPoints += processedTessFactors.numPointsForOutsideEdge[edge];
    }
    m_NumPoints -= 3;

    // inside edge offsets
    SetTessellationParity(processedTessFactors.insideTessFactorParity);
    processedTessFactors.numPointsForInsideTessFactor = NumPointsForTessFactor(processedTessFactors.insideTessFactor);
    {
        int pointCountMin = Odd() ? 4 : 3;
        // max() allows degenerate transition regions when inside TessFactor == 1
        processedTessFactors.numPointsForInsideTessFactor = max(pointCountMin,processedTessFactors.numPointsForInsideTessFactor);
    }

    processedTessFactors.insideEdgePointBaseOffset = m_NumPoints;

    // inside storage, including interior edges above
    {
        int numInteriorRings = (processedTessFactors.numPointsForInsideTessFactor >> 1) - 1;
        int numInteriorPoints;
        if( Odd() )
        {
            numInteriorPoints = TRI_EDGES*(numInteriorRings*(numInteriorRings+1) - numInteriorRings);
        }
        else
        {
            numInteriorPoints = TRI_EDGES*(numInteriorRings*(numInteriorRings+1)) + 1;
        }
        m_NumPoints += numInteriorPoints;
    }

}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::TriGeneratePoints
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::TriGeneratePoints( const PROCESSED_TESS_FACTORS_TRI& processedTessFactors )
{
    // Generate exterior ring edge points, clockwise starting from point V (VW, the U==0 edge)
    int pointOffset = 0;
    int edge;
    for(edge = 0; edge < TRI_EDGES; edge++ )
    {
        int parity = edge&0x1;
        int startPoint = 0;
        int endPoint = processedTessFactors.numPointsForOutsideEdge[edge] - 1;
        for(int p = startPoint; p < endPoint; p++, pointOffset++) // don't include end, since next edge starts with it.
        {
            FXP fxpParam;
            int q = (parity) ? p : endPoint - p; // whether to reverse point order given we are defining V or U (W implicit):
                                                 // edge0, VW, has V decreasing, so reverse 1D points below
                                                 // edge1, WU, has U increasing, so don't reverse 1D points  below
                                                 // edge2, UV, has U decreasing, so reverse 1D points below
            SetTessellationParity(processedTessFactors.outsideTessFactorParity[edge]);
            PlacePointIn1D(processedTessFactors.outsideTessFactorCtx[edge],q,fxpParam);
            if( edge == 0 )
            {
                DefinePoint(/*U*/0,
                            /*V*/fxpParam,
                            /*pointStorageOffset*/pointOffset);
            }
            else
            {
                DefinePoint(/*U*/fxpParam,
                            /*V*/(edge == 2) ? FXP_ONE - fxpParam : 0,
                            /*pointStorageOffset*/pointOffset);
            }
        }
    }

    // Generate interior ring points, clockwise spiralling in
    SetTessellationParity(processedTessFactors.insideTessFactorParity);
    static const int startRing = 1;
    int numRings = (processedTessFactors.numPointsForInsideTessFactor >> 1);
    for(int ring = startRing; ring < numRings; ring++)
    {
        int startPoint = ring;
        int endPoint = processedTessFactors.numPointsForInsideTessFactor - 1 - startPoint;

        for(edge = 0; edge < TRI_EDGES; edge++ )
        {
            int parity = edge&0x1;
            int perpendicularAxisPoint = startPoint;
            FXP fxpPerpParam;
            PlacePointIn1D(processedTessFactors.insideTessFactorCtx,perpendicularAxisPoint,fxpPerpParam);
            fxpPerpParam *= FXP_TWO_THIRDS; // Map location to the right size in barycentric space.
                                         // I (amarp) can draw a picture to explain.
                                         // We know this fixed point math won't over/underflow
            fxpPerpParam = (fxpPerpParam+FXP_ONE_HALF/*round*/)>>FXP_FRACTION_BITS; // get back to n.16
            for(int p = startPoint; p < endPoint; p++, pointOffset++) // don't include end: next edge starts with it.
            {
                FXP fxpParam;
                int q = (parity) ? p : endPoint - (p - startPoint); // whether to reverse point given we are defining V or U (W implicit):
                                                         // edge0, VW, has V decreasing, so reverse 1D points below
                                                         // edge1, WU, has U increasing, so don't reverse 1D points  below
                                                         // edge2, UV, has U decreasing, so reverse 1D points below
                PlacePointIn1D(processedTessFactors.insideTessFactorCtx,q,fxpParam);
                // edge0 VW, has perpendicular parameter U constant
                // edge1 WU, has perpendicular parameter V constant
                // edge2 UV, has perpendicular parameter W constant
                const unsigned int deriv = 2; // reciprocal is the rate of change of edge-parallel parameters as they are pushed into the triangle
                switch(edge)
                {
                case 0:
                    DefinePoint(/*U*/fxpPerpParam,
                                /*V*/fxpParam - (fxpPerpParam+1/*round*/)/deriv, // we know this fixed point math won't over/underflow
                                /*pointStorageOffset*/pointOffset);
                    break;
                case 1:
                    DefinePoint(/*U*/fxpParam - (fxpPerpParam+1/*round*/)/deriv,// we know this fixed point math won't over/underflow
                                /*V*/fxpPerpParam,
                                /*pointStorageOffset*/pointOffset);
                    break;
                case 2:
                    DefinePoint(/*U*/fxpParam - (fxpPerpParam+1/*round*/)/deriv,// we know this fixed point math won't over/underflow
                                /*V*/FXP_ONE - (fxpParam - (fxpPerpParam+1/*round*/)/deriv) - fxpPerpParam,// we know this fixed point math won't over/underflow
                                /*pointStorageOffset*/pointOffset);
                    break;
                }
            }
        }
    }
    if( !Odd() )
    {
        // Last point is the point at the center.
        DefinePoint(/*U*/FXP_ONE_THIRD,
                    /*V*/FXP_ONE_THIRD,
                    /*pointStorageOffset*/pointOffset);
    }
}
//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::TriGenerateConnectivity
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::TriGenerateConnectivity( const PROCESSED_TESS_FACTORS_TRI& processedTessFactors )
{
    // Generate primitives for all the concentric rings, one side at a time for each ring
    static const int startRing = 1;
    int numRings = ((processedTessFactors.numPointsForInsideTessFactor+1) >> 1); // +1 is so even tess includes the center point, which we want to now
    const TESS_FACTOR_CONTEXT* outsideTessFactorCtx[TRI_EDGES] = {&processedTessFactors.outsideTessFactorCtx[Ueq0],
                                            &processedTessFactors.outsideTessFactorCtx[Veq0],
                                            &processedTessFactors.outsideTessFactorCtx[Weq0]};
    TESSELLATOR_PARITY outsideTessFactorParity[TRI_EDGES] = {processedTessFactors.outsideTessFactorParity[Ueq0],
                                            processedTessFactors.outsideTessFactorParity[Veq0],
                                            processedTessFactors.outsideTessFactorParity[Weq0]};
    int numPointsForOutsideEdge[TRI_EDGES] = {processedTessFactors.numPointsForOutsideEdge[Ueq0],
                                              processedTessFactors.numPointsForOutsideEdge[Veq0],
                                              processedTessFactors.numPointsForOutsideEdge[Weq0]};

    int insideEdgePointBaseOffset = processedTessFactors.insideEdgePointBaseOffset;
    int outsideEdgePointBaseOffset = 0;
    int edge;
    for(int ring = startRing; ring < numRings; ring++)
    {
        int numPointsForInsideEdge = processedTessFactors.numPointsForInsideTessFactor - 2*ring;
        int edge0InsidePointBaseOffset = insideEdgePointBaseOffset;
        int edge0OutsidePointBaseOffset = outsideEdgePointBaseOffset;
        for(edge = 0; edge < TRI_EDGES; edge++ )
        {
            int numTriangles = numPointsForInsideEdge + numPointsForOutsideEdge[edge] - 2;

            int insideBaseOffset;
            int outsideBaseOffset;
            if( edge == 2 )
            {
                m_IndexPatchContext.insidePointIndexDeltaToRealValue    = insideEdgePointBaseOffset;
                m_IndexPatchContext.insidePointIndexBadValue            = numPointsForInsideEdge - 1;
                m_IndexPatchContext.insidePointIndexReplacementValue    = edge0InsidePointBaseOffset;
                m_IndexPatchContext.outsidePointIndexPatchBase          = m_IndexPatchContext.insidePointIndexBadValue+1; // past inside patched index range
                m_IndexPatchContext.outsidePointIndexDeltaToRealValue   = outsideEdgePointBaseOffset
                                                                            - m_IndexPatchContext.outsidePointIndexPatchBase;
                m_IndexPatchContext.outsidePointIndexBadValue           = m_IndexPatchContext.outsidePointIndexPatchBase
                                                                            + numPointsForOutsideEdge[edge] - 1;
                m_IndexPatchContext.outsidePointIndexReplacementValue   = edge0OutsidePointBaseOffset;
                SetUsingPatchedIndices(true);
                insideBaseOffset = 0;
                outsideBaseOffset = m_IndexPatchContext.outsidePointIndexPatchBase;
            }
            else
            {
                insideBaseOffset = insideEdgePointBaseOffset;
                outsideBaseOffset = outsideEdgePointBaseOffset;
            }
            if( ring == startRing )
            {
                StitchTransition(/*baseIndexOffset: */m_NumIndices,
                               insideBaseOffset,processedTessFactors.insideTessFactorCtx.numHalfTessFactorPoints,processedTessFactors.insideTessFactorParity,
                               outsideBaseOffset,outsideTessFactorCtx[edge]->numHalfTessFactorPoints,outsideTessFactorParity[edge]);
            }
            else
            {
                StitchRegular(/*bTrapezoid*/true, DIAGONALS_MIRRORED,
                              /*baseIndexOffset: */m_NumIndices,
                              numPointsForInsideEdge,
                              insideBaseOffset,outsideBaseOffset);
            }
            if( 2 == edge )
            {
                SetUsingPatchedIndices(false);
            }
            m_NumIndices += numTriangles*3;
            outsideEdgePointBaseOffset += numPointsForOutsideEdge[edge] - 1;
            insideEdgePointBaseOffset += numPointsForInsideEdge - 1;
            numPointsForOutsideEdge[edge] = numPointsForInsideEdge;
        }
        if( startRing == ring )
        {
            for(edge = 0; edge < TRI_EDGES; edge++ )
            {
                outsideTessFactorCtx[edge] = &processedTessFactors.insideTessFactorCtx;
                outsideTessFactorParity[edge] = processedTessFactors.insideTessFactorParity;
            }
        }
    }
    if( Odd() )
    {
        // Triangulate center (a single triangle)
        DefineClockwiseTriangle(outsideEdgePointBaseOffset, outsideEdgePointBaseOffset+1, outsideEdgePointBaseOffset+2,
                       m_NumIndices);
        m_NumIndices += 3;
    }
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::TessellateIsoLineDomain
// User calls this.
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::TessellateIsoLineDomain( float TessFactor_V_LineDensity, float TessFactor_U_LineDetail )
{
    PROCESSED_TESS_FACTORS_ISOLINE processedTessFactors;
    IsoLineProcessTessFactors(TessFactor_V_LineDensity,TessFactor_U_LineDetail,processedTessFactors);
    if( processedTessFactors.bPatchCulled )
    {
        m_NumPoints = 0;
        m_NumIndices = 0;
        return;
    }
    IsoLineGeneratePoints(processedTessFactors);
    IsoLineGenerateConnectivity(processedTessFactors); // can be done in parallel to IsoLineGeneratePoints
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::IsoLineProcessTessFactors
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::IsoLineProcessTessFactors( float TessFactor_V_LineDensity, float TessFactor_U_LineDetail,
                                                PROCESSED_TESS_FACTORS_ISOLINE& processedTessFactors )
{
    // Is the patch culled?
    if( !(TessFactor_V_LineDensity > 0) || // NaN will pass
        !(TessFactor_U_LineDetail > 0) )
    {
        processedTessFactors.bPatchCulled = true;
        return;
    }
    else
    {
        processedTessFactors.bPatchCulled = false;
    }

    // Clamp edge TessFactors
    float lowerBound = 0.0, upperBound = 0.0;
    switch(m_originalPartitioning)
    {
        case PIPE_TESSELLATOR_PARTITIONING_INTEGER:
        case PIPE_TESSELLATOR_PARTITIONING_POW2: // don�t care about pow2 distinction for validation, just treat as integer
            lowerBound = PIPE_TESSELLATOR_MIN_ODD_TESSELLATION_FACTOR;
            upperBound = PIPE_TESSELLATOR_MAX_EVEN_TESSELLATION_FACTOR;
            break;

        case PIPE_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN:
            lowerBound = PIPE_TESSELLATOR_MIN_EVEN_TESSELLATION_FACTOR;
            upperBound = PIPE_TESSELLATOR_MAX_EVEN_TESSELLATION_FACTOR;
            break;

        case PIPE_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD:
            lowerBound = PIPE_TESSELLATOR_MIN_ODD_TESSELLATION_FACTOR;
            upperBound = PIPE_TESSELLATOR_MAX_ODD_TESSELLATION_FACTOR;
            break;
    }

    TessFactor_V_LineDensity = tess_fmin( PIPE_TESSELLATOR_MAX_ISOLINE_DENSITY_TESSELLATION_FACTOR,
                                    tess_fmax( PIPE_TESSELLATOR_MIN_ISOLINE_DENSITY_TESSELLATION_FACTOR, TessFactor_V_LineDensity ) );
    TessFactor_U_LineDetail = tess_fmin( upperBound, tess_fmax( lowerBound, TessFactor_U_LineDetail ) );

    // Reset our vertex and index buffers.  We have enough storage for the max tessFactor.
    m_NumPoints = 0;
    m_NumIndices = 0;

    // Process tessFactors
    if( HWIntegerPartitioning() )
    {
        TessFactor_U_LineDetail = ceil(TessFactor_U_LineDetail);
        processedTessFactors.lineDetailParity = isEven(TessFactor_U_LineDetail) ? TESSELLATOR_PARITY_EVEN : TESSELLATOR_PARITY_ODD;
    }
    else
    {
        processedTessFactors.lineDetailParity = m_originalParity;
    }

    FXP fxpTessFactor_U_LineDetail = floatToFixed(TessFactor_U_LineDetail);

    SetTessellationParity(processedTessFactors.lineDetailParity);

    ComputeTessFactorContext(fxpTessFactor_U_LineDetail, processedTessFactors.lineDetailTessFactorCtx);
    processedTessFactors.numPointsPerLine = NumPointsForTessFactor(fxpTessFactor_U_LineDetail);

    OverridePartitioning(PIPE_TESSELLATOR_PARTITIONING_INTEGER);

    TessFactor_V_LineDensity = ceil(TessFactor_V_LineDensity);
    processedTessFactors.lineDensityParity = isEven(TessFactor_V_LineDensity) ? TESSELLATOR_PARITY_EVEN : TESSELLATOR_PARITY_ODD;
    SetTessellationParity(processedTessFactors.lineDensityParity);
    FXP fxpTessFactor_V_LineDensity = floatToFixed(TessFactor_V_LineDensity);
    ComputeTessFactorContext(fxpTessFactor_V_LineDensity, processedTessFactors.lineDensityTessFactorCtx);

    processedTessFactors.numLines = NumPointsForTessFactor(fxpTessFactor_V_LineDensity) - 1; // don't draw last line at V == 1.

    RestorePartitioning();

    // Compute some initial data.

    // outside edge offsets
    m_NumPoints = processedTessFactors.numPointsPerLine * processedTessFactors.numLines;
    if( m_outputPrimitive == PIPE_TESSELLATOR_OUTPUT_POINT )
    {
        m_NumIndices = m_NumPoints;
    }
    else // line
    {
        m_NumIndices = processedTessFactors.numLines*(processedTessFactors.numPointsPerLine-1)*2;
    }
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::IsoLineGeneratePoints
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::IsoLineGeneratePoints( const PROCESSED_TESS_FACTORS_ISOLINE& processedTessFactors )
{
    int line, pointOffset;
    for(line = 0, pointOffset = 0; line < processedTessFactors.numLines; line++)
    {
        for(int point = 0; point < processedTessFactors.numPointsPerLine; point++)
        {
            FXP fxpU,fxpV;
            SetTessellationParity(processedTessFactors.lineDensityParity);
            PlacePointIn1D(processedTessFactors.lineDensityTessFactorCtx,line,fxpV);

            SetTessellationParity(processedTessFactors.lineDetailParity);
            PlacePointIn1D(processedTessFactors.lineDetailTessFactorCtx,point,fxpU);

            DefinePoint(fxpU,fxpV,pointOffset++);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::IsoLineGenerateConnectivity
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::IsoLineGenerateConnectivity( const PROCESSED_TESS_FACTORS_ISOLINE& processedTessFactors )
{
    int line, pointOffset, indexOffset;
    if( m_outputPrimitive == PIPE_TESSELLATOR_OUTPUT_POINT )
    {
        for(line = 0, pointOffset = 0, indexOffset = 0; line < processedTessFactors.numLines; line++)
        {
            for(int point = 0; point < processedTessFactors.numPointsPerLine; point++)
            {
                DefineIndex(pointOffset++,indexOffset++);
            }
        }
    }
    else // line
    {
        for(line = 0, pointOffset = 0, indexOffset = 0; line < processedTessFactors.numLines; line++)
        {
            for(int point = 0; point < processedTessFactors.numPointsPerLine; point++)
            {
                if( point > 0 )
                {
                    DefineIndex(pointOffset-1,indexOffset++);
                    DefineIndex(pointOffset,indexOffset++);
                }
                pointOffset++;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::GetPointCount
// User calls this.
//---------------------------------------------------------------------------------------------------------------------------------
int CHWTessellator::GetPointCount()
{
    return m_NumPoints;
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::GetIndexCount()
// User calls this.
//---------------------------------------------------------------------------------------------------------------------------------
int CHWTessellator::GetIndexCount()
{
    return m_NumIndices;
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::GetPoints()
// User calls this.
//---------------------------------------------------------------------------------------------------------------------------------
DOMAIN_POINT* CHWTessellator::GetPoints()
{
    return m_Point;
}
//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::GetIndices()
// User calls this.
//---------------------------------------------------------------------------------------------------------------------------------
int* CHWTessellator::GetIndices()
{
    return m_Index;
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::DefinePoint()
//---------------------------------------------------------------------------------------------------------------------------------
int CHWTessellator::DefinePoint(FXP fxpU, FXP fxpV, int pointStorageOffset)
{
//    WCHAR foo[80];
//    StringCchPrintf(foo,80,L"off:%d, uv=(%f,%f)\n",pointStorageOffset,fixedToFloat(fxpU),fixedToFloat(fxpV));
//    OutputDebugString(foo);
    m_Point[pointStorageOffset].u = fixedToFloat(fxpU);
    m_Point[pointStorageOffset].v = fixedToFloat(fxpV);
    return pointStorageOffset;
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::DefineIndex()
//--------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::DefineIndex(int index, int indexStorageOffset)
{
    index = PatchIndexValue(index);
//    WCHAR foo[80];
//    StringCchPrintf(foo,80,L"off:%d, idx=%d, uv=(%f,%f)\n",indexStorageOffset,index,m_Point[index].u,m_Point[index].v);
//    OutputDebugString(foo);
    m_Index[indexStorageOffset] = index;
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::DefineClockwiseTriangle()
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::DefineClockwiseTriangle(int index0, int index1, int index2, int indexStorageBaseOffset)
{
    // inputs a clockwise triangle, stores a CW or CCW triangle depending on the state
    DefineIndex(index0,indexStorageBaseOffset);
    bool bWantClockwise = (m_outputPrimitive == PIPE_TESSELLATOR_OUTPUT_TRIANGLE_CW) ? true : false;
    if( bWantClockwise )
    {
        DefineIndex(index1,indexStorageBaseOffset+1);
        DefineIndex(index2,indexStorageBaseOffset+2);
    }
    else
    {
        DefineIndex(index2,indexStorageBaseOffset+1);
        DefineIndex(index1,indexStorageBaseOffset+2);
    }
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::DumpAllPoints()
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::DumpAllPoints()
{
    for( int p = 0; p < m_NumPoints; p++ )
    {
        DefineIndex(p,m_NumIndices++);
    }
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::DumpAllPointsAsInOrderLineList()
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::DumpAllPointsAsInOrderLineList()
{
    for( int p = 1; p < m_NumPoints; p++ )
    {
        DefineIndex(p-1,m_NumIndices++);
        DefineIndex(p,m_NumIndices++);
    }
}

//---------------------------------------------------------------------------------------------------------------------------------
// RemoveMSB
//---------------------------------------------------------------------------------------------------------------------------------
int RemoveMSB(int val)
{
    int check;
    if( val <= 0x0000ffff ) { check = ( val <= 0x000000ff ) ? 0x00000080 : 0x00008000; }
    else                    { check = ( val <= 0x00ffffff ) ? 0x00800000 : 0x80000000; }
    for( int i = 0; i < 8; i++, check >>= 1 ) { if( val & check ) return (val & ~check); }
    return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------
// GetMSB
//---------------------------------------------------------------------------------------------------------------------------------
int GetMSB(int val)
{
    int check;
    if( val <= 0x0000ffff ) { check = ( val <= 0x000000ff ) ? 0x00000080 : 0x00008000; }
    else                    { check = ( val <= 0x00ffffff ) ? 0x00800000 : 0x80000000; }
    for( int i = 0; i < 8; i++, check >>= 1 ) { if( val & check ) return check; }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::CleanseParameter()
//---------------------------------------------------------------------------------------------------------------------------------
/* NOTHING TO DO FOR FIXED POINT ARITHMETIC!
void CHWTessellator::CleanseParameter(float& parameter)
{
    // Clean up [0..1] parameter to guarantee that (1 - (1 - parameter)) == parameter.
    parameter = 1.0f - parameter;
    parameter = 1.0f - parameter;

}
*/
//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::NumPointsForTessFactor()
//---------------------------------------------------------------------------------------------------------------------------------
int CHWTessellator::NumPointsForTessFactor( FXP fxpTessFactor )
{
    int numPoints;
    if( Odd() )
    {
        numPoints = (fxpCeil(FXP_ONE_HALF + (fxpTessFactor+1/*round*/)/2)*2)>>FXP_FRACTION_BITS;
    }
    else
    {
        numPoints = ((fxpCeil((fxpTessFactor+1/*round*/)/2)*2)>>FXP_FRACTION_BITS)+1;
    }
    return numPoints;
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::ComputeTessFactorContext()
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::ComputeTessFactorContext( FXP fxpTessFactor, TESS_FACTOR_CONTEXT& TessFactorCtx )
{
    FXP fxpHalfTessFactor = (fxpTessFactor+1/*round*/)/2;
    if( Odd() || (fxpHalfTessFactor == FXP_ONE_HALF)) // fxpHalfTessFactor == 1/2 if TessFactor is 1, but we're pretending we are even.
    {
        fxpHalfTessFactor += FXP_ONE_HALF;
    }
    FXP fxpFloorHalfTessFactor = fxpFloor(fxpHalfTessFactor);
    FXP fxpCeilHalfTessFactor = fxpCeil(fxpHalfTessFactor);
    TessFactorCtx.fxpHalfTessFactorFraction = fxpHalfTessFactor - fxpFloorHalfTessFactor;
    //CleanseParameter(TessFactorCtx.fxpHalfTessFactorFraction);
    TessFactorCtx.numHalfTessFactorPoints = (fxpCeilHalfTessFactor>>FXP_FRACTION_BITS); // for EVEN, we don't include the point always fixed at the midpoint of the TessFactor
    if( fxpCeilHalfTessFactor == fxpFloorHalfTessFactor )
    {
        TessFactorCtx.splitPointOnFloorHalfTessFactor =  /*pick value to cause this to be ignored*/ TessFactorCtx.numHalfTessFactorPoints+1;
    }
    else if( Odd() )
    {
        if( fxpFloorHalfTessFactor == FXP_ONE )
        {
            TessFactorCtx.splitPointOnFloorHalfTessFactor = 0;
        }
        else
        {
				TessFactorCtx.splitPointOnFloorHalfTessFactor = (RemoveMSB((fxpFloorHalfTessFactor>>FXP_FRACTION_BITS)-1)<<1) + 1;
        }
    }
    else
    {
			TessFactorCtx.splitPointOnFloorHalfTessFactor = (RemoveMSB(fxpFloorHalfTessFactor>>FXP_FRACTION_BITS)<<1) + 1;
    }
    int numFloorSegments = (fxpFloorHalfTessFactor * 2)>>FXP_FRACTION_BITS;
    int numCeilSegments = (fxpCeilHalfTessFactor * 2)>>FXP_FRACTION_BITS;
    if( Odd() )
    {
        numFloorSegments -= 1;
        numCeilSegments -= 1;
    }
    TessFactorCtx.fxpInvNumSegmentsOnFloorTessFactor = s_fixedReciprocal[numFloorSegments];
    TessFactorCtx.fxpInvNumSegmentsOnCeilTessFactor = s_fixedReciprocal[numCeilSegments];
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::PlacePointIn1D()
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::PlacePointIn1D( const TESS_FACTOR_CONTEXT& TessFactorCtx, int point, FXP& fxpLocation )
{
    bool bFlip;
    if( point >= TessFactorCtx.numHalfTessFactorPoints )
    {
        point = (TessFactorCtx.numHalfTessFactorPoints << 1) - point;
        if( Odd() )
        {
            point -= 1;
        }
        bFlip = true;
    }
    else
    {
        bFlip = false;
    }
    if( point == TessFactorCtx.numHalfTessFactorPoints )
    {
        fxpLocation = FXP_ONE_HALF; // special casing middle since 16 bit fixed math below can't reproduce 0.5 exactly
        return;
    }
    unsigned int indexOnCeilHalfTessFactor = point;
    unsigned int indexOnFloorHalfTessFactor = indexOnCeilHalfTessFactor;
    if( point > TessFactorCtx.splitPointOnFloorHalfTessFactor )
    {
        indexOnFloorHalfTessFactor -= 1;
    }
    // For the fixed point multiplies below, we know the results are <= 16 bits because
    // the locations on the halfTessFactor are <= half the number of segments for the total TessFactor.
    // So a number divided by a number that is at least twice as big will give
    // a result no bigger than 0.5 (which in fixed point is 16 bits in our case)
    FXP fxpLocationOnFloorHalfTessFactor = indexOnFloorHalfTessFactor * TessFactorCtx.fxpInvNumSegmentsOnFloorTessFactor;
    FXP fxpLocationOnCeilHalfTessFactor = indexOnCeilHalfTessFactor * TessFactorCtx.fxpInvNumSegmentsOnCeilTessFactor;

    // Since we know the numbers calculated above are <= fixed point 0.5, and the equation
    // below is just lerping between two values <= fixed point 0.5 (0x00008000), then we know
    // that the final result before shifting by 16 bits is no larger than 0x80000000.  Once we
    // shift that down by 16, we get the result of lerping 2 numbers <= 0.5, which is obviously
    // at most 0.5 (0x00008000)
    fxpLocation = fxpLocationOnFloorHalfTessFactor * (FXP_ONE - TessFactorCtx.fxpHalfTessFactorFraction) +
                  fxpLocationOnCeilHalfTessFactor * (TessFactorCtx.fxpHalfTessFactorFraction);
    fxpLocation = (fxpLocation + FXP_ONE_HALF/*round*/) >> FXP_FRACTION_BITS; // get back to n.16
    /* Commenting out floating point version.  Note the parameter cleansing it does is not needed in fixed point.
    if( bFlip )
        location = 1.0f - location; // complement produces cleansed result.
    else
        CleanseParameter(location);
    */
    if( bFlip )
    {
        fxpLocation = FXP_ONE - fxpLocation;
    }
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::StitchRegular
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::StitchRegular(bool bTrapezoid,DIAGONALS diagonals,
                                 int baseIndexOffset, int numInsideEdgePoints,
                                 int insideEdgePointBaseOffset, int outsideEdgePointBaseOffset)
{
    int insidePoint = insideEdgePointBaseOffset;
    int outsidePoint = outsideEdgePointBaseOffset;
    if( bTrapezoid )
    {
        DefineClockwiseTriangle(outsidePoint,outsidePoint+1,insidePoint,baseIndexOffset);
        baseIndexOffset += 3; outsidePoint++;
    }
    int p;
    switch( diagonals )
    {
    case DIAGONALS_INSIDE_TO_OUTSIDE:
        // Diagonals pointing from inside edge forward towards outside edge
        for( p = 0; p < numInsideEdgePoints-1; p++ )
        {
            DefineClockwiseTriangle(insidePoint,outsidePoint,outsidePoint+1,baseIndexOffset);
            baseIndexOffset += 3;

            DefineClockwiseTriangle(insidePoint,outsidePoint+1,insidePoint+1,baseIndexOffset);
            baseIndexOffset += 3;
            insidePoint++; outsidePoint++;
        }
        break;
    case DIAGONALS_INSIDE_TO_OUTSIDE_EXCEPT_MIDDLE: // Assumes ODD tessellation
        // Diagonals pointing from outside edge forward towards inside edge

        // First half
        for( p = 0; p < numInsideEdgePoints/2-1; p++ )
        {
            DefineClockwiseTriangle(outsidePoint,outsidePoint+1,insidePoint,baseIndexOffset);
            baseIndexOffset += 3;
            DefineClockwiseTriangle(insidePoint,outsidePoint+1,insidePoint+1,baseIndexOffset);
            baseIndexOffset += 3;
            insidePoint++; outsidePoint++;
        }

        // Middle
        DefineClockwiseTriangle(outsidePoint,insidePoint+1,insidePoint,baseIndexOffset);
        baseIndexOffset += 3;
        DefineClockwiseTriangle(outsidePoint,outsidePoint+1,insidePoint+1,baseIndexOffset);
        baseIndexOffset += 3;
        insidePoint++; outsidePoint++; p+=2;

        // Second half
        for( ; p < numInsideEdgePoints; p++ )
        {
            DefineClockwiseTriangle(outsidePoint,outsidePoint+1,insidePoint,baseIndexOffset);
            baseIndexOffset += 3;
            DefineClockwiseTriangle(insidePoint,outsidePoint+1,insidePoint+1,baseIndexOffset);
            baseIndexOffset += 3;
            insidePoint++; outsidePoint++;
        }
        break;
    case DIAGONALS_MIRRORED:
        // First half, diagonals pointing from outside of outside edge to inside of inside edge
        for( p = 0; p < numInsideEdgePoints/2; p++ )
        {
            DefineClockwiseTriangle(outsidePoint,insidePoint+1,insidePoint,baseIndexOffset);
            baseIndexOffset += 3;
            DefineClockwiseTriangle(outsidePoint,outsidePoint+1,insidePoint+1,baseIndexOffset);
            baseIndexOffset += 3;
            insidePoint++; outsidePoint++;
        }
        // Second half, diagonals pointing from inside of inside edge to outside of outside edge
        for( ; p < numInsideEdgePoints-1; p++ )
        {
            DefineClockwiseTriangle(insidePoint,outsidePoint,outsidePoint+1,baseIndexOffset);
            baseIndexOffset += 3;
            DefineClockwiseTriangle(insidePoint,outsidePoint+1,insidePoint+1,baseIndexOffset);
            baseIndexOffset += 3;
            insidePoint++; outsidePoint++;
        }
        break;
    }
    if( bTrapezoid )
    {
        DefineClockwiseTriangle(outsidePoint,outsidePoint+1,insidePoint,baseIndexOffset);
        baseIndexOffset += 3;
    }
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::StitchTransition()
//---------------------------------------------------------------------------------------------------------------------------------
void CHWTessellator::StitchTransition(int baseIndexOffset,
                                    int insideEdgePointBaseOffset, int insideNumHalfTessFactorPoints,
                                    TESSELLATOR_PARITY insideEdgeTessFactorParity,
                                    int outsideEdgePointBaseOffset, int outsideNumHalfTessFactorPoints,
                                    TESSELLATOR_PARITY outsideTessFactorParity
)
{
    // Tables to assist in the stitching of 2 rows of points having arbitrary TessFactors.
    // The stitching order is governed by Ruler Function vertex split ordering (see external documentation).
    //
    // The contents of the finalPointPositionTable are where vertex i [0..33] ends up on the half-edge
    // at the max tessellation amount given ruler-function split order.
    // Recall the other half of an edge is mirrored, so we only need to deal with one half.
    // This table is used to decide when to advance a point on the interior or exterior.
    // It supports odd TessFactor up to 65 and even TessFactor up to 64.
    static const int finalPointPositionTable[33] =
            { 0, 32, 16, 8, 17, 4, 18, 9, 19, 2, 20, 10, 21, 5, 22, 11, 23,
              1, 24, 12, 25, 6, 26, 13, 27, 3, 28, 14, 29, 7, 30, 15, 31 };

    // The loopStart and loopEnd tables below just provide optimal loop bounds for the
    // stitching algorithm further below, for any given halfTssFactor.
    // There is probably a better way to encode this...

    // loopStart[halfTessFactor] encodes the FIRST entry in finalPointPositionTable[] above which is
    // less than halfTessFactor.  Exceptions are entry 0 and 1, which are set up to skip the loop.
    static const int loopStart[33] =
            {1,1,17,9,9,5,5,5,5,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};
    // loopStart[halfTessFactor] encodes the LAST entry in finalPointPositionTable[] above which is
    // less than halfTessFactor.  Exceptions are entry 0 and 1, which are set up to skip the loop.
    static const int loopEnd[33] =
            {0,0,17,17,25,25,25,25,29,29,29,29,29,29,29,29,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,32};

    if( TESSELLATOR_PARITY_ODD == insideEdgeTessFactorParity )
    {
        insideNumHalfTessFactorPoints -= 1;
    }
    if( TESSELLATOR_PARITY_ODD == outsideTessFactorParity )
    {
        outsideNumHalfTessFactorPoints -= 1;
    }
    // Walk first half
    int outsidePoint = outsideEdgePointBaseOffset;
    int insidePoint = insideEdgePointBaseOffset;

    // iStart,iEnd are a small optimization so the loop below doesn't have to go from 0 up to 31
    int iStart = min(loopStart[insideNumHalfTessFactorPoints],loopStart[outsideNumHalfTessFactorPoints]);
    int iEnd = max(loopEnd[insideNumHalfTessFactorPoints],loopEnd[outsideNumHalfTessFactorPoints]);

    if( finalPointPositionTable[0] < outsideNumHalfTessFactorPoints ) // since we dont' start the loop at 0 below, we need a special case.
    {
        // Advance outside
        DefineClockwiseTriangle(outsidePoint,outsidePoint+1,insidePoint,baseIndexOffset);
        baseIndexOffset += 3; outsidePoint++;
    }

    for(int i = iStart; i <= iEnd; i++)
    {
        if( /*(i>0) && <-- not needed since iStart is never 0*/(finalPointPositionTable[i] < insideNumHalfTessFactorPoints))
        {
            // Advance inside
            DefineClockwiseTriangle(insidePoint,outsidePoint,insidePoint+1,baseIndexOffset);
            baseIndexOffset += 3; insidePoint++;
        }
        if((finalPointPositionTable[i] < outsideNumHalfTessFactorPoints))
        {
            // Advance outside
            DefineClockwiseTriangle(outsidePoint,outsidePoint+1,insidePoint,baseIndexOffset);
            baseIndexOffset += 3; outsidePoint++;
        }
    }

    if( (insideEdgeTessFactorParity != outsideTessFactorParity) || (insideEdgeTessFactorParity == TESSELLATOR_PARITY_ODD))
    {
        if( insideEdgeTessFactorParity == outsideTessFactorParity )
        {
            // Quad in the middle
            DefineClockwiseTriangle(insidePoint,outsidePoint,insidePoint+1,baseIndexOffset);
            baseIndexOffset += 3;
            DefineClockwiseTriangle(insidePoint+1,outsidePoint,outsidePoint+1,baseIndexOffset);
            baseIndexOffset += 3;
            insidePoint++;
            outsidePoint++;
        }
        else if( TESSELLATOR_PARITY_EVEN == insideEdgeTessFactorParity )
        {
            // Triangle pointing inside
            DefineClockwiseTriangle(insidePoint,outsidePoint,outsidePoint+1,baseIndexOffset);
            baseIndexOffset += 3;
            outsidePoint++;
        }
        else
        {
            // Triangle pointing outside
            DefineClockwiseTriangle(insidePoint,outsidePoint,insidePoint+1,baseIndexOffset);
            baseIndexOffset += 3;
            insidePoint++;
        }
    }

    // Walk second half.
    for(int i = iEnd; i >= iStart; i--)
    {
        if((finalPointPositionTable[i] < outsideNumHalfTessFactorPoints))
        {
            // Advance outside
            DefineClockwiseTriangle(outsidePoint,outsidePoint+1,insidePoint,baseIndexOffset);
            baseIndexOffset += 3; outsidePoint++;
        }
        if( /*(i>0) && <-- not needed since iStart is never 0*/ (finalPointPositionTable[i] < insideNumHalfTessFactorPoints))
        {
            // Advance inside
            DefineClockwiseTriangle(insidePoint,outsidePoint,insidePoint+1,baseIndexOffset);
            baseIndexOffset += 3; insidePoint++;
        }
    }
    // Below case is not needed if we didn't optimize loop above and made it run from 31 down to 0.
    if((finalPointPositionTable[0] < outsideNumHalfTessFactorPoints))
    {
        DefineClockwiseTriangle(outsidePoint,outsidePoint+1,insidePoint,baseIndexOffset);
        baseIndexOffset += 3; outsidePoint++;
    }
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHWTessellator::PatchIndexValue()
//--------------------------------------------------------------------------------------------------------------------------------
int CHWTessellator::PatchIndexValue(int index)
{
    if( m_bUsingPatchedIndices )
    {
        if( index >= m_IndexPatchContext.outsidePointIndexPatchBase ) // assumed remapped outide indices are > remapped inside vertices
        {
            if( index == m_IndexPatchContext.outsidePointIndexBadValue )
                index = m_IndexPatchContext.outsidePointIndexReplacementValue;
            else
                index += m_IndexPatchContext.outsidePointIndexDeltaToRealValue;
        }
        else
        {
            if( index == m_IndexPatchContext.insidePointIndexBadValue )
                index = m_IndexPatchContext.insidePointIndexReplacementValue;
            else
                index += m_IndexPatchContext.insidePointIndexDeltaToRealValue;
        }
    }
    else if( m_bUsingPatchedIndices2 )
    {
        if( index >= m_IndexPatchContext2.baseIndexToInvert )
        {
            if( index == m_IndexPatchContext2.cornerCaseBadValue )
            {
                index = m_IndexPatchContext2.cornerCaseReplacementValue;
            }
            else
            {
                index = m_IndexPatchContext2.indexInversionEndPoint - index;
            }
        }
        else if( index == m_IndexPatchContext2.cornerCaseBadValue )
        {
            index = m_IndexPatchContext2.cornerCaseReplacementValue;
        }
    }
    return index;
}


//=================================================================================================================================
// CHLSLTessellator
//=================================================================================================================================

//---------------------------------------------------------------------------------------------------------------------------------
// CHLSLTessellator::CHLSLTessellator
//---------------------------------------------------------------------------------------------------------------------------------
CHLSLTessellator::CHLSLTessellator()
{
    m_LastComputedTessFactors[0] = m_LastComputedTessFactors[1] = m_LastComputedTessFactors[2] =
    m_LastComputedTessFactors[3] = m_LastComputedTessFactors[4] = m_LastComputedTessFactors[5] = 0;
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHLSLTessellator::Init
// User calls this.
//---------------------------------------------------------------------------------------------------------------------------------
void CHLSLTessellator::Init(
    PIPE_TESSELLATOR_PARTITIONING       partitioning,
    PIPE_TESSELLATOR_REDUCTION          insideTessFactorReduction,
    PIPE_TESSELLATOR_QUAD_REDUCTION_AXIS quadInsideTessFactorReductionAxis,
    PIPE_TESSELLATOR_OUTPUT_PRIMITIVE   outputPrimitive)
{
    CHWTessellator::Init(partitioning,outputPrimitive);
    m_LastComputedTessFactors[0] = m_LastComputedTessFactors[1] = m_LastComputedTessFactors[2] =
    m_LastComputedTessFactors[3] = m_LastComputedTessFactors[4] = m_LastComputedTessFactors[5] = 0;
    m_partitioning = partitioning;
    m_originalPartitioning = partitioning;
    switch( partitioning )
    {
    case PIPE_TESSELLATOR_PARTITIONING_INTEGER:
    default:
        break;
    case PIPE_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD:
        m_parity = TESSELLATOR_PARITY_ODD;
        break;
    case PIPE_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN:
        m_parity = TESSELLATOR_PARITY_EVEN;
        break;
    }
    m_originalParity = m_parity;
    m_outputPrimitive = outputPrimitive;
    m_insideTessFactorReduction = insideTessFactorReduction;
    m_quadInsideTessFactorReductionAxis = quadInsideTessFactorReductionAxis;
}
//---------------------------------------------------------------------------------------------------------------------------------
// CHLSLTessellator::TessellateQuadDomain
// User calls this
//---------------------------------------------------------------------------------------------------------------------------------
void CHLSLTessellator::TessellateQuadDomain( float tessFactor_Ueq0, float tessFactor_Veq0, float tessFactor_Ueq1, float tessFactor_Veq1,
                                         float insideTessFactorScaleU, float insideTessFactorScaleV )
{
    QuadHLSLProcessTessFactors(tessFactor_Ueq0,tessFactor_Veq0,tessFactor_Ueq1,tessFactor_Veq1,insideTessFactorScaleU,insideTessFactorScaleV);

    CHWTessellator::TessellateQuadDomain(m_LastComputedTessFactors[0],m_LastComputedTessFactors[1],m_LastComputedTessFactors[2],m_LastComputedTessFactors[3],
                                         m_LastComputedTessFactors[4],m_LastComputedTessFactors[5]);
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHLSLTessellator::QuadHLSLProcessTessFactors
//---------------------------------------------------------------------------------------------------------------------------------
void CHLSLTessellator::QuadHLSLProcessTessFactors( float tessFactor_Ueq0, float tessFactor_Veq0, float tessFactor_Ueq1, float tessFactor_Veq1,
                                               float insideTessFactorScaleU, float insideTessFactorScaleV )
{
    if( !(tessFactor_Ueq0 > 0) ||// NaN will pass
        !(tessFactor_Veq0 > 0) ||
        !(tessFactor_Ueq1 > 0) ||
        !(tessFactor_Veq1 > 0) )
    {
        m_LastUnRoundedComputedTessFactors[0] = tessFactor_Ueq0;
        m_LastUnRoundedComputedTessFactors[1] = tessFactor_Veq0;
        m_LastUnRoundedComputedTessFactors[2] = tessFactor_Ueq1;
        m_LastUnRoundedComputedTessFactors[3] = tessFactor_Veq1;
        m_LastUnRoundedComputedTessFactors[4] = 0;
        m_LastUnRoundedComputedTessFactors[5] = 0;
        m_LastComputedTessFactors[0] =
        m_LastComputedTessFactors[1] =
        m_LastComputedTessFactors[2] =
        m_LastComputedTessFactors[3] =
        m_LastComputedTessFactors[4] =
        m_LastComputedTessFactors[5] = 0;
        return;
    }

    CleanupFloatTessFactor(tessFactor_Ueq0);// clamp to [1.0f..INF], NaN->1.0f
    CleanupFloatTessFactor(tessFactor_Veq0);
    CleanupFloatTessFactor(tessFactor_Ueq1);
    CleanupFloatTessFactor(tessFactor_Veq1);

    // Save off tessFactors so they can be returned to app
    m_LastUnRoundedComputedTessFactors[0] = tessFactor_Ueq0;
    m_LastUnRoundedComputedTessFactors[1] = tessFactor_Veq0;
    m_LastUnRoundedComputedTessFactors[2] = tessFactor_Ueq1;
    m_LastUnRoundedComputedTessFactors[3] = tessFactor_Veq1;

    // Process outside tessFactors
    float outsideTessFactor[QUAD_EDGES] = {tessFactor_Ueq0, tessFactor_Veq0, tessFactor_Ueq1, tessFactor_Veq1};
    int edge, axis;
    TESSELLATOR_PARITY insideTessFactorParity[QUAD_AXES];
    if( Pow2Partitioning() || IntegerPartitioning() )
    {
        for( edge = 0; edge < QUAD_EDGES; edge++ )
        {
            RoundUpTessFactor(outsideTessFactor[edge]);
            ClampTessFactor(outsideTessFactor[edge]); // clamp unbounded user input based on tessellation mode
        }
    }
    else
    {
        SetTessellationParity(m_originalParity); // ClampTessFactor needs it
        for( edge = 0; edge < QUAD_EDGES; edge++ )
        {
            ClampTessFactor(outsideTessFactor[edge]); // clamp unbounded user input based on tessellation mode
        }
    }

    // Compute inside TessFactors
    float insideTessFactor[QUAD_AXES];
    if( m_quadInsideTessFactorReductionAxis == PIPE_TESSELLATOR_QUAD_REDUCTION_1_AXIS )
    {
        switch( m_insideTessFactorReduction )
        {
        case PIPE_TESSELLATOR_REDUCTION_MIN:
            insideTessFactor[U] = tess_fmin(tess_fmin(tessFactor_Veq0,tessFactor_Veq1),tess_fmin(tessFactor_Ueq0,tessFactor_Ueq1));
            break;
        case PIPE_TESSELLATOR_REDUCTION_MAX:
            insideTessFactor[U] = tess_fmax(tess_fmax(tessFactor_Veq0,tessFactor_Veq1),tess_fmax(tessFactor_Ueq0,tessFactor_Ueq1));
            break;
        case PIPE_TESSELLATOR_REDUCTION_AVERAGE:
            insideTessFactor[U] = (tessFactor_Veq0 + tessFactor_Veq1 + tessFactor_Ueq0 + tessFactor_Ueq1) / 4;
            break;
        default:
            unreachable("impossible m_insideTessFactorReduction");
        }
        // Scale inside tessFactor based on user scale factor.

        ClampFloatTessFactorScale(insideTessFactorScaleU); // clamp scale value to [0..1], NaN->0
        insideTessFactor[U] = insideTessFactor[U]*insideTessFactorScaleU;

        // Compute inside parity
        if( Pow2Partitioning() || IntegerPartitioning() )
        {
            ClampTessFactor(insideTessFactor[U]); // clamp reduction + scale result that is based on unbounded user input
            m_LastUnRoundedComputedTessFactors[4] = m_LastUnRoundedComputedTessFactors[5] = insideTessFactor[U]; // Save off TessFactors so they can be returned to app
            RoundUpTessFactor(insideTessFactor[U]);
            insideTessFactorParity[U] =
            insideTessFactorParity[V] =
                (isEven(insideTessFactor[U]) || (FLOAT_ONE == insideTessFactor[U]) )
                ? TESSELLATOR_PARITY_EVEN : TESSELLATOR_PARITY_ODD;
        }
        else
        {
            ClampTessFactor(insideTessFactor[U]); // clamp reduction + scale result that is based on unbounded user input
            m_LastUnRoundedComputedTessFactors[4] = m_LastUnRoundedComputedTessFactors[5] = insideTessFactor[U]; // Save off TessFactors so they can be returned to app
            // no parity changes for fractional tessellation - just use what the user requested
            insideTessFactorParity[U] = insideTessFactorParity[V] = m_originalParity;
        }

        // To prevent snapping on edges, the "picture frame" comes
        // in using avg or max (and ignore inside TessFactor scaling) until it is at least 3.
        if( (TESSELLATOR_PARITY_ODD == insideTessFactorParity[U]) &&
            (insideTessFactor[U] < FLOAT_THREE) )
        {
            if(PIPE_TESSELLATOR_REDUCTION_MAX == m_insideTessFactorReduction)
            {
                insideTessFactor[U] = tess_fmin(FLOAT_THREE,tess_fmax(tess_fmax(tessFactor_Veq0,tessFactor_Veq1),tess_fmax(tessFactor_Ueq0,tessFactor_Ueq1)));
            }
            else
            {
                insideTessFactor[U] = tess_fmin(FLOAT_THREE,(tessFactor_Veq0 + tessFactor_Veq1 + tessFactor_Ueq0 + tessFactor_Ueq1) / 4);
            }
            ClampTessFactor(insideTessFactor[U]); // clamp reduction result that is based on unbounded user input
            m_LastUnRoundedComputedTessFactors[4] = m_LastUnRoundedComputedTessFactors[5] = insideTessFactor[U]; // Save off TessFactors so they can be returned to app
            if( IntegerPartitioning())
            {
                RoundUpTessFactor(insideTessFactor[U]);
                insideTessFactorParity[U] =
                insideTessFactorParity[V] = isEven(insideTessFactor[U]) ? TESSELLATOR_PARITY_EVEN : TESSELLATOR_PARITY_ODD;
            }
        }
        insideTessFactor[V] = insideTessFactor[U];
    }
    else
    {
        switch( m_insideTessFactorReduction )
        {
        case PIPE_TESSELLATOR_REDUCTION_MIN:
            insideTessFactor[U] = tess_fmin(tessFactor_Veq0,tessFactor_Veq1);
            insideTessFactor[V] = tess_fmin(tessFactor_Ueq0,tessFactor_Ueq1);
            break;
        case PIPE_TESSELLATOR_REDUCTION_MAX:
            insideTessFactor[U] = tess_fmax(tessFactor_Veq0,tessFactor_Veq1);
            insideTessFactor[V] = tess_fmax(tessFactor_Ueq0,tessFactor_Ueq1);
            break;
        case PIPE_TESSELLATOR_REDUCTION_AVERAGE:
            insideTessFactor[U] = (tessFactor_Veq0 + tessFactor_Veq1) / 2;
            insideTessFactor[V] = (tessFactor_Ueq0 + tessFactor_Ueq1) / 2;
            break;
        default:
            unreachable("impossible m_insideTessFactorReduction");
        }
        // Scale inside tessFactors based on user scale factor.

        ClampFloatTessFactorScale(insideTessFactorScaleU); // clamp scale value to [0..1], NaN->0
        ClampFloatTessFactorScale(insideTessFactorScaleV);
        insideTessFactor[U] = insideTessFactor[U]*insideTessFactorScaleU;
        insideTessFactor[V] = insideTessFactor[V]*insideTessFactorScaleV;

        // Compute inside parity
        if( Pow2Partitioning() || IntegerPartitioning() )
        {
            for( axis = 0; axis < QUAD_AXES; axis++ )
            {
                ClampTessFactor(insideTessFactor[axis]); // clamp reduction + scale result that is based on unbounded user input
                m_LastUnRoundedComputedTessFactors[4+axis] = insideTessFactor[axis]; // Save off TessFactors so they can be returned to app
                RoundUpTessFactor(insideTessFactor[axis]);
                insideTessFactorParity[axis] =
                    (isEven(insideTessFactor[axis]) || (FLOAT_ONE == insideTessFactor[axis]) )
                    ? TESSELLATOR_PARITY_EVEN : TESSELLATOR_PARITY_ODD;
            }
        }
        else
        {
            ClampTessFactor(insideTessFactor[U]); // clamp reduction + scale result that is based on unbounded user input
            ClampTessFactor(insideTessFactor[V]); // clamp reduction + scale result that is based on unbounded user input
            m_LastUnRoundedComputedTessFactors[4] = insideTessFactor[U]; // Save off TessFactors so they can be returned to app
            m_LastUnRoundedComputedTessFactors[5] = insideTessFactor[V]; // Save off TessFactors so they can be returned to app
             // no parity changes for fractional tessellation - just use what the user requested
            insideTessFactorParity[U] = insideTessFactorParity[V] = m_originalParity;
        }

        // To prevent snapping on edges, the "picture frame" comes
        // in using avg or max (and ignore inside TessFactor scaling) until it is at least 3.
        if( (TESSELLATOR_PARITY_ODD == insideTessFactorParity[U]) &&
            (insideTessFactor[U] < FLOAT_THREE) )
        {
            if(PIPE_TESSELLATOR_REDUCTION_MAX == m_insideTessFactorReduction)
            {
                insideTessFactor[U] = tess_fmin(FLOAT_THREE,tess_fmax(tessFactor_Veq0,tessFactor_Veq1));
            }
            else
            {
                insideTessFactor[U] = tess_fmin(FLOAT_THREE,(tessFactor_Veq0 + tessFactor_Veq1) / 2);
            }
            ClampTessFactor(insideTessFactor[U]); // clamp reduction result that is based on unbounded user input
            m_LastUnRoundedComputedTessFactors[4] = insideTessFactor[U]; // Save off TessFactors so they can be returned to app
            if( IntegerPartitioning())
            {
                RoundUpTessFactor(insideTessFactor[U]);
                insideTessFactorParity[U] = isEven(insideTessFactor[U]) ? TESSELLATOR_PARITY_EVEN : TESSELLATOR_PARITY_ODD;
            }
        }

        if( (TESSELLATOR_PARITY_ODD == insideTessFactorParity[V]) &&
            (insideTessFactor[V] < FLOAT_THREE) )
        {
            if(PIPE_TESSELLATOR_REDUCTION_MAX == m_insideTessFactorReduction)
            {
                insideTessFactor[V] = tess_fmin(FLOAT_THREE,tess_fmax(tessFactor_Ueq0,tessFactor_Ueq1));
            }
            else
            {
                insideTessFactor[V] = tess_fmin(FLOAT_THREE,(tessFactor_Ueq0 + tessFactor_Ueq1) / 2);
            }
            ClampTessFactor(insideTessFactor[V]);// clamp reduction result that is based on unbounded user input
            m_LastUnRoundedComputedTessFactors[5] = insideTessFactor[V]; // Save off TessFactors so they can be returned to app
            if( IntegerPartitioning())
            {
                RoundUpTessFactor(insideTessFactor[V]);
                insideTessFactorParity[V] = isEven(insideTessFactor[V]) ? TESSELLATOR_PARITY_EVEN : TESSELLATOR_PARITY_ODD;
            }
        }

        for( axis = 0; axis < QUAD_AXES; axis++ )
        {
            if( TESSELLATOR_PARITY_ODD == insideTessFactorParity[axis] )
            {
                // Ensure the first ring ("picture frame") interpolates in on all sides
                // as much as the side with the minimum TessFactor.  Prevents snapping to edge.
                if( (insideTessFactor[axis] < FLOAT_THREE) && (insideTessFactor[axis] < insideTessFactor[(axis+1)&0x1]))
                {
                    insideTessFactor[axis] = tess_fmin(insideTessFactor[(axis+1)&0x1],FLOAT_THREE);
                    m_LastUnRoundedComputedTessFactors[4+axis] = insideTessFactor[axis]; // Save off TessFactors so they can be returned to app
                }
            }
        }
    }

    // Save off TessFactors so they can be returned to app
    m_LastComputedTessFactors[0] = outsideTessFactor[Ueq0];
    m_LastComputedTessFactors[1] = outsideTessFactor[Veq0];
    m_LastComputedTessFactors[2] = outsideTessFactor[Ueq1];
    m_LastComputedTessFactors[3] = outsideTessFactor[Veq1];
    m_LastComputedTessFactors[4] = insideTessFactor[U];
    m_LastComputedTessFactors[5] = insideTessFactor[V];
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHLSLTessellator::TessellateTriDomain
// User calls this
//---------------------------------------------------------------------------------------------------------------------------------
void CHLSLTessellator::TessellateTriDomain( float tessFactor_Ueq0, float tessFactor_Veq0, float tessFactor_Weq0,
                                        float insideTessFactorScale )
{
    TriHLSLProcessTessFactors(tessFactor_Ueq0,tessFactor_Veq0,tessFactor_Weq0,insideTessFactorScale);

    CHWTessellator::TessellateTriDomain(m_LastComputedTessFactors[0],m_LastComputedTessFactors[1],m_LastComputedTessFactors[2],m_LastComputedTessFactors[3]);
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHLSLTessellator::TriHLSLProcessTessFactors
//---------------------------------------------------------------------------------------------------------------------------------
void CHLSLTessellator::TriHLSLProcessTessFactors( float tessFactor_Ueq0, float tessFactor_Veq0, float tessFactor_Weq0,
                                  float insideTessFactorScale )
{
    if( !(tessFactor_Ueq0 > 0) || // NaN will pass
        !(tessFactor_Veq0 > 0) ||
        !(tessFactor_Weq0 > 0) )
    {
        m_LastUnRoundedComputedTessFactors[0] = tessFactor_Ueq0;
        m_LastUnRoundedComputedTessFactors[1] = tessFactor_Veq0;
        m_LastUnRoundedComputedTessFactors[2] = tessFactor_Weq0;
        m_LastUnRoundedComputedTessFactors[3] =
        m_LastComputedTessFactors[0] =
        m_LastComputedTessFactors[1] =
        m_LastComputedTessFactors[2] =
        m_LastComputedTessFactors[3] = 0;
        return;
    }

    CleanupFloatTessFactor(tessFactor_Ueq0); // clamp to [1.0f..INF], NaN->1.0f
    CleanupFloatTessFactor(tessFactor_Veq0);
    CleanupFloatTessFactor(tessFactor_Weq0);

    // Save off TessFactors so they can be returned to app
    m_LastUnRoundedComputedTessFactors[0] = tessFactor_Ueq0;
    m_LastUnRoundedComputedTessFactors[1] = tessFactor_Veq0;
    m_LastUnRoundedComputedTessFactors[2] = tessFactor_Weq0;

    // Process outside TessFactors
    float outsideTessFactor[TRI_EDGES] = {tessFactor_Ueq0, tessFactor_Veq0, tessFactor_Weq0};
    int edge;
    if( Pow2Partitioning() || IntegerPartitioning() )
    {
        for( edge = 0; edge < TRI_EDGES; edge++ )
        {
            RoundUpTessFactor(outsideTessFactor[edge]); // for pow2 this rounds to pow2
            ClampTessFactor(outsideTessFactor[edge]); // clamp unbounded user input based on tessellation mode
        }
    }
    else
    {
        for( edge = 0; edge < TRI_EDGES; edge++ )
        {
            ClampTessFactor(outsideTessFactor[edge]); // clamp unbounded user input based on tessellation mode
        }
    }

    // Compute inside TessFactor
    float insideTessFactor;
    switch( m_insideTessFactorReduction )
    {
    case PIPE_TESSELLATOR_REDUCTION_MIN:
        insideTessFactor = tess_fmin(tess_fmin(tessFactor_Ueq0,tessFactor_Veq0),tessFactor_Weq0);
        break;
    case PIPE_TESSELLATOR_REDUCTION_MAX:
        insideTessFactor = tess_fmax(tess_fmax(tessFactor_Ueq0,tessFactor_Veq0),tessFactor_Weq0);
        break;
    case PIPE_TESSELLATOR_REDUCTION_AVERAGE:
        insideTessFactor = (tessFactor_Ueq0 + tessFactor_Veq0 + tessFactor_Weq0) / 3;
        break;
    default:
        unreachable("impossible m_insideTessFactorReduction");
    }

    // Scale inside TessFactor based on user scale factor.
    ClampFloatTessFactorScale(insideTessFactorScale); // clamp scale value to [0..1], NaN->0
    insideTessFactor = insideTessFactor*tess_fmin(FLOAT_ONE,insideTessFactorScale);

    ClampTessFactor(insideTessFactor); // clamp reduction + scale result that is based on unbounded user input
    m_LastUnRoundedComputedTessFactors[3] = insideTessFactor;// Save off TessFactors so they can be returned to app
    TESSELLATOR_PARITY parity;
    if( Pow2Partitioning() || IntegerPartitioning() )
    {
        RoundUpTessFactor(insideTessFactor);
        parity = (isEven(insideTessFactor) || (FLOAT_ONE == insideTessFactor))
                                        ? TESSELLATOR_PARITY_EVEN : TESSELLATOR_PARITY_ODD;
    }
    else
    {
        parity = m_originalParity;
    }

    if( (TESSELLATOR_PARITY_ODD == parity) &&
        (insideTessFactor < FLOAT_THREE))
    {
        // To prevent snapping on edges, the "picture frame" comes
        // in using avg or max (and ignore inside TessFactor scaling) until it is at least 3.
        if(PIPE_TESSELLATOR_REDUCTION_MAX == m_insideTessFactorReduction)
        {
            insideTessFactor = tess_fmin(FLOAT_THREE,tess_fmax(tessFactor_Ueq0,tess_fmax(tessFactor_Veq0,tessFactor_Weq0)));
        }
        else
        {
            insideTessFactor = tess_fmin(FLOAT_THREE,(tessFactor_Ueq0 + tessFactor_Veq0 + tessFactor_Weq0) / 3);
        }
        ClampTessFactor(insideTessFactor); // clamp reduction result that is based on unbounded user input
        m_LastUnRoundedComputedTessFactors[3] = insideTessFactor;// Save off TessFactors so they can be returned to app
        if( IntegerPartitioning())
        {
            RoundUpTessFactor(insideTessFactor);
        }
    }

    // Save off TessFactors so they can be returned to app
    m_LastComputedTessFactors[0] = outsideTessFactor[Ueq0];
    m_LastComputedTessFactors[1] = outsideTessFactor[Veq0];
    m_LastComputedTessFactors[2] = outsideTessFactor[Weq0];
    m_LastComputedTessFactors[3] = insideTessFactor;
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHLSLTessellator::TessellateIsoLineDomain
// User calls this.
//---------------------------------------------------------------------------------------------------------------------------------
void CHLSLTessellator::TessellateIsoLineDomain( float TessFactor_U_LineDetail, float TessFactor_V_LineDensity )
{
    IsoLineHLSLProcessTessFactors(TessFactor_V_LineDensity,TessFactor_U_LineDetail);
    CHWTessellator::TessellateIsoLineDomain(m_LastComputedTessFactors[0],m_LastComputedTessFactors[1]);
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHLSLTessellator::IsoLineHLSLProcessTessFactors
//---------------------------------------------------------------------------------------------------------------------------------
void CHLSLTessellator::IsoLineHLSLProcessTessFactors( float TessFactor_V_LineDensity, float TessFactor_U_LineDetail )
{
    if( !(TessFactor_V_LineDensity > 0) || // NaN will pass
        !(TessFactor_U_LineDetail > 0) )
    {
        m_LastUnRoundedComputedTessFactors[0] = TessFactor_V_LineDensity;
        m_LastUnRoundedComputedTessFactors[1] = TessFactor_U_LineDetail;
        m_LastComputedTessFactors[0] =
        m_LastComputedTessFactors[1] = 0;
        return;
    }

    CleanupFloatTessFactor(TessFactor_V_LineDensity); // clamp to [1.0f..INF], NaN->1.0f
    CleanupFloatTessFactor(TessFactor_U_LineDetail); // clamp to [1.0f..INF], NaN->1.0f

    ClampTessFactor(TessFactor_U_LineDetail); // clamp unbounded user input based on tessellation mode

    m_LastUnRoundedComputedTessFactors[1] = TessFactor_U_LineDetail;    // Save off TessFactors so they can be returned to app

    if(Pow2Partitioning()||IntegerPartitioning())
    {
        RoundUpTessFactor(TessFactor_U_LineDetail);
    }

    OverridePartitioning(PIPE_TESSELLATOR_PARTITIONING_INTEGER);

    ClampTessFactor(TessFactor_V_LineDensity); // Clamp unbounded user input to integer
    m_LastUnRoundedComputedTessFactors[0] = TessFactor_V_LineDensity;    // Save off TessFactors so they can be returned to app

    RoundUpTessFactor(TessFactor_V_LineDensity);

    RestorePartitioning();

    // Save off TessFactors so they can be returned to app
    m_LastComputedTessFactors[0] = TessFactor_V_LineDensity;
    m_LastComputedTessFactors[1] = TessFactor_U_LineDetail;
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHLSLTessellator::ClampTessFactor()
//---------------------------------------------------------------------------------------------------------------------------------
void CHLSLTessellator::ClampTessFactor(float& TessFactor)
{
    if( Pow2Partitioning() )
    {
        TessFactor = tess_fmin( PIPE_TESSELLATOR_MAX_EVEN_TESSELLATION_FACTOR, tess_fmax( TessFactor, PIPE_TESSELLATOR_MIN_ODD_TESSELLATION_FACTOR) );
    }
    else if( IntegerPartitioning() )
    {
        TessFactor = tess_fmin( PIPE_TESSELLATOR_MAX_TESSELLATION_FACTOR, tess_fmax( TessFactor, PIPE_TESSELLATOR_MIN_ODD_TESSELLATION_FACTOR) );
    }
    else if( Odd() )
    {
        TessFactor = tess_fmin( PIPE_TESSELLATOR_MAX_ODD_TESSELLATION_FACTOR, tess_fmax( TessFactor, PIPE_TESSELLATOR_MIN_ODD_TESSELLATION_FACTOR) );
    }
    else // even
    {
        TessFactor = tess_fmin( PIPE_TESSELLATOR_MAX_EVEN_TESSELLATION_FACTOR, tess_fmax( TessFactor, PIPE_TESSELLATOR_MIN_EVEN_TESSELLATION_FACTOR) );
    }
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHLSLTessellator::CleanupFloatTessFactor()
//---------------------------------------------------------------------------------------------------------------------------------
static const int exponentMask = 0x7f800000;
static const int mantissaMask = 0x007fffff;
void CHLSLTessellator::CleanupFloatTessFactor(float& input)
{
    // If input is < 1.0f or NaN, clamp to 1.0f.
    // In other words, clamp input to [1.0f...+INF]
    int bits = *(int*)&input;
    if( ( ( ( bits & exponentMask ) == exponentMask ) && ( bits & mantissaMask ) ) ||// nan?
        (input < 1.0f) )
    {
        input = 1;
    }
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHLSLTessellator::ClampFloatTessFactorScale()
//---------------------------------------------------------------------------------------------------------------------------------
void CHLSLTessellator::ClampFloatTessFactorScale(float& input)
{
    // If input is < 0.0f or NaN, clamp to 0.0f.  > 1 clamps to 1.
    // In other words, clamp input to [0.0f...1.0f]
    int bits = *(int*)&input;
    if( ( ( ( bits & exponentMask ) == exponentMask ) && ( bits & mantissaMask ) ) ||// nan?
        (input < 0.0f) )
    {
        input = 0;
    }
    else if( input > 1 )
    {
        input = 1;
    }
}

//---------------------------------------------------------------------------------------------------------------------------------
// CHLSLTessellator::RoundUpTessFactor()
//---------------------------------------------------------------------------------------------------------------------------------
static const int exponentLSB = 0x00800000;
void CHLSLTessellator::RoundUpTessFactor(float& TessFactor)
{
    // Assume TessFactor is in [1.0f..+INF]
    if( Pow2Partitioning() )
    {
        int bits = *(int*)&TessFactor;
        if( bits & mantissaMask )
        {
            *(int*)&TessFactor = (bits & exponentMask) + exponentLSB;
        }
    }
    else if( IntegerPartitioning() )
    {
        TessFactor = ceil(TessFactor);
    }
}
