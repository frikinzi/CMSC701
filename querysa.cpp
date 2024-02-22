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
#include <numeric>

// Function to compare the reference sequence to the query; also increments char_comparisons and for LCP, it updates the value
int str_comparator(int& start, int start_ref, const std::string& ref, std::string query, int& char_comparisons) {
    int cter = 0;
    int q = query.length();
    int r = ref.length();
    for (int i = start; i < q && i < r; i++) {
        char_comparisons++;
        if (query[i] < ref[i+start_ref]) {
            start += cter;
            return 1;
        } else if (query[i] > ref[i+start_ref]) {
            start += cter;
            return -1;
        }
        cter += 1;
    }
    if (r < q) {
        return -1; // edge case where reference is a substring of query
    }
    start += cter;
    return 0;
}

// Function to calculate LCP 
int calc_lcp(std::string ref, std::string query, int& char_comparisons) {
    int lcp = 0;
    int q = query.length();
    int r = ref.length();
    for (int i = 0; i < q && i < r; i++) {
        char_comparisons++;
        if (query[i] == ref[i]) {
            lcp++;
        } else {
            break;
        }
    }
    return lcp;
}

// Naive search to find lower bound of query in SA
std::vector<int> naive_binary_search_lower(std::vector<int>& vec, const std::string& ref, std::string query) {
    int left = 1;
    int right = vec.size();
    std::vector<int> res = {};
    int char_comparisons = 0;
    int middle = (left + right) / 2;
    //int char_comparisons_upper = 0;
    
    while (left < right) {
        int middle = (left + right) / 2;
        int start = 0;
        int compare_res = str_comparator(start, vec[middle], ref, query, char_comparisons);

        if (compare_res < 0) { //if temp string is smaller
            //char_comparisons += 1;
            left = middle + 1;
        } else {
            //char_comparisons += 1;
            right = middle;
        }
        
    }
    if (left < vec.size() && ref.substr(vec[left], query.length()) == query) {
        return {left, char_comparisons}; 
    } else {
        return {-1, char_comparisons};
    }
}

// naive binary search method to find higher bound (not including this index)
std::vector<int> naive_binary_search_higher(std::vector<int>& vec, const std::string& ref, std::string query) {
    int left = 1;
    int right = vec.size();
    std::vector<int> res = {}; // result should have higher bound index and the # character comparisons performed
    int char_comparisons = 0;
    int middle = (left + right) / 2;
    
    while (left < right) {
        int middle = (left + right) / 2;
        int start = 0;
        int compare_res = str_comparator(start, vec[middle], ref, query, char_comparisons);

        if (compare_res <= 0) { 
            left = middle + 1;
        } else { 
            right = middle;
        }
        
    }
    //char_comparisons_upper = char_comparisons;
    if (left < vec.size()) {
        return {left, char_comparisons}; 
    } else {
        return {-1, char_comparisons}; 
    }
}

// simpaccel function that accounts for LCPs
std::vector<int> simpaccel_binary_search_lower(std::vector<int>& vec, const std::string& ref, std::string query) {
    int left = 1;
    int right = vec.size()-1;
    std::vector<int> res = {};
    int char_comparisons = 0;
    int left_lcp = calc_lcp(ref.substr(vec[left]), query, char_comparisons);
    int right_lcp = calc_lcp(ref.substr(vec[right]), query, char_comparisons);
    
    //int char_comparisons_upper = 0;
    
    while (left < right) {
        int middle = (left + right) / 2;
        int mlr = std::min({left_lcp, right_lcp});

        int compare_res = str_comparator(mlr, vec[middle], ref, query, char_comparisons);

        if (compare_res < 0) { //if temp string is smaller
            left = middle + 1;
            left_lcp = mlr;
        } else {
            right = middle;
            right_lcp = mlr;
        }

        
    }
    if (left < vec.size() && ref.substr(vec[left], query.length()) == query) {
        return {left, char_comparisons}; 
    } else {
        return {-1, char_comparisons};
    }
}

// simpaccel to find higher bound
std::vector<int> simpaccel_binary_search_higher(std::vector<int>& vec, const std::string& ref, std::string query) {
    int left = 1;
    int right = vec.size()-1;
    std::vector<int> res = {};
    int char_comparisons = 0;
    int left_lcp = calc_lcp(ref.substr(vec[left]), query, char_comparisons);
    int right_lcp = calc_lcp(ref.substr(vec[right]), query, char_comparisons);
        
    while (left < right) {
        int middle = (left + right) / 2;
        int mlr = std::min({left_lcp, right_lcp});

        int compare_res = str_comparator(mlr, vec[middle], ref, query, char_comparisons);

        if (compare_res <= 0) { 
            left = middle + 1;
            left_lcp = mlr;
        } else { // temp string greater than query
            right = middle;
            right_lcp = mlr;
        }
        
    }
    //char_comparisons_upper = char_comparisons;
    if (left < vec.size()) {
        return {left, char_comparisons}; 
    } else {
        return {-1, char_comparisons}; 
    }
}

// prefaccel that uses prefix table;
std::vector<int> prefaccel_binary_search_lower(std::vector<int>& vec, const std::string& ref, std::string query, const std::unordered_map<std::string, std::pair<int, int> >& prefs) {
    int kmer_length = prefs.begin()->first.length();

    std::string kmer = query.substr(0,kmer_length);

    int left;
    int right;
    auto it = prefs.find(kmer);
    if (it == prefs.end()) {
        // Key not found
        return {0, 0, -1};
    }
    

    left = it->second.first;
    right = it->second.second + 1;
    std::vector<int> res = {left, right};
    int char_comparisons = 0;
    
    //int char_comparisons_upper = 0;
    
    while (left < right) {
        int middle = (left + right) / 2;
        //std::cout << middle << std::flush << std::endl;
        //std::string temp = ref.substr(vec[middle]);
        int st = 0;

        int compare_res = str_comparator(st, vec[middle], ref, query, char_comparisons);

        if (compare_res < 0) { //if temp string is smaller
            left = middle + 1;
        } else {
            right = middle;
        }

        
    }
    //char_comparisons_upper = char_comparisons;
    if (left < vec.size() && ref.substr(vec[left], query.length()) == query) {
        res.push_back(left);
        return res; 
    } else {
        res.push_back(-1);
        return res;
    }
}

// Prefaccel search to find higher bound
std::vector<int> prefaccel_binary_search_higher(std::vector<int>& vec, std::string ref, std::string query, const std::unordered_map<std::string, std::pair<int, int> >& prefs) {
    int kmer_length = prefs.begin()->first.length();

    std::string kmer = query.substr(0,kmer_length);

    int left;
    int right;
    auto it = prefs.find(kmer);
    if (it == prefs.end()) {
        return {0, 0, -1};
    }

    left = it->second.first;
    right = it->second.second + 1;
    
    std::vector<int> res = {left, right};
    int char_comparisons = 0;
    
    
    while (left < right) {
        int middle = (left + right) / 2;

        int st = 0;
        int compare_res = str_comparator(st, vec[middle], ref, query, char_comparisons);

        if (compare_res <= 0) { 
            left = middle + 1; // if ref string is less than query
        } else { 
            right = middle; // else if ref string is equal to or greater than query
        }
        
    }

    if (left < vec.size()) {
        res.push_back(left);
        return res;
    } else {
        res.push_back(-1);
        return res; 
    }
}

int main(int argc, char* argv[])
{
    
    if (argc != 5) {
        std::cerr << "Usage: query.cpp index query mode output";
        return 1;
    }
    std::string binary_file = argv[1];
    std::string queries = argv[2];
    std::string mode = argv[3];
    std::string out = argv[4];

    std::string seq;
    std::unordered_map<std::string, std::pair<int, int> > pref;
    std::vector<int> sa;

    std::ifstream is(binary_file, std::ios::binary); 
    cereal::BinaryInputArchive iarchive(is);

    // Deserialize data
    iarchive(pref, sa, seq);

    is.close();

    std::ifstream file(queries);
    std::string line;
    std::string t_l;

    std::vector<std::string> names;
    std::vector<std::string> sequences;

    while (std::getline(file, line)) {
        if (!line.empty() && line[0] == '>') {
            names.push_back(line.substr(1));
            sequences.push_back("");
        } else if (!line.empty() && !names.empty()) {
            sequences.back() += line;
        }
    }
    file.close();

    std::ofstream outfile(out);
    int test = 0;
    bool first_it = true;
    // For every sequence, perform search based on mode
    for (int q = 0; q < sequences.size(); q++) {
        if (!first_it) {
            outfile << "\n"; 
        } else {
            first_it = false; 
        }
        std::string query_name = names[q];
        std::string query_sequence = sequences[q];
        if (mode == "naive") {
            std::vector<int> results_left = naive_binary_search_lower(sa, seq, query_sequence);
            std::vector<int> results_right = naive_binary_search_higher(sa, seq, query_sequence);
            outfile << query_name + "\t";
            outfile << results_left[results_left.size() -1] << "\t" << results_right[results_right.size() -1] << "\t";
            if (results_left[0] == -1) {
                outfile << 0;
                continue;
            }
            int diff = results_right[0] - results_left[0];
            outfile << diff << "\t";
            for (int i = results_left[0]; i < results_right[0]; i++) {
                outfile << sa[i] << "\t";
            }
            //outfile << "\n";

        }  if (mode == "simpaccel") {
            std::vector<int> results_left = simpaccel_binary_search_lower(sa, seq, query_sequence);
            std::vector<int> results_right = simpaccel_binary_search_higher(sa, seq, query_sequence);
            outfile << query_name + "\t";
            outfile << results_left[results_left.size() -1] << "\t" << results_right[results_right.size() -1] << "\t";
            if (results_left[0] == -1) {
                outfile << 0;
                continue;
            }
            int diff = results_right[0] - results_left[0];
            outfile << diff << "\t";
            for (int i = results_left[0]; i < results_right[0]; i++) {
                outfile << sa[i] << "\t";
            }

        } if (mode == "prefaccel") {
            std::vector<int> res_prefaccel_left = prefaccel_binary_search_lower(sa, seq, query_sequence, pref);
            std::vector<int> res_prefaccel_upper = prefaccel_binary_search_higher(sa, seq, query_sequence, pref);
            outfile << query_name + "\t" << res_prefaccel_left[0] << "\t" << res_prefaccel_left[1] << "\t";
            if (res_prefaccel_left[2] == -1) {
                outfile << 0;
                continue;
            }
            int diff = res_prefaccel_upper[2] - res_prefaccel_left[2];
            outfile << diff << "\t";
            for (int i = res_prefaccel_left[2]; i < res_prefaccel_upper[2]; i++) {
                outfile << sa[i] << "\t";
            }
        }
    }
        
    
    outfile.close();

    





    return 0;
}