#include "terminal/TicketTerminal.h"
#include "websocket/WebSocket.h"
#include <condition_variable>
#include <filesystem>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>

ticket::UserManager *userM = nullptr;
ticket::TrainManager *trainM = nullptr;
ticket::BillManager *billM = nullptr;

void startComponent() {
	userM = new ticket::UserManager("users/main.db", "users/index");
	trainM = new ticket::TrainManager("train/main.db", "train/ticket.db", "train/trainID", "train/station");
	billM = new ticket::BillManager("bill/head.db", "bill/main.db", "bill/wait");
}
void stopComponent() {
	delete userM;
	delete trainM;
	delete billM;
}
void cleanComponent() {
	userM->~UserManager();
	trainM->~TrainManager();
	billM->~BillManager();
	std::filesystem::remove_all("users");
	std::filesystem::remove_all("train");
	std::filesystem::remove_all("bill");
	new (userM) ticket::UserManager("users/main.db", "users/index");
	new (trainM) ticket::TrainManager("train/main.db", "train/ticket.db", "train/trainID", "train/station");
	new (billM) ticket::BillManager("bill/head.db", "bill/main.db", "bill/wait");
}

struct Task {
	WebSocket *ws;
	int fd;
	std::string cmd;
};

std::queue<Task> tasks;
std::map<int, ticket::TicketTerminal> terms;
std::mutex task_mutex;
std::condition_variable is_task;

bool web_working = false;
bool worker_working = false;

void worker_thread() {
	while (true) {
		std::unique_lock lock(task_mutex);
		is_task.wait(lock, []() { return !tasks.empty() || !worker_working; });
		if (!worker_working) break;
		Task task = tasks.front();
		tasks.pop();
		lock.unlock();

		auto p = terms.find(task.fd);
		if (p == terms.end()) continue;
		ticket::TicketTerminal &term = p->second;

		std::istringstream in(task.cmd);
		std::ostringstream out;
		static char num[40];
		static char line[4096];
		int stat = 0;
		while (in >> num) {
			in.get();
			in.getline(line, 4096);
			if (!in) break;
			out << num << " ";
			stat = term.run(line, out);

			if (stat == 1) break;
			if (stat == 2) {
				for (auto &x: terms) {
					task.ws->send(x.first, "broadcast: cleaning data.");
					x.second.clean_memory();
				}
				cleanComponent();
			}
		}
		task.ws->send(task.fd, out.str());
		if (stat == 1) {
			task.ws->close(task.fd);
		}
	}
}

void web_thread() {
	WebSocket ws;
	ws.bind("127.0.0.1", 10000);

	ws.on_connect = [&](WebSocket &_ws, int fd) {
		terms.emplace(fd, ticket::TicketTerminal{*userM, *trainM, *billM});
		std::cerr << "new session on " << fd << "." << std::endl;
	};
	ws.on_close = [&](WebSocket &ws, int fd) {
		terms.erase(fd);
		std::cerr << "session " << fd << " closed." << std::endl;
	};
	ws.on_message = [](WebSocket &ws, int fd, std::string str) {
		std::cout << "receive:\n"
				  << str << std::endl;
		std::unique_lock lock(task_mutex);
		tasks.emplace(&ws, fd, std::move(str));
		lock.unlock();
		is_task.notify_all();
	};
	ws.start(web_working);
	ws.on_close = nullptr;// ws.close will change terms, which could cause SegmentFault
	for (auto &x: terms) {
		ws.close(x.first);
	}
	terms.clear();
}

int main() {
	std::ios::sync_with_stdio(false);
	std::cin.tie(0);
	startComponent();
	web_working = true;
	worker_working = true;
	std::thread worker(worker_thread);
	std::thread web(web_thread);
	std::string s;
	while (std::cin >> s) {
		if (s != "stop") continue;
		break;
	}
	std::cout << "stopping working service..." << std::endl;
	worker_working = false;
	is_task.notify_all();
	worker.join();
	std::cout << "worker service has shutdown." << std::endl;

	std::cout << "saving files..." << std::endl;
	stopComponent();
	std::cout << "saved." << std::endl;

	std::cout << "stopping web service..." << std::endl;
	web_working = false;
	web.join();
	std::cout << "web service has shutdown." << std::endl;
	return 0;
}