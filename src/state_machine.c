#include "state_machine.h"
#include <stdio.h>

void change_state(struct state_machine* stm, char field) {
    switch (stm->current_state) { 
        case START:
            if (field == FLAG)  // Received FLAG
                stm->current_state = FLAG_RCV;
            break;

        case FLAG_RCV:
            if(field == FLAG){ // Received FLAG
                // Do nothing
            }
            else if (stm->status == RECEIVER) {
                if (field == A_EM_CMD) // Received A command
                    stm->current_state = A_CMD_RCV;

                else if (field == A_EM_RESP) // Received A response
                    stm->current_state = A_ANSWER_RCV;

                else stm->current_state = START; // Received other
            }
            else if (stm->status == TRANSMITTER) {
                if (field == A_RC_CMD)  // Received A command
                    stm->current_state = A_CMD_RCV;
                
                else if (field == A_RC_RESP) // Received A response
                    stm->current_state = A_ANSWER_RCV;

                else stm->current_state = START; // Received other
            }
            break;

        case A_CMD_RCV: 
            if (field == C_SET || field == C_DISC) // Received C (SET or DISC)
                stm->current_state = C_RCV;

            else if (field == NS_1 || field == NS_2) // Received sequence number
                stm->current_state = C_I_RCV;

            else stm->current_state = START; // Received other
            break;

        case A_ANSWER_RCV:
            if (field == C_RR || field == C_REJ || field == C_UA) // Received C (RR or REJ or UA)
                stm->current_state = C_RCV;

            else stm->current_state = START; // Received other
            break;

        case C_I_RCV:
        case C_RCV:
            stm->current_state = BCC_1_RCV; 
            break;

        case BCC_1_RCV:
            if (field == FLAG) // Received Flag
                stm->current_state = STOP; 
            else stm->current_state = D_RCV; // Otherwise, we're receiving data 
            break;

        case D_RCV:
            if (field == FLAG) // Finished receiving DATA
                stm->current_state = STOP;
            break;
    }
}
