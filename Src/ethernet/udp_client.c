#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>

#include "ethernet_defines.h"

struct ip_addr {
  u32_t addr;
};

void udp_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);

u8_t data[300];
__IO uint32_t message_count = 0;
struct udp_pcb *upcb;


/**
  * @brief  Connect to UDP echo server
  * @param  None
  * @retval None
  */
void udp_echoclient_connect(void)
{
  struct ip_addr DestIPaddr;
  err_t err;

  /* Create a new UDP control block  */
  upcb = udp_new();

  if (upcb!=NULL)
  {
    /*assign destination IP address */
    IP4_ADDR( &DestIPaddr, DestIP0, DestIP1,DestIP2, DestIP3 );

    /* configure destination IP address and port */
    err= udp_connect(upcb, &DestIPaddr, UDP_downlink_PORT);

    if (err == ERR_OK)
    {
      /* Set a receive callback for the upcb */
      udp_recv(upcb, udp_receive_callback, NULL);
    }
  }
}

/**
  * @brief This function is called when an UDP datagrm has been received on the port UDP_PORT.
  * @param arg user supplied argument (udp_pcb.recv_arg)
  * @param pcb the udp_pcb which received data
  * @param p the packet buffer that was received
  * @param addr the remote IP address from which the packet was received
  * @param port the remote port from which the packet was received
  * @retval None
  */


void udp_echoclient_send(char message_id,uint32_t* data_array, uint32_t size)
{
  struct pbuf *p;

  if(size < sizeof(data)-1) //check size to avoid buffer overflow
  {
	  memset(&data,0,sizeof(data)); //set data array to zero
	  data[0] = (char)message_id; //set message id byte
	  memcpy(&data[1],data_array,size+1); //copy data to array


	  /* allocate pbuf from pool*/
	 p = pbuf_alloc(PBUF_TRANSPORT,size+1, PBUF_POOL);

	  if (p != NULL)
	  {
	    /* copy data to pbuf */
	    pbuf_take(p, (char*)data, size+1);

	    /* send udp data */
	    udp_send(upcb, p);

	    /* free pbuf */
	    pbuf_free(p);
	  }
  }
}

/**
  * @brief This function is called when an UDP datagrm has been received on the port UDP_PORT.
  * @param arg user supplied argument (udp_pcb.recv_arg)
  * @param pcb the udp_pcb which received data
  * @param p the packet buffer that was received
  * @param addr the remote IP address from which the packet was received
  * @param port the remote port from which the packet was received
  * @retval None
  */
void udp_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{

  /*increment message count */
  message_count++;

/*
  char test_array[90] = {};


         memcpy(&test_array,p->payload,p->len);


         int message_id = (int)test_array[0];

         switch(message_id)
         {
           case 2:

         	  test_array[0] = (char)0x3;
         	 udp_echoclient_send();

             break;
         }
*/



  /* Free receive pbuf */
  pbuf_free(p);
}
