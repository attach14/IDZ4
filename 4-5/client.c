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
    int sock;
    struct sockaddr_in echoServAddr;
    unsigned short echoServPort;
    char* servIP;
    unsigned int echoLen;
    int bytesRcvd, totalBytesRcvd;
    int n;
    int fromAddr, fromSize;
    if (argc != 4) {
        printf("Wrong number of input arguments: %d instead of 3\n", argc - 1);
        return 0;
    }

    servIP = argv[1];                  /* First arg: server IP address (dotted quad) */
    echoServPort = atoi(argv[2]); /* Use given port, if any */ /* Second arg: string to echo */
    n = atoi(argv[3]);
    if (n <= 0 || n > 10) {
        printf("Wrong number of accountants: it must be > 0 and < 11\n");
        return 0;
    }
    (void)signal(SIGINT, my_handler);
    srand(0);
    for (int i = 0; i < n + 1; ++i)
    {
        /* Create a reliable, socket using UDP */
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
        if (i == 0) {
            numbers[0] = n;
            if (sendto(sock, numbers, echoLen, 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) != echoLen)
            {
                perror("sendto() sent a different number of bytes than expected");
                exit(1);
            }
            sleep(1);
            continue;
        }
        echoLen = sizeof(numbers); /* Determine input length */

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
        printf("OK\n");
        while (totalBytesRcvd < echoLen)
        {
            if ((bytesRcvd = recvfrom(sock, res, sizeof(res), 0, (struct sockaddr *)&fromAddr, &fromSize)) < 0)
            {
                perror("recvfrom() failed or connection closed prematurely");
                exit(1);
            }
            totalBytesRcvd += bytesRcvd;
        }
        printf("Accountant %d got new task\n", i);
        printf("\n"); /* Print a final linefeed */
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
        int cur = fork();
        if (cur == 0) {
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

            /* Establish the connection to the echo server */
            double ans = calc(a, b, epsilon, delta_x, delta_y, coeff);
            printf("Accountant %d finished calculation: area = %lf\n", i, ans);
            numbers[0] = 1; //finish
            numbers[1] = ans;  //ans
            echoLen = sizeof(numbers);
            if (sendto(sock, numbers, echoLen, 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) != echoLen)
            {
                perror("sendto() sent a different number of bytes than expected");
                exit(1);
            }
            sleep(1);
            close(sock);
            return 0;
        }
        close(sock);
    }
    close(sock);
    exit(0);
}