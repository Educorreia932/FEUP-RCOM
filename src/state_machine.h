#define FLAG 0x7E
#define ESCAPE 0x7D
#define FLAG_STUFF 0x5E    // 0x7E XOR 0x20
#define ESCAPE_STUFF 0x5D  // 0x7D XOR 0x20 

#define A_EM_CMD 0x03
#define A_EM_RESP 0x01
#define A_RC_RESP 0x03
#define A_RC_CMD 0x01

#define C_SET 0x03
#define C_DISC 0x0C
#define C_UA 0x07
#define C_RR 0x05 // Apply OR
#define C_REJ 0x01
#define C_I 0x40 // Change to include number of sequence

#define BAUDRATE B38400
#define TIMEOUT 3
#define NUM_TRANSMITIONS 3
#define MAX_SIZE 255

enum state {
    START,
    FLAG_RCV,
    A_ANSWER_RCV,
    A_CMD_RCV,
    C_ANSWER_RCV,
    C_CMD_RCV,
    C_I_RCV,
    C_RR,
    C_REJ,
    BBC_0_RCV,
    BBC_1_RCV,
    BBC_2_RCV,
    D_RCV,
    STOP
};

enum Status {
    TRANSMITTER,
    RECEIVER
};

struct state_machine {
    int current_state;
    enum Status status; /* TRANSMITTER | RECEIVER */
};

void change_state(struct state_machine* stm, char field);
