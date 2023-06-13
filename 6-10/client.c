#include "data.h"

int sock;
void my_handler(int nsig)
{
    close(sock);
    exit(0);
}

double f(double bad_x, double coeff[], double delta_x, double delta_y) {
    double x = bad_x + delta_x;
    return (coeff[3] * (x * x * x) + coeff[2] * (x * x) + coeff[1] * x +
        coeff[0] + delta_y);
}

double calc(double a, double b, double epsilon, double delta_x, double delta_y,
    double coeff[]) {
    double ans = 0;
    double I1 = ((b - a) / 2) *
        (f(a, coeff, delta_x, delta_y) + f(b, coeff, delta_x, delta_y));
    double m = (a + b) / 2;
    double I2 = ((b - a) / 4) * (f(a, coeff, delta_x, delta_y) +
        2 * f(m, coeff, delta_x, delta_y) +
        f(b, coeff, delta_x, delta_y));
    if (fabs(I1 - I2) <= 3 * (b - a) * epsilon) {
        return I2;
    }
    ans += calc(a, m, epsilon, delta_x, delta_y, coeff);
    ans += calc(m, b, epsilon, delta_x, delta_y, coeff);
    return ans;
}


int main(int argc, char* argv[])
{
    struct sockaddr_in echoServAddr;
    unsigned short echoServPort;
    char* servIP;
    unsigned int echoLen;
    int bytesRcvd, totalBytesRcvd;
    int n;
    int fromAddr, fromSize;
    if (argc != 3) {
        printf("Wrong number of input arguments: %d instead of 2\n", argc - 1);
        return 0;
    }

    servIP = argv[1];                  /* First arg: server IP address (dotted quad) */
    echoServPort = atoi(argv[2]); /* Use given port, if any */ /* Second arg: string to echo */
    (void)signal(SIGINT, my_handler);
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("socket() failed");
        exit(1);
    }
    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
    echoServAddr.sin_port = htons(echoServPort);      /* Server port */
    double numbers[2];
    echoLen = sizeof(numbers);
    numbers[0] = 0; //finish
    numbers[1] = 0;  //ans
    if (sendto(sock, numbers, echoLen, 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) != echoLen)
    {
        perror("sendto() sent a different number of bytes than expected");
        exit(1);
    }
    double res[9];
    echoLen = sizeof(res);
    totalBytesRcvd = 0;
    fromSize = sizeof(fromAddr);
    while (totalBytesRcvd < echoLen)
    {
        if ((bytesRcvd = recvfrom(sock, res, sizeof(res), 0, (struct sockaddr *)&fromAddr, &fromSize)) < 0)
        {
            perror("recvfrom() failed or connection closed prematurely");
            exit(1);
        }
        totalBytesRcvd += bytesRcvd;
    }
    double a = res[0];
    double b = res[1];
    double epsilon = res[2];
    double delta_x = res[3];
    double delta_y = res[4];
    double coeff[4];
    coeff[0] = res[5];
    coeff[1] = res[6];
    coeff[2] = res[7];
    coeff[3] = res[8];
    double ans = calc(a, b, epsilon, delta_x, delta_y, coeff);
    numbers[0] = 1; //finish
    numbers[1] = ans;  //ans
    echoLen = sizeof(numbers);
    sleep(1);
    if (sendto(sock, numbers, echoLen, 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) != echoLen)
    {
        perror("sendto() sent a different number of bytes than expected");
        exit(1);
    }
    close(sock);
    exit(0);
}