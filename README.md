Prab Singh - 2121645<br>MG7013 - Embedded Systems<br>03 June 2019
# Assignment 2 - Monochrome OLED display pong game
## USAGE INSTRUCTIONS
1. Download and install the `Adafruit GFX` library, and the `Adafruit SSD1306` library for use with your Arduino compatible board.
2. Download [folder](https://github.com/Mr-645/Assignment-2/tree/master/Assignment%202%20-%20OLED%20Display) to get the Arduino sketch/C++ programme
4. Navigate to the `/src` folder and run `main.cpp`.
## REPORT
<img src="/Fully_built_prototype.jpg" alt="Fully built prototype">

---

### Project purpose and general description

**Use a display and run a game** on a microcontroller with buttons and a potentiometer for control.

*Criteria*: 
Implement the famous game Pong as a single player version
1. Game Design/implementation must be a state-machine
2. Must have buttons for start/stop, and reset.
3. Must use the potentiometer as the paddle controller

*Rules*:
- Implement a start-screen, the actual game, and an endscreen
- Display a player score (game-screen and end-screen)
- Implement a continues increase of the balls velocity
- Implement debouncing of your button(s)
- Design and Implementation as a state-machine
- Feel free to use the Adafruit libraries: Adafruit_SSD1306 and Adafruit_GFX

---

### Description of design process steps

The game is a modified version of a generic OLED PONG game by [eholk](https://github.com/eholk/).
The link to eholk's code is in the appendix.
Links to obtain the libraries required to use the OLED screen are also in the appendix.

#### Pin choice
Pin D2 was used for the `start/pause` functionality as an interrupt.
Pin D3 was used for the `reset` functionality as an interrupt.
Pin A0 was used for controlling the human player's paddle as an analogue input.

#### Game sequence
The pong game goes left-to-right and right-to-left across the screen.
The game has two players: the human on the right, and the CPU on the left.
If the ball hits the wall behind a paddle, the other player earns a point.
The score is shown in the centre-top of the screen.
The fist to 5 points wins.

#### Gameplay instructions
Control the paddle with the potentiometer. Reset the game with the left button. Pause/play with the right button.

#### State machine operation
The switch case function has four cases/states.
1. Start screen
2. Playing
3. Paused
4. End screen

<img src="/State_flow_diagram.png" alt="State flow diagram">

This is the switch case code:
```C
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
```

#### Layout and scheming
For the sake of producing readable code, I felt that it was best for as much functionality to be encased inside of methods.
There are various methods for just about everything that the game needs to do.

The states call functions to run, and actions are carried out depending on what the event or state is. Some actions might be setting variables, running other methods, or transitioning to another state.

The methods and variables are well named, so the programme flow should be easy to understand.

---

### Component choice and schematic

I decided to incorporate a potentiometer to control the human player's paddle. I map the analogue input to the height of the screen.
The two push-buttons I had were used for the reset and start/stop functionality. They are used as hardware interrupts.
The display is a monochrome 128 x 64 OLED running the SSD1306 driver. It's interfaced using i2c.

<img src="/Nano_OLED_Schematic.png" alt="Schematic">

---

### Limitations of project and potential improvements
The whole system works without any glitches. The only drawback is the slow processor of the ATMega328p on the Nano.
This drawback is the reason why the ball can't move any faster.

The only room for improvement is using a faster microcontroller.

---

### Appendix
#### Library reference

1. The [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
2. The [Adafruit SSD1306 Library](https://github.com/adafruit/Adafruit_SSD1306)

An infographic for the Nano is available [here](https://i.pinimg.com/736x/c4/87/21/c487213e9081fb0050878a02304e5693.jpg), it shows pin type information and current capability too. 

#### Source code of my PONG game
The source code for `main.cpp` is available [here](/Assignment-2/blob/master/Assignment 2 - OLED Display/src/main.cpp)

#### Original PONG game
The link to eholk's original is [this](https://github.com/eholk/Arduino-Pong/blob/master/pong.ino)