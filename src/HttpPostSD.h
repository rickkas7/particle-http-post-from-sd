#ifndef __HTTPPOST_SD
#define __HTTPPOST_SD

#include "Particle.h"
#include "SdFat.h"

class HttpPostSD {
public:
	HttpPostSD(SdFat &sd);
	virtual ~HttpPostSD();

	void setup();
	void loop();

	HttpPostSD &withPath(const char *path) { this->path = path; return *this; }
	HttpPostSD &withHost(const char *host) { this->host = host; return *this;}
	HttpPostSD &withPort(int port) { this->port = port; return *this;}
	HttpPostSD &withPathPartOfUrl(const char *pathPartOfUrl) { this->pathPartOfUrl = pathPartOfUrl; return *this; }
	HttpPostSD &withContentType(const char *contentType) { this->contentType = contentType; return *this; }

	bool start();
	void cleanup();
	void runState();
	void writeState();
	void responseWaitState();

protected:
	static const int IDLE_STATE = 0;
	static const int RUN_STATE = 1;
	static const int WRITE_STATE = 2;
	static const int RESPONSE_WAIT_STATE = 3;

	static const size_t BUFFER_SIZE = 512;

	int state = IDLE_STATE;
	SdFat sd;
	String path;
	String host;
	int port = 80;
	String pathPartOfUrl;
	String contentType = "text/plain";
	File file;
	char buffer[BUFFER_SIZE];
	TCPClient tcpClient;
	size_t fileSize;
	size_t fileOffset;
	size_t count;
	bool success;
};

#endif /* __HTTPPOST_SD */
