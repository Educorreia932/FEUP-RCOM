#include "state_machine.h"
#include <stdio.h>

void change_state(struct state_machine* stm, char field) {
    switch (stm->current_state) {
        case START:
            if (field == FLAG) 
                stm->current_state = FLAG_RCV;

            break;

        case FLAG_RCV:
            if (stm->status == RECEIVER) {
                if (field == A_EM_CMD)
                    stm->current_state = A_CMD_RCV;

                else if (field == A_EM_RESP)
                    stm->current_state = A_ANSWER_RCV;

                else
                    stm->current_state = START;
            }

            else if (stm->status == TRANSMITTER) {
                if (field == A_RC_CMD)
                    stm->current_state = A_CMD_RCV;

                else if (field == A_RC_RESP)
                    stm->current_state = A_ANSWER_RCV;

                else
                    stm->current_state = START;
            }

            break;

        case A_CMD_RCV:
            if (field == C_SET || field == C_DISC)
                stm->current_state = C_CMD_RCV;

            else if (field == 0x40)
                stm->current_state = C_I_RCV;

            else
                stm->current_state = START;

            break;

        case A_ANSWER_RCV:
            if (field == C_RR || field == C_REJ || field == C_UA)
                stm->current_state = C_ANSWER_RCV;

            else
                stm->current_state = START;

            break;

        case C_ANSWER_RCV:
        case C_CMD_RCV:
            // TODO: Check BBC
            if (1)
                stm->current_state = BCC_0_RCV;

            else
                stm->current_state = START;

            break;

        case BCC_0_RCV:
            if (field == FLAG)
                stm->current_state = STOP;

            else
                stm->current_state = START;

            break;

        case C_I_RCV:
            // TODO: Check BCC
            if (1)
                stm->current_state = BCC_1_RCV;

            else
                stm->current_state = START;
           
            break;

        case BCC_1_RCV:
            if (1)
                stm->current_state = D_RCV;

            else
                stm->current_state = START;

            break;

        case D_RCV: // TODO: last byte will be BCC2
            if (field == FLAG)
                stm->current_state = STOP;

            break;
    }
}
