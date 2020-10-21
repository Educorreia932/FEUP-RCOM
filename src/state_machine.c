#include "state_machine.h"
#include <stdio.h>

int change_state(int current_state, char field) {
    switch (current_state) {
        case START:
            if (field == FLAG) 
                return FLAG_RCV;

            else
                return START;

            break;

        case FLAG_RCV:
            if (field == A_RC_RESP || field == A_EM_CMD)
                return A_CMD_RCV;

            else if (field == A_EM_RESP || field == A_RC_CMD)
                return A_ANSWER_RCV;

            else
                return START;

            break;

        case A_ANSWER_RCV:
            if (field == C_RR || field == C_REJ)
                return C_ANSWER_RCV;

            else
                return START;

            break;

        case A_CMD_RCV:
            if (field == C_SET || field == C_DISC || field == C_UA)
                return C_CMD_RCV;

            // Include information state

            else
                return START;

            break;

        case C_ANSWER_RCV:
        case C_CMD_RCV:
            // TODO: Check BBC
            if (1)
                return BBC_0_RCV;

            else
                return START;

            break;

        case C_I_RCV:
            break;

        case BBC_0_RCV:
            if (field == FLAG)
                return STOP;

            else
                return START;

            break;
    }
}
