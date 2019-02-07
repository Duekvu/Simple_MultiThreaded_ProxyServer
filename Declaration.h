/*
  Duc Vu
  Proj 1 - CPSC 4510 Networking
  Prof: Dr.Zhu
*/

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include <pthread.h>
#include <queue>
#include <semaphore.h>
#include <regex.h>
#include <stdio.h>
#include <semaphore.h>

const int MAX_BUF_LENGTH = 4096;
const int  NUM_THREAD = 30;
const char *pattern = "(GET) (http://)([a-zA-z.]*)+/(.*) (HTTP/1.0)";
const std:: string INTERNAL_ERR= "500 Internal Error\n";
std::queue<int> sock_queue;

pthread_mutex_t sock_queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sock_queue_cond = PTHREAD_COND_INITIALIZER;


struct Header
{
  std::string method;
  std::string host_name;
  std::string file_path;
  std::string port;
  std::string http_version;
};

int setConnection_w_client(int &port);
int setConnection_w_remote_server(std::string host, std::string port);
bool writetoSock(int sock, std::string request);
bool readFromSock(int sock, std::string &rcv, const bool &read_from_client);
bool parse_browser_request(std::string& req,Header&h);
std::string make_request_with_header(const Header &h);
bool handle_client(int sock_w_client);
void *add_sock_queue(void *prams);
void *worker_thread(void *);


