
/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

// Coordinate class implementation
#include "addrcommon.h"
#include "coord.h"

namespace Addr
{
namespace V2
{

Coordinate::Coordinate()
{
    dim = DIM_X;
    ord = 0;
}

Coordinate::Coordinate(enum Dim dim, INT_32 n)
{
    set(dim, n);
}

VOID Coordinate::set(enum Dim d, INT_32 n)
{
    dim = d;
    ord = static_cast<INT_8>(n);
}

UINT_32 Coordinate::ison(const UINT_32 *coords) const
{
    UINT_32 bit = static_cast<UINT_32>(1ull << static_cast<UINT_32>(ord));

    return (coords[dim] & bit) ? 1 : 0;
}

enum Dim Coordinate::getdim()
{
    return dim;
}

INT_8 Coordinate::getord()
{
    return ord;
}

BOOL_32 Coordinate::operator==(const Coordinate& b)
{
    return (dim == b.dim) && (ord == b.ord);
}

BOOL_32 Coordinate::operator<(const Coordinate& b)
{
    BOOL_32 ret;

    if (dim == b.dim)
    {
        ret = ord < b.ord;
    }
    else
    {
        if (dim == DIM_S || b.dim == DIM_M)
        {
            ret = TRUE;
        }
        else if (b.dim == DIM_S || dim == DIM_M)
        {
            ret = FALSE;
        }
        else if (ord == b.ord)
        {
            ret = dim < b.dim;
        }
        else
        {
            ret = ord < b.ord;
        }
    }

    return ret;
}

BOOL_32 Coordinate::operator>(const Coordinate& b)
{
    BOOL_32 lt = *this < b;
    BOOL_32 eq = *this == b;
    return !lt && !eq;
}

BOOL_32 Coordinate::operator<=(const Coordinate& b)
{
    return (*this < b) || (*this == b);
}

BOOL_32 Coordinate::operator>=(const Coordinate& b)
{
    return !(*this < b);
}

BOOL_32 Coordinate::operator!=(const Coordinate& b)
{
    return !(*this == b);
}

Coordinate& Coordinate::operator++(INT_32)
{
    ord++;
    return *this;
}

// CoordTerm

CoordTerm::CoordTerm()
{
    num_coords = 0;
}

VOID CoordTerm::Clear()
{
    num_coords = 0;
}

VOID CoordTerm::add(Coordinate& co)
{
    // This function adds a coordinate INT_32o the list
    // It will prevent the same coordinate from appearing,
    // and will keep the list ordered from smallest to largest
    UINT_32 i;

    for (i = 0; i < num_coords; i++)
    {
        if (m_coord[i] == co)
        {
            break;
        }
        if (m_coord[i] > co)
        {
            for (UINT_32 j = num_coords; j > i; j--)
            {
                m_coord[j] = m_coord[j - 1];
            }
            m_coord[i] = co;
            num_coords++;
            break;
        }
    }

    if (i == num_coords)
    {
        m_coord[num_coords] = co;
        num_coords++;
    }
}

VOID CoordTerm::add(CoordTerm& cl)
{
    for (UINT_32 i = 0; i < cl.num_coords; i++)
    {
        add(cl.m_coord[i]);
    }
}

BOOL_32 CoordTerm::remove(Coordinate& co)
{
    BOOL_32 remove = FALSE;
    for (UINT_32 i = 0; i < num_coords; i++)
    {
        if (m_coord[i] == co)
        {
            remove = TRUE;
            num_coords--;
        }

        if (remove)
        {
            m_coord[i] = m_coord[i + 1];
        }
    }
    return remove;
}

BOOL_32 CoordTerm::Exists(Coordinate& co)
{
    BOOL_32 exists = FALSE;
    for (UINT_32 i = 0; i < num_coords; i++)
    {
        if (m_coord[i] == co)
        {
            exists = TRUE;
            break;
        }
    }
    return exists;
}

VOID CoordTerm::copyto(CoordTerm& cl)
{
    cl.num_coords = num_coords;
    for (UINT_32 i = 0; i < num_coords; i++)
    {
        cl.m_coord[i] = m_coord[i];
    }
}

UINT_32 CoordTerm::getsize()
{
    return num_coords;
}

UINT_32 CoordTerm::getxor(const UINT_32 *coords) const
{
    UINT_32 out = 0;
    for (UINT_32 i = 0; i < num_coords; i++)
    {
        out = out ^ m_coord[i].ison(coords);
    }
    return out;
}

VOID CoordTerm::getsmallest(Coordinate& co)
{
    co = m_coord[0];
}

UINT_32 CoordTerm::Filter(INT_8 f, Coordinate& co, UINT_32 start, enum Dim axis)
{
    for (UINT_32 i = start;  i < num_coords;)
    {
        if (((f == '<' && m_coord[i] < co) ||
             (f == '>' && m_coord[i] > co) ||
             (f == '=' && m_coord[i] == co)) &&
            (axis == NUM_DIMS || axis == m_coord[i].getdim()))
        {
            for (UINT_32 j = i; j < num_coords - 1; j++)
            {
                m_coord[j] = m_coord[j + 1];
            }
            num_coords--;
        }
        else
        {
            i++;
        }
    }
    return num_coords;
}

Coordinate& CoordTerm::operator[](UINT_32 i)
{
    return m_coord[i];
}

BOOL_32 CoordTerm::operator==(const CoordTerm& b)
{
    BOOL_32 ret = TRUE;

    if (num_coords != b.num_coords)
    {
        ret = FALSE;
    }
    else
    {
        for (UINT_32 i = 0; i < num_coords; i++)
        {
            // Note: the lists will always be in order, so we can compare the two lists at time
            if (m_coord[i] != b.m_coord[i])
            {
                ret = FALSE;
                break;
            }
        }
    }
    return ret;
}

BOOL_32 CoordTerm::operator!=(const CoordTerm& b)
{
    return !(*this == b);
}

BOOL_32 CoordTerm::exceedRange(const UINT_32 *ranges)
{
    BOOL_32 exceed = FALSE;
    for (UINT_32 i = 0; (i < num_coords) && (exceed == FALSE); i++)
    {
        exceed = ((1u << m_coord[i].getord()) <= ranges[m_coord[i].getdim()]);
    }

    return exceed;
}

// coordeq
CoordEq::CoordEq()
{
    m_numBits = 0;
}

VOID CoordEq::remove(Coordinate& co)
{
    for (UINT_32 i = 0; i < m_numBits; i++)
    {
        m_eq[i].remove(co);
    }
}

BOOL_32 CoordEq::Exists(Coordinate& co)
{
    BOOL_32 exists = FALSE;

    for (UINT_32 i = 0; i < m_numBits; i++)
    {
        if (m_eq[i].Exists(co))
        {
            exists = TRUE;
        }
    }
    return exists;
}

VOID CoordEq::resize(UINT_32 n)
{
    if (n > m_numBits)
    {
        for (UINT_32 i = m_numBits; i < n; i++)
        {
            m_eq[i].Clear();
        }
    }
    m_numBits = n;
}

UINT_32 CoordEq::getsize()
{
    return m_numBits;
}

UINT_64 CoordEq::solve(const UINT_32 *coords) const
{
    UINT_64 out = 0;
    for (UINT_32 i = 0; i < m_numBits; i++)
    {
        out |= static_cast<UINT_64>(m_eq[i].getxor(coords)) << i;
    }
    return out;
}

VOID CoordEq::solveAddr(
    UINT_64 addr, UINT_32 sliceInM,
    UINT_32 *coords) const
{
    UINT_32 BitsValid[NUM_DIMS] = {0};

    CoordEq temp = *this;

    memset(coords, 0, NUM_DIMS * sizeof(coords[0]));

    UINT_32 bitsLeft = 0;

    for (UINT_32 i = 0; i < temp.m_numBits; i++)
    {
        UINT_32 termSize = temp.m_eq[i].getsize();

        if (termSize == 1)
        {
            INT_8 bit = (addr >> i) & 1;
            enum Dim dim = temp.m_eq[i][0].getdim();
            INT_8 ord = temp.m_eq[i][0].getord();

            ADDR_ASSERT((ord < 32) || (bit == 0));

            BitsValid[dim] |= 1u << ord;
            coords[dim] |= bit << ord;

            temp.m_eq[i].Clear();
        }
        else if (termSize > 1)
        {
            bitsLeft++;
        }
    }

    if (bitsLeft > 0)
    {
        if (sliceInM != 0)
        {
            coords[DIM_Z] = coords[DIM_M] / sliceInM;
            BitsValid[DIM_Z] = 0xffffffff;
        }

        do
        {
            bitsLeft = 0;

            for (UINT_32 i = 0; i < temp.m_numBits; i++)
            {
                UINT_32 termSize = temp.m_eq[i].getsize();

                if (termSize == 1)
                {
                    INT_8 bit = (addr >> i) & 1;
                    enum Dim dim = temp.m_eq[i][0].getdim();
                    INT_8 ord = temp.m_eq[i][0].getord();

                    ADDR_ASSERT((ord < 32) || (bit == 0));
                    ADDR_ASSERT(dim < DIM_S);

                    BitsValid[dim] |= 1u << ord;
                    coords[dim] |= bit << ord;

                    temp.m_eq[i].Clear();
                }
                else if (termSize > 1)
                {
                    CoordTerm tmpTerm = temp.m_eq[i];

                    for (UINT_32 j = 0; j < termSize; j++)
                    {
                        enum Dim dim = temp.m_eq[i][j].getdim();
                        INT_8 ord = temp.m_eq[i][j].getord();

                        ADDR_ASSERT(dim < DIM_S);

                        if (BitsValid[dim] & (1u << ord))
                        {
                            UINT_32 v = (((coords[dim] >> ord) & 1) << i);
                            addr ^= static_cast<UINT_64>(v);
                            tmpTerm.remove(temp.m_eq[i][j]);
                        }
                    }

                    temp.m_eq[i] = tmpTerm;

                    bitsLeft++;
                }
            }
        } while (bitsLeft > 0);
    }
}

VOID CoordEq::copy(CoordEq& o, UINT_32 start, UINT_32 num)
{
    o.m_numBits = (num == 0xFFFFFFFF) ? m_numBits : num;
    for (UINT_32 i = 0; i < o.m_numBits; i++)
    {
        m_eq[start + i].copyto(o.m_eq[i]);
    }
}

VOID CoordEq::reverse(UINT_32 start, UINT_32 num)
{
    UINT_32 n = (num == 0xFFFFFFFF) ? m_numBits : num;

    for (UINT_32 i = 0; i < n / 2; i++)
    {
        CoordTerm temp;
        m_eq[start + i].copyto(temp);
        m_eq[start + n - 1 - i].copyto(m_eq[start + i]);
        temp.copyto(m_eq[start + n - 1 - i]);
    }
}

VOID CoordEq::xorin(CoordEq& x, UINT_32 start)
{
    UINT_32 n = ((m_numBits - start) < x.m_numBits) ? (m_numBits - start) : x.m_numBits;
    for (UINT_32 i = 0; i < n; i++)
    {
        m_eq[start + i].add(x.m_eq[i]);
    }
}

UINT_32 CoordEq::Filter(INT_8 f, Coordinate& co, UINT_32 start, enum Dim axis)
{
    for (UINT_32 i = start; i < m_numBits;)
    {
        UINT_32 m = m_eq[i].Filter(f, co, 0, axis);
        if (m == 0)
        {
            for (UINT_32 j = i; j < m_numBits - 1; j++)
            {
                m_eq[j] = m_eq[j + 1];
            }
            m_numBits--;
        }
        else
        {
            i++;
        }
    }
    return m_numBits;
}

VOID CoordEq::shift(INT_32 amount, INT_32 start)
{
    if (amount != 0)
    {
        INT_32 numBits = static_cast<INT_32>(m_numBits);
        amount = -amount;
        INT_32 inc = (amount < 0) ? -1 : 1;
        INT_32 i = (amount < 0) ? numBits - 1 : start;
        INT_32 end = (amount < 0) ? start - 1 : numBits;
        for (; (inc > 0) ? i < end : i > end; i += inc)
        {
            if ((i + amount < start) || (i + amount >= numBits))
            {
                m_eq[i].Clear();
            }
            else
            {
                m_eq[i + amount].copyto(m_eq[i]);
            }
        }
    }
}

CoordTerm& CoordEq::operator[](UINT_32 i)
{
    return m_eq[i];
}

VOID CoordEq::mort2d(Coordinate& c0, Coordinate& c1, UINT_32 start, UINT_32 end)
{
    if (end == 0)
    {
        ADDR_ASSERT(m_numBits > 0);
        end = m_numBits - 1;
    }
    for (UINT_32 i = start; i <= end; i++)
    {
        UINT_32 select = (i - start) % 2;
        Coordinate& c = (select == 0) ? c0 : c1;
        m_eq[i].add(c);
        c++;
    }
}

VOID CoordEq::mort3d(Coordinate& c0, Coordinate& c1, Coordinate& c2, UINT_32 start, UINT_32 end)
{
    if (end == 0)
    {
        ADDR_ASSERT(m_numBits > 0);
        end = m_numBits - 1;
    }
    for (UINT_32 i = start; i <= end; i++)
    {
        UINT_32 select = (i - start) % 3;
        Coordinate& c = (select == 0) ? c0 : ((select == 1) ? c1 : c2);
        m_eq[i].add(c);
        c++;
    }
}

BOOL_32 CoordEq::operator==(const CoordEq& b)
{
    BOOL_32 ret = TRUE;

    if (m_numBits != b.m_numBits)
    {
        ret = FALSE;
    }
    else
    {
        for (UINT_32 i = 0; i < m_numBits; i++)
        {
            if (m_eq[i] != b.m_eq[i])
            {
                ret = FALSE;
                break;
            }
        }
    }
    return ret;
}

BOOL_32 CoordEq::operator!=(const CoordEq& b)
{
    return !(*this == b);
}

} // V2
} // Addr
