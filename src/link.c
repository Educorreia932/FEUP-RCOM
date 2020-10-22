#include "link.h"
#include <string.h>

struct linkLayer llink;
struct termios oldtio, newtio;
struct sigaction action;
int fd;

bool flag = true;
int alarm_counter = 0;



void alarm_handler() {
    printf("Alarm # %d\n", alarm_counter);
    flag = true;
    alarm_counter++;
}

int send_supervision_frame(int fd, char a, char c) {
    unsigned char buf[5];

    buf[0] = FLAG;
    buf[1] = a;
    buf[2] = c;
    buf[3] = a ^ c; // BCC
    buf[4] = FLAG;

    return write(fd, buf, 5);
}



//Stuffing

char *byte_stuffing(char *packet) {
    int length = strlen(packet);
    int counter = length;

    // Calculate size of data to allocate the necessary space
    for (int c = 0; c < length; c++) {
        if (packet[c] == FLAG || packet[c] == ESCAPE)
            counter++;
    }

    char *frame = (char *)malloc(counter);
    int index = 0;

    // Fill the frame, replacing flage and escape occurrences
    for (int c = 0; c < length; c++) {
        if (packet[c] == FLAG) {
            frame[index] = ESCAPE;
            frame[++index] = FLAG_STUFF;
        }

        else if (packet[c] == ESCAPE) {
            frame[index] = ESCAPE;
            frame[++index] = ESCAPE_STUFF;
        }

        else
            frame[index] = packet[c];

        index++;
    }

    return frame;
}

char *byte_destuffing(char *packet) {
    int length = strlen(packet);
    int counter = length;

    // Calculate size of data to allocate the necessary space
    for (int c = 0; c < length; c++) {
        if (packet[c] == ESCAPE) {
            counter--;
            c++;
        }
    }

    char *frame = (char *)malloc(counter);
    int index = 0;

    // Fill the frame, replacing escaped occurrences
    for (int c = 0; c < length; c++) {
        if (packet[c] == ESCAPE)
            frame[index] = packet[++c] ^ 0x20;

        else
            frame[index] = packet[c];

        index++;
    }

    return frame;
}


//I Frames

char * create_information_frame(char* packet) {
    packet = byte_stuffing(packet); // Byte-stuff packet

    unsigned char* buf = malloc(strlen(packet) + 5); //TODO: check size after suffing

    buf[0] = FLAG;
    buf[1] = A_EM_CMD;
    buf[2] = llink.sequenceNumber; //TODO: Change N(s) place 0S000000
    buf[3] = A_EM_CMD ^ llink.sequenceNumber; // BCC_1

    int BCC_2 = packet[0];

    int i;
 
    for (i = 4; i < strlen(packet); i++) { //strlen??
        buf[i] = packet[i - 4];
        
        BCC_2 ^= packet[i];
    }

    buf[i] = BCC_2;
    buf[i + 1] = FLAG;

    return packet;
}

int write_I_frame(char * packet) //TODO: needs length in arguments??
{
    //Prepare frame to send
    char * frame = create_information_frame(packet);

    char buf[MAX_SIZE]; //TODO: Check size
    char frame_type; // RR or REJ

    //Star state machine
    struct state_machine stm;
    stm.current_state = START;
    stm.status = TRANSMITTER;   

    alarm_counter = 0;
    bool receivedRR = false;
    int n; //Written characters

    while (alarm_counter < llink.numTransmissions) {
        if (flag) {
            alarm(llink.timeout); // Activactes alarm
            flag = false;

            n = write(fd, frame, sizeof(frame)); // Sends I Frame

            if (n == -1) {
                perror("Failed to send I Frame message.");
                return -1;
            }

            printf("Sent I Frame. \n");
        }

        // Receives ACK
        if (read(fd, buf, 1) < 0) {
            // Read was not interrupted by the alarm, so it wasn't the cause for errno
            if (errno != EINTR) {
                perror("Failed to read.");
                return -1;
            }
        }

        // No error occured
        else {
            change_state(&stm, buf[0]); // TODO: Check if it is ACK msg (state machine)

            if(stm.current_state == C_RR) //RR frame
                frame_type = C_RR;

            else if (stm.current_state == C_REJ) //REJ frame
                frame_type = C_REJ;

            else if (stm.current_state == STOP){ //Read frame successfuly
                
                if(frame_type == C_RR)  //Check frame type (RR or REJ)
                    receivedRR = true;
                
                else{              //Received REJ frame. Need to resend I Frame (flag = 0).
                    flag = 0;
                    stm.current_state = START;  //Restart state_machine
                }
            }
        }
        
        if (receivedRR) {    // Read successfully RR msg
            printf("Received RR message.\n");
            llink.sequenceNumber = !llink.sequenceNumber; // 0 or 1
            break;
        }
    }

    if (alarm_counter == llink.numTransmissions)
    {
        perror("Failed to establish send Frame / Receive ACK.\n");
        return -1;
    }

    return n;
}

char* receive_information_frame(int fd) {
    // TODO: Turn this function into one with a more general purpose and use the state machine
    char* packet = (char*) malloc(20000);
    char* buf = malloc(20000);

    int counter = 0;

    read(fd, buf, 20000);

    buf = byte_destuffing(buf);

    printf("%s\n", buf);

    return buf;
}



/**
 * Opens serial port device and sets configurations.
 * Sends SET & UA frames.
 */
int establish_connection(char *port, enum Status status) {
    strcpy(llink.port, port);
    llink.baudRate = BAUDRATE;
    llink.sequenceNumber = 0;
    llink.timeout = TIMEOUT;
    llink.numTransmissions = NUM_TRANSMITIONS;

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
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
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

        // Tries to send Set Message
        while (alarm_counter < llink.numTransmissions) {
            if (flag) {
                alarm(llink.timeout); // Activactes alarm
                flag = false;

                int n = send_supervision_frame(fd, A_EM_CMD, C_SET); // Sends SET message

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

        if (alarm_counter == llink.numTransmissions)
        {
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

        int n = send_supervision_frame(fd, A_RC_RESP, C_UA); // Sends UA message

        if (n == -1) {
            perror("Failed to send UA message.");
            exit(1);
        }

        printf("Sent UA message.\n");

        printf("Established connection\n");
    }

    return fd;
}