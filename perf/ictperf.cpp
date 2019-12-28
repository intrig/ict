#include <bitstring.h>
#include <command.h>
#include <ict.h>

using std::cerr;

template <typename Op> void time_op(int n, Op op) {
    ict::timer time;
    time.start();
    for (int i = 0; i < n; ++i)
        op();
    time.stop();
    cerr << time << '\n';
}

template <typename Op, typename EndOp>
void time_op(int n, Op op, EndOp end_op) {
    ict::timer time;
    time.start();
    for (int i = 0; i < n; ++i)
        op();
    end_op();
    time.stop();
    cerr << time << '\n';
}

static void in_bits(int s, int n) {
    // create a giant bitstring
    auto bits = ict::random_bitstring(1024);

    time_op(n, [&]() {
        ict::ibitstream ibs(bits);
        while (!ibs.eobits()) {
            auto b = ibs.read(s);
        }
    });
}

static void out_bits(int s, int n) {
    // create a giant bitstring
    auto bits = ict::random_bitstring(s);

    ict::obitstream obs;
    time_op(n, [&]() { obs << bits; }, [&]() { auto x = obs.bits(); });
}

int main(int argc, char **argv) {
    bool input = false;
    bool output = false;
    try {
        ict::command line("ictperf", "ict performance tests",
                          "ictperf [options]");
        line.add(ict::option("ibits", 'i', "input bitstream",
                             [&] { input = true; }));
        line.add(ict::option("obits", 'o', "output bitstream",
                             [&] { output = true; }));

        line.parse(argc, argv);
        if (input) {
            in_bits(3, 100000);
            in_bits(5, 100000);
            in_bits(8, 100000);
            in_bits(11, 100000);
        }

        if (output) {
            out_bits(3, 10000000);
            out_bits(5, 10000000);
            out_bits(8, 10000000);
            out_bits(11, 10000000);
        }
    } catch (std::exception &e) {
        cerr << "exception: " << e.what() << '\n';
    }
}
