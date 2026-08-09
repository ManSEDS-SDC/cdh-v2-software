

#include "cdh_csp_server.h"
#include "csp/csp.h"
#include <stdint.h>

typedef struct port_service_s{ 
    uint8_t port; 
    cdh_csp_service service;
} port_service_t; 

port_service_t services[16] = {0};

void cdh_csp_service_add(uint8_t port, cdh_csp_service service)
{
    for (int i = 0; i < 16; i++)
    {
        if (services[i].service == 0)
        {
            services[i].port = port;
            services[i].service = service;

            break;
        }
    }
}

void cdh_csp_server_task(void *argument)
{
    /* Create socket with no specific socket options, e.g. accepts CRC32, HMAC, etc. if enabled during compilation */
	csp_socket_t sock = {0};

	/* Bind socket to all ports, e.g. all incoming connections will be handled here */
	csp_bind(&sock, CSP_ANY);

	/* Create a backlog of 10 connections, i.e. up to 10 new connections can be queued */
	csp_listen(&sock, 10);

    /* Wait for connections and then process packets on the connection */
	while (1)
    {
		/* Wait for a new connection, 100 mS timeout */
		csp_conn_t *conn;
		if ((conn = csp_accept(&sock, 100)) == NULL)
        {
			/* timeout */
			continue;
		}
        
        /* Read packets on connection, default is 100 ms */
        csp_packet_t *packet;
        while ((packet = csp_read(conn, 100)) != NULL)
        {
            int dport = csp_conn_dport(conn);
            
            uint8_t buf[256];

            for (int i = 0; i < 16; i++)
            {
                if (services[i].port == dport && services[i].service)
                {
                    // callback
                    int buf_len = (services[i].service)(packet, buf);

                    // an error occoured (should not happen)
                    if (buf_len == -1){
                        // free packet
                        csp_buffer_free(packet);
                        packet = NULL;

                        break;
                    }

                    memcpy(packet->data, buf, buf_len);

                    csp_sendto_reply(packet, packet, CSP_O_SAME);

                    packet = NULL;
                    break;
                }
            }
            
            if (packet != NULL)
            {
                /* Call the default CSP service handler, handle pings, buffer use, etc. */
                csp_service_handler(packet);
            }
        }

        /* Close current connection */
        csp_close(conn);
	}
}