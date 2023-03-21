# Bug 1

## A) How is your program acting differently than you expect it to?
- In our WriteIndex file, our crc wasn't working properly on step 3.
It would cause a segfault when getting there

## B) Brainstorm a few possible causes of the bug
- The CRC checker wasn't in the right spot to function properly
- Not all the bytes are being entered into the CRC

## C) How you fixed the bug and why the fix was necessary
- We had to fseek to the start of the doctable before fread ing the bytes.
Before, we had fseek to the beginning of the entire file, when the CRC is only
suppose to be reading from the start of the doctable, and not include the header

# Bug 2

## A) How is your program acting differently than you expect it to?
- valgrind is incredibly slow (takes 10+ minutes)

## B) Brainstorm a few possible causes of the bug
- A for/while loop isn't being terminated when it should
- Reading/writing something inproperly as disk accesses are slow
- Valgrind sucks :(

## C) How you fixed the bug and why the fix was necessary
- I saw a hint on edstem that a slow program could be due to the reading
one byte at a time from the CRC. That was exactly what we are doing. We
changed it to use a 512 byte buffer and read into buffer.

# Bug 3

## A) How is your program acting differently than you expect it to?
- Valgrind was leaking memory from QueryProcessor, specifically from the readers

## B) Brainstorm a few possible causes of the bug
- QueryProcessor isn't freeing something at the end when it needs to
- Maybe one of the methods I called is giving me back something in heap
- 

## C) How you fixed the bug and why the fix was necessary
- I was not calling delete on DocIDTableReader* that was returned from LookupWord
