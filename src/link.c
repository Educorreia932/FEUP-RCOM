#include "link.h"

struct termios oldtio, newtio;
struct sigaction action;
bool flag = true;
int fd,  alarm_counter = 0;

void alarm_handler();

int byte_stuffing(unsigned char* packet, int length, unsigned char** frame) {
    *frame = NULL;
    *frame = (unsigned char*) malloc(MAX_SIZE);

    int index = 0;
    // Fill the frame, replacing flage and escape occurrences
    for (int c = 0; c < length; c++) {
        if (packet[c] == FLAG) {
            (*frame)[index] = ESCAPE;
            (*frame)[++index] = FLAG_STUFF;
        }
        else if (packet[c] == ESCAPE) {
            (*frame)[index] = ESCAPE;
            (*frame)[++index] = ESCAPE_STUFF;
        }
        else (*frame)[index] = packet[c];

        index++;
    }

    return index; // Return size of result
}

int byte_destuffing(unsigned char* packet, int length, unsigned char** frame) {
    *frame = NULL;
    *frame = (unsigned char*) malloc(MAX_SIZE);

    int index = 0;
    // Fill the frame, replacing escaped occurrences
    for (int c = 0; c < length; c++) {
        if (packet[c] == ESCAPE) (*frame)[index] = packet[++c] ^ 0x20;
        else (*frame)[index] = packet[c];

        index++;
    }

    return index; // Returns size of result 
}

/**
 * Creates & sends a supervision frame.
 */
int write_supervision_frame(int fd, char a, char c) {
    unsigned char buf[5];

    buf[0] = FLAG; // F
    buf[1] = a; // A
    buf[2] = c; // C
    buf[3] = a ^ c; // BCC
    buf[4] = FLAG; // F

    return write(fd, buf, 5);
}

/**
 * Creates an information frame.
 */
int create_information_frame(unsigned char* packet, int length, unsigned char** frame) {
    // Calculates BCC2
    unsigned char BCC_2 = packet[0];
    for (int i = 1; i < length; i++)
        BCC_2 ^= packet[i];

    // Byte stufing
    unsigned char *stuffed_bcc, *stuffed_data;
    int bcc_length = byte_stuffing(&BCC_2, 1, &stuffed_bcc);       // Byte-stuff BCC_2
    int new_length = byte_stuffing(packet, length, &stuffed_data); // Byte-stuff packet

    // Allocs memory for frame
    *frame = (unsigned char*) malloc(new_length + 5 + bcc_length); //TODO: check size after stuffing, cant' exceed MAX_SIZE
    
    // Fills frame
    (*frame)[0] = FLAG;                                    // F
    (*frame)[1] = A_EM_CMD;                                // A
    (*frame)[2] = llink->sequenceNumber & SEQUENCE_MASK_S; // Sequence number
    (*frame)[3] = A_EM_CMD ^ (*frame)[2];                  // BCC_1
    memcpy(*frame + 4, stuffed_data, new_length);          // Stuffed Packets
    
    new_length += 4; // Updates length (4 bytes from F,A,Ns,BCC)
    memcpy(*frame + new_length, stuffed_bcc, bcc_length); // BCC_2
    new_length += bcc_length; // Updates length (adds bcc 2 length)

    (*frame)[new_length++] = FLAG; // F

    free(stuffed_bcc);
    free(stuffed_data);

    return new_length; // Returns length of result
}

/**
 * Sends and information frame.
 */
int write_info_frame(int fd, unsigned char* packet, int length) {
    // Prepares frame to send
    unsigned char* frame;
    int frame_length = create_information_frame(packet, length, &frame);
    
    bool bcc_success = true;
    unsigned char bcc_result;
    char C;

    char buffer[1];

    // Start state machine
    struct state_machine stm;
    stm.current_state = START;
    stm.status = TRANSMITTER;

    alarm_counter = 0;
    flag = true;
    int n; // Written characters

    // Writing Frame Loop
    while (alarm_counter < llink->numTransmissions) {
        if (flag) {
            alarm(llink->timeout); // Activactes alarm
            flag = false;

            n = write(fd, frame, frame_length); // Sends I Frame

            if (n == -1) {
                perror("Failed to send information frame message.");
                return -1;
            }

            printf("Sent information frame. \n");
        }

        if (read(fd, buffer, 1) < 0) {
            // Read was not interrupted by the alarm, so it wasn't the cause for errno
            if (errno != EINTR) {
                perror("Failed to read.");
                return -1;
            }
        }

        // No error occurred
        else {
            change_state(&stm, buffer[0]);

            switch (stm.current_state) {
                case A_ANSWER_RCV:
                    bcc_result = buffer[0];

                    break;

                case C_RCV:
                    bcc_result ^= buffer[0];
                    C = buffer[0];

                    break;

                case BCC_1_RCV:
                    if (bcc_result != buffer[0])
                        bcc_success = false;

                    break;

                case STOP:
                    // Check frame type (RR or REJ)
                    if ((C == C_RR || C == (C_RR & SEQUENCE_MASK_R)) && bcc_success) {
                        printf("Received RR message.\n");
                        alarm(0);
                        llink->sequenceNumber = ~llink->sequenceNumber; // 0 or 1

                        return n;
                    }

                    else { // Received REJ frame. Need to resend I Frame (flag = 1).
                        printf("Received REJ message.\n");
                        flag = true;
                        alarm_counter = 0;
                        stm.current_state = START; //Restart state machine
                    }
            }
        }
    }

    // free(frame);

    if (alarm_counter == llink->numTransmissions) {
        perror("Failed to receive acknowledgement.\n");
        return -1;
    }

    return -1;
}

int read_info_frame(int fd, unsigned char** data_field) {
    bool changeNs = false;
    int counter = 0, length;
    char buffer[1];
    bool bcc_success = true, discard = false;
    unsigned char bcc_result;

    bool received_info = false;

    struct state_machine stm;

    stm.status = RECEIVER;
    stm.current_state = START;

    unsigned char frame[MAX_SIZE];

    while (!received_info) {
        if (read(fd, buffer, 1) < 0) {
            perror("Failed to read.");
            exit(1);
        }

        else {
            if (stm.current_state == START)
                counter = 0;

            change_state(&stm, buffer[0]);

            switch (stm.current_state) {
                case A_CMD_RCV:
                    bcc_result = buffer[0];

                    break;

                case C_I_RCV:
                    if (buffer[0] != (llink->sequenceNumber & SEQUENCE_MASK_S))
                        discard = true;

                    else
                       changeNs = true;
                case C_RCV:
                    bcc_result ^= buffer[0];

                    break;

                case BCC_1_RCV:
                    if (bcc_result != buffer[0])
                        bcc_success = false;

                    break;

                case D_RCV: // Store the data field and BCC_2
                    frame[counter] = buffer[0];
                    counter++;

                    break;

                case STOP:
                    length = byte_destuffing(frame, counter, data_field);

                    bcc_result = (*data_field)[0];

                    for (int i = 1; i < length - 1; i++)
                        bcc_result ^= (*data_field)[i];

                    if (bcc_result != (*data_field)[length - 1])
                        bcc_success = false;

                    int n;
                    
                    printf("Received information frame.\n");

                    if (bcc_success) {
                        printf("Sent RR message.\n");

                        n = write_supervision_frame(fd, A_RC_RESP, C_RR | (llink->sequenceNumber && SEQUENCE_MASK_R));
                        received_info = true;
                        if(changeNs)      llink->sequenceNumber = ~llink->sequenceNumber;
                        changeNs = false;
                    }

                    else {
                        printf("Sent REJ message.\n");
                        n = write_supervision_frame(fd, A_RC_RESP, C_REJ | (llink->sequenceNumber && SEQUENCE_MASK_R));
                        stm.current_state = START;

                        memset(frame, 0, counter); // Clean up the frame
                        counter = 0;
                        bcc_success = true;
                    }

                    if (n < 0) {
                        perror("Failed to send acknowledgement message.");
                        exit(1);
                    }

                    break;
            }
        }
    }

    if (discard)
        return -1;

    return length;
}

/**
 * Opens serial port device and sets configurations.
 * Sends SET & UA frames.
 */
int establish_connection(char* port, enum Status status) {
    llink->sequenceNumber = 0;

    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */
    fd = open(port, O_RDWR | O_NOCTTY);

    if (fd < 0) {
        perror(port);
        exit(-1);
    }

    /* Save current port settings */
    if (tcgetattr(fd, &oldtio) == -1) {
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = llink->baudrate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* Set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 0; /* Inter-character timer unused */
    newtio.c_cc[VMIN] = 1;  /* Blocking read until 5 chars received */

    /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) próximo(s) caracter(es)
    */

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    // Set state machine
    struct state_machine stm;

    stm.status = status;
    stm.current_state = START;

    char buf[5];

    if (status == TRANSMITTER) {
        // Set alarm handler
        action.sa_handler = &alarm_handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;

        // Installs co-routine that attends interruption
        if (sigaction(SIGALRM, &action, NULL) < 0) {
            perror("Failed to set SIGALARM handler.\n");
            exit(1);
        }

        bool receivedUA = false;

        // Tries to send SET Message
        while (alarm_counter < llink->numTransmissions) {
            if (flag) {
                alarm(llink->timeout); // Activactes alarm
                flag = false;

                int n = write_supervision_frame(fd, A_EM_CMD, C_SET); // Sends SET message

                if (n == -1) {
                    perror("Failed to send SET message.");
                    exit(1);
                }

                printf("Sent SET message. \n");
            }

            // Tries to receive UA message
            if (read(fd, buf, 1) < 0) {
                // Read was not interrupted by the alarm, so it wasn't the cause for errno
                if (errno != EINTR) {
                    perror("Failed to read.");
                    exit(1);
                }
            }

            // No error occured
            else {
                change_state(&stm, buf[0]); // Check if it is UA msg (state machine)

                if (stm.current_state == STOP)
                    receivedUA = true;
            }

            if (receivedUA) {
                alarm(0);
                printf("Received UA message.\n");
                printf("Established connection\n");
                break; // Read successfully UA msg
            }
        }

        if (alarm_counter == llink->numTransmissions) {
            perror("Failed to establish connection.\n");
            exit(1);
        }

    }

    else if (status == RECEIVER) {
        bool receivedSet = false;

        while (!receivedSet) {
            // Tries to receive SET message
            if (read(fd, buf, 1) < 0) {
                perror("Failed to read.");
                exit(1);
            }

            else {
                change_state(&stm, buf[0]); // Check if it is SET msg (state machine)

                if (stm.current_state == STOP)
                    receivedSet = true;
            }
        }

        printf("Received SET message.\n");

        int n = write_supervision_frame(fd, A_RC_RESP, C_UA); // Sends UA message

        if (n == -1) {
            perror("Failed to send UA message.");
            exit(1);
        }

        printf("Sent UA message.\n");
        printf("Established connection\n");
    }

    return fd;
}

int finish_connection(int fd, enum Status status) {
    alarm_counter = 0;
    flag = true;

    // Set state machine
    struct state_machine stm;

    stm.status = status;
    stm.current_state = START;

    char buf[5];

    if (status == TRANSMITTER) {
        bool receivedDISC = false;

        // Tries to send DISC Message
        while (alarm_counter < llink->numTransmissions) {
            if (flag) {
                alarm(llink->timeout); // Activactes alarm
                flag = false;

                int n = write_supervision_frame(fd, A_EM_CMD, C_DISC); // Sends DISC message

                if (n == -1) {
                    perror("Failed to send DISC message.");
                    exit(1);
                }

                printf("Sent DISC message. \n");
            }

            // Tries to receive DISC message
            if (read(fd, buf, 1) < 0) {
                // Read was not interrupted by the alarm, so it wasn't the cause for errno
                if (errno != EINTR) {
                    perror("Failed to read.");
                    exit(1);
                }
            }

            // No error occured
            else {
                change_state(&stm, buf[0]); // Check if it is UA msg (state machine)

                if (stm.current_state == STOP)
                    receivedDISC = true;
            }

            if (receivedDISC) {
                alarm(0);
                printf("Received DISC message.\n");

                int n = write_supervision_frame(fd, A_EM_CMD, C_UA); // Sends UA message

                if (n == -1) {
                    perror("Failed to send UA message.");
                    exit(1);
                }

                printf("Sent UA message. \n");
                printf("Finished connection\n");

                break;
            }
        }

        if (alarm_counter == llink->numTransmissions) {
            perror("Failed to establish connection.\n");
            exit(1);
        }

    }

    else if (status == RECEIVER) {
        bool receivedDISC = false;
        bool receivedUA = false;

        while (!receivedDISC) {
            // Tries to receive DISC message
            if (read(fd, buf, 1) < 0) {
                perror("Failed to read.");
                exit(1);
            }

            else {
                change_state(&stm, buf[0]); // Check if it is DISC msg (state machine)

                if (stm.current_state == STOP)
                    receivedDISC = true;
            }
        }

        printf("Received DISC message.\n");

        int n = write_supervision_frame(fd, A_RC_CMD, C_DISC); // Sends UA message

        if (n == -1) {
            perror("Failed to send DISC message.");
            exit(1);
        }

        printf("Sent DISC message.\n");

        while (!receivedUA) {
            // Tries to receive UA message
            if (read(fd, buf, 1) < 0) {
                perror("Failed to read.");
                exit(1);
            }

            else {
                change_state(&stm, buf[0]); // Check if it is UA msg (state machine)

                if (stm.current_state == STOP)
                    receivedUA = true;
            }
        }

        printf("Received UA message.\n");
        printf("Finished connection\n");
    }

    return 0;
}

/**
 * Handles an alarm signal.
 */
void alarm_handler() {
    printf("Alarm # %d\n", alarm_counter);
    flag = true; // Resets flag so it tries again
    alarm_counter++; 
}