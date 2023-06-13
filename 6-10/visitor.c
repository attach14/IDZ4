#include "data.h"

int sock;                         /* Socket */

void my_handler(int nsig)
{
    close(sock);
    exit(0);
}

int main(int argc, char* argv[]) {
    struct sockaddr_in multicastAddr; /* Multicast Address */
    char *multicastIP;                /* IP Multicast Address */
    unsigned short multicastPort;     /* Port */
    int recvMsgSize;
    struct ip_mreq multicastRequest;  /* Multicast address join structure */

     if (argc != 3)    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage: %s <Multicast IP> <Multicast Port>\n", argv[0]);
        exit(1);
    }

    multicastIP = argv[1];        /* First arg: Multicast IP address (dotted quad) */
    multicastPort = atoi(argv[2]);/* Second arg: Multicast port */

    /* Create a best-effort datagram socket using UDP */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket() failed");
        return 0;
    }
    (void)signal(SIGINT, my_handler);
    /* Construct bind structure */
    memset(&multicastAddr, 0, sizeof(multicastAddr));   /* Zero out structure */
    multicastAddr.sin_family = AF_INET;                 /* Internet address family */
    multicastAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
    multicastAddr.sin_port = htons(multicastPort);      /* Multicast port */

    /* Bind to the multicast port */
    if (bind(sock, (struct sockaddr *) &multicastAddr, sizeof(multicastAddr)) < 0) {
        perror("bind() failed");
        return 0;
    }

    /* Specify the multicast group */
    multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastIP);
    /* Accept multicast from any interface */
    multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
    /* Join the multicast address */
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &multicastRequest,
          sizeof(multicastRequest)) < 0) {
            perror("setsockopt() failed");
            return 0;
        }
    double res[2];
    /* Receive a single datagram from the server */
    for(;;) {
        if ((recvMsgSize = recvfrom(sock, res, sizeof(res), 0, NULL, 0)) < 0) {
            perror("recvfrom() failed");
            return 0;
        }
        if (res[0] < 0) {
            printf("Server: The End\n");
            close(sock);
            return 0;
        }
        if (res[0] > 0) {
            printf("Accountant finished calculation: area = %lf\n", res[1]);
            continue;
        }
        printf("Accountant got new task\n");
    }
    exit(0);
}