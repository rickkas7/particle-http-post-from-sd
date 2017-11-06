#include "HttpPostSD.h"


HttpPostSD::HttpPostSD(SdFat &sd) {

}

HttpPostSD::~HttpPostSD() {

}

void HttpPostSD::setup() {
}

void HttpPostSD::loop() {
	switch(state) {
	case IDLE_STATE:
		break;

	case RUN_STATE:
		runState();
		break;

	case WRITE_STATE:
		writeState();
		break;

	case RESPONSE_WAIT_STATE:
		responseWaitState();
		break;
	}
}

void HttpPostSD::cleanup() {
	file.close();
	tcpClient.stop();
	state = IDLE_STATE;
}


bool HttpPostSD::start() {
	if (state != IDLE_STATE) {
		Log.info("can't start, currently in use");
		return false;
	}

	cleanup();
	success = false;

	// Does path exist?
	if (path.length() == 0 || host.length() == 0 || pathPartOfUrl.length() == 0) {
		Log.info("required parameter not set");
		return false;
	}

	if (!file.open(path, O_READ)) {
		Log.info("file does not exist");
		return false;
	}

	fileSize = file.size();

	Log.info("sending %s, size=%d", path.c_str(), fileSize);

	if (!tcpClient.connect(host, port)) {
		cleanup();

		Log.info("unable to connect to %s:%d", host.c_str(), port);

		return false;
	}

	Log.info("connected to %s:%d", host.c_str(), port);

	// Send HTTP header
	count = snprintf(buffer, sizeof(buffer),
			"POST %s HTTP/1.0\015\012"
			"Host: %s\015\012"
			"Content-Length: %u\015\012"
			"Content-Type: %s\015\012"
			"\015\012",
			pathPartOfUrl.c_str(),
			host.c_str(),
			fileSize,
			contentType.c_str()
			);
	tcpClient.write((const uint8_t *)buffer, count);

	fileOffset = 0;
	state = RUN_STATE;
	return true;
}

void HttpPostSD::runState() {
	if (fileOffset >= fileSize) {
		// Done sending body, wait for response code
		state = RESPONSE_WAIT_STATE;
		fileOffset = 0;
		return;
	}

	count = fileSize - fileOffset;
	if (count > BUFFER_SIZE) {
		count = BUFFER_SIZE;
	}

	if (file.read(buffer, count) < 0) {
		Log.info("read failed");
		cleanup();
		return;
	}

	state = WRITE_STATE;
}

void HttpPostSD::writeState() {
	if (!tcpClient.connected()) {
		Log.info("remote side closed connection");
		cleanup();
		return;
	}

	int result = tcpClient.write((const uint8_t *)buffer, count);
	if (result == -16) {
		// Special case: write buffer is full, so just send this buffer again (Photon only)
		return;
	}
	if (result <= 0) {
		Log.info("write failed");
		cleanup();
		return;
	}

	Log.info("sent %d bytes", result);

	// In practice, result == count always on the Photon and Electron. A partial write
	// never occurs, but the code here will handle it, though slightly inefficiently as it
	// will read from flash again. But again, this never happens.

	fileOffset += result;
	state = RUN_STATE;
}

void HttpPostSD::responseWaitState() {
	count = BUFFER_SIZE - fileOffset - 1;

	if (tcpClient.connected()) {
		int result = tcpClient.read((uint8_t *)&buffer[fileOffset], count);
		if (result > 0) {
			fileOffset += result;
		}
	}
	else {
		// Disconnected before response
		Log.info("server disconnected before sending response");
		cleanup();
		return;
	}

	buffer[fileOffset] = 0;
	if (strchr(buffer, '\015') != NULL) {
		// Got a line of response
		char *cp = strchr(buffer, ' ');
		if (cp) {
			if (strncmp(&cp[1], "200", 3) == 0) {
				// Success
				Log.info("success code from server");
				success = true;
			}
			else {
				// Failure
				Log.info("non-success code from server");
			}
		}
		else {
			//
			Log.info("unexpected server response");
		}
		cleanup();
		return;
	}


}


