#ifndef snapCterm_SNet
#define snapCterm_SNet

extern char io_initialized;
extern int sockfd, bytes, pfd;
extern struct sockaddr_in remoteaddr;
extern struct hostent *he;
extern char host_name[];

void CommsInit(void);
void RX(void);
void TX(void);

#endif