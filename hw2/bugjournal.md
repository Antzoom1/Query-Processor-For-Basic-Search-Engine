# Bug 1

## A) How is your program acting differently than you expect it to?
- FileParser was putting more words into the table that it was suppose to

## B) Brainstorm a few possible causes of the bug
- I looked back the compiler errors and there was a error in the InsertContent method.
- AddWordPosition could be causing the bug too as its called in the method
- Words are getting repeated in the table?

## C) How you fixed the bug and why the fix was necessary
- We fixed the bug by putting the statement that was in the else branch when the char is not alphabetic, "word_start = cur_ptr + 1;" out of the if statement. This caused an issue when it was in the if statement, when there were two consecutive non alphabetic words, it'll think it just finished a word and add the previous string (which was not alphabetic) into the table. This ended up adding a lot of junk into the table.

# Bug 2

## A) How is your program acting differently than you expect it to?
- When I was compiling the MemIndex.c file and running the tests for it, The tests were resulting in a infinite loop.

## B) Brainstorm a few possible causes of the bug
- There was no increment in the MemIndex_Search
- Nothing was getting printed out in the test
- 

## C) How you fixed the bug and why the fix was necessary
- I first tried a series of print statements to figure out if the loop was the problem. There were actually no print statements then when I tested it again so I looked elsewhere. Since the loop involved the iterator, I double checked it and I found the error. I was allocating memory for the hashtable where I was supposed to allocate memory for the HTIterator. I just called the method HTIterator_allocate and everything worked smoothly. This was because I was allocating for the wrong type of data so It never even went to the while loop.


# Bug 3

## A) How is your program acting differently than you expect it to?
- MemIndex_Search is giving me back a NULL linked list when I call it in searchshell.c

## B) Brainstorm a few possible causes of the bug
- MemIndex_Search has a bug - unlikely as it passes the tests
- I didn't initialize the MemIndex correctly
- One of my arguments into MemIndex_Search is incorrect
- The array I'm passing in isn't being malloced correctly
- 

## C) How you fixed the bug and why the fix was necessary
- I double checked and made sure MemIndex_NumWords was not 0, which it was not. I checked that my array size int passed in is correct, and it matches up
to the number of strings I inputed. So it seems like the array isn't right. In order to support an indefinite amount of strings in the query, I used
a dynamic sized array that is realloc when filled, so the code is complex and could have a bug. After a lot of debugging, it turns out that sort of
the problem and I had it malloced as char* and not char**. char* is a single string while I needed an array of strings.
