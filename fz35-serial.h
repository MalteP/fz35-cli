#ifndef FZ35_SERIAL_H
#define FZ35_SERIAL_H

// Structure for setup parameters
struct parameters_t {
  float ovp;
  float ocp;
  float opp;
  float lvp;
  float oah;
  int ohp_h;
  int ohp_m;
};

// Structure for measurement data
struct measurement_t {
  float v;
  float a;
  float ah;
  int h;
  int m;
};

// External functions
int serial_open(const char *port);
int serial_command(int serial_port, const char *command);
int serial_read_parameters(int serial_port, struct parameters_t *p);
int serial_read_measurement(int serial_port, struct measurement_t *m);
int serial_close(int serial_port);

#endif