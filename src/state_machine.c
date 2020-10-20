#include "state_machine.h"

void change_state(struct state_machine machine, char field) {
    enum state current_state = machine.current_state;

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

