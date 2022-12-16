#include "BenchMark.h"

int main(int argc, char *argv[]) {

    lpi_init_library();

    struct globalopts opts;
    char *filterstring = NULL;
    int threads = 1;
    int bufferresults = 10;
    opts.dir_method = DIR_METHOD_PORT;
    opts.only_dir0 = false;
    opts.only_dir1 = false;
    opts.require_both = false;
    opts.nat_hole = false;
    opts.ignore_rfc1918 = false;
    opts.local_mac = NULL;
    opts.file_url = "../../TEMP/";
    opts.trace_file = NULL;

    const char* filenames = "iptraces.txt";
    unsigned long long buf_size = 5000000000;

    for (int memory = 8192; memory <= 8192 * 8; memory = memory + 8192) {

        std::cout << "Start Memory: " << memory / 1024 << " KB." << std::endl;
        std::cout << std::endl;

        std::string file;
        std::ifstream tracefiles(filenames);
        if (!tracefiles.is_open()) {
            std::cout << "Error opening file" << std::endl;
            return -1;
        }

        uint32_t K[3] = {1000, 100, 1000};
        uint32_t hash_num = 2;
        double r_CF = 0.99;

        epoch_clear();
        for (std::string file; getline(tracefiles, file);) {
            int first = file.find(".pcap");
            std::string trace_file = file.substr(0, first);
            std::string suffix = file.substr(first);
            opts.trace_file = (char *)trace_file.c_str();
            opts.suffix = (char *)suffix.c_str();

            /*if (lpi(opts, filterstring, threads, bufferresults) != 0) {
                std::cout << "lpi function error!" << std::endl;
            }*/

            BenchMark dataset("../../TEMP/lpiout_", trace_file, buf_size);

            add_epoch();

            dataset.HHMultiBench_AppSketch(memory, true, 0, K, hash_num);
            dataset.HHMultiBench_USS(memory, true, 0, K);
            dataset.Bench_WavingSketch(memory, true, 0, K);
            dataset.SingleBench_WavingSketch(memory, true, 0, K);
            dataset.Bench_HeavyGuardian(memory, true, 0, K);
            dataset.SingleBench_HeavyGuardian(memory, true, 0, K);
            dataset.Bench_ColdFilter(memory, true, 0, K, hash_num, r_CF);
            dataset.SingleBench_ColdFilter(memory, true, 0, K, hash_num, r_CF);
            dataset.HHMultiBench_DMatrix(memory, true, 0, K, hash_num);
            dataset.Bench_CocoWaving(memory, true, 0, K, hash_num);
        }

        std::cout << "Memory: " << memory / 1024 << " KB." << std::endl;
        std::cout << std::endl;
        print_KKK();
        print_Time();
    }

    lpi_free_library();
    return 0;
}
