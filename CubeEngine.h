/*
* CubeEngine.h - Library for developing games on a 6x6x6 RGB LED Cube.
*/
#ifndef CubeEngine_h
#define CubeEngine_h

#include "Arduino.h"

class CubeEngine
{
    public:

        /***********************************
         * BEGIN ENGINE SPECIFIC CODE
         **********************************/

        // The names of the sprite attributes
        // These are used to reference an attribute
        const byte AN_STATE      = 0;
        const byte AN_COLOUR     = 1;
        const byte AN_VISIBILITY = 2;
        const byte AN_X          = 3;
        const byte AN_Y          = 4;
        const byte AN_Z          = 5;
        const byte AN_WRAP       = 6;
        const byte AN_MOVE       = 7;
        const byte AN_DIRECTION  = 8;
        const byte AN_SPEED      = 9;
        const byte AN_DEFEND     = 10;
        const byte AN_ATTACK     = 11;
        
        // The values of the sprite attributes
        // These value are written so they can be used directly with 'bit-wise or'
        //   in the getSpriteAttribute/setSpriteAttribute functions
        const byte AV_LIVE               = B00001000; // state
        const byte AV_DEAD               = B00000000;
        const byte AV_ZERO               = B00000000; // coordinates
        const byte AV_ONE                = B00000001; 
        const byte AV_TWO                = B00000010; 
        const byte AV_THREE              = B00000011; 
        const byte AV_FOUR               = B00000100; 
        const byte AV_FIVE               = B00000101; 
        const byte AV_OFF                = B00000000; // colour
        const byte AV_RED                = B01000000; 
        const byte AV_GREEN              = B10000000; 
        const byte AV_BLUE               = B11000000;
        const byte AV_VISIBLE            = B10000000; // visibility
        const byte AV_INVISIBLE          = B00000000; 
        const byte AV_WRAP               = B01000000; // wrap
        const byte AV_NOWRAP             = B00000000; 
        const byte AV_MOVE               = B10000000; // Move
        const byte AV_NOMOVE             = B00000000; 
        const byte AV_UP                 = B00000000; // Direction
        const byte AV_DOWN               = B00001000;
        const byte AV_LEFT               = B00010000;
        const byte AV_RIGHT              = B00011000;
        const byte AV_BACK               = B00100000;
        const byte AV_FRONT              = B00101000;
        const byte AV_BACK_UP_LEFT       = B00110000;
        const byte AV_BACK_UP_RIGHT      = B00111000;
        const byte AV_BACK_DOWN_LEFT     = B01000000;
        const byte AV_BACK_DOWN_RIGHT    = B01001000;
        const byte AV_FRONT_UP_LEFT      = B01010000;
        const byte AV_FRONT_UP_RIGHT     = B01011000;
        const byte AV_FRONT_DOWN_LEFT    = B01100000;
        const byte AV_FRONT_DOWN_RIGHT   = B01101000;
        const byte AV_SPEED0             = B00000000; // Speed
        const byte AV_SPEED1             = B00000001;
        const byte AV_SPEED2             = B00000010;
        const byte AV_SPEED3             = B00000011;
        const byte AV_SPEED4             = B00000100;
        const byte AV_SPEED5             = B00000101;
        const byte AV_SPEED6             = B00000111;
        const byte AV_KEEP_ALIVE         = B00000000; // Attack/Defend
        const byte AV_KILL               = B00000001;
        const byte AV_JUMP               = B00000010;
        const byte AV_ENDGAME            = B00000011;

        // Cube engine construbtor
        CubeEngine(int latchPin, int clockPin, int dataPin,
                   int layer0, int layer1, int layer2,
                   int layer3, int layer4, int layer5);

        // Attribute related functions
        void setSpriteAttribute(int spriteNum, byte name, byte value);
        byte getSpriteAttribute(int spriteNum, byte name);
        void setRandomSpritePosition(int spriteNum);
        void setRandomSpriteColour(int spriteNum);
        void setRandomSpriteDirection(int spriteNum);

        // Sprite movement functions
        void moveSprite(int spriteNum, byte direction);
        void autoMoveSprites();

        // Multiplexing and painting functions
        void mplex();
        
        /***********************************
         * END ENGINE SPECIFIC CODE
         **********************************/

        /***********************************
         * BEGIN HARDWARE SPECIFIC CODE
         **********************************/

        // setLED been made public so that programs can create patterns that require
        // more sprites than have been made available.
        void setLED(int layer, int row, int column, byte colour);
        
        /***********************************
         * END HARDWARE SPECIFIC CODE
         **********************************/
    private:

        /***********************************
         * BEGIN ENGINE SPECIFIC CODE
         **********************************/

        // This array holds the game sprites
        // This is limited to 25 in order to save memory
        //
        // The unsigned long has 32 bits, which are divided into 4 8-bit groups
        //
        // Group 0: 0 - 2   x position
        //          3 - 5   y position
        //          6 - 7   colour
        // Group 1: 0 - 2   z position
        //          3 - 6   direction
        //          7       visibility
        // Group 2: 0 - 2   attack
        //          3 - 5   defend
        //          6       wrap
        //          7       movement
        // Group 3: 0 - 2   speed
        //          3       state
        unsigned long sprites[25];   // one-indexed
        const byte SPRITE_SIZE = 24; // zero-indexed, used for looping

        // Automove sprite timers
        const int AM_SPEED0_DIF = 4000;        // How many milliseconds between moves
        const int AM_SPEED1_DIF = 2000;
        const int AM_SPEED2_DIF = 1000;
        const int AM_SPEED3_DIF = 500;
        const int AM_SPEED4_DIF = 250;
        const int AM_SPEED5_DIF = 125;
        const int AM_SPEED6_DIF = 50;
        unsigned long AM_SPEED0_PERIOD = 0;     // Total time elapsed since last move
        unsigned long AM_SPEED1_PERIOD = 0;
        unsigned long AM_SPEED2_PERIOD = 0;
        unsigned long AM_SPEED3_PERIOD = 0; 
        unsigned long AM_SPEED4_PERIOD = 0; 
        unsigned long AM_SPEED5_PERIOD = 0; 
        unsigned long AM_SPEED6_PERIOD = 0;

        // Attribute functions
        byte getGroup(int spriteNum, int group);        
        void writeSpriteAttribute(int spriteNum, int groupNum, byte group);
        byte getAttribute(int spriteNum, int groupNum, byte mask, int shift);
        byte clearAttribute(int spriteNum, int groupNum, byte mask);

        // Movement functions
        void moveX(int spriteNum, byte direction);
        void moveY(int spriteNum, byte direction);
        void moveZ(int spriteNum, byte direction);

        /***********************************
         * END ENGINE SPECIFIC CODE
         **********************************/

         /***********************************
         * BEGIN HARDWARE SPECIFIC CODE
         **********************************/

        // These variables let us know which pins are connected to
        // the cube
        int latchPin, clockPin, dataPin, layer0, layer1,
            layer2, layer3, layer4, layer5;

        // Colour codes for registers
        const byte REG_OFF   = 0;
        const byte REG_RED   = 1;
        const byte REG_GREEN = 2;
        const byte REG_BLUE  = 3;

        // Counters used for multiplexing
        volatile int mplexCounter = 0;         
        volatile int layerCounter = 0;
        
        // Holds the states of the all the LEDs in the cube
        // The state of each LED is stored using 2-bits
        // 00 - off     01 - red
        // 10 - green   11 - blue
        const byte DATA_SIZE = 53; // zero-indexed, used for looping
        byte data[54];             // one-indexed

        // Register and data functions        
        void killDataArray();
        void killRegisters();

        /***********************************
         * END HARDWARE SPECIFIC CODE
         **********************************/

};  

#endif