/*
  Duc Vu
  Proj 1 - CPSC 4510 Networking
  Prof: Dr.Zhu
*/

#include "Declaration.h"



/*
    Establish the connection with client in specified port.
    Return -1 upon failure.
    Return the socket with specified port number
*/
int setConnection_w_client(int &port)
{
  int serverfd, newSocket, opt = 1;
  struct sockaddr_in address;
  socklen_t addrlen = sizeof(address);
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  if ((serverfd = socket(PF_INET, SOCK_STREAM, 0)) == 0)
  {
    return -1;
  }
  if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
  {
    return -1;
  }
  if (bind(serverfd, (struct sockaddr *)&address, addrlen) < 0)
  {
    return -1;
  }
  if (listen(serverfd, 30) < 0)
  {
    return -1;
  }
  if ((newSocket = accept(serverfd, (struct sockaddr *)&address, &addrlen)) == -1)
  {
    return -1;
  }
  close(serverfd);
  return newSocket;
}

/*
    Establish connection with remote server.
    Return sockfd if connection succesfully set up with remote server
    Return -1 otherwise 
*/
int setConnection_w_remote_server(std::string host, std::string port)
{
  struct addrinfo hints, *addrs;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  memset(&hints, 0, sizeof hints);
  const int status = getaddrinfo(host.c_str(), port.c_str(), &hints, &addrs);
  if (status != 0)
    return -1;
  int sfd = -1;
  for (struct addrinfo *addr = addrs; addr != NULL; addr = addr->ai_next)
  {
    sfd = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
    if (sfd == -1)
      continue;
    if (connect(sfd, addr->ai_addr, addr->ai_addrlen) == 0)
      break; // succesfully connect to the remote server.

    // Connect failed.
    close(sfd);
  }
  freeaddrinfo(addrs);
  return sfd;
}

/*
    writing request to the socket. 
    Return FALSE if write failed, True otherwise
*/
bool writetoSock(int sock, std::string request)
{
  int bytes_sent = 0;
  while (bytes_sent < (int)request.length())
  {
    int x = write(sock, request.c_str(), request.length() - bytes_sent);
    if (x == -1)
    {
      return false;
    }
    request = request.substr(x);
    bytes_sent += x;
  }
  return true;
}

/*
    Receive data from socket, write it to the rcv string. 
    Proxy only start parsing and sending request when User hits enter. 
    Return false if Read failed, True otherwise
*/
bool readFromSock(int sock, std::string &rcv, const bool &read_from_client)
{
  char *buffer = new char[MAX_BUF_LENGTH];
  int nDataLength = 0;
  while ((nDataLength = recv(sock, buffer, MAX_BUF_LENGTH, 0)) > 0)
  {
    rcv.append(buffer, nDataLength);
    if (read_from_client == 1 && rcv.substr(rcv.length() - 4) == "\r\n\r\n")
    {
      // Check if the Telnet have completed transfer data to proxy server.
      break;
    }
  }
  delete buffer;
  if (nDataLength < 0)
    return false;

  return true;
}

bool parse_browser_request(std::string& req, Header&h){
  const char *req_cform = req.c_str();
  int rc;
  regex_t preg;
  size_t nmatch = 7;
  char buffer [1024];
  regmatch_t pmatch[7];
  if (0 != (rc = regcomp(&preg, pattern,REG_ICASE|REG_EXTENDED ))){
    regerror(rc, &preg, buffer, 1024);
    // fprintf(stderr,"grep: %s (%s)\n", buffer, pattern); 
    // printf("regcomp() failed, returning nonzero (%d)\n", rc);
    return false;
  }
  if (0 != (rc = regexec(&preg, req_cform, nmatch, pmatch, 0)))
  {
    // printf("Failed to match '%s' with '%s',returning %d.\n",req_cform, pattern, rc);
    return false;
  }
  h.method = std::string(&req_cform[pmatch[1].rm_so], &req_cform[pmatch[1].rm_eo]);
  h.host_name = std::string(&req_cform[pmatch[3].rm_so], &req_cform[pmatch[3].rm_eo]);
  h.port = "80";
  h.file_path ='/'+ std::string(&req_cform[pmatch[4].rm_so], &req_cform[pmatch[4].rm_eo]);
  h.http_version = std::string(&req_cform[pmatch[5].rm_so], &req_cform[pmatch[5].rm_eo]);
  
  return true;
}

/*
    Making request to the remote server based on user's request. 
*/
std::string make_request_with_header(const Header &h)
{
  std::string url_header = h.method + " " + h.file_path + " " + h.http_version + "\r\n" 
                          + "Host: "  + h.host_name + "\r\n" 
                          + "Connection: close\r\n"
                           // Could add more headers here...
                           + "\r\n";
  return url_header;
}

bool handle_client(int sock_w_client)
{
  std::string rcv = "";
  Header h;
  bool isClient = true;
  if (!readFromSock(sock_w_client, rcv, isClient))
  {
    writetoSock(sock_w_client, INTERNAL_ERR);
    close(sock_w_client);
    return false;
  }

  if (!parse_browser_request(rcv, h))
  {
    writetoSock(sock_w_client, INTERNAL_ERR);
    close(sock_w_client);
    return false;
  }

  // Connecting to the remote server.
  // Only start connectino with the remote if Client's request is valid.
  int sock_w_server = setConnection_w_remote_server(h.host_name, h.port);
  if (sock_w_server == -1)
  {
    writetoSock(sock_w_client, INTERNAL_ERR);
    close(sock_w_server);
    close(sock_w_client);
    return false;
  }
  if (!writetoSock(sock_w_server, make_request_with_header(h)))
  {
    writetoSock(sock_w_client, INTERNAL_ERR);
    close(sock_w_server);
    close(sock_w_client);
    return false;
  }
  rcv = "";
  if (!readFromSock(sock_w_server, rcv, !isClient))
  {
    writetoSock(sock_w_client, INTERNAL_ERR);
    close(sock_w_server);
    close(sock_w_client);
    return false;
  }
  if (!writetoSock(sock_w_client, rcv))
  {
    writetoSock(sock_w_client, INTERNAL_ERR);
    close(sock_w_server);
    close(sock_w_client);
    return false;
  }
  close(sock_w_server);
  close(sock_w_client);
  return true;
}

void *add_sock_queue(void *prams)
{
  int *port = (int *)prams;
  while (true)
  {
    int socket = setConnection_w_client(*port);
    if (socket == -1)
      continue;
    // When it goes to here it means the connection to client has been established.
    pthread_mutex_lock(&sock_queue_lock);
    sock_queue.push(socket);
    pthread_mutex_unlock(&sock_queue_lock);
    // Signal the worker thread.
    pthread_cond_signal(&sock_queue_cond);
  }
  delete port;
  return NULL;
}

void *worker_thread(void *)
{
  while (true)
  {
    pthread_mutex_lock(&sock_queue_lock);
    /* Critical section */
    /* go here mean that there is at least 1 client waiting. */
    /* go grab the front element of the queue and pop it.*/
    while (sock_queue.empty())
    /* 
    pthread_cond_wait unlocks the mutex just before it sleeps 
    but then it reaquires the mutex (which may require waiting) when it is signalled, before it wakes up. 
    So if the signalling thread holds the mutex (the usual case), the waiting thread will not proceed until 
    the signalling thread also unlocks the mutex.
    */pthread_cond_wait(&sock_queue_cond, &sock_queue_lock);
       

    int socket = sock_queue.front();
    sock_queue.pop();

    /* End of crtical section */
    pthread_mutex_unlock(&sock_queue_lock);
    if (!handle_client(socket))
      continue;
  }
  return NULL;
}

int main(int argc, char const *argv[])
{
  pthread_t threads[NUM_THREAD];
  if (argc < 2)
  {
    std::cout << "Usage ./proxy port#\n";
    return -1;
  }
  Header h;

  int port = atoi(argv[1]);
  void *p = (void *)&port;
  // Reserver thread 0 to listen from client's socket.
  pthread_create(&threads[0], NULL, add_sock_queue, p);
  for (int i = 1; i < NUM_THREAD; ++i)
    pthread_create(&threads[i], NULL, worker_thread, NULL);
  for (int i = 0; i < NUM_THREAD; ++i)
  {
    pthread_join(threads[i], NULL);
  }

  pthread_mutex_destroy(&sock_queue_lock);
  pthread_cond_destroy(&sock_queue_cond);
  return 0;
}
