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

// Feature test macro for strtok_r (c.f., Linux Programming Interface p. 63)
#define _XOPEN_SOURCE 600

// To enable getline() on GNU systems
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "libhw1/CSE333.h"
#include "./CrawlFileTree.h"
#include "./DocTable.h"
#include "./MemIndex.h"

//////////////////////////////////////////////////////////////////////////////
// Helper function declarations, constants, etc
static void Usage(void);
static void ProcessQueries(DocTable* dt, MemIndex* mi);
static int GetNextLine(FILE* f, char** ret_str);


//////////////////////////////////////////////////////////////////////////////
// Main
int main(int argc, char** argv) {
  if (argc != 2) {
    Usage();
  }

  // Implement searchshell!  We're giving you very few hints
  // on how to do it, so you'll need to figure out an appropriate
  // decomposition into functions as well as implementing the
  // functions.  There are several major tasks you need to build:
  //
  //  - Crawl from a directory provided by argv[1] to produce and index
  //  - Prompt the user for a query and read the query from stdin, in a loop
  //  - Split a query into words (check out strtok_r)
  //  - Process a query against the index and print out the results
  //
  // When searchshell detects end-of-file on stdin (cntrl-D from the
  // keyboard), searchshell should free all dynamically allocated
  // memory and any other allocated resources and then exit.
  //
  // Note that you should make sure the fomatting of your
  // searchshell output exactly matches our solution binaries
  // to get full points on this part.
  printf("Indexing '%s'\n", argv[1]);
  MemIndex* mi = MemIndex_Allocate();
  DocTable* dt = DocTable_Allocate();

  if (!CrawlFileTree(argv[1], &dt, &mi)) {
    // CrawlFileTree failed, meaning path given was not indexable.
    printf("Path '%s' is not indexable\n", argv[1]);
    Usage();
    return EXIT_FAILURE;
  }

  ProcessQueries(dt, mi);

  printf("Shutting down...\n");
  DocTable_Free(dt);
  MemIndex_Free(mi);

  return EXIT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////
// Helper function definitions

static void Usage(void) {
  fprintf(stderr, "Usage: ./searchshell <docroot>\n");
  fprintf(stderr,
          "where <docroot> is an absolute or relative " \
          "path to a directory to build an index under.\n");
  exit(EXIT_FAILURE);
}

static void ProcessQueries(DocTable* dt, MemIndex* mi) {
  while (1) {
    printf("enter query:\n");
    char *query = NULL;
    if (GetNextLine(stdin, &query)) {
      char *token;
      char *saveptr;
      int i = 0;
      int arrSize = 16;
      char **arr = (char **)malloc(sizeof(char *) * arrSize);

      // Split query into words, get first word
      token = strtok_r(query, " \n\t", &saveptr);
      while (token != NULL) {
        // Make string lowercase
        for (int j = 0; j < strlen(token); j++) {
          token[j] = tolower(token[j]);
        }
        // Add word to array
        arr[i] = token;
        i++;

        // If the array is full, double the size
        if (i == arrSize) {
          arrSize *= 2;
          arr = (char **)realloc(arr, sizeof(char *) * arrSize);
        }
        // Get next word
        token = strtok_r(NULL, " \n\t", &saveptr);
      }
      arrSize = i;

      // Search the index for the array of words
      LinkedList *ll = MemIndex_Search(mi, arr, arrSize);
      if (ll != NULL) {
        // Loop through the linked list using iterator
        LLIterator *iter = LLIterator_Allocate(ll);
        while (LLIterator_IsValid(iter)) {
          SearchResult *sr;
          LLIterator_Get(iter, (LLPayload_t *)&sr);
          printf("  %s (%d)\n", DocTable_GetDocName(dt, sr->doc_id), sr->rank);
          LLIterator_Next(iter);
        }
        // Free iterator and linked list
        LLIterator_Free(iter);
        LinkedList_Free(ll, free);
      }
      // Free the array of words
      free(arr);
      // Free the query string as getline allocates memory for it
      free(query);
    } else {
      // EOF detected
      break;
    }
  }
}

static int GetNextLine(FILE *f, char **ret_str) {
  // Use getline to read a line from the file.
  size_t len = 0;
  if ((getline(ret_str, &len, f) != -1)) {
    return 1;
  }
  return 0;
}
