// port 1:
#define P1_D0 A3 // PF4
#define P1_D1 A2 // PF5
#define P1_D2 A1 // PF6
#define P1_D3 A0 // PF7
#define P1_TH 15 // PB1
#define P1_TR 14 // PB3
#define P1_TL 16 // PB2

// port 2:
#define P2_D0 3  // PD0
#define P2_D1 2  // PD1
#define P2_D2 1  // PD3
#define P2_D3 0  // PD2
#define P2_TL 7  // PE6
#define P2_TH 8  // PB4
#define P2_TR 9  // PB5

#include <avr/interrupt.h>
#include "RawHID.h"
#include "pademu.h"

class Port {
    report_t normal, oneshot;

    int port_num;
    int th, tr, tl;
    void (*nybble)(int);

public:
    Port(int num,
         int th, int tr, int tl,
         void (*setupfunc)(void),
         void (*nybble)(int)
        )
        : port_num(num), th(th), tr(tr), tl(tl), nybble(nybble)
    {
        pinMode(th, INPUT);
        pinMode(tr, INPUT);
        digitalWrite(tl, 1);
        pinMode(tl, OUTPUT);
        setupfunc();

        normal.size = 0;
        oneshot.size = 0;
        nybble(0xf);
    }

    void set_report(report_t *report, bool is_oneshot) {
        report_t *target = is_oneshot ? &oneshot : &normal;
        memcpy(target, report, report->size + 1);
    }

    void poll(void) {
        report_t *current = oneshot.size ? &oneshot : &normal;

        if (current->size)
            nybble(0x1);
        else
            nybble(0xf);

        if (digitalRead(th))
            return;

        if (!current->size)
            return;

        send(current);

        if (current == &oneshot) {
            current->size = 0;
            RawHID.write((uint8_t*)"\0\0", 2);  // tell PC it's complete
        }
    }

private:
    void tx_byte(uint8_t data) {
        while (digitalRead(tr))
            if (digitalRead(th))
                return;

        nybble(data >> 4);
        digitalWrite(tl, 0);

        while (!digitalRead(tr))
            if (digitalRead(th))
                return;

        nybble(data & 0xf);

        digitalWrite(tl, 1);
    }

    void send(report_t *report) {
        report->data[report->size] = 0x01;

        for (int i=0; i<report->size + 1; i++) {
            tx_byte(report->data[i]);
            if (digitalRead(th)) {
                // Saturn ended the transaction but we still have bytes to send
                Serial.print("port ");
                Serial.print(port_num, DEC);
                Serial.print(" overrun at ");
                Serial.println(i, DEC);
                return;
            }
        }

        int timeout = 100;
        do {
            timeout--;
            if (!timeout) {
                // Saturn was expecting more bytes than we had
                Serial.print("port ");
                Serial.print(port_num, DEC);
                Serial.println(" underrun");
                while (!digitalRead(th));
                return;
            }

            delayMicroseconds(100);
        } while (!digitalRead(th));
    }
};

static void nybble1(int data) {
    PORTF = data << 4;
}

static void setup1(void) {
    DDRF |= 0xf0;
}

static void nybble2(int data) {
    PORTD = data & 3;
    if (data & 4)
        PORTD |= 8;
    if (data & 8)
        PORTD |= 4;
}

static void setup2(void) {
    DDRD |= 0x0f;
}

static uint8_t hid_buf[255];

Port port[] = {
    Port(1, P1_TH, P1_TR, P1_TL, setup1, nybble1),
    Port(2, P2_TH, P2_TR, P2_TL, setup2, nybble2),
};

void setup(void) {
    RawHID.begin(hid_buf, sizeof(hid_buf));

    Serial.begin(115200);
    //while (!Serial);
}

static void handle_usb(void) {
    int plen = RawHID.available();
    if (!plen)
        return;

    // RawHID plonks the packet at the end of the buffer...
    pademu_cmd_t *cmd = (pademu_cmd_t*)(hid_buf + sizeof(hid_buf) - plen);

    Port &p = port[cmd->target & 1];
    bool oneshot = !!(cmd->target & CMD_ONESHOT);
    p.set_report(&cmd->report, oneshot);

    RawHID.enable();    // resets the available byte counter to allow for next packet
}

void loop(void) {
    handle_usb();

    port[0].poll();
    port[1].poll();
}
