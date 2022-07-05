#include <ESP8266WiFi.h>

#define CHANNEL_1 1
#define CHANNEL_2 2
#define CHANNEL_3 3

#define IR_SEND_PIN 0
#define HTTP_SERVER_PORT 80

#define WIFI_SSID ""
#define WIFI_PASS ""

WiFiServer srv(HTTP_SERVER_PORT);

bool channel_1 = false;
bool channel_2 = false;
bool channel_3 = false;

const int32_t signal_for_b1[] = {
    -9200, 4500,
    -650, 550, -600, 500, -650, 550, -650, 550,
    -600, 550, -600, 550, -600, 550, -650, 1600,
    -650, 1600, -600, 1650, -600, 1650, -600, 1650,
    -600, 1600, -650, 1600, -650, 1600, -600, 550,
    -650, 550, -600, 1650, -600, 550, -600, 550,
    -650, 500, -650, 550, -600, 550, -600, 550,
    -650, 1600, -650, 550, -600, 1650, -600, 1600,
    -650, 1600, -600, 1650, -600, 1650, -600, 1650,
    -600, 39350, -9200, 2200, -600};

const int32_t signal_for_b2[] = {
    -9200, 4500,
    -600, 600, -600, 600, -600, 600, -600, 600,
    -600, 600, -600, 600, -600, 600, -600, 1600,
    -600, 1600, -600, 1600, -600, 1600, -600, 1600,
    -600, 1600, -600, 1600, -600, 1600, -600, 600,
    -600, 600, -600, 600, -600, 1600, -600, 600,
    -600, 600, -600, 600, -600, 600, -600, 600,
    -600, 1600, -600, 1600, -600, 600, -600, 1600,
    -600, 1600, -600, 1600, -600, 1600, -600, 1600,
    -600, 39400, -9200, 2200, -600};

const int32_t signal_for_b3[] = {
    -9200, 4500,
    -600, 600, -600, 600, -600, 600, -600, 600,
    -600, 600, -600, 600, -600, 600, -600, 1600,
    -600, 1600, -600, 1600, -600, 1600, -600, 1600,
    -600, 1600, -600, 1600, -600, 1600, -600, 600,
    -600, 600, -600, 1600, -600, 1600, -600, 600,
    -600, 600, -600, 600, -600, 600, -600, 600,
    -600, 1600, -600, 600, -600, 600, -600, 1600,
    -600, 1600, -600, 1600, -600, 1600, -600, 1600,
    -600, 39400, -9200, 2200, -600};

void write_signal(int32_t output_pin, const int32_t signal[], size_t size, bool blink = false)
{
    for (int i = 0; i < size; ++i)
    {
        int32_t timing = signal[i];
        bool result = timing > 0;

        digitalWrite(IR_SEND_PIN, static_cast<uint8_t>(result));
        if (blink)
        {
            digitalWrite(LED_BUILTIN, static_cast<uint8_t>(result));
        }

        delayMicroseconds(abs(timing));

        digitalWrite(IR_SEND_PIN, static_cast<uint8_t>(!result));
        if (blink)
        {
            digitalWrite(LED_BUILTIN, static_cast<uint8_t>(!result));
        }
    }
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(IR_SEND_PIN, OUTPUT);

    // pinMode(CHANNEL_1, FUNCTION_3);
    // pinMode(CHANNEL_3, FUNCTION_3);

    pinMode(CHANNEL_1, INPUT);
    pinMode(CHANNEL_2, INPUT);
    pinMode(CHANNEL_3, INPUT);

    // Serial.begin(115200);
    // Serial.println();

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    // Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        // Serial.print(".");
    }
    // Serial.println();

    // Serial.print("Connected, IP address: ");
    // Serial.println(WiFi.localIP());

    srv.begin();
}

void loop()
{
    // Check if a client has connected
    WiFiClient client = srv.available();
    if (!client)
    {
        return;
    }

    // Wait until the client sends some data
    while (!client.available())
    {
        delay(1);
    }

    // Read the first line of the request
    String request = client.readStringUntil('\r');
    // Serial.println(request);
    client.flush();

    if (request.indexOf("/b1") != -1)
    {
        int size = sizeof(signal_for_b1) / sizeof(signal_for_b1[0]);
        write_signal(IR_SEND_PIN, signal_for_b1, size, true);
    }
    else if (request.indexOf("/b2") != -1)
    {
        int size = sizeof(signal_for_b2) / sizeof(signal_for_b2[0]);
        write_signal(IR_SEND_PIN, signal_for_b2, size, true);
    }
    else if (request.indexOf("/b3") != -1)
    {
        int size = sizeof(signal_for_b3) / sizeof(signal_for_b3[0]);
        write_signal(IR_SEND_PIN, signal_for_b3, size, true);
    }

    // Return the response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: plain/text");
    client.println(""); // headers ends with \r\n\r\n

    channel_1 = digitalRead(CHANNEL_1) == HIGH;
    channel_2 = digitalRead(CHANNEL_2) == HIGH;
    channel_3 = digitalRead(CHANNEL_3) == HIGH;

    client.printf("%d:%d:%d\n", channel_1, channel_2, channel_3);
}
