#include "utils.h"

#include <stdbool.h>

enum state {
	START,
	FLAG_RCV,
	A_ANSWER_RCV,
    A_CMD_RCV,
	C_ANSWER_RCV,
    C_CMD_RCV,
    C_I_RCV,
    BBC_0_RCV,
    BBC_1_RCV,
    BBC_2_RCV,
    D_RCV,
	STOP
};

struct state_machine {
    enum state current_state;
    bool is_client;
};

void change_state(struct state_machine machine, char field) {
    enum state current_state = machine.current_state;
    bool is_client = machine.is_client;

    switch (machine.current_state) {
        case START:
            if (field == FLAG)
                current_state = FLAG_RCV;

            else 
                current_state = START;

            break;

        case FLAG_RCV:
            if (field == A_RC_RESP || field == A_EM_CMD)    
                current_state = A_ANSWER_RCV;

            else if (field == A_EM_RESP || field == A_RC_CMD)
                current_state = A_CMD_RCV;

            else 
                current_state = START;

            break;

        case A_ANSWER_RCV:
            if (field == C_UA || field == C_RR || field == C_REJ) 
                current_state = C_ANSWER_RCV;

            else 
                current_state = START;

            break;

        case A_CMD_RCV:
            if (field == C_SET || field == C_DISC)                
                current_state = C_CMD_RCV;

            // Include information state

            else 
                current_state = START;

            break;

        case C_ANSWER_RCV:
        case C_CMD_RCV:
            // BBC

            // else 
            // current_state = START;

            break;

        case C_I_RCV:
            break;
    }
}

