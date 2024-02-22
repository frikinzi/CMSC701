#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <unordered_map> 
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/utility.hpp>

int main(int argc, char* argv[]) {

    std::string path = argv[1];
    int k = std::atoi(argv[2]);
    std::string out = argv[3];

    // read file
    //std::cout << "reading file..." << std::flush;
    std::ifstream file(path);
    std::string line;
    // holds the sequence of the string
    std::string seq;
    // for each line, if line doesn't start with >, append string to seq. assumes there is only one ref seq
    
    while (std::getline(file, line)) {
        //std::string tmp = line;
        if (!line.empty() && line[0] != '>') {
            //tmp.erase(std::remove(tmp.begin(), tmp.end(), '\n'), tmp.end());
            seq += line;
        }
        
    }
    seq += "$";

    auto suffixComparator = [&](int a, int b) {
        int n = seq.length();
        for (int k = 0; a + k < n && b + k < n; ++k) {
            if (seq[a + k] != seq[b + k])
                return seq[a + k] < seq[b + k];
        }
        return (n - a) < (n - b);
    };

    //std::cout << "initializing..." << std::flush;
    
    //initialize suffix array
    std::vector<int> suffix_array_unsorted;
    //int suffix_array_unsorted[seq.length()];

    //build initial index
    for (int i = 0; i < seq.length(); i++) {
        suffix_array_unsorted.push_back(i);
    }
    std::sort(suffix_array_unsorted.begin(), suffix_array_unsorted.end(), suffixComparator);
    std::vector<int> sorted = suffix_array_unsorted;

    std::unordered_map<std::string, std::pair<int, int>> kmer_map;
    std::string curr_string = "$"; 
    int sl = seq.length();

    for (int j = 0; j < sorted.size(); ++j) {
        int st = sorted[j];
        if (sl - st < k) {
            continue; 
        }
        
        if (seq.compare(st, k, curr_string) != 0) {
            if (curr_string != "$") {
                kmer_map[curr_string].second = j - 1;
            }
            curr_string = seq.substr(st, k);
            kmer_map[curr_string] = std::make_pair(j, j); 
        } else if (j == sorted.size() - 1) {
            kmer_map[curr_string].second = j;
        }
    }
    //to delete: print kmer map
    //for (auto const& i : kmer_map) {
    //    std::cout << i.first << ": " << i.second.first << " " << i.second.second << std::endl;
    //}

    //create binary file
    std::ofstream os(out, std::ios::binary);
    cereal::BinaryOutputArchive archive(os);
    archive( kmer_map, sorted, seq );


    return 0;
}