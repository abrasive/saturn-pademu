#define CMD_PORT_1  0x00
#define CMD_PORT_2  0x01
#define CMD_ONESHOT 0x02

#define STAT_ONESHOT    0x02

#pragma pack(push, 1)

typedef struct {
    uint8_t size;
    uint8_t data[127];
} report_t;

typedef struct {
    uint8_t target;
    report_t report;
} pademu_cmd_t;

#pragma pack(pop)
