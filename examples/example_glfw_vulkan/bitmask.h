#pragma once

template <class EnumType>
struct BitMask
{
    size_t m_value;

    BitMask()
    {
        m_value = 0u;
    }

    void setBit( const EnumType flag, bool targetValue )
    {
        if( targetValue )
        {
            setBit( flag );
        }
        else
        {
            clearBit( flag );
        }
    }

    void setBit( const EnumType flag )
    {
        m_value |= 1ull << (int)flag;
    }

    void clearBit( const EnumType flag )
    {
        m_value &= ~( 1ull << (int)flag );
    }

    bool isBitSet( const EnumType flag ) const
    {
        return ( m_value >> (int)flag ) & 1u;
    }

    void clear()
    {
        m_value = 0u;
    }
};
