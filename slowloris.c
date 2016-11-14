#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <unistd.h>
#include <pthread.h>

const char *err_usage = "Usage: ./slowloris <# threads> <target ipv4 addr>";

struct arg_bundle{
	char *target_addr;
	unsigned int target_port;
	int thread_num;
};

/*******************************************************************************
 * Input Functions
 ******************************************************************************/

/*******************************************************************************
 * Network Functions
 ******************************************************************************/

/*******************************************************************************
 * Thread Functions
 ******************************************************************************/

void *loris(void *_arg){
	struct arg_bundle *b = (struct arg_bundle*)_arg;
	int sockfd;
    struct sockaddr_in t_addr;
	struct hostent *server;

	printf("Thread %i started.\n", b->thread_num);

	// --- Init connection 

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		perror("Error opening socket");
		return (void*)NULL;
	}

	server = gethostbyname(b->target_addr);
    if (server == NULL) {
		perror("Error resolving host");
		return (void*)NULL;
    }

	memset(&t_addr, '0', sizeof(t_addr));
	t_addr.sin_family = AF_INET;
	t_addr.sin_port = htons(b->target_port);
	bcopy((char *)server->h_addr_list[0], 
		(char *)&t_addr.sin_addr.s_addr,
		server->h_length);

	if(connect(sockfd, (struct sockaddr*)&t_addr, sizeof(t_addr)) < 0){
		perror("Error connecting to target");
		return (void*)NULL;
	}

	printf("Thread %i initialized.\n", b->thread_num);

	while(1){
		break;
	}

	close(sockfd);

	return (void*)NULL;
}

/*******************************************************************************
 * Main
 ******************************************************************************/

int main(int argc, char *argv[]){
	struct arg_bundle *arg_bundles;
	unsigned int num_threads;
	pthread_t *threads;

	// --- Validate and parse arguments 

	if(argc != 3 || argv[1] == NULL || argv[2] == NULL){
		perror(err_usage);
		return EXIT_FAILURE;
	}

	num_threads = atoi(argv[1]);
	if(num_threads <= 0){
		perror(err_usage);
		return EXIT_FAILURE;
	}

	threads = malloc(sizeof(pthread_t) * num_threads);

	arg_bundles = malloc(sizeof(struct arg_bundle) * num_threads);

	// --- Start

	for(int i = 0; i < num_threads; i++){
		arg_bundles[i].target_addr = argv[2];
		arg_bundles[i].target_port = 80;
		arg_bundles[i].thread_num = i + 1;

		pthread_create(&threads[i], NULL, loris, (void*)&arg_bundles[i]);
	}

	// --- End

	for(int i = 0; i < num_threads; i++){
		pthread_join(threads[i], NULL);
	}

	return 0;
}