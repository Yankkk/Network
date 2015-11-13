#ifndef _TCP_H_
#define _TCP_H_


//****************************************************************************************************************
typedef struct _tcp_packet_t
{
   // unsigned short int source_port, dest_port;      //4bytes
    unsigned int seq;                               //4bytes
    unsigned int ack;                               //4bytes
  //  unsigned short int data_offset;                 //2bytes
 //   unsigned char padding;                          //1byte
 //   unsigned char flags;                            //1byte
    //flags & 0x80 = SYN
    //flags & 0x40 = ACK
    //flags & 0x20 = FIN
    unsigned int total;                 //4bytes
  //  unsigned short int check_sum;                   //2bytes
}tcp_t;                                             //sizeof(tcp_t) = 20

#define MSS 1460         //MSS = 1420bytes = 20bytes of header + 1400 of payload
#define SYN_MASK 0x80
#define ACK_MASK 0x40
#define FIN_MASK 0x20

#endif
