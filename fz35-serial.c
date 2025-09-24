#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <sys/time.h>
#include <sys/select.h>
#include "fz35-serial.h"

// Open serial port of FZ35 electronic load
int serial_open(const char *port) {
  int serial_port = open(port, O_RDWR);
  if(serial_port < 0) {
    return -1;
  }

  struct termios serial_config;
  tcgetattr(serial_port, &serial_config);

  // 9600 baud
  cfsetispeed(&serial_config, B9600);
  cfsetospeed(&serial_config, B9600);

  serial_config.c_cflag &= ~(CSIZE | PARENB); // Clear character size, no parity
  serial_config.c_cflag |= CS8;               // Set 8 bit size
  serial_config.c_cflag |= (CLOCAL | CREAD);  // Ignore modem control lines, enable receiver
  serial_config.c_iflag |= (IGNPAR | IGNBRK); // Ignore parity errors and break condition
  serial_config.c_iflag |= IGNCR;             // Ignore carriage return on input
  serial_config.c_lflag |= ICANON;            // Enable canonical mode
  serial_config.c_lflag &= ~(ECHO);           // Disable echo

  tcsetattr(serial_port, TCSANOW, &serial_config);

  return serial_port;
}

// Clear serial input buffer
int serial_flushinput(int serial_port) {
  return tcflush(serial_port, TCIFLUSH);
}

// Send serial command
int serial_send(int serial_port, const char *data) {
  // Send string
  int rc = write(serial_port, data, strlen(data));

  // Wait until output is written
  tcdrain(serial_port);

  return rc;
}

// Read serial reply using a 2sec timeout
int serial_read(int serial_port, void *data, size_t size) {
  struct timeval timeout;
  fd_set readfds;
  // Set timeout to 2 seconds
  timeout.tv_sec = 2;
  timeout.tv_usec = 0;
  // Clear fd_set and append serial port
  FD_ZERO(&readfds);
  FD_SET(serial_port, &readfds);
  // Wait for data available
  int rc = select(serial_port + 1, &readfds, NULL, NULL, &timeout);
  // Error or timeout occured?
  if(rc == -1 || rc == 0) {
    return -1;
  }
  // Read serial buffer
  return read(serial_port, data, size);
}

// Read positive reply from device
int serial_receive_success(int serial_port) {
  char response[64] = "";
  int bytes_read = read(serial_port, response, sizeof(response));
  if(bytes_read <= 0) {
    return -1;
  }
  // Check response
  // TODO: Handle alarm status
  if(strncmp(response, "sucess\n", sizeof(response)) == 0 || strncmp(response, "success\n", sizeof(response)) == 0) {
    return 0;
  }
  return -1;
}

// Receive and parse parameter read request
int serial_receive_parameters(int serial_port, struct parameters_t *p) {
  char response[64] = "";
  int bytes_read = read(serial_port, response, sizeof(response));
  if(bytes_read <= 0) {
    return -1;
  }
  // Parse response
  if(sscanf(response, "OVP:%f, OCP:%f, OPP:%f, LVP:%f,OAH:%f,OHP:%d:%d", &(p->ovp), &(p->ocp), &(p->opp), &(p->lvp), &(p->oah), &(p->ohp_h), &(p->ohp_m)) != 7) {
    return -1;
  }
  return 0;
}

// Send serial command
int serial_command(int serial_port, const char *command) {
  serial_flushinput(serial_port);
  serial_send(serial_port, command);
  return serial_receive_success(serial_port);
}

// Request parameters
int serial_read_parameters(int serial_port, struct parameters_t *p) {
  serial_flushinput(serial_port);
  serial_send(serial_port, "read");
  return serial_receive_parameters(serial_port, p);
}

// Receive measurement data
int serial_read_measurement(int serial_port, struct measurement_t *m) {
  char response[64] = "";
  serial_flushinput(serial_port);
  int bytes_read = serial_read(serial_port, response, sizeof(response));
  if(bytes_read <= 0) {
    return -1;
  }
  // Parse response
  if(sscanf(response, "%fV,%fA,%fAh,%d:%d", &(m->v), &(m->a), &(m->ah), &(m->h), &(m->m)) != 5) {
    return -1;
  }
  return 0;
}

// Close serial port
int serial_close(int serial_port) {
  return close(serial_port);
}