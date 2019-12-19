#include "bitstringunit.h"
#include <algorithm>
#include <bitstring.h>
#include <cstdint>
#include <cstring>
#include <vector>

namespace ict {
struct store_init {
    size_t bit_size;
    size_t byte_size;
    bool local;
};

void bitstring_unit::bitstring_sanity() {
    ict::bitstring a;

    const size_t ptr_bytes = sizeof(char *);
    const size_t ptr_bits = ptr_bytes * 8;

    std::vector<store_init> init = {
        {0, 0, true},
        {1, 1, true},
        {2, 1, true},
        {3, 1, true},
        {4, 1, true},
        {5, 1, true},
        {6, 1, true},
        {7, 1, true},
        {8, 1, true},
        {9, 2, true},
        {10, 2, true},
        {16, 2, true},
        {17, 3, true},

        {ptr_bits - 1, ptr_bytes, true},
        {ptr_bits, ptr_bytes, true},
        {ptr_bits + 1, ptr_bytes + 1, false},
        {8 * 1024 - 1, 1024, false},
        {8 * 1024, 1024, false},
        {8 * 1024 + 1, 1025, false},
    };

    std::vector<ict::bitstring> stores;

    for (auto test : init) {
        ict::bitstring a(test.bit_size);
        IT_ASSERT_MSG(test.bit_size, a.empty() == (test.bit_size == 0));
        IT_ASSERT_MSG(test.bit_size, a.bit_size() == test.bit_size);
        IT_ASSERT_MSG(test.bit_size << ", byte_size = " << a.byte_size(),
                      a.byte_size() == test.byte_size);
        IT_ASSERT_MSG(test.bit_size, a.local() == test.local);
        stores.push_back(a);
    }

    // copy constructor
    int i = 0;
    for (auto &test : stores) {
        std::memset(test.begin(), i, test.byte_size());

        ict::bitstring x(test);

        IT_ASSERT(x.empty() == test.empty());
        IT_ASSERT(x.local() == test.local());
        IT_ASSERT(x.bit_size() == test.bit_size());
        IT_ASSERT(x.byte_size() == test.byte_size());
        IT_ASSERT(std::equal(x.begin(), x.end(), test.begin()));
        IT_ASSERT_MSG(i << ": " << x, x == test);

        if (!x.empty()) {
            std::memset(x.begin(), 0xFF, x.byte_size());
            IT_ASSERT_MSG(i << ": " << x, x != test);
        }
        x.clear();
        IT_ASSERT(x.empty());
        IT_ASSERT(x.local() == true);
        IT_ASSERT(x.bit_size() == 0);
        IT_ASSERT(x.byte_size() == 0);

        ++i;
    }

    // assignment operator
    i = 0;
    for (auto &test : stores) {
        ict::bitstring x;
        x = test;
        IT_ASSERT(x.empty() == test.empty());
        IT_ASSERT(x.local() == test.local());
        IT_ASSERT(x.bit_size() == test.bit_size());
        IT_ASSERT(x.byte_size() == test.byte_size());
        IT_ASSERT(std::equal(x.begin(), x.end(), test.begin()));
        IT_ASSERT_MSG(i << ": " << x, x == test);

        if (!x.empty()) {
            std::memset(x.begin(), 0xFF, x.byte_size());
            IT_ASSERT_MSG(i << ": " << x, x != test);
        }

        x.clear();
        IT_ASSERT(x.empty());
        IT_ASSERT(x.local() == true);
        IT_ASSERT(x.bit_size() == 0);
        IT_ASSERT(x.byte_size() == 0);
        ++i;
    }
}

void bitstring_unit::obs() {
    ict::obitstream os;

    os << ict::bitstring("@111");
    os << ict::bitstring("@000");

    IT_ASSERT(os.bits() == "@111000");
    os << os.bits();
    IT_ASSERT(os.bits() == "@111000111000");

    auto t = os.bits();
    std::string base_string =
        ict::to_bin_string(t.begin(), t.end(), t.bit_size());
    auto s = base_string;
    auto b = os.bits();
    ict::obitstream bs;
    std::ostringstream ss;
    ss << "@";

    for (int i = 0; i < 1000; ++i) {
        bs << b;
        ss << s;
        auto ts = ss.str();
        IT_ASSERT(bs.bits() == ts.c_str());
        IT_ASSERT(bs.bits().bit_size() == ts.size() - 1);
    }
    t = bs.bits();
    std::string t2 = "@" + ict::to_bin_string(t.begin(), t.end(), t.bit_size());
    IT_ASSERT_MSG(ss.str() << " == " << t2, ss.str() == t2);
    {
        ict::obitstream os;
        os << ict::bitstring("@111");
        os << ict::bitstring("@000");
        auto bits = os.bits(); // bits = @111000
        IT_ASSERT(bits == "@111000");
    }
}

void bitstring_unit::ibs_constraint() {
    ict::bitstring bits(27);
    ict::ibitstream ibs(bits);

    IT_ASSERT(ibs.remaining() == 27);
    ibs.constrain(20);
    IT_ASSERT(ibs.remaining() == 20);
    ibs.unconstrain();
    IT_ASSERT(ibs.remaining() == 27);

    {
        ict::constraint c(ibs, 15);
        IT_ASSERT_MSG(ibs.remaining(), ibs.remaining() == 15);
    }

    IT_ASSERT(ibs.remaining() == 27);

    {
        ict::constraint c(ibs, 15);
        IT_ASSERT_MSG(ibs.remaining(), ibs.remaining() == 15);
        {
            ict::constraint c(ibs, 10);
            IT_ASSERT_MSG(ibs.remaining(), ibs.remaining() == 10);
        }
        IT_ASSERT_MSG(ibs.remaining(), ibs.remaining() == 15);
    }
    IT_ASSERT(ibs.remaining() == 27);

    {
        auto bits = ict::bitstring(27); // bits.bit_size() = 27
        ict::ibitstream is(bits);       // is.remaining() = 27
        {
            ict::constraint c(is, 15); // is.remaining() = 15
        }
        // is.remaining() == 27
    }
}

void bitstring_unit::ibs() {
    ict::bitstring bits("@111000");
    ict::ibitstream is2(bits);
    IT_ASSERT_MSG(is2.remaining() << " == 6", is2.remaining() == 6);
    IT_ASSERT(!is2.eobits());

    auto a = is2.read(1);
    IT_ASSERT(a == "@1");
    IT_ASSERT_MSG(is2.remaining(), is2.remaining() == 5);
    IT_ASSERT(!is2.eobits());

    a = is2.read(3);
    IT_ASSERT(!is2.eobits());
    IT_ASSERT(is2.remaining() == 2);
    IT_ASSERT(a == "@110");

    a = is2.read(2);
    IT_ASSERT(is2.eobits());
    IT_ASSERT(is2.remaining() == 0);
    IT_ASSERT(a == "@00");

    {
        auto bits = ict::bitstring("@111000");
        ict::ibitstream is(bits);
        auto a = is.read(3); // a = @111
        auto b = is.read(3); // b = @000
        IT_ASSERT(a == "@111");
        IT_ASSERT(b == "@000");
    }
}

void bitstring_unit::from_string() {
    {
        ict::bitstring a("#00");
        IT_ASSERT(a.bit_size() == 8);
        IT_ASSERT(a.byte_size() == 1);
        IT_ASSERT(a == "#00");
        IT_ASSERT(a == "00");
        IT_ASSERT(a == "@00000000");
        IT_ASSERT(ict::to_hex_string(a.begin(), a.end()) == "00");
    }
    {
        ict::bitstring a("00");
        IT_ASSERT(a.bit_size() == 8);
        IT_ASSERT(a.byte_size() == 1);
        IT_ASSERT(a == "#00");
        IT_ASSERT(a == "00");
        IT_ASSERT(a == "@00000000");
        IT_ASSERT(ict::to_string(a) == "#00");
    }
    {
        ict::bitstring a("FF");
        IT_ASSERT(a.bit_size() == 8);
        IT_ASSERT(a.byte_size() == 1);
        IT_ASSERT(a == "#FF");
        IT_ASSERT(a == "FF");
        IT_ASSERT(a == "@11111111");
        IT_ASSERT(ict::to_string(a) == "#FF");
    }
    {
        ict::bitstring a("80");
        IT_ASSERT(a.bit_size() == 8);
        IT_ASSERT(a.byte_size() == 1);
        IT_ASSERT(a == "#80");
        IT_ASSERT(a == "80");
        IT_ASSERT(a == "@10000000");
        IT_ASSERT(ict::to_string(a) == "#80");
    }
    {
        ict::bitstring a("@1");
        IT_ASSERT(a.bit_size() == 1);
        IT_ASSERT(a.byte_size() == 1);
        IT_ASSERT(a == "@1");
        IT_ASSERT(a != "00");
        auto s = ict::to_hex_string(a.begin(), a.end());
        IT_ASSERT_MSG(s, s == "80");
    }
}

void test_convert(size_t bit_size) {
    IT_ASSERT(bit_size > 0);
    // IT_WARN("converting " << bit_size);
    ict::bitstring bits = ict::from_integer<unsigned>(1, bit_size);
    std::string s = std::string(bit_size - 1, '0') + '1';
    IT_ASSERT(bits.bit_size() == bit_size);
    IT_ASSERT_MSG(s.size() << " == " << bit_size, s.size() == bit_size);

    auto sbits = ict::to_bin_string(bits.begin(), bits.end(), bits.bit_size());

    IT_ASSERT_MSG("(" << bits.bit_size() << ") " << bits << " == " << s,
                  sbits == s);

    auto n = ict::to_integer<uint64_t>(bits);
    IT_ASSERT_MSG(bits << ": " << n << " == 1", n == 1);
}

void bitstring_unit::int_convert() {
    {
        ict::bitstring bs0 = ict::from_integer<unsigned>(0, 1);
        IT_ASSERT_MSG(bs0, bs0 == "@0");
        IT_ASSERT(bs0.bit_size() == 1);
        IT_ASSERT_MSG(ict::to_integer<unsigned>(bs0),
                      ict::to_integer<unsigned>(bs0) == 0);

        ict::bitstring bs1 = ict::from_integer<unsigned>(1, 1);
        IT_ASSERT(bs1.bit_size() == 1);
        IT_ASSERT_MSG(bs1, bs1 == "@1");
        IT_ASSERT_MSG(ict::to_integer<unsigned>(bs1),
                      ict::to_integer<unsigned>(bs1) == 1);

        ict::bitstring bs2 = ict::from_integer<unsigned>(2, 2);
        IT_ASSERT(bs2.bit_size() == 2);
        IT_ASSERT(ict::to_integer<unsigned>(bs2) == 2);

        ict::bitstring bs3 = ict::from_integer<unsigned>(3, 3);
        IT_ASSERT(bs3.bit_size() == 3);
        IT_ASSERT(ict::to_integer<unsigned>(bs3) == 3);

        ict::bitstring bs15 = ict::from_integer<unsigned>(15, 15);
        IT_ASSERT(bs15.bit_size() == 15);
        IT_ASSERT(ict::to_integer<unsigned>(bs15) == 15);

        ict::bitstring bs31 = ict::from_integer<unsigned>(31, 31);
        IT_ASSERT(bs31.bit_size() == 31);
        IT_ASSERT(ict::to_integer<unsigned>(bs31) == 31);

        ict::bitstring bs32 = ict::from_integer<unsigned>(32, 32);
        IT_ASSERT(bs32.bit_size() == 32);
        IT_ASSERT(ict::to_integer<unsigned>(bs32) == 32);
    }
    {
        ict::bitstring bs1 = ict::from_integer<unsigned>(1, 15);
        IT_ASSERT(bs1.bit_size() == 15);
        IT_ASSERT(ict::to_integer<unsigned>(bs1) == 1);

        ict::bitstring bs32 = ict::from_integer<unsigned>(0xFF12, 32);
        IT_ASSERT(bs32.bit_size() == 32);
        IT_ASSERT(ict::to_integer<unsigned>(bs32) == 0xFF12);
    }
    {
        ict::bitstring bs1 = ict::from_integer<unsigned>(3, 4);
        IT_ASSERT(bs1.bit_size() == 4);
        IT_ASSERT(bs1 == "@0011");
        auto n = ict::to_integer<unsigned long long>(bs1);
        IT_ASSERT_MSG(n << " == 3",
                      ict::to_integer<unsigned long long>(bs1) == 3);
    }
    {
        ict::bitstring bs1 = ict::from_integer<unsigned>(3, 9);
        IT_ASSERT(bs1.bit_size() == 9);
        IT_ASSERT_MSG(ict::to_string(bs1), bs1 == "@000000011");
        auto n = ict::to_integer<unsigned long long>(bs1);
        IT_ASSERT_MSG(ict::to_string(n) << ": " << n << " == 3", n == 3);
    }

    for (int i = 1; i < 66; ++i)
        test_convert(i);

    {
        ict::bitstring bs1 = ict::from_integer<unsigned>(23, 33);
        IT_ASSERT(bs1.bit_size() == 33);
        auto n = ict::to_integer<uint64_t>(bs1);
        IT_ASSERT_MSG(ict::to_string(bs1) << ": " << n << " == 23", n == 23);
    }
    {
        ict::bitstring bs1 = ict::from_integer<unsigned>(23, 33);
        IT_ASSERT(bs1.bit_size() == 33);
        auto n = ict::to_integer<unsigned long long>(bs1);
        IT_ASSERT_MSG(ict::to_string(bs1) << ": " << n << " == 23", n == 23);
    }
    // ill conceived calls
    {
        ict::bitstring bs = ict::from_integer<unsigned>(1, 0);
        IT_ASSERT(bs.empty());
        IT_ASSERT(bs.bit_size() == 0);

        ict::bitstring bs2 = ict::from_integer<unsigned>(2, 1);
        IT_ASSERT(bs2.bit_size() == 1);

        ict::bitstring bs3 = ict::from_integer<unsigned>(120, 3);
        IT_ASSERT(ict::to_integer<unsigned>(bs3) == 0);
        ict::bitstring v5 = ict::from_integer<unsigned>(5, 3);
        IT_ASSERT(ict::to_integer<unsigned>(v5) == 5);
        ict::bitstring v6 = ict::from_integer<unsigned>(6, 3);
        IT_ASSERT(ict::to_integer<unsigned>(v6) == 6);
        ict::bitstring v7 = ict::from_integer<unsigned>(7, 3);
        IT_ASSERT(ict::to_integer<unsigned>(v7) == 7);
        ict::bitstring v8 = ict::from_integer<unsigned>(8, 3);
        IT_ASSERT(ict::to_integer<unsigned>(v8) == 0);
    }
}

void bitstring_unit::modern_pad() {
    {
        ict::bitstring bs1(2, "111111");
        IT_ASSERT(bs1.bit_size() == 6);
        bs1 = ict::detail::pad_left(bs1);
        IT_ASSERT_MSG(bs1.bit_size(), bs1.bit_size() == 8);
        IT_ASSERT(bs1 == "@00111111");

        // make sure it doesn't do any more padding
        ict::detail::pad_left(bs1);
        IT_ASSERT(bs1.bit_size() == 8);
        IT_ASSERT(bs1 == "@00111111");
    }
#if 0 // not even sure what this stuff should do... pad_left(10) doesn't make
      // sense
    {
        ict::bitstring bs1(2, "111111");
        bs1.padLeft(10);
        IT_ASSERT_MSG(bs1.info(), bs1 == "@0000111111");
    }
    {
        ict::bitstring bs1(50);
        bs1.padLeft();
        IT_ASSERT_MSG(bs1.info(), bs1.bit_size() == 56);
    }
    {
        ict::bitstring bs1(2, "111111");
        bs1.padRight();
        IT_ASSERT_MSG(bs1.info(), bs1 == "@11111100");

        // make sure it doesn't do any more padding
        bs1.padRight();
        IT_ASSERT_MSG(bs1.info(), bs1 == "@11111100");
    }
    {
        ict::bitstring bs1(2, "111111");
        bs1.padRight(10);
        IT_ASSERT_MSG(bs1.info(), bs1 == "@1111110000");
    }
    {
        ict::bitstring bs1(50);
        bs1.padRight();
        IT_ASSERT_MSG(bs1.info(), bs1.bit_size() == 56);
    }
        auto bs = ict::bitstring(2, "10000");
        IT_ASSERT(bs.bit_size() == 5);
        bs.padLeft();
        IT_ASSERT(bs.bit_size() == 8);
        IT_ASSERT_MSG(bs.info(), bs == "@00010000");
        IT_ASSERT_MSG(bs.info(), IT::toInteger<int>(bs) == 16);
#endif
}
void bitstring_unit::modern_replace() {
    {
        // these first 3 tests were added because of a bug found when the
        // fromNumber() line was added.
        ict::bitstring bs = ict::from_integer<unsigned>(1, 1);
        IT_ASSERT_MSG("length = " << bs.bit_size(), bs.bit_size() == 1);
        ict::bitstring bs2(2, "0");
        IT_ASSERT_MSG("bs2 length = " << bs2.bit_size(), bs2.bit_size() == 1);
        bs = ict::detail::replace_bits(bs, 0, bs2);
        IT_ASSERT_MSG("bs2 length = " << bs2.bit_size(), bs2.bit_size() == 1);
        IT_ASSERT_MSG("length = " << bs.bit_size(), bs.bit_size() == 1);
        IT_ASSERT(bs.at(0) == 0);
    }
    {
        ict::bitstring bs = ict::from_integer<unsigned>(0, 1);
        ict::bitstring bs2(2, "1");
        bs = ict::detail::replace_bits(bs, 0, bs2);
        IT_ASSERT(bs.at(0) == 1);
    }
    {
        ict::bitstring bs = ict::from_integer<unsigned>(0, 1);
        ict::bitstring bs2 = ict::from_integer<unsigned>(1, 1);
        bs = ict::detail::replace_bits(bs, 0, bs2);
        IT_ASSERT(bs.at(0) == 1);
    }
    { // just one bit
        ict::bitstring bs1(1);
        bs1.set(0);
        ict::bitstring bs2(1);
        bs1 = ict::detail::replace_bits(bs1, 0, bs2);
        IT_ASSERT(bs1.bit_size() == 1);
        IT_ASSERT(bs1.at(0) == 0);

        ict::bitstring bs3(2, "11");
        bs1 = ict::detail::replace_bits(bs1, 0, bs3);
        IT_ASSERT(bs1.bit_size() == 1);
        IT_ASSERT(bs1.at(0) == 1);
    }
    {
        ict::bitstring bs1(2, "11111");
        ict::bitstring bs2(2, "00");
        ict::bitstring bs3;

        bs3 = bs1;
        bs3 = ict::detail::replace_bits(bs3, 0, bs2);
        IT_ASSERT(bs3 == "@00111");

        bs3 = bs1;
        bs3 = ict::detail::replace_bits(bs3, 1, bs2);
        IT_ASSERT(bs3 == "@10011");

        bs3 = bs1;
        bs3 = ict::detail::replace_bits(bs3, 2, bs2);
        IT_ASSERT(bs3 == "@11001");

        bs3 = bs1;
        bs3 = ict::detail::replace_bits(bs3, 3, bs2);
        IT_ASSERT(bs3 == "@11100");

        bs3 = bs1;
        bs3 = ict::detail::replace_bits(bs3, 4, bs2);
        IT_ASSERT(bs3 == "@11110");

        bs3 = bs1;
        bs3 = ict::detail::replace_bits(bs3, 5, bs2);
        IT_ASSERT(bs3 == "@11111");

        auto foo = ict::bitstring(8);
        auto rep = ict::bitstring("@111111");
        foo = ict::detail::replace_bits(foo, 2, rep);
        IT_ASSERT(foo == "@00111111");
    }
}

void bitstring_unit::modern_gsm7() {
    /*

     Unpacked: "12345678"
     0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38

     Packed:
    31
    D9
    8C
    56
    B3
    DD
    70
    */
    ict::bitstring bs("31D98C56B3DD70");

    auto packed = ict::detail::to_uchar_array(bs);

    IT_ASSERT(packed.size() == 7);
    IT_ASSERT_MSG(packed[0], packed[0] == 0x31);
    IT_ASSERT_MSG(packed[1], packed[1] == 0xD9);
    IT_ASSERT_MSG(packed[2], packed[2] == 0x8C);
    IT_ASSERT_MSG(packed[3], packed[3] == 0x56);
    IT_ASSERT_MSG(packed[4], packed[4] == 0xB3);
    IT_ASSERT_MSG(packed[5], packed[5] == 0xDD);
    IT_ASSERT_MSG(packed[6], packed[6] == 0x70);

    auto unpacked = ict::detail::unpack_bytes(packed);
    IT_ASSERT(unpacked.size() == 8);
    IT_ASSERT_MSG(unpacked[0], unpacked[0] == 0x31);
    IT_ASSERT_MSG(unpacked[1], unpacked[1] == 0x32);
    IT_ASSERT_MSG(unpacked[2], unpacked[2] == 0x33);
    IT_ASSERT_MSG(unpacked[3], unpacked[3] == 0x34);
    IT_ASSERT_MSG(unpacked[4], unpacked[4] == 0x35);
    IT_ASSERT_MSG(unpacked[5], unpacked[5] == 0x36);
    IT_ASSERT_MSG(unpacked[6], unpacked[6] == 0x37);
    IT_ASSERT_MSG(unpacked[7], unpacked[7] == 0x38);

    IT_ASSERT_MSG("[" << ict::gsm7(bs) << "]", ict::gsm7(bs) == "12345678");

    ict::bitstring hello("C8329BFD06");
    IT_ASSERT(ict::gsm7(hello) == "Hello");

    ict::bitstring t1("@11011");
    t1.remove(2, 1);
    IT_ASSERT_MSG(to_string(t1), t1 == "@1111");

    ict::bitstring t2("@11011");
    IT_ASSERT(t2.remove(2, 1) == "@1111");

    ict::bitstring t3("@1111000");
    IT_ASSERT(t3.remove(6, 1) == "@111100");

    ict::bitstring t4("@11011");
    t4.remove(0, 1);
    IT_ASSERT_MSG(to_string(t4), t4 == "@1011");

    {
        ict::bitstring bs1("@111100011111010"); // 15

        // 6, 1
        IT_ASSERT(bs1.substr(0, 6) == "@111100");
        IT_ASSERT(ict::bitstring(bs1.bit_begin(), bs1.bit_begin() + 6) ==
                  "@111100");
        IT_ASSERT(bs1.substr(6 + 1, 15 - (6 + 1)) == "@11111010");
    }
    {
        size_t begin = 6;
        size_t len = 1;
        ict::bitstring bs1("@111100011111010"); // 15
        ict::bitstring t1 = bs1.substr(0, begin);
        IT_ASSERT(t1 == "@111100");
        IT_ASSERT(t1.bit_size() == begin);

        ict::bitstring t2 =
            bs1.substr(begin + len, bs1.bit_size() - (begin + len));
        IT_ASSERT(t2.bit_size() == bs1.bit_size() - (begin + len));
        IT_ASSERT(t2 == "@11111010");

        ict::bitstring t3(t1.bit_size() + t2.bit_size());
        IT_ASSERT(t3.bit_size() == bs1.bit_size() - len);
        t3 = ict::detail::replace_bits(t3, 0, t1);
        t3 = ict::detail::replace_bits(t3, begin, t2);
        IT_ASSERT_MSG(ict::to_string(t3), t3 == "@11110011111010");
    }

    {
        ict::bitstring raw(
            "@11110000111110101111000010011100110111000111011010001011111011010"
            "11000111011110000111110001111000010011100000011");
        ict::bitstring rem(
            "@11110001111101011110000100111001101110001110110100010111110110101"
            "1000111011110000111110001111000010011100000011");
        IT_ASSERT(raw.remove(6, 1) == rem);
        ict::bitstring pre("@1111000");
        IT_ASSERT(rem.substr(0, 7) == pre);

        ict::bitstring post(
            "@11111010111100001001110011011100011101101000101111101101011000111"
            "011110000111110001111000010011100000011");
        IT_ASSERT(rem.substr(7, rem.bit_size() - 7) == post);
        IT_ASSERT_MSG(ict::gsm7(post), ict::gsm7(post) == "zasdmnbvcxzasd");
    }

    ict::bitstring bs1("@111100011111010");
    ict::bitstring bs2("@11110011111010");
    IT_ASSERT(bs1.remove(6, 1) == bs2);

    // remove 1 fill bit, and parse first 7 bits as a character
    IT_ASSERT(ict::gsm7(ict::bitstring("@01111000")) == "x");

    // parse the rest as a plain sms message with no header
    IT_ASSERT(ict::gsm7(ict::bitstring(
                  "@11111010111100001001110011011100011101101000101111101101011"
                  "000111011110000111110001111000010011100000011")) ==
              "zasdmnbvcxzasd");

    // put them together to get the complete message
    std::ostringstream os;
    os << ict::gsm7(ict::bitstring("@01111000"))
       << ict::gsm7(ict::bitstring(
              "@111110101111000010011100110111000111011010001011111011010110001"
              "11011110000111110001111000010011100000011"));
    IT_ASSERT(os.str() == "xzasdmnbvcxzasd");

    {
        ict::bitstring raw(
            "@11110000111110101111000010011100110111000111011010001011111011010"
            "11000111011110000111110001111000010011100000011");
        std::string gsm_message = ict::gsm7(raw, 1);
        // now do it all within gsm7() call with fill bit parameter
        IT_ASSERT_MSG(gsm_message, gsm_message == "xzasdmnbvcxzasd");
    }
}

void bitstring_unit::modern_sms_difficult() {
    // E139F92CCF9BF379333D9FA7CD6435DBED86CBC17034992C041809042510C87456A301
    // should be: asdgryfyyftyy43256778908422!@#$$%^^&gjh

    std::string result = "asdgryfyyftyy43256778908422!@#$$%  &gjh";

    ict::bitstring sm("E139F92CCF9BF379333D9FA7CD6435DBED86CBC17034992C04180904"
                      "2510C87456A301");

    auto packed = ict::detail::to_uchar_array(sm);
    auto unpacked = ict::detail::unpack_bytes(packed);
    ict::detail::map_to_ascii(unpacked);

    IT_ASSERT(unpacked.size() == result.size());

    std::string sms(unpacked.begin(), unpacked.end());

    IT_ASSERT(sms == result);

    IT_ASSERT(ict::gsm7(sm) == result.c_str());
}

void random_copy(size_t n) {
    auto bs = random_bitstring(n);
    auto l = bs.bit_end() - bs.bit_begin();
    IT_ASSERT_MSG(l << " != " << n, l == n);
    auto x = bitstring(n);
    // std::cerr << "copying " << bs << '\n';
    bit_copy(bs.bit_begin(), bs.bit_end(), x.bit_begin());
    IT_ASSERT(bs.bit_size() == n);
    IT_ASSERT(x.bit_size() == n);
    IT_ASSERT_MSG("copying " << n << " bits: " << bs << " == " << x, bs == x);

    x = bitstring(n); // set to zero
    detail::bit_copy_n(bs.bit_begin(), bs.bit_size(), x.bit_begin());
    IT_ASSERT(bs.bit_size() == n);
    IT_ASSERT(x.bit_size() == n);
    IT_ASSERT_MSG("copying " << n << " bits: " << bs << " == " << x, bs == x);
}

void bitstring_unit::bit_iterators() {
    bit_iterator first;
    IT_ASSERT(first == bit_iterator());
    ++first;
    first++;
    --first;
    first--;
    IT_ASSERT(first == bit_iterator());

    {
        auto bs = bitstring("@111001");
        auto x = bitstring(bs.bit_size());
        IT_ASSERT(bs.bit_end() - bs.bit_begin() == 6);
        ict::bit_copy(bs.bit_begin(), bs.bit_end(), x.bit_begin());
        IT_ASSERT_MSG(bs << " != " << x, bs == x);
    }
    for (auto n = 1; n < 1024; ++n)
        random_copy(n);

    {
        auto x = random_bitstring(1024);
        auto first = x.bit_begin();
        auto y = first;
        IT_ASSERT(first == y);
        first++;
        first--;
        IT_ASSERT(first == y);
        first += 10;
        y += 10;
        IT_ASSERT(first == y);
        IT_ASSERT(y == first);
        auto z = y;
        z -= 10;
        IT_ASSERT(z == x.bit_begin());
    }

    {
        auto x = random_bitstring(1024);
        auto y = bitstring(1024);

        for (auto i = x.bit_begin(), j = y.bit_begin(); i != x.bit_end();
             ++i, ++j) {
            j->value(*i);
        }
        IT_ASSERT(x == y);
    }
}

namespace detail {
void const_test(const bitstring &bs) {
    auto first = bs.begin();
    auto last = bs.end();
    while (first != last) {
        IT_ASSERT(*first != 0);
        ++first;
    }
}
} // namespace detail

void bitstring_unit::const_bit_iterators() {
    ict::bitstring sm("E139F92CCF9BF379333D9FA7CD6435DBED86CBC17034992C04180904"
                      "2510C87456A301");
    detail::const_test(sm);
}

} // namespace ict
int main(int, char **) {
    ict::bitstring_unit test;
    ict::unit_test<ict::bitstring_unit> ut(&test);
    return ut.run();
}
