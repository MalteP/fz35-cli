#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include "fz35-serial.h"

void print_usage(char *name);

int main(int argc, char *argv[]) {
  int rc;                 // Return code of last command
  int serial_port;        // Serial port file descriptor
  float last_amps = 0;    // Last set  load current
  char buf[32];           // Temporary buffer
  struct parameters_t p;  // Parameter data structure
  struct measurement_t m; // Measurement data structure

  // We need at least two arguments (port and one command)
  if(argc <= 2) {
    fprintf(stderr, "Error: Too few arguments.\n\n");
    print_usage(argv[0]);
    exit(1);
  }

  // Open serial port given in first argument
  serial_port = serial_open(argv[1]);
  if(serial_port < 0) {
    fprintf(stderr, "Error: Could not open %s.\n\n", argv[1]);
    print_usage(argv[0]);
    exit(1);
  }

  // Parse other arguments in a loop
  for(int i = 2; i < argc; ++i) {
    rc = -2;
    char *arg = argv[i];
    int len = strlen(argv[i]);

    // Commands start, stop, on, off: Enable / disable UART communication, enable / disable load
    if(strcmp(arg, "start") == 0 || strcmp(arg, "stop") == 0 || strcmp(arg, "on") == 0 || strcmp(arg, "off") == 0) {
      rc = serial_command(serial_port, arg);
    }

    // Command slp: Sleep for one second
    if(strcmp(arg, "slp") == 0) {
      sleep(1);
      rc = 0;
    }

    // Command x.xxA: Set load current
    if(len > 1 && arg[len - 1] == 'A') {
      // Parse and format value, i.e. 1A to 1.00A
      if(sscanf(arg, "%fA", &last_amps) == 1) {
        snprintf(buf, sizeof(buf), "%04.2fA", last_amps);
        rc = serial_command(serial_port, buf);
      } else {
        rc = -1;
      }
    }

    // Command data: Read raw status data line, i.e. voltage, current, capacity and discharge time
    if(strcmp(arg, "data") == 0) {
      rc = serial_read_measurement(serial_port, &m);
      if(rc == 0) {
        printf("%05.2fV,%04.2fA,%5.3fAh,%02d:%02d\r\n", m.v, m.a, m.ah, m.h, m.m);
      }
    }

    // Command getVolts: Read voltage [V], i.e. 12.34
    if(strcmp(arg, "getVolts") == 0) {
      rc = serial_read_measurement(serial_port, &m);
      if(rc == 0) {
        printf("%05.2f\r\n", m.v);
      }
    }

    // Command getAmps: Read current [A], i.e. 1.23
    if(strcmp(arg, "getAmps") == 0) {
      rc = serial_read_measurement(serial_port, &m);
      if(rc == 0) {
        printf("%04.2f\r\n", m.a);
      }
    }

    // Command getWatts: Read power [W], i.e. 1.23
    if(strcmp(arg, "getWatts") == 0) {
      rc = serial_read_measurement(serial_port, &m);
      if(rc == 0) {
        printf("%04.2f\r\n", (m.v * m.a));
      }
    }

    // Command getAmpHours: Read capacity [Ah], i.e. 1.234
    if(strcmp(arg, "getAmpHours") == 0) {
      rc = serial_read_measurement(serial_port, &m);
      if(rc == 0) {
        printf("%05.3f\r\n", m.ah);
      }
    }

    // Command getTime: Read discharge time [hh:mm], i.e. 12:34
    if(strcmp(arg, "getTime") == 0) {
      rc = serial_read_measurement(serial_port, &m);
      if(rc == 0) {
        printf("%02d:%02d\r\n", m.h, m.m);
      }
    }

    // Command getCsvVoltsAmps: Read voltage and current as CSV, i.e. 12.34,5.00
    if(strcmp(arg, "getCsvVoltsAmps") == 0) {
      rc = serial_read_measurement(serial_port, &m);
      if(rc == 0) {
        printf("%05.2f,%04.2f\r\n", m.v, m.a);
      }
    }

    // Command getCsvRow: Read set current, voltage, current and power as CSV, i.e. 0.50,12.34,0.50,6.17
    if(strcmp(arg, "getCsvRow") == 0) {
      rc = serial_read_measurement(serial_port, &m);
      if(rc == 0) {
        printf("%04.2f,%05.2f,%04.2f,%04.2f\r\n", last_amps, m.v, m.a, (m.v * m.a));
      }
    }

    // Command settings: Read protection and discharge limits
    if(strcmp(arg, "settings") == 0) {
      rc = serial_read_parameters(serial_port, &p);
      if(rc == 0) {
        printf("OVP:%04.1f, OCP:%04.2f, OPP:%05.2f, LVP:%04.1f,OAH:%05.3f,OHP:%02d:%02d\r\n", p.ovp, p.ocp, p.opp, p.lvp, p.oah, p.ohp_h, p.ohp_m);
      }
    }

    // Command setup: Initialize default protection values and discharge limits
    if(strcmp(arg, "setup") == 0) {
      const char setup_cmds[][10] = {"stop", "OVP:25.2", "OCP:5.10", "OPP:35.50", "LVP:1.5", "OAH:0.000", "OHP:00:00", "start"};
      for(int i = 0; i < (sizeof(setup_cmds) / 10); i++) {
        rc = serial_command(serial_port, setup_cmds[i]);
        if(rc < 0) {
          break;
        }
      }
    }

    // Commands LVP/OVP/OCP/OPP/OAH/OPP: Set protection values and discharge limits
    if(strncmp(arg, "LVP:", 4) == 0 || strncmp(arg, "OVP:", 4) == 0 || strncmp(arg, "OCP:", 4) == 0 || strncmp(arg, "OPP:", 4) == 0 || strncmp(arg, "OAH:", 4) == 0 || strncmp(arg, "OHP:", 4) == 0) {
      rc = serial_command(serial_port, arg);
    }

    // Verify successful command execution
    if(rc < 0) {
      if(rc < -1) {
        fprintf(stderr, "Error: Command unknown \"%s\"\n\n", arg);
      } else {
        fprintf(stderr, "Error: Command failed \"%s\"\n\n", arg);
      }
      print_usage(argv[0]);
      // Abort loop
      break;
    }
  }

  // Finally close serial connection
  serial_close(serial_port);
  return 0;
}

// Print possible commands
void print_usage(char *name) {
  fprintf(stderr, "usage: %s port command [command] [...]\n", name);
  fprintf(stderr, "  e.g. %s /dev/ttyUSB0 1.00A on\n\n", name);
  fprintf(stderr, "Possible commands:\n");
  fprintf(stderr, "  on ................ Enable load\n");
  fprintf(stderr, "  off ............... Disable load\n");
  fprintf(stderr, "  start ............. Enable UART status data communication\n");
  fprintf(stderr, "  stop .............. Disable UART status data communication\n");
  fprintf(stderr, "  slp ............... Sleep 1 second\n");
  fprintf(stderr, "  x.xxA ............. Set load current\n");
  fprintf(stderr, "  data .............. Read raw status data (voltage, current, capacity, discharge time)\n");
  fprintf(stderr, "  getVolts .......... Read voltage [V]\n");
  fprintf(stderr, "  getAmps ........... Read current [A]\n");
  fprintf(stderr, "  getWatts .......... Read power [W]\n");
  fprintf(stderr, "  getAmpHours ....... Read capacity [Ah]\n");
  fprintf(stderr, "  getTime ........... Read discharge time [hh:mm]\n");
  fprintf(stderr, "  getCsvVoltsAmps ... Read voltage and current as CSV\n");
  fprintf(stderr, "  getCsvRow ......... Read status data as CSV (set current, voltage, current, power)\n");
  fprintf(stderr, "  settings .......... Read protection and discharge limits\n");
  fprintf(stderr, "  setup ............. Initialize default protection values and discharge limits\n");
  fprintf(stderr, "  LVP:x.x ........... Set under voltage protection (default 1.5)\n");
  fprintf(stderr, "  OVP:xx.x .......... Set over voltage protection (default 25.2)\n");
  fprintf(stderr, "  OCP:x.xx .......... Set over current protection (default 5.10)\n");
  fprintf(stderr, "  OPP:xx.xx ......... Set over power protection (default 35.50)\n");
  fprintf(stderr, "  OAH:x.xxx ......... Set maximum capacity (default 0.000)\n");
  fprintf(stderr, "  OHP:xx:xx ......... Set maximum discharge time (default 00:00)\n");
}