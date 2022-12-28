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
	static int inot = ERR;
        static int      iflags  = 0;
        if (inot == ERR) {
                inot = inotify_init1(iflags);
                if (inot == ERR) return ERR;
        }
	return inot;
}

int file_watch_fd (int fd, const char * filename ) {
        
	static uint32_t mask    = IN_MODIFY;
        int             watch;

        watch = inotify_add_watch(fd, filename, mask);
        if (watch == ERR) return ERR;
        return  watch;
}

#define EVENT_SIZE  ( sizeof (struct inotify_event) )

#define BUF_LEN     ( 1024 *  EVENT_SIZE + 64)


int read_inotify_events(std::queue<inotify_event *>& q, int fd)
{
	char buffer[BUF_LEN];
	size_t buffer_i, event_size, queue_event_size, length;
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
		//Start searching for \n occurrences
		fs.seekg(-1, std::ios_base::cur);
		for(int i = fs.tellg(); i > 0; i--)
		{
			if(fs.peek() == '\n')
			{
				//Found
				fs.get();
				break;
			}
			//Move one character back
			fs.seekg(i, std::ios_base::beg);
		}
		getline(fs, lastline);
	}
	return lastline;
}

void event_loop(const std::vector<std::string>& files_and_dirs, int timeout, std::function<void(const std::vector<std::string>& files, struct inotify_event& event)> handle_event)
{
	std::vector<int> files_fd;
	int inot = open_init_fd();
	struct pollfd fdset;
	fdset.fd = inot;
	fdset.events = POLLIN;
	fdset.revents = 0;
	
	for(int i = 0; i < files_and_dirs.size(); i++){
		int fw_fd = file_watch_fd(inot, files_and_dirs[i].c_str());
	}
	
	do {
	    	int rc = poll(&fdset, 1, -1);
		if(rc == -1)
		{
			perror("poll() failed");
		}
		printf("events ready\n");
		if(fdset.revents & POLLIN)
		{
			std::queue<inotify_event *> queue;
			if(read_inotify_events(queue, fdset.fd) > 0) 
			{
				while(!queue.empty())
				{
					handle_event(files_and_dirs, *queue.front());
					free(queue.front());
					queue.pop();
				}
			}
		}

    	} while(true);
}

