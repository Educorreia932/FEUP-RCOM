#define FLAG 0x7E

#define A_EM_CMD 0x03
#define A_RC_RESP 0x03
#define A_RC_CMD 0x01
#define A_EM_RESP 0x01

#define C_SET 0x03
#define C_UA 0x07

int send_trama(int fd, char a, char c){
  unsigned char buf[5];

  buf[0] = FLAG;
  buf[1] = a;
  buf[2] = c;
  buf[3] = a ^ c; // BCC
  buf[4] = FLAG;

  return write(fd, buf, 5);
}
