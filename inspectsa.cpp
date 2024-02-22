#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <cmath>
#include <unordered_map> 
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/utility.hpp>
#include <numeric>

int main(int argc, char* argv[])
{
    
    if (argc != 4) {
        std::cerr << "Usage: inspectsa.cpp <BINARY FILE> <SAMPLE_RATE> <OUTPUT>";
        return 1;
    }
    std::string binary_file = argv[1];
    int sample_rate = std::atoi(argv[2]);
    std::string out = argv[3];

    //std::cout << binary_file;

    std::string seq;
    std::unordered_map<std::string, std::pair<int, int> > pref;
    std::vector<int> sa;

    std::ifstream is(binary_file, std::ios::binary); 
    cereal::BinaryInputArchive iarchive(is);

    // Deserialize the data
    iarchive(pref, sa, seq);

    is.close();

    //with_separator(sa);

    std::vector<int> lcps;

    for (int i = 1; i < sa.size(); i++) {
    int pref_len = 0;
    for (int j = 0; j + sa[i] < seq.length() && j + sa[i - 1] < seq.length(); j++) {
        if (seq[sa[i] + j] != seq[sa[i - 1] + j]) {
            break;
        }
        pref_len++;
    }
    lcps.push_back(pref_len);
}
//
    //for (int i = 1; i < sa.size(); i++) {
    //    std::string str1 = seq.substr(sa[i-1]);
    //    std::string str2 = seq.substr(sa[i]);
//
    //    int si = str1.length() < str2.length()? str1.length() : str2.length();
    //    int pref_len = 0;
    //    if (si > 1) {
    //        for (int j = 0; j < si-1; j++) {
    //            if (str1[j] == str2[j]) {
    //                pref_len += 1;
    //            } else {
    //                break;
    //            }
    //        }
    //    }
//
    //    lcps.push_back(pref_len);
    //}
//
    //with_separator(lcps);

    double average = std::accumulate(lcps.begin(), lcps.end(), 0.0) / lcps.size();
    std::sort(lcps.begin(), lcps.end());
    double median = 0.0;
    if (lcps.size() % 2 == 0) {
        median = (lcps[lcps.size() / 2-1] + lcps[lcps.size() / 2]) / 2.0;
    } else {
        median = lcps[lcps.size() / 2];
    }
    int max = lcps[lcps.size()-1];

    std::ofstream outfile(out);
    outfile << average << std::endl;
    outfile << median << std::endl;
    outfile << max << std::endl;
    for (int k = 0; k <= floor(sa.size() / sample_rate); k++) {
        outfile << sa[k * sample_rate];
        outfile << "\t";
    }
    outfile.close();

    return 0;
}