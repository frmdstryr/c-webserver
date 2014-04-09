/*
 * webserver.c
 *
 *  Created on: Mar 24, 2014
 *      Author: jrm
 */
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <pthread.h>
#include "restart.h"
#include "helpers.h"
#include "dbg.h"
#include "array.h"
#include "list.h"
#include "sockets.h"
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

static bool LISTENING = TRUE;
static char *SERVER_ROOT = "Debug/public";
/**
 * An interface to communicate via pipes
 */
typedef struct Client {
	int socket;
	char *host;
	char *root;
} Client;

Client *new_client(char *host,int socket);
int client_main(Client *self);
bool client_destroy(Client *self);
void client_printf(Client *self,const char *fmt,...);
int client_sendline(Client *self,const char *fmt,...);

Client *new_client(char *host, int socket) {
	Client *client = malloc(sizeof(Client));
	check(client != NULL,"Failed to create client!");

	client->socket = socket;
	client->host = host;

	// Return object client to parent;
	return client;
	error:
		return NULL;
}

/**
 * Send a line to the child, if used in the child thread
 * it sends a line to the parent.
 */
int client_sendline(Client *self,const char *fmt,...) {
	char *msg;
	va_list args;
	va_start(args,fmt);
	vasprintf(&msg,fmt,args);
	int nbytes = r_write(self->socket,msg,(size_t)strlen(msg));
	client_printf(self,"Wrote '%s'\n",msg);
	va_end(args);
	free(msg);
	return nbytes;
}

/**
 * Send a line to the child, if used in the child thread
 * it sends a line to the parent.
 */
int client_sendfile(Client *self,const char *filename) {
	int fd=-1;
	check((fd=r_open2(filename,O_RDONLY))>2,"Couldn't open %s!",filename);
	int nbytes = r_copyfile(fd,self->socket);
	check(r_close(fd)!=-1,"Couldn't close %s!",filename);
	return nbytes;
	error:
		return -1;
}

/**
 * Read a line from the child, if used in the child thread
 * it reads a line from the parent.
 */
int client_readline(Client *self,char *buf,int nbytes) {
	return r_readline(self->socket,buf,nbytes);
}

/**
 * Client thread function. Handle's the client's request.
 *
 * There are three parts to the response message: the status line, the response headers, and the entity
 * body. The status line and response headers are terminated by the character sequence CRLF. We
 * are going to respond with a status line, and a single response header. In the case of a request for
 * a nonexistent file, we return 404 Not Found in the status line of the response message, and
 * include an error message in the form of an HTML document in the entity body.
 *
 * Now we can send the status line and our single header line to the browser by writing into the
 * socket's output stream. Now that the status line and header line with delimiting CRLF have been
 * placed into the output stream on their way to the browser, it is time to do the same with the entity
 * body. If the requested file exists, we send the file. If the requested file does not exist, we send the
 * HTML-encoded error message that we have prepared.
 *
 * Now we can send the status line and our single header line to the browser by writing into the
 * socket's output stream. Now that the status line and header line with delimiting CRLF have been
 * placed into the output stream on their way to the browser, it is time to do the same with the entity
 * body. If the requested file exists, we send the file. If the requested file does not exist, we send the
 * HTML-encoded error message that we have prepared.
 *
 * After sending the entity body, the work in this thread has finished, so we close the streams and
 * socket before terminating.
 */
int client_main(Client *self) {
	int status = 500;
	char *filename = "/index.html";
	char *path;
	char *content_type;// = "text/html";
	char **parts;
	long content_length = 0;
	client_printf(self,"Connected!\n");
	// TODO: Parse request
	int nbytes = 4096;
	char buf[nbytes];
	while (client_readline(self,buf,nbytes)>0) {
		client_printf(self,"Read: %s",buf);
		if (strstr(buf,"GET")!=NULL && strstr(buf,"HTTP")!=NULL) {
			h_mk_argv(buf," ",&parts);
			if (strcmp(parts[1],"/")!=0) { // serve index.html
				filename = parts[1];
			}
			free(parts);
		} else if (strcmp(buf,"\r\n")==0 || strcmp(buf,"")==0|| strcmp(buf,"\r")==0|| strcmp(buf,"\n")==0) {
			break;
		}
	}

	// make the file path
	asprintf(&path,"%s%s",self->root,filename);
	client_printf(self,"Serving %s\n",path);

	if (!h_fexists(path)) {
		status = 404;
		content_length = 27;
		content_type = "text/html";
		client_sendline(self,"HTTP/1.1 %i File not found\r\nContent-Length: %i\r\nContent-Type: %s\r\n\r\n<h1>404 File not found</h1>",status,content_length,content_type);
	} else {
		status = 200;
		content_length = h_fsize(path);
		content_type = h_fmimetype(filename);
		client_sendline(self,"HTTP/1.1 %i OK\r\nContent-Length: %i\r\nContent-Type: %s\r\n\r\n",status,content_length,content_type);
		check(client_sendfile(self,path)==content_length,"Error writing to client");
	}
	free(path);

	client_printf(self,"Status=%i\n",status);

	// Close and quit
	check(r_close(self->socket)==0,"[Client %d]: Failed to close socket\n",(int)pthread_self());
	check(client_destroy(self),"[Client %d]: Failed to cleanup client\n",(int)pthread_self());
	return EXIT_SUCCESS;

	error:
		return -1;
}

void client_printf(Client *client,const char *fmt,...) {
	printf("[Client %d] ",(int)pthread_self());
	va_list args;
	va_start(args,fmt);
	vprintf(fmt,args);
	va_end(args);
	fflush(stdout);
}

bool client_destroy(Client *self) {
	if(self!=NULL) {
		if (self->root !=NULL) {
			free(self->root);
		}
		free(self);
		return TRUE;
	}
	return FALSE;
}

typedef struct ThreadPool {
	pthread_mutex_t lock; // lock for the queue
	sem_t tasks_waiting;
	List *queue;
	sem_t threads_available;
	Array *threads;
} ThreadPool;

ThreadPool *new_thread_pool(int threads);
int thread_pool_scheduler(ThreadPool *self);
int thread_pool_wait(ThreadPool *self);

ThreadPool *new_thread_pool(int threads) {
	ThreadPool *self = malloc(sizeof(ThreadPool));
	int error;
	pthread_t tid;
	pthread_attr_t tattr;
	assertf(self != NULL,"[ThreadPool] Failed to create thread pool.");

	// Create the mutex's to access the queue
	assertf(pthread_mutex_init(&self->lock, NULL)==0,"[ThreadPool] Failed to init lock");

	assertf(sem_init(&self->tasks_waiting, 0, 0)==0,"[ThreadPool] Failed to init task semaphore");
	assertf(sem_init(&self->threads_available, 0, threads-1)==0,"[ThreadPool] Failed to init thread semaphore");

	// Create the queue
	self->queue = new_list();

	// Create an array to hold the active threads
	self->threads = new_array(sizeof(pthread_t),threads-1); // -1 because the scheduler uses one

	// Start the scheduler
	array_set(self->threads,0,tid);
	assertf(pthread_attr_init(&tattr)==0,"[ThreadPool] Failed to create an attribute object");
	assertf(pthread_attr_setscope(&tattr, PTHREAD_SCOPE_SYSTEM)==0,"[ThreadPool] Failed to set scope to system");
	assertf(pthread_create(&tid, &tattr, thread_pool_scheduler, self)==0,"[ThreadPool] Failed ot create scheduler thread");
	return self;

}

/**
 * Scheduler loop that checks for tasks in the queue
 * and creates threads to handle them;
 */
int thread_pool_scheduler(ThreadPool *self) {
	log_info("[ThreadPool] Scheduler started!");
	int i=0;
	int j=0;
	while (sem_wait(&self->tasks_waiting)==0) { // Block until a task is ready

		// Get a task off the queue
		pthread_mutex_lock(&self->lock);
		List *task = (List *) list_shift(self->queue);
		pthread_mutex_unlock(&self->lock);

		log_debug("[ThreadPool] Got a task!");

		// If the task is empty, quit
		if (task==NULL) {
			break;
		}

		// Wait for a thread to be available
		check(sem_wait(&self->threads_available)==0,"[ThreadPool] Scheduler error waiting for thread available");

		// Create a thread to handle the task
		pthread_t tid;
		pthread_attr_t tattr;
		void *arg = list_pop(task); // Pop arg off the list
		void *(*handler)(void *) = list_pop(task); // Pop handler off the list
		list_clear_destroy(task);
		log_debug("[ThreadPool] Creating handler thread!");
		if (pthread_attr_init(&tattr)!=0){
			log_error("[ThreadPool] Failed to create an attribute object");
			// else failed to create it so make the thread still available again
			sem_post(&self->threads_available);
		} else if (pthread_attr_setscope(&tattr, PTHREAD_SCOPE_SYSTEM)!=0){
			log_error("[ThreadPool] Failed to set scope to system");
			// else failed to create it so make the thread still available again
			sem_post(&self->threads_available);
		} else if (pthread_create(&tid, &tattr, handler, arg)!=0) {
			log_error("[ThreadPool] Failed to create thread!");
			// else failed to create it so make the thread still available again
			sem_post(&self->threads_available);
		} else {
			// eveything is ok!
			array_push(self->threads,tid);
		}

		/// Done: Cleanup any finished threads
		j = array_count(self->threads);
		for (i=1;i<j;i++) {
			tid = array_get(self->threads,i);
			if (!(pthread_kill(tid,0)==0)) { // only if Thread is dead
				pthread_join(tid,NULL); // Clean up
				array_swap(self->threads,i,j); // Push to back
				array_pop(self->threads); // Remove from array
				sem_post(&self->threads_available); // tell the thread pool another thread is available
				log_debug("[ThreadPool] Thread %d finished!",tid);
				j--;
			}
		}
	}

	log_info("[ThreadPool] Scheduler stopped!");
	return EXIT_SUCCESS;

	error:
		return -1;
}
/**
 * Put the handler in the queue to be handled by a worker thread
 */
bool thread_pool_apply_async(ThreadPool *self,void *(*handler)(void *), void *arg) {
	List *task = new_list(); // cleaned up when handled
	list_push(task,handler);
	list_push(task,arg);

	pthread_mutex_lock(&self->lock);
	list_push(self->queue,task);
	pthread_mutex_unlock(&self->lock);

	// Notify a task is waiting
	if (sem_post(&self->tasks_waiting)!=0) {
		log_error("[ThreadPool] Failed to send task to queue!");
		return FALSE;
	}

	return TRUE;
}

int thread_pool_wait(ThreadPool *self) {
	int i,j;
	pthread_t tid;
	j = array_count(self->threads);
	for (i=0;i<j;i++) {
		tid = array_get(self->threads,i);
		pthread_kill(tid,9);
		pthread_join(tid,NULL);
	}

}

bool thread_pool_destroy(ThreadPool *self) {
	if(self!=NULL) {
		log_debug("[ThreadPool] Cleaning up...");
		pthread_mutex_destroy(&self->lock);
		sem_destroy(&self->tasks_waiting);
		sem_destroy(&self->threads_available);
		free(self);
		log_debug("[ThreadPool] Done!");
		return TRUE;
	}
	return FALSE;
}


typedef struct Server {
	int port;
	int socket;
	bool listening;
	ThreadPool *thread_pool;
} Server;

Server *new_server(int port) {
	Server *self = malloc(sizeof(Server));
	assertf(self != NULL,"[Server] Failed to create server.");
	self->listening = TRUE;
	self->port = port;
	self->thread_pool = new_thread_pool(50);
	return self;
}

/**
 * Main server listen run loop
 */
int server_run(Server *self) {
	int socket;
	char cwd[1024];
	getcwd(cwd,1024);

	// Create socket, bind and start listening...
	assertf((self->socket = u_open(self->port)) != -1,"[Server] Failed to bind to port %i:",self->port);

	log_info("[Server] Server listening on *:%i",self->port);

	while (self->listening) {
		char *host;

		// Wait for a new client to make a request
		if ((socket = u_accept(self->socket, host, 255)) == -1) {
			if (self->listening) {
				log_error("[Server] Failed to accept connection!"); // log_error prints out errno message
			} // else we aborted, this is not an error
			continue;
		}

		// Create a client object to handle the request
		Client *client = new_client(host,socket);
		asprintf(&client->root,"%s/%s",cwd,SERVER_ROOT);

		// Put the client in the thread pool queue
		if (!thread_pool_apply_async(self->thread_pool,client_main,client)) {
			log_error("[Server] Failed to handle client request!");
		}
	}

	// Clean up threads
	thread_pool_wait(self->thread_pool);
	thread_pool_destroy(self->thread_pool);

	log_info("[Server] Stopped!\n");
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
	int port;
	Server *server;
	struct sigaction act;
	struct sigaction act2;

	if (argc!=2) {
		printf("Usage: webserver <port>\n");
		exit(1);
	}

	port = atoi(argv[1]);
	server = new_server(port);

	/**
	 * If the server receives a SIGINT
	 * (ctrl-c) or SIGHUP, it should ignore it.
	 */
	void handle_signals(int signo) {
		log_info("[Server] Server ignoring signal %i!\n",signo);
	}

	/**
	 * If the server receives a SIGTERM or SIGQUIT, it
	 * should gracefully terminate.
	 */
	void handle_signals2(int signo) {
		log_info("[Server] Telling server to shut down!\n");
		server->listening = FALSE;
		r_close(server->socket); // break out of the listen loop
	}

	// Setup signals
	act.sa_handler = handle_signals;
	act.sa_flags = 0;
	if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1) || (sigaction(SIGTSTP, &act, NULL) == -1)) {
		assertf(FALSE,"[Server] Failed to set SIGINT and SIGTSTP handlers");
	}

	act2.sa_handler = handle_signals2;
	act2.sa_flags = 0;
	if ((sigemptyset(&act2.sa_mask) == -1) || (sigaction(SIGTERM, &act2, NULL) == -1) || (sigaction(SIGQUIT, &act2, NULL) == -1)) {
		assertf(FALSE,"[Server] Failed to set SIGTERM and SIGQUIT handlers");
	}

	// Start the server
	return server_run(server);
}
