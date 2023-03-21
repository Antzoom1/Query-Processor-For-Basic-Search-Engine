/*
 * Copyright ©2023 Justin Hsia.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Winter Quarter 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include "./QueryProcessor.h"

#include <iostream>
#include <algorithm>
#include <list>
#include <string>
#include <vector>
#include <unordered_map>

extern "C" {
  #include "./libhw1/CSE333.h"
}

using std::list;
using std::sort;
using std::string;
using std::vector;
using std::unordered_map;

namespace hw3 {

// Helper: Intersect two maps and store the result in map1
static void IntersectMaps(unordered_map<string, int> &map1,
                          unordered_map<string, int> &map2);

QueryProcessor::QueryProcessor(const list<string>& index_list, bool validate) {
  // Stash away a copy of the index list.
  index_list_ = index_list;
  array_len_ = index_list_.size();
  Verify333(array_len_ > 0);

  // Create the arrays of DocTableReader*'s. and IndexTableReader*'s.
  dtr_array_ = new DocTableReader* [array_len_];
  itr_array_ = new IndexTableReader* [array_len_];

  // Populate the arrays with heap-allocated DocTableReader and
  // IndexTableReader object instances.
  list<string>::const_iterator idx_iterator = index_list_.begin();
  for (int i = 0; i < array_len_; i++) {
    FileIndexReader fir(*idx_iterator, validate);
    dtr_array_[i] = fir.NewDocTableReader();
    itr_array_[i] = fir.NewIndexTableReader();
    idx_iterator++;
  }
}

QueryProcessor::~QueryProcessor() {
  // Delete the heap-allocated DocTableReader and IndexTableReader
  // object instances.
  Verify333(dtr_array_ != nullptr);
  Verify333(itr_array_ != nullptr);
  for (int i = 0; i < array_len_; i++) {
    delete dtr_array_[i];
    delete itr_array_[i];
  }

  // Delete the arrays of DocTableReader*'s and IndexTableReader*'s.
  delete[] dtr_array_;
  delete[] itr_array_;
  dtr_array_ = nullptr;
  itr_array_ = nullptr;
}

/* This structure is used to store a index-file-specific query result.
typedef struct {
  DocID_t doc_id;  // The document ID within the index file.
  int     rank;    // The rank of the result so far.
} IdxQueryResult;*/

vector<QueryProcessor::QueryResult>
QueryProcessor::ProcessQuery(const vector<string>& query) const {
  Verify333(query.size() > 0);

  // STEP 1.
  // (the only step in this file)
  vector<QueryResult> final_result;

  // Map that stores document name -> rank over multiple words
  unordered_map<string, int> combined_map;

  // Process query for each index file
  for (int i = 0; i < array_len_; i++) {
    // Get the respective index and doc table readers
    IndexTableReader *itr = itr_array_[i];
    DocTableReader *dtr = dtr_array_[i];

    // Clear the combined map from past index files
    combined_map.clear();

    bool firstWord = true;
    // Iterate through the query words for this index
    for (const string &word : query) {
      // Create map to store docID -> rank for this word
      unordered_map<string, int> word_map;
      DocIDTableReader* didtr = itr->LookupWord(word);

      // Word not found in this index, go to next index
      if (didtr == nullptr) {
        // Clear the combined map as intersection will be empty
        combined_map.clear();
        break;
      }

      list<DocIDElementHeader> doc_list = didtr->GetDocIDList();
      for (auto& doc_ele : doc_list) {
        // Get document name from docID
        string doc_name;
        // There must be a document name for each docID in the index
        Verify333(dtr->LookupDocID(doc_ele.doc_id, &doc_name));

        // Intersect the existing doc_rank_map
        word_map[doc_name] += doc_ele.num_positions;
      }
      // Only intersect maps if this is not the first word
      if (!firstWord) {
        IntersectMaps(combined_map, word_map);
      } else {
        combined_map = word_map;
        firstWord = false;
      }
      delete didtr;
    }

    // Push map into final result
    for (auto& kv : combined_map) {
      QueryResult qr;
      qr.document_name = kv.first;
      qr.rank = kv.second;
      final_result.push_back(qr);
    }
  }

  // Sort the final results.
  sort(final_result.begin(), final_result.end());
  return final_result;
}

static void IntersectMaps(unordered_map<string, int> &map1,
                        unordered_map<string, int> &map2) {
  // Iterate through map1
  for (auto it = map1.begin(); it != map1.end(); ) {
    // Check if the key is in map2
    if (map2.find(it->first) == map2.end()) {
      // Key not in map2, remove from map1
      it = map1.erase(it);
    } else {
      // Key in map2, add the rank
      it->second += map2[it->first];
      it++;
    }
  }
}

}  // namespace hw3
