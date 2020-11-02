#include "link.h"
#include <string.h>

struct termios oldtio, newtio;
struct sigaction action;
int fd;

bool flag = true;
int alarm_counter = 0;

// TODO: Cleanup serial port with fflush when necessary

void alarm_handler() {
    printf("Alarm # %d\n", alarm_counter);
    flag = true;
    alarm_counter++;
}

int write_supervision_frame(int fd, char a, char c) {
    unsigned char buf[5];

    buf[0] = FLAG;
    buf[1] = a;
    buf[2] = c;
    buf[3] = a ^ c; // BCC
    buf[4] = FLAG;

    return write(fd, buf, 5);
}

// Stuffing

int byte_stuffing(unsigned char* packet, int length, unsigned char** frame) {
    int counter = length;

    // Calculate size of data to allocate the necessary space
    for (int c = 0; c < length; c++) {
        if (packet[c] == FLAG || packet[c] == ESCAPE)
            counter++;
    }

    *frame = NULL;
    *frame = (unsigned char*) malloc(counter);
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

        else
            (*frame)[index] = packet[c];

        index++;
    }

    return counter;
}

int byte_destuffing(unsigned char* packet, int length, unsigned char** frame) {
    int counter = length;

    // Calculate size of data to allocate the necessary space
    for (int c = 0; c < length; c++) {
        if (packet[c] == ESCAPE) {
            counter--;
            c++;
        }
    }

    *frame = NULL;
    *frame = (unsigned char*) malloc(counter);
    int index = 0;

    // Fill the frame, replacing escaped occurrences
    for (int c = 0; c < length; c++) {
        if (packet[c] == ESCAPE)
            (*frame)[index] = packet[++c] ^ 0x20;

        else
            (*frame)[index] = packet[c];

        index++;
    }

    return counter;
}

// Information Frames

int create_information_frame(unsigned char* packet, int length, unsigned char** frame) {
    unsigned char BCC_2 = packet[4];

    for (int i = 5; i < length; i++)
        BCC_2 ^= packet[i];

    unsigned char* stuffed;
    int new_length = byte_stuffing(packet, length, &stuffed); // Byte-stuff packet

    *frame = (unsigned char*) malloc(new_length + 6); //TODO: check size after stuffing, cant' exceed MAX_SIZE

    (*frame)[0] = FLAG;                                  // F
    (*frame)[1] = A_EM_CMD;                              // A
    (*frame)[2] = llink->sequenceNumber & SEQUENCE_MASK; // N(s) place 0S000000 C
    (*frame)[3] = A_EM_CMD ^ (*frame)[2];                // BCC_1

    memcpy(*frame + 4, stuffed, new_length);
    free(stuffed);

    new_length += 4;

    int n = byte_stuffing(&BCC_2, 1, &stuffed); // Byte-stuff BCC_2
    memcpy(*frame + new_length, stuffed, n); // BCC_2

    (*frame)[new_length++] = FLAG;

    return new_length;
}

int write_info_frame(int fd, unsigned char* packet, int length) {
    unsigned char* frame;

    // Prepare frame to send
    length = create_information_frame(packet, length, &frame);

    char buffer[1];
    char frame_type; // RR or REJ

    // Start state machine
    struct state_machine stm;
    stm.current_state = START;
    stm.status = TRANSMITTER;
    stm.sequence_number = &llink->sequenceNumber;

    alarm_counter = 0;
    flag = true;
    bool receivedRR = false;
    int n; // Written characters

    while (alarm_counter < llink->numTransmissions) {
        if (flag) {
            alarm(llink->timeout); // Activactes alarm
            flag = false;

            for (int i = 0; i < length; i++) {
                // printf("BUFFER %x\n", frame[0]);
                n = write(fd, frame, 1); // Sends I Frame

                frame++;

                if (n == -1) {
                    perror("Failed to send information Frame message.");
                    return -1;
                }
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
            // if (stm.current_state == C_RR_RCV) //RR frame
            //     frame_type = C_RR_RCV;

            // else if (stm.current_state == C_REJ_RCV) //REJ frame
            //     frame_type = C_REJ_RCV;

            // else if (stm.current_state == STOP) { //Read frame successfuly
            //     if (frame_type == C_RR_RCV)       //Check frame type (RR or REJ)
            //         receivedRR = true;

            //     else { //Received REJ frame. Need to resend I Frame (flag = 0).
            //         flag = 0;
            //         stm.current_state = START; //Restart state_machine
            //     }
            // }

            change_state(&stm, buffer[0]); // Check if it is SET msg (state machine)

            if (stm.current_state == STOP) {
                printf("Received RR message.\n");
                llink->sequenceNumber = ~llink->sequenceNumber; // 0 or 1

                break;
            }
        }
    }

    // free(frame);

    if (alarm_counter == llink->numTransmissions) {
        perror("Failed receive ACK.\n");
        return -1;
    }

    return n;
}

int read_info_frame(int fd, unsigned char** data_field) {
    int counter = 0;
    char buffer[1];

    bool received_info = false;

    struct state_machine stm;

    stm.status = RECEIVER;
    stm.current_state = START;
    stm.sequence_number = &llink->sequenceNumber;

    unsigned char frame[MAX_SIZE];

    while (!received_info) {
        if (read(fd, buffer, 1) < 0) {
            perror("Failed to read.");
            exit(1);
        }

        else {
            if (stm.current_state == START)
                counter = 0;

            change_state(&stm, buffer[0]); // Check if it is SET msg (state machine)

            if (stm.current_state == D_RCV) {
                frame[counter] = buffer[0];
                counter++;
            }

            if (stm.current_state == STOP)
                received_info = true;
        }
    }

    int n = write_supervision_frame(fd, A_RC_RESP, C_RR);

    if (n < 0) {
        perror("Failed to send ACK message.");
        exit(1);
    }

    printf("Sent ACK message.\n");

    int length = byte_destuffing(frame, counter, data_field);
    
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