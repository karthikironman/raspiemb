#include <iostream>
#include <fstream>
#include <wiringPi.h>
#include <deque>
#include <ctime>
#include <thread> // for std::thread
#include "cpp-httplib/httplib.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

using namespace std;
using namespace httplib;

#define MAXTIMINGS 85
#define DHTPIN 7
int dht11_dat[5] = {0,0,0,0,0};

struct Data {
    time_t timestamp;
    bool state;
};

deque<Data> circularBuffer;
const int bufferCapacity = 3600;


void updateState() {
    bool state = digitalRead(1);

    // Create new data entry
    Data newData;
    newData.timestamp = time(nullptr);
    newData.state = state;

    // Add new data to circular buffer
    circularBuffer.push_back(newData);

    // Remove oldest data if buffer exceeds capacity
    if (circularBuffer.size() > bufferCapacity) {
        circularBuffer.pop_front();
    }

    // Toggle the state of GPIO0
    digitalWrite(0, !digitalRead(0));

    // Log the state change
    cout << "state = " << newData.state << ", timestamp = " << newData.timestamp << endl;
}

void serverThread() {
    Server server;
    
    server.Get("/latest_state", [](const Request& req, Response& res) {
    if (circularBuffer.empty()) {
        res.status = 404;
        res.set_content("No data available", "text/plain");
        cout << "No data available for latest_state endpoint" << endl;
        return;
    }

    const Data& latestData = circularBuffer.back();
    string response = "{";
    response += "\"state\": " + to_string(latestData.state) + ",";
    response += "\"timestamp\": " + to_string(latestData.timestamp);
    response += "}";

    res.set_content(response, "application/json");
    cout << "Request received for latest_state endpoint" << endl;
});
    

    server.Get("/all_data", [](const Request& req, Response& res) {
        if (circularBuffer.empty()) {
            res.status = 404;
            res.set_content("No data available", "text/plain");
            cout << "No data available for all_data endpoint" << endl;
            return;
        }

        string response = "[";
        for (const auto& entry : circularBuffer) {
            response += "{ \"timestamp\": " + to_string(entry.timestamp) +
                        ", \"state\": " + to_string(entry.state) + " },";
        }
        response.pop_back();
        response += "]";

        res.set_content(response, "application/json");
        cout << "Request received for all_data endpoint" << endl;
    });

    server.listen("0.0.0.0", 8080);
}

int main() {
    wiringPiSetup();
    pinMode(0, OUTPUT);
    pinMode(1, INPUT);

    cout << "Program started" << endl;

    // Start server in a separate thread
    thread serverThread([]() { ::serverThread(); });

    // Main program loop
    while (1) {
        updateState();
        delay(1000);
    }

    return 0;
}
