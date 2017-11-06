#include "Particle.h"

#include "SdFat.h"
#include "HttpPostSD.h"

SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler;

// This is set up to use primary SPI with DMA
// SD Adapter   Electron
// SS (CS)      A2
// SCK          A3
// MISO (DO)    A4
// MOSI (DI)    A5
SdFat sd;
HttpPostSD postSD(sd);

const unsigned long CARD_CHECK_PERIOD = 30000;
unsigned long lastCardCheck = 5000 - CARD_CHECK_PERIOD;

void checkCardForUpdates();

void setup() {
	Serial.begin(9600);

	postSD.setup();
}

void loop() {
	postSD.loop();

	if (millis() - lastCardCheck >= CARD_CHECK_PERIOD) {
		lastCardCheck = millis();
		checkCardForUpdates();
	}
}


void checkCardForUpdates() {
	if (!sd.begin(A2, SPI_HALF_SPEED)) {
		Serial.println("no card");
		return;
	}

	postSD.withPath("DATA.TXT");
	postSD.withHost("server.example.com").withPort(8087).withPathPartOfUrl("/upload");

	postSD.start();
}

