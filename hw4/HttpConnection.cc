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

#include <stdint.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <string>
#include <vector>

#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpConnection.h"

using std::map;
using std::string;
using std::vector;

namespace hw4 {

static const char* kHeaderEnd = "\r\n\r\n";
static const int kHeaderEndLen = 4;

bool HttpConnection::GetNextRequest(HttpRequest* const request) {
  // Use WrappedRead from HttpUtils.cc to read bytes from the files into
  // private buffer_ variable. Keep reading until:
  // 1. The connection drops
  // 2. You see a "\r\n\r\n" indicating the end of the request header.
  //
  // Hint: Try and read in a large amount of bytes each time you call
  // WrappedRead.
  //
  // After reading complete request header, use ParseRequest() to parse into
  // an HttpRequest and save to the output parameter request.
  //
  // Important note: Clients may send back-to-back requests on the same socket.
  // This means WrappedRead may also end up reading more than one request.
  // Make sure to save anything you read after "\r\n\r\n" in buffer_ for the
  // next time the caller invokes GetNextRequest()!

  // STEP 1:
  size_t res = buffer_.find(kHeaderEnd);
  while (res == string::npos) {
    // Read more bytes from the file.
    char buf[1024];
    int read_res = WrappedRead(fd_,
      reinterpret_cast<unsigned char*>(buf), 1024);
    // checks if the connection drops
    if (read_res == -1) {
      return false;
    }
    // checks if the end of the request header is reached
    buffer_.append(buf, read_res);
    res = buffer_.find(kHeaderEnd);
  }
  // parse the request and sets the buffer
  *request = ParseRequest(buffer_.substr(0, res + kHeaderEndLen));
  buffer_ = buffer_.substr(res + kHeaderEndLen);
  return true;
}

bool HttpConnection::WriteResponse(const HttpResponse& response) const {
  string str = response.GenerateResponseString();
  int res = WrappedWrite(fd_,
                         reinterpret_cast<const unsigned char*>(str.c_str()),
                         str.length());
  if (res != static_cast<int>(str.length()))
    return false;
  return true;
}

HttpRequest HttpConnection::ParseRequest(const string& request) const {
  HttpRequest req("/");  // by default, get "/".

  // Plan for STEP 2:
  // 1. Split the request into different lines (split on "\r\n").
  // 2. Extract the URI from the first line and store it in req.URI.
  // 3. For the rest of the lines in the request, track the header name and
  //    value and store them in req.headers_ (e.g. HttpRequest::AddHeader).
  //
  // Hint: Take a look at HttpRequest.h for details about the HTTP header
  // format that you need to parse.
  //
  // You'll probably want to look up boost functions for:
  // - Splitting a string into lines on a "\r\n" delimiter
  // - Trimming whitespace from the end of a string
  // - Converting a string to lowercase.
  //
  // Note: If a header is malformed, skip that line.

  // STEP 2:
  // Split the request into different lines (split on "\r\n").
  std::vector<std::string> lines;
  std::vector<std::string> headers;
  boost::split(lines, request, boost::is_any_of("\r\n"), boost::
    token_compress_on);
  // Extract the URI from the first line and store it in req.URI.
  std::vector<std::string> first_line;
  // splits the first line
  boost::split(first_line, lines[0], boost::is_any_of(" "), boost::
    token_compress_on);
  // if there is no space, then the first line is the URI
  if (first_line.size() == 3) {
    req.set_uri(first_line[1]);
  }
  // For the rest of the lines in the request, track the header name and
  // value and store them in req.headers_.
  for (uint32_t i = 1; i < lines.size(); i++) {
    boost::split(headers, lines[i], boost::is_any_of(": "), boost::
      token_compress_on);
    if (headers.size() == 2) {
      // Trimming whitespace from the end of a string
      boost::to_lower(headers[0]);
      req.AddHeader(headers[0], headers[1]);
    }
  }
  return req;
}

}  // namespace hw4
