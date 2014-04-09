/*
 * helpers.c
 *
 *  Created on: Feb 13, 2014
 *      Author: jrm
 *
 *  Slightly modified from code in:
 *     Unix Systems Programming: Communication, Concurrency, and Threads
 *     By Kay A. Robbins , Steven Robbins
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "restart.h"
#include "helpers.h"
#include <sys/stat.h>


/**
 * Splits a string into an array of arguments
 * @see
 */
int h_mk_argv(const char *s, const char *delimiters, char ***argvp) {
	int error;
	int i;
	int numtokens;
	const char *snew;
	char *saveptr;
	char *saveptr2;
	char *t;
	if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
		errno = EINVAL;
		return -1;
	}
	*argvp = NULL;
	snew = s + strspn(s, delimiters);
	/* snew is real start of string */
	if ((t = malloc(strlen(snew) + 1)) == NULL)
		return -1;
	strcpy(t, snew);
	numtokens = 0;
	if (strtok_r(t, delimiters,&saveptr) != NULL)
		/* count the number of tokens in s */
		for (numtokens = 1; strtok_r(NULL, delimiters,&saveptr) != NULL; numtokens++) ;
	/* create argument array for ptrs to the tokens */
	if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL) {
		error = errno;
		free(t);
		errno = error;
		return -1;
	}
	/* insert pointers to tokens into the argument array */
	if (numtokens == 0)free(t);
	else {
		strcpy(t, snew);
		**argvp = strtok_r(t, delimiters,&saveptr2);
		for (i = 1; i < numtokens; i++)
			*((*argvp) + i) = strtok_r(NULL, delimiters,&saveptr2);
	}
	*((*argvp) + numtokens) = NULL;
	/* put in final NULL pointer */
	return numtokens;
}



/**
 * Creates a child process to execute a command.
 * @return pid_t of the process
 */
pid_t h_run_cmd(const char *cmd,const bool wait) {
	pid_t childpid;
	char **args;

	// If invalid args, return invalid
	if (cmd == NULL) {
		errno = EINVAL;
		return -1;
	}

	// Parse the cmd into args
	h_mk_argv(cmd," \t\n",&args);

	// Spawn the new process
	childpid = fork();
	if (childpid == -1) {
		perror("Failed to fork");
		return childpid;
	}

	// If child process, execute the cmd
	if (childpid == 0) {
		execvp(args[0], &args[0]);
		perror("Child failed to execvp the command");
		exit(-1);
	}

	// Free args
	if (args == NULL) {
		if (*args != NULL) {
			free(*args);
		}
		free(args);
	}

	// Wait for it to finish (if requested)
	if (wait) {
		if (childpid != r_wait(NULL)) {
			perror("Failed to wait");
		}
	}
	return childpid;
}

int h_len(const char** array) {
  int count = 0;
  while(array[count]) count++;
  return count;
}

off_t h_fsize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}

int h_fexists(const char *filename) {
  struct stat st;
  return (stat (filename, &st) == 0);
}

char *h_fmimetype(const char *filename) {
	int i=0;
	char *f;
	char *resp;
	asprintf(&f,"%s",filename);
	for(i = 0; f[i]; i++){
	  f[i] = tolower(f[i]);
	}
	// Should us a hashmap, but whatever
	if (strstr(f,".html")!=NULL||strstr(f,".php")!=NULL||strstr(f,".htm")!=NULL) {
		resp = "text/html";
	} else if (strstr(f,".js")!=NULL) {
		resp = "text/javascript";
	} else if (strstr(f,".css")!=NULL) {
		resp = "text/css";
	} else if (strstr(f,".png")!=NULL||strstr(f,".x-png")!=NULL) {
		resp = "image/png";
	} else if (strstr(f,".jpg")!=NULL||strstr(f,".jpeg")!=NULL||strstr(f,".jpe")!=NULL) {
		resp = "image/jpeg";
	} else if (strstr(f,".txt")!=NULL) {
		resp = "text/plain";
	} else {
		resp = "application/octet-stream";
	}
	free(f);
	return resp;
}

/**
 * If the assert fails, print an error message.

void assertf(int expr,const char *fmt,...) {
	if (!expr) {
		va_list args;
		va_start(args,fmt);
		vprintf(fmt,args);
		if (errno) {perror("\n");}
		va_end(args);
	}
	assert(expr);
}
*/

/**
 * Returns TRUE if the string is in the given array.
 */
//bool h_str_in(const char *s, const char *a[]) {
//	int l = h_len(a);
//	int i=0;
//	for (i=0;i<l;i++) {
//		if (strcmp(s,a[i])==0) {
//			return TRUE;
//		}
//	}
//	return FALSE;
//}

///**
// * print to a string by appending the two
// */
//char *h_sprintf(char *str, char *format,...) {
//	va_list args;
//	va_start(args, format);
//	char *buf;
//	vsprintf(buf,format, args);
//	va_end(args);
//	return h_strcat(str,buf);
//}
//
///**
// * Append two strings
// */
//char *h_strcat(char *str1, char *str2) {
//	char * new_str ;
//	if((new_str = malloc(strlen(str1)+strlen(str2)+1)) != NULL){
//		new_str[0] = '\0';   // ensures the memory is an empty string
//		strcat(new_str,str1);
//		strcat(new_str,str2);
//		return new_str;
//	}
//	return NULL;
//}
#if defined(__SVR4) && defined(__sun)
int vasprintf(char **ret, const char *format, va_list args) {
	va_list copy;
	va_copy(copy, args);

	/* Make sure it is determinate, despite manuals indicating otherwise */
	*ret = 0;

	int count = vsnprintf(NULL, 0, format, args);
	if (count >= 0) {
		char* buffer = malloc(count + 1);
		if (buffer != NULL) {
			count = vsnprintf(buffer, count + 1, format, copy);
			if (count < 0)
				free(buffer);
			else
				*ret = buffer;
		}
	}
	va_end(args);  // Each va_start() or va_copy() needs a va_end()

	return count;
}

int asprintf(char **strp, const char *fmt, ...) {
	int size;
	va_list args;
	va_start(args, fmt);
	size = vasprintf(strp, fmt, args);
	va_end(args);
	return size;
}

#define _GETLINE_BUFLEN 255

ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
	int c;
	size_t alloced = 0;
	char *linebuf;

	if (*lineptr == NULL) {
		linebuf = malloc(sizeof(char) * (_GETLINE_BUFLEN + 1));
		alloced = _GETLINE_BUFLEN + 1;
	} else {
		linebuf = *lineptr;
		alloced = *n;
	}
	ssize_t linelen = 0;

	do {
		c = fgetc(stream);
		if (c == EOF) {
			break;
		}
		if (linelen >= alloced) {
			linebuf = realloc(linebuf, sizeof(char) * (alloced + _GETLINE_BUFLEN + 1));
			alloced += (_GETLINE_BUFLEN + 1);
		}
		*(linebuf + linelen) = (unsigned char)c;
		linelen++;
	} while (c != '\n');

	/* empty line means EOF or some other error */
	if (linelen == 0) {
		if (linebuf != NULL && *lineptr == NULL) {
			free(linebuf);
			linebuf = NULL;
		}
		linelen = -1;
		*n = alloced;
	} else {
		if (linebuf != NULL) {
			linebuf[linelen] = '\0';
		}
		*n = alloced;
		*lineptr = linebuf;
	}

	return linelen;
}
#endif
