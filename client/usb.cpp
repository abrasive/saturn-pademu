extern "C" {
    #include "rawhid/hid.h"
}
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../pademu/pademu.h"
#include "peripheral.h"
#include "usb.h"
#include <iostream>

Padulator::Padulator() {
    int n = rawhid_open(1, 0x2341, -1, 0xffc0, 0x0c00);
    if (n <= 0) {
        std::cerr << "Could not find Padulator USB" << std::endl;
        exit(1);
    }
}

void Padulator::send_report(int port, struct Report report) {
    pademu_cmd_t cmd;
    cmd.target = port;
    if (report.oneshot)
        cmd.target |= 2;
    cmd.report.size = report.size;
    memcpy(cmd.report.data, report.data, report.size);

    printf("USB: ");
    for (int i=0; i<2+report.size; i++)
        printf("%02X ", ((uint8_t*)&cmd)[i]);
    printf("\n");

    int ret = rawhid_send(0, &cmd, 2 + report.size, 1000);
    if (ret <= 0)
        std::cerr << "Failed to send command: " << ret << std::endl;

    if (report.oneshot)
        ret = rawhid_recv(0, &cmd, 64, 1000);
}
