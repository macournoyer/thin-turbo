#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <sys/types.h>
#include <arpa/inet.h>

#define EV_STANDALONE 1
#include <ev.h>

#include "backend.h"

void thin_start_connection(struct thin_backend *backend, int fd, struct sockaddr_in remote_addr);
void thin_connection_recv(EV_P_ struct ev_io *watcher, int revents);
void thin_connection_send(EV_P_ struct ev_io *watcher, int revents);
void thin_connection_close(EV_P_ struct ev_io *watcher, int revents);

#endif /* _CONNECTION_H_ */
