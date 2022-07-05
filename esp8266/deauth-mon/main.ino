#include <EncButton.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <EEPROM.h>

#define BUTTON_PIN 0
#define OLED_RESET 16       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define WIFI_MAX_CHANNEL 14
#define LineVal 24

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long pkts = 0;
unsigned long deauths = 0;
unsigned long maxVal = 0;
int current_channel = 1;
double multiplicator = 0.0;

unsigned long prevTime = 0;
unsigned long curTime = 0;

unsigned int val[128];

EncButton<EB_TICK, BUTTON_PIN> button;

void sniffer(uint8_t *buf, uint16_t len)
{
    pkts++;
    if (buf[12] == 0xA0 || buf[12] == 0xC0)
    {
        deauths++;
    }
}

void recalclulates_scaling()
{
    maxVal = 1;
    for (int i = 0; i < SCREEN_WIDTH - 1; i++)
    {
        if (val[i] > maxVal)
            maxVal = val[i];
    }
    if (maxVal > LineVal)
        multiplicator = (double)LineVal / (double)maxVal;
    else
        multiplicator = 1;
}

#define LOGO_HEIGHT 16
#define LOGO_WIDTH 16 * 3

static const unsigned char PROGMEM logo_bmp[] =
    {
        0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
        0b00000000, 0b00000000, 0b00000111, 0b11100000, 0b00000000, 0b00000000,
        0b00000000, 0b00000000, 0b00011101, 0b10111000, 0b00000000, 0b00000000,
        0b00000000, 0b00000000, 0b01110000, 0b00001110, 0b00000000, 0b00000000,
        0b00000000, 0b00000011, 0b11000000, 0b00000011, 0b11000000, 0b00000000,
        0b00000000, 0b00001111, 0b00000111, 0b11100000, 0b11110000, 0b00000000,
        0b00000000, 0b00011100, 0b00011100, 0b00111000, 0b00111000, 0b00000000,
        0b00000000, 0b00110000, 0b11110001, 0b10001111, 0b00001100, 0b00000000,
        0b00000000, 0b00000000, 0b00000111, 0b11100000, 0b00000000, 0b00000000,
        0b00000000, 0b00000000, 0b00000011, 0b11000000, 0b00000000, 0b00000000,
        0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
        0b00000011, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11000000,
        0b00000011, 0b11000011, 0b00001100, 0b00110000, 0b11000011, 0b11000000,
        0b00000011, 0b11000011, 0b00001100, 0b00110000, 0b11000011, 0b11000000,
        0b00000011, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11000000,
        0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000};

void setup()
{
    Serial.begin(9600);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
    // Clear the buffer
    display.clearDisplay();

    display.drawBitmap(
        (display.width() - LOGO_WIDTH) / 2,
        (display.height() - LOGO_HEIGHT) / 2,
        logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);

    display.display();
    delay(2000); // Pause for 2 seconds

    /* setup wifi */
    wifi_set_opmode(STATION_MODE);
    wifi_promiscuous_enable(0);
    WiFi.disconnect();
    wifi_set_promiscuous_rx_cb(sniffer);
    wifi_set_channel(current_channel);
    wifi_promiscuous_enable(1);
}

void loop()
{
    button.tick();

    curTime = millis();

    // if (button.hasClicks(1))
    if (button.step())
    {
        current_channel++;
        if (current_channel > WIFI_MAX_CHANNEL)
        {
            current_channel = 1;
        }

        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);

        display.setCursor(0, 0);
        display.print("# ");
        display.print(pkts);

        display.setCursor(128 - (5 * 12), 0);
        display.print("! ");
        display.print(deauths);

        display.setCursor(128 - (5 * 5), 0);
        display.print("@ ");
        display.print(current_channel);

        wifi_set_channel(current_channel);
        for (int i = 0; i < SCREEN_WIDTH - 1; i++)
        {
            val[i] = 0;
        }

        display.display();

        pkts = 0;
        deauths = 0;

        return;
    }

    if (curTime - prevTime >= 1000)
    {
        prevTime = curTime;

        //move every packet bar one pixel to the left
        for (int i = 0; i < SCREEN_WIDTH - 1; i++)
        {
            val[i] = val[i + 1];
        }
        val[SCREEN_WIDTH - 1] = pkts;

        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);

        display.setCursor(0, 0);
        display.print("# ");
        display.print(pkts);

        display.setCursor(128 - (5 * 12), 0);
        display.print("! ");
        display.print(deauths);

        display.setCursor(128 - (5 * 5), 0);
        display.print("@ ");
        display.print(current_channel);

        recalclulates_scaling();

        for (int i = 0; i < SCREEN_WIDTH - 1; ++i)
        {
            display.drawLine(i, SCREEN_HEIGHT - 1, i, (SCREEN_HEIGHT - 1) - val[i] * multiplicator, SSD1306_WHITE);
        }

        display.display();
        pkts = deauths = 0;
    }
}
