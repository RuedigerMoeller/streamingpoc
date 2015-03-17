/*********************************************************************************
 * Copyright (C) 2012 Sybase, Inc.                                               *
 * All rights reserved. Unpublished rights reserved under U.S. copyright laws.   *
 * This product is the confidential and trade secret information of Sybase, Inc. *
 *********************************************************************************/

#ifndef __DataValue_hpp__
#define __DataValue_hpp__

#ifdef WIN32
#include "win_stdint.hpp"
#else
#include <stdint.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <algorithm>

#define GET_SET_STRINGLEN_PLUS_ONE(DV) (reinterpret_cast<DataTypes::LSTRING*>(&(DV)->val)->len = strlen((DV)->val.stringv) + 1)
#define COPY_STRING_WITH_LEN( DEST, SRC ) memcpy( (DEST), (SRC)->val.stringv, reinterpret_cast<DataTypes::LSTRING*>(&(SRC)->val)->len )
#define GET_STRING_LEN(DV) (reinterpret_cast<DataTypes::LSTRING*>(&(DV)->val)->len) 

namespace DataTypes 
{
    typedef int64_t money_t;         // representation of money type 
    typedef int64_t timestampval_t;  // representation of timestamp type 
    typedef int64_t hirestime_t;     // the new high-resolution time type, in microseconds since epoch
    typedef int64_t interval_t;      // representation of interval type 
	typedef int64_t time_of_day_t;  // representation of time type
    typedef uint8_t byte_t;
    typedef uint32_t byte_size_t;

    struct LSTRING {
        const char * stringv;
        size_t len;
    };
    
    class FixedDecimalValue
    {
    public:
        inline void setDigits(int intDigits, int fractDigits) {
            itsDigits = (intDigits << 8) | fractDigits;
        }
        inline int getIntDigits() const { return itsDigits >> 8; }
        inline int getFractDigits() const { return itsDigits & 0xff; }

        bool setDecimalValue(int precision, int scale, const char * decimal_string);
        bool setDecimalValue(int precision, int scale, int decimal_int);
        bool setDecimalValue(int precision, int scale, int64_t decimal_int64);
        bool setDecimalValue(int precision, int scale, double decimal_double);

        inline void swap(FixedDecimalValue & other)
        {
            std::swap(itsHi, other.itsHi);
            std::swap(itsLo, other.itsLo);
            std::swap(itsDigits, other.itsDigits);
        }

        // Change the precision and scale of the FixedDecimalValue to be the type
        // of the sum of this FixedDecimalValue and the specified one.
        bool toAdditionResultType( FixedDecimalValue& first, FixedDecimalValue& other );

        // Change the precision and scale of the FixedDecimalValue to be the type
        // of the difference of this FixedDecimalValue and the specified one.
        bool toSubtractionResultType( FixedDecimalValue& first, FixedDecimalValue& other );

        // Change the precision and scale of the FixedDecimalValue to be the type
        // of the product of this FixedDecimalValue and the specified one.
        bool toMultiplicationResultType( FixedDecimalValue& first, FixedDecimalValue& other );

        // Change the precision and scale of the FixedDecimalValue to be the type
        // of the quotient of this FixedDecimalValue and the specified one.
        bool toDivisionResultType( FixedDecimalValue& first, FixedDecimalValue& other );

        bool toSameScale( FixedDecimalValue& first, FixedDecimalValue & other );

        bool isNegative() const ;
        inline bool isZero() const { return itsHi == 0 && itsLo == 0; }

        bool checkPrecisionOverflow();

        static int32_t toMaxScale(FixedDecimalValue& first, FixedDecimalValue& second);

        inline bool operator==(const FixedDecimalValue & other) const { return itsHi == other.itsHi && itsLo == other.itsLo; }
        inline bool operator!=(const FixedDecimalValue & other) const { return itsHi != other.itsHi || itsLo != other.itsLo; }
        inline bool operator<(const FixedDecimalValue & other) const
        {
            if ((long long)itsHi < (long long)other.itsHi)
                return true;
            if (itsHi != other.itsHi)
                return false;
            return itsLo < other.itsLo;
        }
        inline bool operator>(const FixedDecimalValue &other) const { return other.operator<(*this); }
        inline bool operator<=(const FixedDecimalValue &other) const { return !other.operator<(*this); }
        inline bool operator>=(const FixedDecimalValue &other) const { return !operator<(other); }

        unsigned long long itsHi;
        unsigned long long itsLo;
        uint16_t itsDigits;
    }; // FixedDecimalValue
    
    typedef FixedDecimalValue fixeddecimal_t;

    class BinaryValue
    {
    public:
        inline bool empty() const
        {
            return (_data == NULL) || (_used == 0);
        }

        inline void swap(BinaryValue & other)
        {
            std::swap(_data, other._data);
            std::swap(_used, other._used);
        }

        /**
         * Comparison function
         */
        int compare(const BinaryValue& i_rhs) const;

        const byte_t* 	_data;      // points to the first byte of the data in the buffer
        byte_size_t     _used;      // length of data used in the buffer
    }; // BinaryValue

    typedef BinaryValue binary_t;

    /// A union type for data values
    struct DataValue {
        union {
            bool booleanv;
            int16_t int16v;
            int32_t int32v;
            int64_t int64v;
            interval_t intervalv;
            money_t moneyv;
            double  doublev;
            time_of_day_t timev;
            time_t  datev;
            timestampval_t timestampv;
            const char *  stringv;
            hirestime_t bigdatetimev;
            binary_t binaryv;
            fixeddecimal_t fixeddecimalv;
            void * objectv;
        } val;
        bool null;

        DataValue()
        {
            memset( &val, 0, sizeof(val));
            null = true;
        }

        DataValue &setBoolean(bool v) {
            val.booleanv = v;
            null = false;
            return *this;
        }
        DataValue &setInt32(int32_t v) {
            val.int32v = v;
            null = false;
            return *this;
        }
        DataValue &setInt64(int64_t v) {
            val.int64v = v;
            null = false;
            return *this;
        }
        DataValue &setInterval(interval_t v) {
            val.intervalv = v;
            null = false;
            return *this;
        }
        DataValue &setMoney(money_t v) {
            val.moneyv = v;
            null = false;
            return *this;
        }
        DataValue &setDouble(double v) {
            val.doublev = v;
            null = false;
            return *this;
        }
        DataValue &setTime(time_of_day_t v) {
            val.timev = v;
            null = false;
            return *this;
        }
        DataValue &setDate(time_t v) {
            val.datev = v;
            null = false;
            return *this;
        }
        DataValue &setTimestamp(timestampval_t v) {
            val.timestampv = v;
            null = false;
            return *this;
        }
        DataValue &setString(const char *v) {
            val.stringv = v;
            null = false;
            return *this;
        }
        DataValue &setObject(void *v) {
            val.objectv = v;
            null = false;
            return *this;
        }
        DataValue &setBigdatetime(hirestime_t v) {
            val.bigdatetimev = v;
            null = false;
            return *this;
        }
        DataValue &setBinary(binary_t v) {
            val.binaryv = v;
            null = false;
            return *this;
        }
        DataValue &setFixedDecimal(fixeddecimal_t v) {
            val.fixeddecimalv = v;
            null = false;
            return *this;
        }
    };
}
#endif

