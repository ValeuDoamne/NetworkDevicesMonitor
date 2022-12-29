#pragma once
#include <queue>
#include <vector>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <poll.h>
#include <functional>
#include <cstring>
#include <fstream>

#define ERR -1

int open_init_fd()
{
	static int inot    = ERR;
	static int iflags  = 0;
	if (inot == ERR) {
		inot = inotify_init1(iflags);
		if (inot == ERR)
		{
			perror("Cannot initialize inotify");
		}
	}
	return inot;
}

void file_watch_fd (int fd, const char *filename)
{

	static uint32_t mask = IN_MODIFY;
	int             watch;

	watch = inotify_add_watch(fd, filename, mask);
	if (watch == ERR)
	{
		perror("Cannot file/dir to watch");
	}
}

#define EVENT_SIZE  (sizeof (struct inotify_event))

#define BUF_LEN     (1024 *  EVENT_SIZE + 64)


int read_inotify_events(std::queue<inotify_event *>& q, int fd)
{
	char buffer[BUF_LEN];
	size_t buffer_i, event_size, length;
	int count = 0;
	struct inotify_event *pevent;

	length = read(fd, buffer, BUF_LEN);
	if(length <= 0)
		return length;
	buffer_i = 0;
	for(buffer_i = 0; buffer_i < length; buffer_i += sizeof(struct inotify_event) + event_size)
	{
		pevent = (struct inotify_event *)&buffer[buffer_i];

		struct inotify_event *queue_entry = (struct inotify_event *)malloc(sizeof(struct inotify_event) + pevent->len);
		memcpy(queue_entry, pevent, sizeof(struct inotify_event) + pevent->len);
		q.push(queue_entry);

		event_size = pevent->len;
		count++;
	}
	return count;
}

std::string get_last_line(const std::string& filename)
{
	std::string lastline = "";
	std::ifstream fs{filename};
	if(fs.is_open())
	{
		fs.seekg(-1, std::ios_base::end);
		fs.seekg(-1, std::ios_base::cur);
		for(int i = fs.tellg(); i > 0; i--)
		{
			if(fs.peek() == '\n')
			{
				fs.get();
				break;
			}
			//Move one character back
			fs.seekg(i, std::ios_base::beg);
		}
		std::getline(fs, lastline);
	}
	return lastline;
}

std::string get_filename_from_event(const std::vector<std::string>& files_and_dirs, const inotify_event& event)
{
	std::string filename = "";
	if(event.len > 0)
	{
		filename = files_and_dirs[event.wd-1] + "/" + event.name;
	} else {
		filename = files_and_dirs[event.wd-1];
	}
	return filename;
}

void handle_events(const std::vector<std::string> files_and_dirs, std::queue<inotify_event*>& events_queue, std::function<void(const std::string&)> handle_event)
{
	while(!events_queue.empty())
	{
		auto &event = *events_queue.front();
		handle_event(get_filename_from_event(files_and_dirs, event));
		free(events_queue.front());
		events_queue.pop();
	}
}

void event_loop(const std::vector<std::string>& files_and_dirs, int timeout, std::function<void(const std::string& file)> handle_event)
{
	std::vector<int> files_fd;
	struct pollfd fdset;
	int inot = open_init_fd();

	fdset.fd = inot;
	fdset.events = POLLIN;
	fdset.revents = 0;

	for(size_t i = 0; i < files_and_dirs.size(); i++){
		file_watch_fd(inot, files_and_dirs[i].c_str());
	}

	do {
		int rc = poll(&fdset, 1, timeout);
		if(rc == -1)
		{
			perror("poll() failed");
		}
		if(fdset.revents & POLLIN)
		{
			std::queue<inotify_event *> queue;
			if(read_inotify_events(queue, fdset.fd) > 0)
			{
				handle_events(files_and_dirs, queue, handle_event);
			}
		}

	} while(true);
}

