//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/ict for license.
#include "bitstringunit.h"

#include "bitstring.h"
#include "netvar.h"

#include <stdio.h>
#include <cstdint>


void ict::bitstring_unit::toUint32()
{
    {
        ict::bitstring bs1(16, "FFFFFFFF");
        IT_ASSERT(bs1.bit_size() == 32);
        IT_ASSERT(ict::to_integer<unsigned int>(bs1) == 0xFFFFFFFF);
        unsigned int x;

        bs1 = ict::from_integer(0x7FFFFFFF);
        IT_ASSERT(bs1.bit_size() == 32);
        x = ict::to_integer<unsigned int>(bs1);
        IT_ASSERT_MSG("0x" << std::hex << std::uppercase << x,  x == 0x7FFFFFFF);
    }
    {
        ict::bitstring bs1(16, "0"); // makes an empty string 
        IT_ASSERT(bs1.bit_size() == 0);
        unsigned int x = ict::to_integer<unsigned int>(bs1);
        IT_ASSERT(x == 0);
    }
    {
        // bs.length() < sizeof(T) * 8
        ict::bitstring bs1(16, "0A");
        IT_ASSERT(bs1.bit_size() == 8);
        unsigned int x = ict::to_integer<unsigned int>(bs1);
        IT_ASSERT_MSG(std::hex << x, x == 0xA);
    }

#if 0 // this now throws exception
    {
        // bs.length() > sizeof(T) * 8
        ict::bitstring bs(16, "0A000000FF");
        IT_ASSERT(bs.length() == 40);
        unsigned int x = ict::to_integer<unsigned int>(bs);
        IT_ASSERT_MSG(hex << x, x == 0xFF); // 0A is ignored
    }
#endif

    // lets do some bool tests now
    {
        IT_ASSERT_MSG(sizeof(bool), 1 == sizeof(bool));
        ict::bitstring bs(2, "1");
        IT_ASSERT(true == ict::to_integer<bool>(bs));
        bs = ict::from_integer<bool>(true);
        IT_ASSERT(true == ict::to_integer<bool>(bs));

        bs = ict::from_integer<bool>(false);
        IT_ASSERT_MSG(bs.bit_size(), bs.bit_size() == 8);
        IT_ASSERT(false == ict::to_integer<bool>(bs));

        bs = ict::from_integer(false, 1);
        IT_ASSERT_MSG(bs.bit_size(), bs.bit_size() == 1);
        IT_ASSERT(false == ict::to_integer<bool>(bs));

        bs = ict::bitstring(2, "1");
        IT_ASSERT(true == ict::to_integer<bool>(bs));
        bs = ict::bitstring(2, "0");
        IT_ASSERT(false == ict::to_integer<bool>(bs));
        bs = ict::bitstring(2, "00000001");
        IT_ASSERT(true == ict::to_integer<bool>(bs));
        // TODO: hmm, this works on the 64 bit machine, not on mingw
        // who's right?
        //bs = ict::bitstring(2, "10010000");
        //IT_ASSERT(true == ict::to_integer<bool>(bs));
        bs = ict::bitstring(2, "00000000");
        IT_ASSERT(false == ict::to_integer<bool>(bs));

#if 0 // this now throws exception
        // now try it with a nine bit bitstring
        // should only use the bottom 8 bits 
        bs = ict::bitstring(2, "000000001");
        IT_ASSERT(true == ict::to_integer<bool>(bs));
        bs = ict::bitstring(2, "100000000"); // yep, first bit is ignored
        IT_ASSERT(false == ict::to_integer<bool>(bs));
#endif
        
    }
    {
        ict::bitstring bs1(16, "00000005");
        IT_ASSERT(5 == ict::to_integer<unsigned int>(bs1));
        bs1.resize(31);
        IT_ASSERT(2 == ict::to_integer<unsigned int>(bs1));
    }
    {
        ict::bitstring bs1(16, "01EBB4");
        IT_ASSERT(0x01EBB4 == ict::to_integer<unsigned int>(bs1));
    }
    {
        ict::bitstring bs1(2, "10001");
        IT_ASSERT(0x11 == ict::to_integer<unsigned int>(bs1));
    }


    {
        ict::bitstring bs1(2, "10110011110001");
        IT_ASSERT_MSG(bs1.bit_size(), bs1.bit_size() == 14);
        IT_ASSERT_MSG(sizeof(unsigned int), sizeof(unsigned int) == 4);
        IT_ASSERT_MSG(sizeof(uint32_t), sizeof(uint32_t) == 4);
        auto n = ict::to_integer<unsigned int>(bs1);
        IT_ASSERT(n == 11505);
        IT_ASSERT(11505 == ict::to_integer<unsigned int>(bs1));
    }

    // endian stuff
    {
        ict::bitstring bs("0001");
        IT_ASSERT_MSG(bs.bit_size(), bs.bit_size() == 16);
        IT_ASSERT(ict::to_integer<uint16_t>(bs, true) == 0x0001);
        auto n = ict::to_integer<uint16_t>(bs, false);
        IT_ASSERT_MSG(n, n == 0x0100);
    }
#if 0
#endif
}

template <typename T>
void test_endian(ict::bitstring bs, T value, bool big = true) {
    T n = ict::to_integer<T>(bs, big);
    IT_ASSERT(n == value);
}

void ict::bitstring_unit::endian() {
    test_endian<uint8_t> ("01", 1);
    test_endian<uint16_t>("0001", 1);
    test_endian<uint32_t>("00000001", 1);
    test_endian<uint64_t>("00000000 00000001", 1);

    test_endian<uint8_t> ("01", 1, false);
    test_endian<uint16_t>("0100", 1, false);
    test_endian<uint32_t>("01000000", 1, false);
    test_endian<uint64_t>("01000000 00000000", 1, false);

    test_endian<uint8_t> ("01", 0x01);
    test_endian<uint16_t>("0201", 0x0201);
    test_endian<uint32_t>("04030201", 0x04030201);
    test_endian<uint64_t>("08070605 04030201", 0x0807060504030201);

    test_endian<uint8_t> ("01", 0x01, false);
    test_endian<uint16_t>("0102", 0x0201, false);
    test_endian<uint32_t>("01020304", 0x04030201, false);
    test_endian<uint64_t>("01020304 05060708", 0x0807060504030201, false);

    // same thing, but with int
    test_endian<int8_t> ("01", 1);
    test_endian<int16_t>("0001", 1);
    test_endian<int32_t>("00000001", 1);
    test_endian<int64_t>("00000000 00000001", 1);

    test_endian<int8_t> ("01", 1, false);
    test_endian<int16_t>("0100", 1, false);
    test_endian<int32_t>("01000000", 1, false);
    test_endian<int64_t>("01000000 00000000", 1, false);

    test_endian<int8_t> ("01", 0x01);
    test_endian<int16_t>("0201", 0x0201);
    test_endian<int32_t>("04030201", 0x04030201);
    test_endian<int64_t>("08070605 04030201", 0x0807060504030201);

    test_endian<int8_t> ("01", 0x01, false);
    test_endian<int16_t>("0102", 0x0201, false);
    test_endian<int32_t>("01020304", 0x04030201, false);
    test_endian<int64_t>("01020304 05060708", 0x0807060504030201, false);

    // negative
    test_endian<uint8_t>("@1111 1111", 255);
    test_endian<int8_t> ("@1111 1111", -1);
    test_endian<int8_t> ("@1111 1110", -2);
    test_endian<int8_t> ("@1111 1101", -3);
    test_endian<int8_t> ("@1000 0000", -128);

    // things that are smaller size that T
    test_endian<uint8_t>("@1", 1);
    test_endian<uint8_t>("@01", 1);
    test_endian<uint8_t>("@001", 1);
    test_endian<uint8_t>("@0001", 1);

    test_endian<uint16_t>("FF", 0xFF);
    test_endian<int16_t>("FF", 0xFF);
}

void ict::bitstring_unit::expand_endian() {
    // convert 16 bit to 32 bit, with endian swap
    auto n = ict::to_integer<uint32_t>("0100", false);
    IT_ASSERT_MSG(n, n == 1);
    test_endian<uint32_t>("0100", 1, false);
}

void ict::bitstring_unit::toUint64()
{
    {
        ict::bitstring bs1(16, "0000000000000001");
        IT_ASSERT(bs1.bit_size() == 64);
        unsigned long long v = ict::to_integer<unsigned long long>(bs1);
        IT_ASSERT_MSG(v << " != " << 0x0000000000000001, 
            v == 0x0000000000000001);
    }
    {
        ict::bitstring bs1(16, "00000001");
        IT_ASSERT(bs1.bit_size() == 32);
        unsigned long long v = ict::to_integer<unsigned long long>(bs1);
        IT_ASSERT_MSG(v << " != " << 0x00000001, v == 0x00000001);
    }
    {
        ict::bitstring bs1(16, "FFFFFFFF");
        unsigned long long v = ict::to_integer<unsigned long long>(bs1);
        IT_ASSERT_MSG(v << " != " << 0xFFFFFFFF, v == 0xFFFFFFFF);
    }
    {
        ict::bitstring bs1(16, "FFFFFFFF");
        unsigned long long v = ict::to_integer<unsigned long long>(bs1);
        IT_ASSERT(v == 0xFFFFFFFF);
    }
    {
        ict::bitstring bs1(16, "FFFFFFFFFFFFFFFF");
        unsigned long long v = ict::to_integer<unsigned long long>(bs1);
        IT_ASSERT(v == 0xFFFFFFFFFFFFFFFF);
    }
}

template <typename T, typename U>
bool approx(T v, U f, double delta = 0.001) {
    return (f > (v - delta)) && (f < (v + delta));
}

void ict::bitstring_unit::fromNumber() {
    {
        ict::netvar<unsigned short> t(25);
        unsigned short u = t;
        IT_ASSERT_MSG(u << ": " << t, u == 25); 
    }
    {
        ict::bitstring bs(16, "42238077"); // 40.875454
        IT_ASSERT(bs.bit_size() == 32);
        float f = ict::to_integer<float>(bs);
        IT_ASSERT_MSG("float value: " << f, approx(f, 40.875));
    }
    {
        ict::bitstring bs=ict::from_integer<long>(32);
        long f = ict::to_integer<long>(bs);
        IT_ASSERT_MSG(bs.bit_size() << ":" << sizeof(long), bs.bit_size() ==
        sizeof(long) * 8);
        IT_ASSERT_MSG(f, f == 32);
    }
    {
        unsigned short n = 0x0001;
        ict::bitstring bs = ict::from_integer(n);
        IT_ASSERT_MSG("length = " << bs.bit_size(), bs.bit_size() == 16);
        IT_ASSERT_MSG(bs, bs == "0001");
        unsigned short f = ict::to_integer<unsigned short>(bs);
        IT_ASSERT(n == f);
    }
    {
        float n = 40.875;
        ict::bitstring bs = ict::from_integer(n);
        IT_ASSERT_MSG("length = " << bs.bit_size(), bs.bit_size() == 32);
        float f = ict::to_integer<float>(bs);
        IT_ASSERT_MSG("float value: " << f, approx(n, f));
    }
}

void ict::bitstring_unit::toInteger()
{
    IT_ASSERT(ict::to_integer<int>(ict::from_integer(8)) == 8);

    ict::bitstring bs = ict::bitstring(2, "1000");
    IT_ASSERT(ict::to_integer<int>(bs) == 8);
    bs = ict::from_integer(16);
    IT_ASSERT(ict::to_integer<int>(bs) == 16);

    ict::bitstring bs2 = ict::bitstring(2, "10000");
    IT_ASSERT(bs2.bit_size() == 5);
    int r = ict::to_integer<int>(bs2);
    IT_ASSERT(r == 16);
    {
        // 0000 0000 0010 0000
        // .... ..00 0010 000.
        ict::bitstring bs = ict::bitstring(16, "0020");
        ict::bitstring sub = bs.substr(6, 9);
        ict::bitstring p(16);
        ict::bitstring t(sub);
        ict::bitstring bs3(p);
        bs3 = ict::util::replace_bits(bs3, 6, sub);
        //bs3.replace(6, sub);
        IT_ASSERT(bs3.bit_size() == 16);
        IT_ASSERT_MSG(sub << ", " << bs3, bs3 =="@0000000000100000");
    }
    {
        ict::bitstring bs = ict::bitstring(16, "0020"); // 0000 0000 0010 0000
        ict::bitstring sub = bs.substr(6, 9);      // .... ..00 0010 000.
        //IT_ASSERT_MSG(sub.info(), sub.toBinString() == "000010000");

        sub = ict::util::pad_left(sub, 16);
        //sub.padLeft(16);  // 0000 0000 0001 0000
        IT_ASSERT(sub =="@0000000000010000");
        
        char const * c = sub.begin();
        IT_ASSERT_MSG(std::hex << (int) c[0], c[0] == 0x00);
        IT_ASSERT(c[1] == 0x10);

        //IT_ASSERT(sub.padLeft().length() == 16);
        //IT_ASSERT(sub.padLeft(32).length() == 32);
        int r = ict::to_integer<int>(sub);
        IT_ASSERT(r == 16);
    }
    {

        ict::bitstring bs = ict::bitstring(16, "0020"); // 0000 0000 0010 0000
        IT_ASSERT(ict::to_integer<unsigned int>(bs.substr(7, 9)) == 32);
    }
}

