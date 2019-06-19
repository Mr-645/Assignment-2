#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define Right_button 2 //Pin D2 on Nano [interrupt]
#define Left_button 3 //Pin D3 on Nano [interrupt]
#define ledPin 13 //LED builtin pin for showing when the debounce function is ignoring button presses
#define potPin A0 //Potentiometer for controlling paddle

uint32_t debounceDelay = 150;            //Debounce time - Switch ignore time
volatile uint32_t lastDebounceTime = 0;  //This will store the last time the LED was updated
volatile bool debounceDelayDone = false; //Whether or not the debounce delay has occurred

const unsigned long PADDLE_RATE = 1; //Refresh time for updating paddle position
const unsigned long BALL_RATE = 36; //Refresh time for updating ball position
const uint8_t PADDLE_HEIGHT = 24;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino Reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t ball_x = 64, ball_y = 32; //Starting position of ball
uint8_t ball_dir_x = 1, ball_dir_y = 1; //Starting direction of ball
unsigned long ball_update;

unsigned long paddle_update;
const uint8_t CPU_X = 12; //Starting position of CPU player paddle
uint8_t cpu_y = 16;

const uint8_t PLAYER_X = 115; //Starting position of human player paddle
uint8_t player_y = 15;

uint8_t CPU_player_points = 0;
uint8_t Human_player_points = 0;
bool start_pause_toggle = 0; //Paused if true (1)
bool left_reset_pressed = 0; //Reset button press FLAG
bool right_startPause_pressed = 0; //Start/pause button press FLAG

enum States //These are the four states of the state machine
{
    Start_screen,
    Playing,
    Paused,
    End_screen
};
States current_state = Start_screen; //Declares current state

void display_start_screen()
{
    uint8_t ball_x = 64, ball_y = 32; //Reset ball position
    uint8_t ball_dir_x = 1, ball_dir_y = 1; //Reset ball direction
    // Display splash screen
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println(F("Score 5 points to win"));
    display.println(F("Press R_btn to begin"));
    display.display();
    start_pause_toggle = 0;
    CPU_player_points = 0;
    Human_player_points = 0;
}

bool setup_game_screen(bool finished_game_screen_setup)
{
    finished_game_screen_setup = false;
    display.clearDisplay();
    display.drawRect(0, 0, 128, 64, WHITE);

    //Initialise the score display board
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(50, 5);
    display.print(CPU_player_points);
    display.setCursor(60, 5);
    display.print(" - ");
    display.setCursor(80, 5);
    display.print(Human_player_points);
    display.display();
    finished_game_screen_setup = true;
    return finished_game_screen_setup;
}

void update_score_board()
{
    //Update the score display board
    display.clearDisplay();
    display.drawRect(0, 0, 128, 64, WHITE);
    display.setTextSize(1); // Draw 2X-scale text
    display.setTextColor(WHITE);
    display.setCursor(50, 5);
    display.print(CPU_player_points);
    display.setCursor(60, 5);
    display.print(" - ");
    display.setCursor(80, 5);
    display.print(Human_player_points);
    if(current_state == Paused){
        display.setTextSize(2); // Write the word "PAUSED" to the screen
        display.setTextColor(WHITE);
        display.setCursor(30, 30);
        display.println("PAUSED");
    }
    display.display();
}

void run_game()
{
    unsigned long time = millis();
    if (time > ball_update)
    {
        uint8_t new_x = ball_x + ball_dir_x; //Set the next pixel where the ball will be
        uint8_t new_y = ball_y + ball_dir_y;

        // Check if we hit the vertical walls
        if (new_x == 0 || new_x == 127)
        {
            //Increment the score of a player depending on which wall gets hit
            if (new_x == 0)
            {
                Human_player_points = Human_player_points + 1;
            }

            if (new_x == 127)
            {
                CPU_player_points = CPU_player_points + 1;
            }

            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;

            update_score_board();
        }

        // Check if we hit the horizontal walls.
        if (new_y == 0 || new_y == 63)
        {
            ball_dir_y = -ball_dir_y; //Set the next pixel where the ball will be
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

        if ((ball_x > 48 and ball_x < 88) and (ball_y > 4 and ball_y < 12))
        {
            //update_score_board(); //Un-comment this line if you want to get rid of the phantom balls when
                                    //the ball moves across the scoreboard and leaves its impressions behind
        }
        else
        {
            display.drawPixel(ball_x, ball_y, BLACK); //Replace the old pixel where the ball was with a blank space
            display.drawPixel(new_x, new_y, WHITE);
        }

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
        if (cpu_y + half_paddle > ball_y) //This handles CPU paddle movement
        {
            cpu_y -= 1;
        }
        if (cpu_y + half_paddle < ball_y)
        {
            cpu_y += 1;
        }

        if (cpu_y < 1)
        {
            cpu_y = 1;
        }
        if (cpu_y + PADDLE_HEIGHT > 63)
        {
            cpu_y = 63 - PADDLE_HEIGHT;
        }
        display.drawFastVLine(CPU_X, cpu_y, PADDLE_HEIGHT, WHITE);

        // Player paddle
        display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, BLACK);

        player_y = map(analogRead(potPin), 0, 926, 0, (63 - PADDLE_HEIGHT)); //This handles POT position
        display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, WHITE);     //and human paddle movement   

        display.display();
    }
}

void display_end_screen()
{
    display.clearDisplay(); // Display the end message
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(1, 10);

    if (CPU_player_points == 5)
    {
        display.println("CPU player wins");
    }
    else if (Human_player_points == 5)
    {
        display.println("Human player wins");
    }

    display.display();
}

void switch_bounce_handler() // Take care of switch debouncing
{
    if (millis() - lastDebounceTime >= debounceDelay)
    {
        debounceDelayDone = true;
        digitalWrite(ledPin, LOW); //Turns the builtin LED off if the debounce timer has finished (not ignoring inputs)
    }
    else
    {
        debounceDelayDone = false;
        digitalWrite(ledPin, HIGH); //Turns the LED on if the timer is still running (ignoring inputs)
    }
}

void ISR_start_pause() //Function to handle start/pause functionality
{
    if (debounceDelayDone == true) //If debounceDelayDone is true, do the following
    {
        start_pause_toggle = not start_pause_toggle;//Toggle between start/pause upon button press
        right_startPause_pressed = true; //Set start/pause FLAG
        lastDebounceTime = millis(); //Reset last debounce time
    }
}

void ISR_reset() //Function to handle ISR_reset functionality
{
    if (debounceDelayDone == true) //If debounceDelayDone is true, do the following
    {
        left_reset_pressed = true; //Set reset FLAG
        lastDebounceTime = millis(); //Reset last debounce time
    }
}

void setup() //Initialise the OLED display, the ball, etc.
{
    Serial.begin(9600);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    { // Address 0x3D for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }

    pinMode(ledPin, OUTPUT);
    pinMode(potPin, INPUT);
    pinMode(Right_button, INPUT_PULLUP);
    pinMode(Left_button, INPUT_PULLUP);

    //Run the ISR_start_pause function when the pin goes low
    attachInterrupt(digitalPinToInterrupt(Right_button), ISR_start_pause, FALLING);

    //Run the ISR_reset function when the pin goes low
    attachInterrupt(digitalPinToInterrupt(Left_button), ISR_reset, FALLING);

    display.clearDisplay();

    ball_update = millis();
    paddle_update = ball_update;
}

void loop() //The state machine lies here
{
    switch (current_state)
    {
    case Start_screen:
        display_start_screen();

        if (right_startPause_pressed)
        {
            bool finished_game_screen_setup;
            if (setup_game_screen(finished_game_screen_setup)){
                current_state = Playing;
            }
        }
        break;
    case Playing:
        run_game();
        if (start_pause_toggle)
        {
            current_state = Paused;
        }
        else if (not start_pause_toggle)
        {
            current_state = Playing;
        }

        if (left_reset_pressed == true){
            current_state = Start_screen;
        }

        //End the game if one player gets to 5 points
        if (CPU_player_points == 5 || Human_player_points == 5)
        {
            current_state = End_screen;
        }
        break;
    case Paused:
        update_score_board();
        if (start_pause_toggle)
        {
            current_state = Paused;
        }
        else if (not start_pause_toggle)
        {
            setup_game_screen(NULL);
            current_state = Playing;
        }

        if(left_reset_pressed == true){
            current_state = Start_screen;
        }

        break;
    case End_screen:
        display_end_screen();

        if (left_reset_pressed){
            current_state = Start_screen;
        }
        break;
    default:
        // statements
        break;
    }

    switch_bounce_handler();
    right_startPause_pressed = false; //Reset the flags
    left_reset_pressed = false;
}