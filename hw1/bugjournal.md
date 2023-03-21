# Bug 1

## A) How is your program acting differently than you expect it to?
- My code for LinkedList_Push is not passing the tests, resulting in
incorrect linked list but no errors

## B) Brainstorm a few possible causes of the bug
- I'm messing something up while shifting around the pointers,
perhaps I'm setting the tail to something that is null?
- Maybe I mised something in the spec for this because after
going through and drawing out how the new node is getting added in,
everything seems correct

## C) How you fixed the bug and why the fix was necessary
- Turns out push is adding to the head of the linked list.
I thought it was adding to the tail, spent so long debugging
when I should have just reread the spec.


# Bug 2

## A) How is your program acting differently than you expect it to?
- I'm using LinkedList_Append in HashTable_Insert, but it seems that
I'm inserting garbage (from what I see printing it out) rather 
than the newkeyvalue.

## B) Brainstorm a few possible causes of the bug
- I think I might be casting wrong? LinkedList_Append takes type
LLPayload_t which is typedef'd to void *, but I am getting
the address of newkeyvalue which is a pointer
- newkeyvalue is getting deallocated or freed somewhere?
- Actually its inserting the right thing when I print it out
right after inserting, so it's definitely something with allocation


## C) How you fixed the bug and why the fix was necessary
- Realized that the parameter newkeyvalue was not a pointer,
which means the data inside it is stored on the stack, so I was
storing a pointer in the linked list to HashTable_Insert's stack frame.
Once the insert finishes, that stack frame is destroyed. Instead,
I need to malloc space for it on the heap, and copy the values into there.


# Bug 3

## A) How is your program acting differently than you expect it to?
-  The program was returning false to the HTIterator_Next method implementation.
I was expecting it to return true after calling the HTIterator_Get method.

## B) Brainstorm a few possible causes of the bug
- I think it could have resulted in an incorrect implementation in the HTIterator_Get 
method I rerouted back towards the implementation that could have resulted into a error.
- I also thought that I forgot to add in a negate symbol in checking if certain iterators 
were valid.
- I also thought that I was returning the wrong thing for the isValid method.

## C) How you fixed the bug and why the fix was necessary
- I fixed the bug by using a not symbol for "if (!LLIterator_IsValid(iter->bucket_it))."
I think I simply overlooked it and it was causing the iterator to move on to the next
bucket every time instead of only when the current LLIterator gets to the end.
