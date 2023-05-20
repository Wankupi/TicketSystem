#pragma once

#include <functional>
#include <string>

class WebSocket {
public:
	WebSocket();
	~WebSocket();
	int bind(const char *addr, int port);
	void start(bool &check_continue);
	void send(int fd, std::string const &msg);
	void close(int fd);

public:
	std::function<void(WebSocket &, int)> on_connect;
	std::function<void(WebSocket &, int, std::string)> on_message;
	std::function<void(WebSocket &, int)> on_close;

private:
	int server_fd = 0;
	int epoll_fd = 0;
};
