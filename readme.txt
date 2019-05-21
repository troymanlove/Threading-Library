This has the following functions created and functioning:
thread_create, thread_yield, thread_exit

Commented out is the start of a mutex inclusion.
This will probably be done by the end of this weekend but is not yet completed. 

This threading uses a doubly linked list that functions as sort of a round robin circular queue for storing the structs of threads.
