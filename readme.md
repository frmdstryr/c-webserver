>
> readme.md
>
>  Created on: April 9, 2014
>      Author: jairus martin
>

## Description of program (what it does) ##

- A simple webserver using threads to handle requests.

## How to compile ##

make

##  How to run ##

./webserver <port>


Files to be served must be put in:
getcwd()/public/

## Design overview ##

When the server is started it creates a thread pool allowing 50
simultaneous connections.  Then the server binds to the port given
and sit's in a loop waiting to accept connections.

When a new connection is made, the server creates a client object 
with the socket and puts a task containing the client and handler function 
(ie client_main) in the thread pool's queue. The queue is protected
by a mutex so only one thread can access it at a time. The thread pool has
a background thread running that waits for tasks in the queue, and if 
one is ready, it waits for a thread to be available (ie threads_available 
semaphore >0), and then it creates a thread to run the task. Each thread 
is spawned using a kernel level thread so they can be truely asyncronous. 
The thread pool's scheduler thread also checks if each thread is still
alive or not and join's any dead threads making another thread available
be used by the pool.

The client_main function reads the request from the socket and
serves the requested file, then closes the socket and frees memory
used.

## How you may have handled any ambiguities in the specification ##



## Any known bugs or problems ##

- There's a memory leak somewhere, every request uses ~24kb...
