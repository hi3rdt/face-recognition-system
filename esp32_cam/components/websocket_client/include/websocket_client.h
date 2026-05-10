#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

void websocket_client_init(const char *uri);
void websocket_client_send_text(const char *text);

#endif