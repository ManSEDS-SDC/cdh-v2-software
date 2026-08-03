#ifndef CDH_CSP_SERVER_H
#define CDH_CSP_SERVER_H

#include "csp/csp_types.h"
#include <stdint.h>

typedef int (*cdh_csp_service)(csp_packet_t *request, uint8_t reply[256]);

void cdh_csp_service_add(uint8_t port, cdh_csp_service service);

void cdh_csp_server_task(void *argument);

#endif // CDH_CSP_SERVER_H