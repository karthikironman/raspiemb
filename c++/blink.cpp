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
int dht11_dat[5] = {0, 0, 0, 0, 0};

struct Data
{
    time_t timestamp;
    bool state;
    float temperature; // Added temperature field
    float humidity;    // Added humidity field
};

deque<Data> circularBuffer;
const int bufferCapacity = 3600;

void read_dht11_dat()
{
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;
    float f;

    dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;

    pinMode(DHTPIN, OUTPUT);
    digitalWrite(DHTPIN, LOW);
    delay(18);
    digitalWrite(DHTPIN, HIGH);
    delayMicroseconds(40);
    pinMode(DHTPIN, INPUT);

    for (i = 0; i < MAXTIMINGS; i++)
    {
        counter = 0;
        while (digitalRead(DHTPIN) == laststate)
        {
            counter++;
            delayMicroseconds(1);
            if (counter == 255)
            {
                break;
            }
        }
        laststate = digitalRead(DHTPIN);

        if (counter == 255)
            break;

        if ((i >= 4) && (i % 2 == 0))
        {
            dht11_dat[j / 8] <<= 1;
            if (counter > 16)
                dht11_dat[j / 8] |= 1;
            j++;
        }
    }

    if ((j >= 40) &&
        (dht11_dat[4] == ((dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF)))
    {
        float humidity = (float)dht11_dat[0] + (float)dht11_dat[1] / 10.0;
        float temperature = (float)dht11_dat[2] + (float)dht11_dat[3] / 10.0;
        printf("Humidity = %.1f %% Temperature = %.1f C (%.1f F)\n", humidity, temperature, temperature * 9 / 5 + 32);

        // Add temperature and humidity readings to circular buffer
        Data newData;
        newData.timestamp = time(nullptr);
        newData.temperature = temperature;
        newData.humidity = humidity;
        circularBuffer.push_back(newData);

        // Remove oldest data if buffer exceeds capacity
        if (circularBuffer.size() > bufferCapacity)
        {
            circularBuffer.pop_front();
        }
    }
    else
    {
        printf("Data not good, skip\n");
    }
}

void updateState()
{
    bool state = digitalRead(1);

    // Create new data entry
    Data newData;
    newData.timestamp = time(nullptr);
    newData.state = state;

    // Add new data to circular buffer
    circularBuffer.push_back(newData);

    // Remove oldest data if buffer exceeds capacity
    if (circularBuffer.size() > bufferCapacity)
    {
        circularBuffer.pop_front();
    }

    // Toggle the state of GPIO0
    digitalWrite(0, !digitalRead(0));

    // Log the state change
    cout << "state = " << newData.state << ", timestamp = " << newData.timestamp << endl;
}

void serverThread()
{
    Server server;

    // Enable CORS for all endpoints
    
    // Define a lambda function for setting default headers
     auto createDefaultHeaders = []() {
        httplib::Headers headers;
        headers.emplace("Access-Control-Allow-Origin", "*");
        headers.emplace("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        headers.emplace("Access-Control-Allow-Headers", "Content-Type");
        return headers;
    };

    // Set default headers using the function
    server.set_default_headers(createDefaultHeaders());

    server.Get("/latest_state", [](const Request &req, Response &res)
               {
        if (circularBuffer.empty()) {
            res.status = 404;
            res.set_content("No data available", "text/plain");
            cout << "No data available for latest_state endpoint" << endl;
            return;
        }

        const Data& latestData = circularBuffer.back();
        string response = "{";
        response += "\"state\": " + to_string(latestData.state) + ",";
        response += "\"temperature\": " + to_string(latestData.temperature) + ",";
        response += "\"humidity\": " + to_string(latestData.humidity) + ",";
        response += "\"timestamp\": " + to_string(latestData.timestamp);
        response += "}";

        res.set_content(response, "application/json");
        cout << "Request received for latest_state endpoint" << endl; });

    server.Get("/all_data", [](const Request &req, Response &res)
               {
        if (circularBuffer.empty()) {
            res.status = 404;
            res.set_content("No data available", "text/plain");
            cout << "No data available for all_data endpoint" << endl;
            return;
        }

        string response = "[";
        for (const auto& entry : circularBuffer) {
            response += "{ \"timestamp\": " + to_string(entry.timestamp) +
                        ", \"state\": " + to_string(entry.state) +
                        ", \"temperature\": " + to_string(entry.temperature) +
                        ", \"humidity\": " + to_string(entry.humidity) + " },";
        }
        response.pop_back();
        response += "]";

        res.set_content(response, "application/json");
        cout << "Request received for all_data endpoint" << endl; });

    server.listen("0.0.0.0", 8080);
}

int main()
{
    wiringPiSetup();
    pinMode(0, OUTPUT);
    pinMode(1, INPUT);

    cout << "Program started" << endl;

    // Start server in a separate thread
    thread serverThread([]()
                        { ::serverThread(); });

    // Main program loop
    while (1)
    {
        read_dht11_dat(); // Read DHT11 data
        updateState();    // Update state
        delay(1000);
    }

    return 0;
}
