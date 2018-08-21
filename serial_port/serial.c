//https://stackoverflow.com/a/6947758
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pty.h>
#include <stdio.h>
#include "serial.h"

#define NUM_SERIAL_PORT (SP3 - KERNEL + 1)

#define NONE -1

#define error_message(fmt, ...) \
do \
{ \
    fprintf(stderr, fmt, ##__VA_ARGS__); \
} while (0);

char* get_partition_name(int i)
{
    switch (i+KERNEL) {
        case KERNEL:
            fprintf(stdout, "Partitition: KERNEL ");
            return "KERNEL";
        case MULTIPLEXER:
            fprintf(stdout, "Partitition: MULTIPLEXER ");
            return "MULTIPLEXER";
        case FREERTOS:
            fprintf(stdout, "Partitition: FREERTOS ");
            return "FREERTOS";
        case NETWORK:
            fprintf(stdout, "Partitition: NETWORK ");
            return "NETWORK";
        case SECURE:
            fprintf(stdout, "Partitition: SECURE ");
            return "SECURE";
        case NORMAL:
            fprintf(stdout, "Partitition: NORMAL ");
            return "NORMAL";
        case OWNER:
            fprintf(stdout, "Partitition: OWNER ");
            return "OWNER";
        case SP1:
            fprintf(stdout, "Partitition: SP1 ");
            return "SP1";
        case SP2:
            fprintf(stdout, "Partitition: SP2 ");
            return "SP2";
        case SP3:
            fprintf(stdout, "Partitition: SP3 ");
            return "SP3";
        default:
            fprintf(stdout, "unknown partition number :%d ", i);
            return "UNKNOWN";
    }
}


int
set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                error_message ("error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                error_message ("error %d from tcsetattr", errno);
                return -1;
        }
        return 0;
}

void
set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                error_message ("error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                error_message ("error %d setting term attributes", errno);
}

void open_free_pty(int* fd_master, int* fd_slave)
{
    char ttyname[20];

    if (openpty(fd_master, fd_slave, ttyname, NULL, NULL) != 0){
        error_message ("error %d getting a free pty", errno);
    } else {
        fprintf(stdout, "Read pty: %s\n", ttyname);
    }
}

void demultiplex(char* buf, size_t n, int* fd_masters)
{
    static int previous_port = NONE;
    size_t i = 0;

    while (i<n) {
        if (previous_port != NONE){
                write(fd_masters[previous_port], &buf[i], 1);
                previous_port = NONE;
        }
        else if (buf[i] >= (char) KERNEL){
            int serial_port = buf[i] - (char) KERNEL;
            if (serial_port < NUM_SERIAL_PORT && i < n -1){
                previous_port = NONE;
                write(fd_masters[serial_port], &buf[++i], 1);
            } else if (i == n -1)
                previous_port = serial_port;
            else {
                fprintf(stdout, "%c", buf[i]);
            }
        } else {
            fprintf(stdout, "%c", buf[i]);
        }
        i++;
    }
}

void main ()
{
    char *portname = "/dev/ttyUSB0";
    int fd_masters[NUM_SERIAL_PORT], fd_slaves[NUM_SERIAL_PORT], i = 0, wstatus;

    for (i=0; i<NUM_SERIAL_PORT; i++){
        char* partition = get_partition_name(i);
        open_free_pty(&fd_masters[i], &fd_slaves[i]);

        pid_t pid = fork();
        if (pid == 0){ //chlid
            char *argv[] = {"screen", "-d", "-m", "-S", partition, "-t", partition, ttyname(fd_slaves[i]), NULL};
            execv("/usr/bin/screen", argv);
        } else { //parent
            waitpid(pid, &wstatus, WEXITED);
        }
    }

    int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
    {
            error_message ("error %d opening %s: %s", errno, portname, strerror (errno));
            return;
    }

    set_interface_attribs (fd, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
    set_blocking (fd, 0);                // set no blocking

    // write (fd, "hello!\n", 7);           // send 7 character greeting

    // usleep ((7 + 25) * 100);             // sleep enough to transmit the 7 plus
                                         // receive 25:  approx 100 uS per char transmit
    char buf [100];
    size_t n = read (fd, buf, sizeof buf);  // read up to 100 characters if ready to read

    while (1) {
        if (n) {
            demultiplex(buf, n, fd_masters);
        }
        n = read (fd, buf, sizeof buf);  // read up to 100 characters if ready to read
    }
}
