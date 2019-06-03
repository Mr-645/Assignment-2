#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define UP_BUTTON 2
#define DOWN_BUTTON 3
#define ledPin 13 //LED builtin pin for showing when the debounce function is ignoring button presses

uint32_t debounceDelay = 50;    //Debounce time - Switch ignore time
uint32_t lastDebounceTime = 0;  //This will store the last time the LED was updated
bool debounceDelayDone = false; //Whether or not the debounce delay has occurred

const unsigned long PADDLE_RATE = 33;
const unsigned long BALL_RATE = 16;
const uint8_t PADDLE_HEIGHT = 24;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t ball_x = 64, ball_y = 32;
uint8_t ball_dir_x = 1, ball_dir_y = 1;
unsigned long ball_update;

unsigned long paddle_update;
const uint8_t CPU_X = 12;
uint8_t cpu_y = 16;

const uint8_t PLAYER_X = 115;
uint8_t player_y = 16;

static bool up_state = false;
static bool down_state = false;

enum States
{
    moving_up,
    moving_down,
    Idle
};
States current_state = Idle;

//Function to handle move_up functionality
void moveUpFunc()
{
    if (debounceDelayDone == true)
    {
        //If debounceDelayDone is true, do the following
        current_state = moving_up;
        lastDebounceTime = millis(); //Reset last debounce time
    }
}

//Function to handle move_down functionality
void moveDownFunc()
{
    if (debounceDelayDone == true)
    {
        //If debounceDelayDone is true, do the following
        current_state = moving_down;
        lastDebounceTime = millis(); //Reset last debounce time
    }
}

void setup()
{
    Serial.begin(9600);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    { // Address 0x3D for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }

    pinMode(ledPin, OUTPUT);
    pinMode(UP_BUTTON, INPUT_PULLUP);
    pinMode(DOWN_BUTTON, INPUT_PULLUP);

    //Run the moveUpFunc function when the pin goes low
    attachInterrupt(digitalPinToInterrupt(UP_BUTTON), moveUpFunc, FALLING);

    //Run the moveDownFunc function when the pin goes low
    attachInterrupt(digitalPinToInterrupt(DOWN_BUTTON), moveDownFunc, FALLING);

    // Display splash screen
    display.clearDisplay();
    display.setTextSize(1.9); // Draw 2X-scale text
    display.setTextColor(WHITE);
    display.setCursor(10, 10);
    display.println("Loading...");
    display.display();
    delay(2000);

    display.clearDisplay();
    display.drawRect(0, 0, 128, 64, WHITE);

    ball_update = millis();
    paddle_update = ball_update;
}

void loop()
{
    unsigned long time = millis();
    //------------------------------------------------------------
    // Switch case functions and main timing stuff
    switch (current_state)
    {
        //State 0
    case moving_up:
        if (digitalRead(UP_BUTTON) == LOW)
        {
            up_state = HIGH;
            current_state = moving_up;
        }
        else
        {
            up_state = LOW;
            current_state = Idle;
        }
        // current_state = moving_up;
        display.display();
        break;
        //State 1
    case moving_down:
        if (digitalRead(DOWN_BUTTON) == LOW)
        {
            down_state = HIGH;
            current_state = moving_down;
        }
        else
        {
            down_state = LOW;
            current_state = Idle;
        }
        display.display();
        break;
        //State 2
    case Idle:
        current_state = Idle;
        display.display();
        break;
    default:
        // statements
        break;
    }
    //------------------------------------------------------------
    /*
    //------------------------------------------------------------
    // Display number on screen
    display.setTextSize(1); // Draw 2X-scale text
    display.setTextColor(WHITE);
    display.setCursor(64, 5);
    display.println(current_state);
    display.display();
    //------------------------------------------------------------
    */
    if (time > ball_update)
    {
        uint8_t new_x = ball_x + ball_dir_x;
        uint8_t new_y = ball_y + ball_dir_y;

        // Check if we hit the vertical walls
        if (new_x == 0 || new_x == 127)
        {
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        // Check if we hit the horizontal walls.
        if (new_y == 0 || new_y == 63)
        {
            ball_dir_y = -ball_dir_y;
            new_y += ball_dir_y + ball_dir_y;
        }

        // Check if we hit the CPU paddle
        if (new_x == CPU_X && new_y >= cpu_y && new_y <= cpu_y + PADDLE_HEIGHT)
        {
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        // Check if we hit the player paddle
        if (new_x == PLAYER_X && new_y >= player_y && new_y <= player_y + PADDLE_HEIGHT)
        {
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        display.drawPixel(ball_x, ball_y, BLACK);
        display.drawPixel(new_x, new_y, WHITE);
        ball_x = new_x;
        ball_y = new_y;

        ball_update += BALL_RATE;

        display.display();
    }

    if (time > paddle_update)
    {
        paddle_update += PADDLE_RATE;

        // CPU paddle
        display.drawFastVLine(CPU_X, cpu_y, PADDLE_HEIGHT, BLACK);
        const uint8_t half_paddle = PADDLE_HEIGHT >> 1;
        if (cpu_y + half_paddle > ball_y)
        {
            cpu_y -= 1;
        }
        if (cpu_y + half_paddle < ball_y)
        {
            cpu_y += 1;
        }
        if (cpu_y < 1)
            cpu_y = 1;
        if (cpu_y + PADDLE_HEIGHT > 63)
            cpu_y = 63 - PADDLE_HEIGHT;
        display.drawFastVLine(CPU_X, cpu_y, PADDLE_HEIGHT, WHITE);

        // Player paddle
        display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, BLACK);
        if (up_state)
        {
            player_y -= 5;
        }
        if (down_state)
        {
            player_y += 5;
        }
        up_state = down_state = false;
        if (player_y < 1)
            player_y = 1;
        if (player_y + PADDLE_HEIGHT > 63)
            player_y = 63 - PADDLE_HEIGHT;
        display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, WHITE);

        display.display();
    }

    //------------------------------------------------------------
    // Take care of switch debouncing
    if (millis() - lastDebounceTime >= debounceDelay)
    {
        debounceDelayDone = true;
        digitalWrite(ledPin, LOW); //Turns the builtin LED off if the debounce timer has finished (not ignoring inputs)
    }
    else
    {
        debounceDelayDone = false;
        digitalWrite(ledPin, HIGH); //Turns the LED on if the timer is still running (ignoring inputs)
        //display.clearDisplay();
    }
    //------------------------------------------------------------
}