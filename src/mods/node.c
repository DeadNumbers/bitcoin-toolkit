#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <errno.h>
#include "node.h"
#include "message.h"
#include "mem.h"
#include "assert.h"

#define MAX_MESSAGE_QUEUE 100

struct Node {
	int sockfd;
	Message mqueue[MAX_MESSAGE_QUEUE];
};

/*
 * Static Function Declarations
 */
static Node node_connect(const char *, int);
static void node_disconnect(Node);
static void node_write(Node, unsigned char *, size_t);
static int node_read(Node, unsigned char**);
static void node_free(Node);
static int node_read_messages(Node);

/*
 * External functions
 */
Node node_new(const char *host, int port) {
	int i;
	Node n;

	assert(host);
	assert(port);

	n = node_connect(host, port);
	
	if (n) {
		for (i = 0; i < MAX_MESSAGE_QUEUE; ++i) {
			n->mqueue[i] = NULL;
		}
	}

	return n;
}

void node_destroy(Node n) {
	assert(n);

	node_disconnect(n);
	node_free(n);
}

void node_write_message(Node n, Message m) {
	int r;
	unsigned char *s;
	size_t l;

	assert(n);
	assert(m);

	s = ALLOC(message_sizeof());

	r = message_serialize(s, &l, m);
	if (r < 0)
	{
		// return negative value
	}

	node_write(n, s, l);

	FREE(s);
}

Message node_get_message(Node n, char *command) {
	int i, c;
	Message m = NULL;

	c = node_read_messages(n);

	if (c < 0)
	{
		perror("Host Read Error");
		return NULL;
	}

	// Check if desired message is in the queue
	for (i = 0; i < MAX_MESSAGE_QUEUE; ++i) {
		if (n->mqueue[i] == NULL) {
			break;
		}
		if (message_cmp_command(n->mqueue[i], command) == 0) {
			m = n->mqueue[i];
			break;
		}
	}

	// Bump the position of the remaining messages in the queue
	while (++i < MAX_MESSAGE_QUEUE) {
		if (n->mqueue[i]) {
			n->mqueue[i-1] = n->mqueue[i];
			n->mqueue[i] = NULL;
		} else {
			break;
		}
	}

	return m;
}

int node_socket(Node n) {
	assert(n);
	
	return n->sockfd;
}

/*
 * Static functions
 */
static Node node_connect(const char *host, int port) {
	int t, sockfd = 0;
	struct hostent *server;
	struct sockaddr_in serv_addr;
	Node r;
	
	assert(host);
	assert(port);
	
	// Set up a socket in the AF_INET domain (Internet Protocol v4 addresses)
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd >= 0);
	
	// Get a pointer to 'hostent' containing info about host.
	server = gethostbyname(host);
	if (!server)
	{
		return NULL;
	}
	
	// Initializing serv_addr memory footprint to all integer zeros ('\0')
	memset(&serv_addr, 0, sizeof(serv_addr));
	
	// Setting up our serv_addr structure
	serv_addr.sin_family = AF_INET;       // Internet Protocol v4 addresses
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	serv_addr.sin_port = htons(port);     // Convert port byte order to 'network byte order'
	
	// Connect to server. Error if can't connect.
	t = connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
	if (t < 0)
	{
		return NULL;
	}
	
	NEW(r);

	r->sockfd = sockfd;
	
	return r;
}

static void node_disconnect(Node n) {
	assert(n);
	assert(n->sockfd);
	
	close(n->sockfd);
}

static void node_write(Node n, unsigned char *data, size_t l) {
	ssize_t r;

	assert(n);
	assert(n->sockfd);
	assert(data);
	assert(l);
	
	r = write(n->sockfd, data, l);

	if (r < 0) {
		// We don't necessarily need to kill the program on a write error.
		// For now just display an error message and continue.
		perror("Node Write Error");
	} else {
		assert(r > 0);
	}
}

static int node_read(Node n, unsigned char** buffer) {
	int r;
	int l = 0;
	
	assert(n);
	assert(buffer);

	// Get length of data waiting to be read
	ioctl(n->sockfd, FIONREAD, &r);

	if (r > 0) {

		// Allocate buffer if null
		if (*buffer == NULL) {
			*buffer = ALLOC(r + 1);
		}

		// read incoming data in to buffer
		l = read(n->sockfd, *buffer, r);
		
		return l;
	}
	
	return 0;
}

static void node_free(Node n) {
	FREE(n);
}

static int node_read_messages(Node n) {
	Message m;
	unsigned char *s = NULL;
	int l, i, j, r, c;
	
	assert(n);

	j = 0;
	c = 0;

	l = node_read(n, &s);

	// If read() returns an error, pass it up the call stack
	if (l < 0)
	{
		return l;
	}

	while (l > 0) {
		m = ALLOC(message_sizeof());
		r = message_deserialize(m, s + j, (size_t)l);
		if (r < 0)
		{
			// return negative value here
		}
		j += r;
		l -= j;

		r = message_is_valid(m);
		if (r < 0)
		{
			// return a negative value here
		}

		if (r) {
			for (i = 0; i < MAX_MESSAGE_QUEUE; ++i) {
				if (n->mqueue[i] == NULL) {
					break;
				}
			}
			assert(i < MAX_MESSAGE_QUEUE);
			n->mqueue[i] = m;
			
			++c;
		} else {
			FREE(m);
		}
	}
	return c;
}
