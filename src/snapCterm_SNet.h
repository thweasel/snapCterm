#ifndef snapCterm_SNet
#define snapCterm_SNet

extern int sockfd, bytes, pfd;
extern struct sockaddr_in remoteaddr;
extern struct hostent *he;
extern char host_name[];

void CommsInit(void);
void RX(void);
void TX(void);

void Draw_Menu(void);
void menu(void);

#endif