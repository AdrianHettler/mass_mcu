#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"

#include "ethernet_defines.h"
#include "..//message_ids.h"

static struct tcp_pcb *tcp_echoserver_pcb;

/* ECHO protocol states */
enum tcp_echoserver_states
{
  ES_NONE = 0,
  ES_ACCEPTED,
  ES_RECEIVED,
  ES_CLOSING
};

/* structure for maintaing connection infos to be passed as argument
   to LwIP callbacks*/
struct tcp_echoserver_struct
{
  u8_t state;             /* current connection state */
  struct tcp_pcb *pcb;    /* pointer on the current tcp_pcb */
  struct pbuf *p;         /* pointer on the received/to be transmitted pbuf */
};


static err_t tcp_echoserver_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t tcp_echoserver_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void tcp_echoserver_error(void *arg, err_t err);
static err_t tcp_echoserver_poll(void *arg, struct tcp_pcb *tpcb);
static err_t tcp_echoserver_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void tcp_echoserver_send(struct tcp_pcb *tpcb, struct tcp_echoserver_struct *es);
static void tcp_echoserver_connection_close(struct tcp_pcb *tpcb, struct tcp_echoserver_struct *es);


/**
  * @brief  Initializes the tcp echo server
  * @param  None
  * @retval None
  */
void tcp_echoserver_init(void)
{
  /* create new tcp pcb */
  tcp_echoserver_pcb = tcp_new();

  if (tcp_echoserver_pcb != NULL)
  {
    err_t err;

    /* bind echo_pcb to port 7 (ECHO protocol) */
    err = tcp_bind(tcp_echoserver_pcb, IP_ADDR_ANY, TCP_uplink_PORT);

    if (err == ERR_OK)
    {
      /* start tcp listening for echo_pcb */
      tcp_echoserver_pcb = tcp_listen(tcp_echoserver_pcb);

      /* initialize LwIP tcp_accept callback function */
      tcp_accept(tcp_echoserver_pcb, tcp_echoserver_accept);
    }
    else
    {
      /* deallocate the pcb */
      memp_free(MEMP_TCP_PCB, tcp_echoserver_pcb);
    }
  }
}

/**
  * @brief  This function is the implementation of tcp_accept LwIP callback
  * @param  arg: not used
  * @param  newpcb: pointer on tcp_pcb struct for the newly created tcp connection
  * @param  err: not used
  * @retval err_t: error status
  */
static err_t tcp_echoserver_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  err_t ret_err;
  struct tcp_echoserver_struct *es;

  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);

  /* set priority for the newly accepted tcp connection newpcb */
  tcp_setprio(newpcb, TCP_PRIO_MIN);

  /* allocate structure es to maintain tcp connection informations */
  es = (struct tcp_echoserver_struct *)mem_malloc(sizeof(struct tcp_echoserver_struct));
  if (es != NULL)
  {
    es->state = ES_ACCEPTED;
    es->pcb = newpcb;
    es->p = NULL;

    /* pass newly allocated es structure as argument to newpcb */
    tcp_arg(newpcb, es);

    /* initialize lwip tcp_recv callback function for newpcb  */
    tcp_recv(newpcb, tcp_echoserver_recv);

    /* initialize lwip tcp_err callback function for newpcb  */
    tcp_err(newpcb, tcp_echoserver_error);

    /* initialize lwip tcp_poll callback function for newpcb */
    tcp_poll(newpcb, tcp_echoserver_poll, 1);

    ret_err = ERR_OK;
  }
  else
  {
    /*  close tcp connection */
    tcp_echoserver_connection_close(newpcb, es);
    /* return memory error */
    ret_err = ERR_MEM;
  }
  return ret_err;
}


/**
  * @brief  This function is the implementation for tcp_recv LwIP callback
  * @param  arg: pointer on a argument for the tcp_pcb connection
  * @param  tpcb: pointer on the tcp_pcb connection
  * @param  pbuf: pointer on the received pbuf
  * @param  err: error information regarding the reveived pbuf
  * @retval err_t: error code
  */

 struct message
        {

             ushort length;
             char message_id;
        };
#include <stdio.h>
#include <string.h>
ushort test = 0;
static err_t tcp_echoserver_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  struct tcp_echoserver_struct *es;
  err_t ret_err;

  LWIP_ASSERT("arg != NULL",arg != NULL);

  es = (struct tcp_echoserver_struct *)arg;

  /* if we receive an empty tcp frame from client => close connection */
  if (p == NULL)
  {
    /* remote host closed connection */
    es->state = ES_CLOSING;
    if(es->p == NULL)
    {
       /* we're done sending, close connection */
       tcp_echoserver_connection_close(tpcb, es);
    }
    else
    {
      /* we're not done yet */
      /* acknowledge received packet */
      tcp_sent(tpcb, tcp_echoserver_sent);

      /* send remaining data*/
      tcp_echoserver_send(tpcb, es);
    }
    ret_err = ERR_OK;
  }
  /* else : a non empty frame was received from client but for some reason err != ERR_OK */
  else if(err != ERR_OK)
  {
    /* free received pbuf*/
    if (p != NULL)
    {
      es->p = NULL;
      pbuf_free(p);
    }
    ret_err = err;
  }
  else if(es->state == ES_ACCEPTED)
  {
    /* first data chunk in p->payload */
    es->state = ES_RECEIVED;

    /* store reference to incoming pbuf (chain) */
    es->p = p;


    /* initialize LwIP tcp_sent callback function */
    tcp_sent(tpcb, tcp_echoserver_sent);

    /* send back the received data (echo) */
    tcp_echoserver_send(tpcb, es);

    ret_err = ERR_OK;
  }
  else if (es->state == ES_RECEIVED)
  {
    /* more data received from client and previous data has been already sent*/
    if(es->p == NULL)
    {
      es->p = p;

			char data_received[100] = { }; //payload buffer
			int data_size = p->len;

			if (data_size <= sizeof(data_received)) {
				memcpy(&data_received, p->payload, data_size); //copy tcp payload in buffer

				char message_id = data_received[0]; //first byte is message id

				char data_send[10] = { };
				switch (message_id) {
				case tcp_ping:


					data_send[0] = tcp_pong;

					es->p->payload = (void*) &data_send;
					//es->p->len = 2;

					/* send back received data */
					tcp_echoserver_send(tpcb, es);

					break;

				case exp_release_structures:


					data_send[0] = tcp_ok;
					data_send[1] = message_id;

					es->p->payload = (void*) &data_send;
					es->p->len = 2;
					es->p->tot_len = 2;

					/* send back received data */
					tcp_echoserver_send(tpcb, es);

					break;

				case exp_uv_on:


					if(data_received[1] == 1)
					{
						HAL_GPIO_WritePin(GPIO_LED_1_GPIO_Port, GPIO_LED_1_Pin, GPIO_PIN_SET);
					}
					else if(data_received[1] == 2)
					{
						HAL_GPIO_WritePin(GPIO_LED_2_GPIO_Port, GPIO_LED_2_Pin, GPIO_PIN_SET);
					}

					data_send[0] = tcp_ok;
					data_send[1] = message_id;

					es->p->payload = (void*) &data_send;
					es->p->len = 2;
					es->p->tot_len = 2;

					/* send back received data */
					tcp_echoserver_send(tpcb, es);

					break;

				case exp_uv_off:

					if (data_received[1] == 1) {
						HAL_GPIO_WritePin(GPIO_LED_1_GPIO_Port, GPIO_LED_1_Pin,
								GPIO_PIN_RESET);
					} else if (data_received[1] == 2) {
						HAL_GPIO_WritePin(GPIO_LED_2_GPIO_Port, GPIO_LED_2_Pin,
								GPIO_PIN_RESET);
					}

					data_send[0] = tcp_ok;
					data_send[1] = message_id;

					es->p->payload = (void*) &data_send;
					es->p->len = 2;
					es->p->tot_len = 2;

					/* send back received data */
					tcp_echoserver_send(tpcb, es);

					break;

				case exp_valves_manual_on:

					if (data_received[1] == 1) {
						HAL_GPIO_WritePin(GPIO_V1_GPIO_Port, GPIO_V1_Pin,
								GPIO_PIN_SET);
					} else if (data_received[1] == 2) {
						HAL_GPIO_WritePin(GPIO_V2_GPIO_Port, GPIO_V2_Pin,
								GPIO_PIN_SET);
					} else if (data_received[1] == 3) {
						HAL_GPIO_WritePin(GPIO_V3_GPIO_Port, GPIO_V3_Pin,
								GPIO_PIN_SET);
					} else if (data_received[1] == 4) {
						HAL_GPIO_WritePin(GPIO_V4_GPIO_Port, GPIO_V4_Pin,
								GPIO_PIN_SET);
					}

					data_send[0] = tcp_ok;
					data_send[1] = message_id;

					es->p->payload = (void*) &data_send;
					es->p->len = 2;
					es->p->tot_len = 2;

					/* send back received data */
					tcp_echoserver_send(tpcb, es);

					break;

				case exp_valves_manual_off:

					if (data_received[1] == 1) {
						HAL_GPIO_WritePin(GPIO_V1_GPIO_Port, GPIO_V1_Pin,
								GPIO_PIN_RESET);
					} else if (data_received[1] == 2) {
						HAL_GPIO_WritePin(GPIO_V2_GPIO_Port, GPIO_V2_Pin,
								GPIO_PIN_RESET);
					} else if (data_received[1] == 3) {
						HAL_GPIO_WritePin(GPIO_V3_GPIO_Port, GPIO_V3_Pin,
								GPIO_PIN_RESET);
					} else if (data_received[1] == 4) {
						HAL_GPIO_WritePin(GPIO_V4_GPIO_Port, GPIO_V4_Pin,
								GPIO_PIN_RESET);
					}

					data_send[0] = tcp_ok;
					data_send[1] = message_id;

					es->p->payload = (void*) &data_send;
					es->p->len = 2;
					es->p->tot_len = 2;

					/* send back received data */
					tcp_echoserver_send(tpcb, es);

					break;

				case exp_start_inflation:

					if (data_received[1] == 1) {
						HAL_GPIO_WritePin(GPIO_V1_GPIO_Port, GPIO_V1_Pin,
								GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIO_V2_GPIO_Port, GPIO_V2_Pin,
														GPIO_PIN_SET);
					} else if (data_received[1] == 2) {
						HAL_GPIO_WritePin(GPIO_V3_GPIO_Port, GPIO_V3_Pin,
								GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIO_V4_GPIO_Port, GPIO_V4_Pin,
														GPIO_PIN_SET);
					}

					data_send[0] = tcp_ok;
					data_send[1] = message_id;

					es->p->payload = (void*) &data_send;
					es->p->len = 2;
					es->p->tot_len = 2;

					/* send back received data */
					tcp_echoserver_send(tpcb, es);

					break;

				case exp_stop_inflation:

					if (data_received[1] == 1) {
						HAL_GPIO_WritePin(GPIO_V1_GPIO_Port, GPIO_V1_Pin,
								GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIO_V2_GPIO_Port, GPIO_V2_Pin,
								GPIO_PIN_RESET);
					} else if (data_received[1] == 2) {
						HAL_GPIO_WritePin(GPIO_V3_GPIO_Port, GPIO_V3_Pin,
								GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIO_V4_GPIO_Port, GPIO_V4_Pin,
								GPIO_PIN_RESET);
					}

					data_send[0] = tcp_ok;
					data_send[1] = message_id;

					es->p->payload = (void*) &data_send;
					es->p->len = 2;
					es->p->tot_len = 2;

					/* send back received data */
					tcp_echoserver_send(tpcb, es);

					break;




				}

			}

		} else {
			struct pbuf *ptr;

			/* chain pbufs to the end of what we recv'ed previously  */
			ptr = es->p;
			pbuf_chain(ptr, p);
    }
    ret_err = ERR_OK;
  }

  /* data received when connection already closed */
  else
  {
    /* Acknowledge data reception */
    tcp_recved(tpcb, p->tot_len);

    /* free pbuf and do nothing */
    es->p = NULL;
    pbuf_free(p);
    ret_err = ERR_OK;
  }
  return ret_err;
}

/**
  * @brief  This function implements the tcp_err callback function (called
  *         when a fatal tcp_connection error occurs.
  * @param  arg: pointer on argument parameter
  * @param  err: not used
  * @retval None
  */
static void tcp_echoserver_error(void *arg, err_t err)
{
  struct tcp_echoserver_struct *es;

  LWIP_UNUSED_ARG(err);

  es = (struct tcp_echoserver_struct *)arg;
  if (es != NULL)
  {
    /*  free es structure */
    mem_free(es);
  }
}

/**
  * @brief  This function implements the tcp_poll LwIP callback function
  * @param  arg: pointer on argument passed to callback
  * @param  tpcb: pointer on the tcp_pcb for the current tcp connection
  * @retval err_t: error code
  */
static err_t tcp_echoserver_poll(void *arg, struct tcp_pcb *tpcb)
{
  err_t ret_err;
  struct tcp_echoserver_struct *es;

  es = (struct tcp_echoserver_struct *)arg;
  if (es != NULL)
  {
    if (es->p != NULL)
    {
      /* there is a remaining pbuf (chain) , try to send data */
      tcp_echoserver_send(tpcb, es);
    }
    else
    {
      /* no remaining pbuf (chain)  */
      if(es->state == ES_CLOSING)
      {
        /*  close tcp connection */
        tcp_echoserver_connection_close(tpcb, es);
      }
    }
    ret_err = ERR_OK;
  }
  else
  {
    /* nothing to be done */
    tcp_abort(tpcb);
    ret_err = ERR_ABRT;
  }
  return ret_err;
}

/**
  * @brief  This function implements the tcp_sent LwIP callback (called when ACK
  *         is received from remote host for sent data)
  * @param  None
  * @retval None
  */

static err_t tcp_echoserver_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  struct tcp_echoserver_struct *es;

  LWIP_UNUSED_ARG(len);

  es = (struct tcp_echoserver_struct *)arg;

  if(es->p != NULL)
  {
    /* still got pbufs to send */
    tcp_echoserver_send(tpcb, es);
  }
  else
  {
    /* if no more data to send and client closed connection*/
    if(es->state == ES_CLOSING)
      tcp_echoserver_connection_close(tpcb, es);
  }
  return ERR_OK;
}


/**
  * @brief  This function is used to send data for tcp connection
  * @param  tpcb: pointer on the tcp_pcb connection
  * @param  es: pointer on echo_state structure
  * @retval None
  */
static void tcp_echoserver_send(struct tcp_pcb *tpcb, struct tcp_echoserver_struct *es)
{
  struct pbuf *ptr;
  err_t wr_err = ERR_OK;

  while ((wr_err == ERR_OK) &&
         (es->p != NULL) &&
         (es->p->len <= tcp_sndbuf(tpcb)))
  {

    /* get pointer on pbuf from es structure */
    ptr = es->p;

    /* enqueue data for transmission */
    wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);

    if (wr_err == ERR_OK)
    {
      u16_t plen;

      plen = ptr->len;

      /* continue with next pbuf in chain (if any) */
      es->p = ptr->next;

      if(es->p != NULL)
      {
        /* increment reference count for es->p */
        pbuf_ref(es->p);
      }

      /* free pbuf: will free pbufs up to es->p (because es->p has a reference count > 0) */
      pbuf_free(ptr);

      /* Update tcp window size to be advertized : should be called when received
      data (with the amount plen) has been processed by the application layer */
      tcp_recved(tpcb, plen);
   }
   else if(wr_err == ERR_MEM)
   {
      /* we are low on memory, try later / harder, defer to poll */
     es->p = ptr;
   }
   else
   {
     /* other problem ?? */
   }
  }
}

/**
  * @brief  This functions closes the tcp connection
  * @param  tcp_pcb: pointer on the tcp connection
  * @param  es: pointer on echo_state structure
  * @retval None
  */
static void tcp_echoserver_connection_close(struct tcp_pcb *tpcb, struct tcp_echoserver_struct *es)
{

  /* remove all callbacks */
  tcp_arg(tpcb, NULL);
  tcp_sent(tpcb, NULL);
  tcp_recv(tpcb, NULL);
  tcp_err(tpcb, NULL);
  tcp_poll(tpcb, NULL, 0);

  /* delete es structure */
  if (es != NULL)
  {
    mem_free(es);
  }

  /* close tcp connection */
  tcp_close(tpcb);
}


