# Bug 1

## A) How is your program acting differently than you expect it to?
- When we ran the server, we could go to the page, but the server would
immediately give the following error and crash:
Assertion 'close_nointr(fd) != -EBADF' failed at src/basic/fd-util.c:74, function safe_close(). Aborting.

## B) Brainstorm a few possible causes of the bug
- It seems like that error is about double closing the same file descriptor
- Something in ServerSocket.cc was incorrect and not accepting a client correctly
- Something in the while loop in HttpServer is closing a file descriptor to early

## C) How you fixed the bug and why the fix was necessary
- I saw that in HttpConnection class destructor, it would close it's file descriptor.
I realized in my while loop, I was recreating an HttpConnection each time, which
was causing it to close when it fell out of scope. By moving the HttpConnection
initialization to outside the while loop, the server would keep the connection alive.

# Bug 2

## A) How is your program acting differently than you expect it to?
- IsPathSafe() doesn't pass all the tests, it doesn't work on
"test_files/ok_not_really/private.txt"

## B) Brainstorm a few possible causes of the bug
- .find() isn't working how I'm using it
- Edge case where the beginning matches but the end isn't

## C) How you fixed the bug and why the fix was necessary
- I looked at the test .cc file and found it failed at
"test_files/ok_not_really/private.txt"
I realized I had to add an extra "/" to the path of the root dir,
so it wouldn't match the "test_files/ok"

# Bug 3

## A) How is your program acting differently than you expect it to?
- Going to /query (an invalid url) would cause the server to seg fault. 

## B) Brainstorm a few possible causes of the bug
- My code checks for "/query" before passing the URI into the URLParser,
probably something there is not right
- Bug in the URLParser that doesn't handle when the input is malformed? 
Unlikely as we didn't write this method

## C) How you fixed the bug and why the fix was necessary
- I had to check the return map by URLParser and make sure it wasn't
empty before trying to access its elements.
