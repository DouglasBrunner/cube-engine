#include "CubeEngine.h" 
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif


/***********************************
 * BEGIN ENGINE SPECIFIC CODE
 **********************************/

/**
 * This constructor sets the Arduino pins and clears both the shift-registers and data array
 */
CubeEngine::CubeEngine(int latchPin, int clockPin, int dataPin,
                      int layer0, int layer1, int layer2,
                      int layer3, int layer4, int layer5) {

    // Set clases private members
    this->latchPin = latchPin;
    this->clockPin = clockPin;
    this->dataPin  = dataPin;
    this->layer0   = layer0;
    this->layer1   = layer1;
    this->layer2   = layer2;
    this->layer3   = layer3;
    this->layer4   = layer4;
    this->layer5   = layer5;

    //set pins to output
    pinMode(this->latchPin, OUTPUT);
    pinMode(this->clockPin, OUTPUT);
    pinMode(this->dataPin, OUTPUT);

    // Set register pins in known state
    digitalWrite(this->clockPin, LOW);  
    digitalWrite(this->latchPin, LOW); 
    digitalWrite(this->dataPin, LOW);  

    // Set layer pins to output
    pinMode(this->layer0, OUTPUT);
    pinMode(this->layer1, OUTPUT);
    pinMode(this->layer2, OUTPUT);
    pinMode(this->layer3, OUTPUT);
    pinMode(this->layer4, OUTPUT);
    pinMode(this->layer5, OUTPUT); 

    // set data array to off
    this->killDataArray();

    // set all registers to off
    this->killRegisters();

    // Ensure variables are properly defined
    this->layerCounter = 0;
    this->mplexCounter = 0;
        
}

// Attribute related functions


/*
 * Sets the attribute of the requested sprite
 */
void CubeEngine::setSpriteAttribute(int spriteNum, byte name, byte value) {

    // Holds the attribute group
    byte group;

    // Holds the group number
    int groupNum;

    // holds the bit-mask
    byte mask;
   
    // Set state
    if (name == this->AN_STATE) {

        // Set group numner
        groupNum = 3;

        // Clear old attribute
        mask = B11110111;

    // Set colour        
    } else if (name == this->AN_COLOUR) {
        groupNum = 0;
        mask = B00111111;

    // Set visibility
    } else if (name == this->AN_VISIBILITY) {
        groupNum = 1;
        mask = B01111111;

    // Set x-coordinate
    } else if (name == this->AN_X) {
        groupNum = 0;
        mask = B11111000;

    // set y-coordinate        
    } else if (name == this->AN_Y) {
        groupNum = 0;
        mask = B11000111;

        // Shift value into Y position (otherwise it overwrites X)
        value = value << 3;

    // set z-coordinate
    } else if (name == this->AN_Z) {
        groupNum = 1;
        mask = B11111000;

    // Set wrap
    } else if (name == this->AN_WRAP) {
        groupNum = 2;
        mask = B10111111;

    // Set movement
    } else if (name == this->AN_MOVE) {
        groupNum = 2;
        mask = B01111111;

    // Set direction
    } else if (name == this->AN_DIRECTION) {
        groupNum = 1;
        mask = B10000111;

    // Set speed
    } else if (name == this->AN_SPEED) {
        groupNum = 3;
        mask = B11111000;

    // Set defend
    } else if (name == this->AN_DEFEND) {
        groupNum = 2;
        group = group & B11111000;

        // Shift value into defend position
        value = value << 3;

    // Set attack
    } else if (name == this->AN_ATTACK) {
        groupNum = 2;
        mask = B11000111;

    }

    // kill LED at current position is the attribute update is movement
    if (name == this->AN_X || name == this->AN_Y || name == this->AN_Z) {
      // Get current coordinates
      int x = getSpriteAttribute(spriteNum, this->AN_X);
      int y = getSpriteAttribute(spriteNum, this->AN_Y);
      int z = getSpriteAttribute(spriteNum, this->AN_Z);
  
      // Turn off current LED
      // This removes the need to manually sync the attribute and data arrays
      // Since only one sprite can exist on an LED at a time we know that the LED must turn off
      setLED(x, y, z, this->AV_OFF);
    }
    
    // Set new attribute
    group = this->clearAttribute(spriteNum, groupNum, mask) | value;

    // Write new attribute
    this->writeSpriteAttribute(spriteNum, groupNum, group);

}

/**
 * Clears an attribute and prepares if for being reset
 */
byte CubeEngine::clearAttribute(int spriteNum, int groupNum, byte mask) {
    return this->getGroup(spriteNum, groupNum) & mask;
}

/*
 * Writes an attribute group to a sprite
 */
void CubeEngine::writeSpriteAttribute(int spriteNum, int groupNum, byte group) {

    // get the four groups
    byte groupZero  = this->getGroup(spriteNum, 0);
    byte groupOne   = this->getGroup(spriteNum, 1);
    byte groupTwo   = this->getGroup(spriteNum, 2);
    byte groupThree = this->getGroup(spriteNum, 3);

    // Overwrite group to be updated
    switch (groupNum) {
        case 0:
            groupZero = group;
            break;
        case 1:
            groupOne = group;
            break;
        case 2:
            groupTwo = group;
            break;
        case 3:
            groupThree = group;
            break;
    }

    // Shift and load the groups into variable
    unsigned long attributes;
    attributes = groupThree;
    attributes = (attributes << 8) | groupTwo;
    attributes = (attributes << 8) | groupOne;
    attributes = (attributes << 8) | groupZero;

    // Update sprite attributes
    sprites[spriteNum] = attributes;

    if ((this->getSpriteAttribute(spriteNum, AN_VISIBILITY) == AV_VISIBLE)  &&
      (this->getSpriteAttribute(spriteNum, AN_STATE) == AV_LIVE)) {
         this->setLED(this->getSpriteAttribute(spriteNum, AN_X),
          this->getSpriteAttribute(spriteNum, AN_Y), 
          this->getSpriteAttribute(spriteNum, AN_Z), 
          this->getSpriteAttribute(spriteNum, AN_COLOUR));
      } else {
        this->setLED(this->getSpriteAttribute(spriteNum, AN_X),
          this->getSpriteAttribute(spriteNum, AN_Y), 
          this->getSpriteAttribute(spriteNum, AN_Z), 
          this->getSpriteAttribute(spriteNum, AN_COLOUR));
      }


}


/*
 * Gets the attribute of the requested sprite
 */
byte CubeEngine::getSpriteAttribute(int spriteNum, byte name) {

    // Holds the attribute group
    byte group;

    // Holds the group number
    int groupNum;

    // Get state
    if (name == this->AN_STATE) {
        group = this->getAttribute(spriteNum, 3, B00001000, 0);

    // Get colour        
    } else if (name == this->AN_COLOUR) {
        group = this->getAttribute(spriteNum, 0, B11000000, 0);

    // Get visibility
    } else if (name == this->AN_VISIBILITY) {
        group = this->getAttribute(spriteNum, 1, B10000000, 0);

    // Get x-coordinate
    } else if (name == this->AN_X) {
        group = this->getAttribute(spriteNum, 0, B00000111, 0);

    // Get y-coordinate        
    } else if (name == this->AN_Y) {
        group = this->getAttribute(spriteNum, 0, B00111000, 3);

    // Get z-coordinate
    } else if (name == this->AN_Z) {
        group = this->getAttribute(spriteNum, 1, B00000111, 0);

    // Get wrap
    } else if (name == this->AN_WRAP) {
        group = this->getAttribute(spriteNum, 2, B01000000, 0);

    // Get movement
    } else if (name == this->AN_MOVE) {
        group = this->getAttribute(spriteNum, 2, B10000000, 0);

    // Get direction
    } else if (name == this->AN_DIRECTION) {
        group = this->getAttribute(spriteNum, 1, B01111000, 0);

    // Get speed
    } else if (name == this->AN_SPEED) {
        group = this->getAttribute(spriteNum, 3, B00000111, 0);

    // Get defend
    } else if (name == this->AN_DEFEND) {
        group = this->getAttribute(spriteNum, 2, B00000111, 3);

    // Get attack
    } else if (name == this->AN_ATTACK) {
        group = this->getAttribute(spriteNum, 2, B00111000, 0);

    }

    // Return the attribute value
    return group;
}

/**
 * Returns the requested sprite attribute
 */
byte CubeEngine::getAttribute(int spriteNum, int groupNum, byte mask, int shift) {
        
        // This holds the attribute to be returned
        byte group;

        // Get attributes group
        group = this->getGroup(spriteNum, groupNum);

        // Mask Other Attributes
        group = group & mask;

        // Shift value out of defend position
        if (shift > 0) {
            group = group >> shift;
        }

        return group;
}


/*
 * Sets a sprite to a random colour
 */
void CubeEngine::setRandomSpriteColour(int spriteNum) {

    // Used for switching so we can use the preset values
    long switchVal = random(0,3);

    // Holds the selected colour
    byte colour;

    switch (switchVal) {
        case 0:
            colour = this->AV_RED;
            break;
        case 1:
            colour = this->AV_GREEN;
            break;
        case 2:
            colour = this->AV_BLUE;
            break;
    }

    this->setSpriteAttribute(spriteNum, this->AN_COLOUR, colour);
}


/* 
 * Moves a sprite to a random position 
 */
void CubeEngine::setRandomSpritePosition(int spriteNum) {
    this->setSpriteAttribute(spriteNum, this->AN_X, random(0,5));
    this->setSpriteAttribute(spriteNum, this->AN_Y, random(0,5));
    this->setSpriteAttribute(spriteNum, this->AN_Z, random(0,5));
}

/*
 * Randomly changes the direction of the sprite
 */
void CubeEngine::setRandomSpriteDirection(int spriteNum) {

    // Used for switching so we can use the preset values
    long switchVal = random(0,13);

    // Holds the selected direction
    byte direction;

    switch (switchVal) {
        case 0:
            direction = this->AV_UP;
            break;
        case 1:
            direction = this->AV_DOWN;
            break;
        case 2:
            direction = this->AV_LEFT;
            break;
        case 3:
            direction = this->AV_RIGHT;
            break;
        case 4:
            direction = this->AV_FRONT;
            break;
        case 5:
            direction = this->AV_BACK;
            break;
        case 6:
            direction = this->AV_BACK_UP_LEFT;
            break;
        case 7:
            direction = this->AV_BACK_UP_RIGHT;
            break;
        case 8:
            direction = this->AV_BACK_DOWN_LEFT;
            break;
        case 9:
            direction = this->AV_BACK_DOWN_RIGHT;
            break;
        case 10:
            direction = this->AV_FRONT_UP_LEFT;
            break;
        case 11:
            direction = this->AV_FRONT_UP_RIGHT;
            break;
        case 12:
            direction = this->AV_FRONT_DOWN_LEFT;
            break;
        case 13:
            direction = this->AV_FRONT_DOWN_RIGHT;
            break;
        
    }
    
    this->setSpriteAttribute(spriteNum, this->AN_DIRECTION, direction);    

}

/*
 * Returns the requested byte group of a sprite
 */
byte CubeEngine::getGroup(int spriteNum, int groupNum) {

    // Holds a copy of the sprite attributes
    long attributesCopy = sprites[spriteNum];

    // Bit-shift and return the appropriate group
    switch (groupNum) {
        case 0:
            return attributesCopy;
            break;
        case 1:
            return attributesCopy >> 8;
            break;
        case 2:
            return attributesCopy >> 16;
            break;
        case 3:
            return attributesCopy >> 24;
            break;
    }

}



// movement functions

/* 
 * Move a sprite in one direction
 *
 * @todo make sure this respects visibility, state and move settings
 */
void CubeEngine::moveSprite(int spriteNum, byte direction) {

    // Move the sprite
    if (direction == this->AV_UP) {
        this->moveY(spriteNum, this->AV_UP);


    } else if (direction == this->AV_RIGHT) {
        this->moveX(spriteNum, this->AV_RIGHT);


    } else if (direction == this->AV_DOWN) {
        this->moveY(spriteNum, this->AV_DOWN);


    } else if (direction == this->AV_LEFT) {
        this->moveX(spriteNum, this->AV_LEFT);


    } else if (direction == this->AV_FRONT) {
        this->moveZ(spriteNum, this->AV_FRONT);


    } else if (direction == this->AV_BACK) {
        this->moveZ(spriteNum, this->AV_BACK);

    } else if (direction == this->AV_BACK_UP_LEFT) {
        this->moveX(spriteNum, this->AV_LEFT);
        this->moveY(spriteNum, this->AV_UP);
        this->moveZ(spriteNum, this->AV_BACK);


    } else if (direction == this->AV_BACK_UP_RIGHT) {
        this->moveX(spriteNum, this->AV_RIGHT);
        this->moveY(spriteNum, this->AV_UP);
        this->moveZ(spriteNum, this->AV_BACK);


    } else if (direction == this->AV_BACK_DOWN_LEFT) {
        this->moveX(spriteNum, this->AV_LEFT);
        this->moveY(spriteNum, this->AV_DOWN);
        this->moveZ(spriteNum, this->AV_BACK);


    } else if (direction == this->AV_BACK_DOWN_RIGHT) {
        this->moveX(spriteNum, this->AV_RIGHT);
        this->moveY(spriteNum, this->AV_DOWN);
        this->moveZ(spriteNum, this->AV_BACK);


    } else if (direction == this->AV_FRONT_UP_LEFT) {
        this->moveX(spriteNum, this->AV_LEFT);
        this->moveY(spriteNum, this->AV_UP);
        this->moveZ(spriteNum, this->AV_FRONT);


    } else if (direction == this->AV_FRONT_UP_RIGHT) {
        this->moveX(spriteNum, this->AV_RIGHT);
        this->moveY(spriteNum, this->AV_UP);
        this->moveZ(spriteNum, this->AV_FRONT);


    } else if (direction == this->AV_FRONT_DOWN_LEFT) {
        this->moveX(spriteNum, this->AV_LEFT);
        this->moveY(spriteNum, this->AV_DOWN);
        this->moveZ(spriteNum, this->AV_FRONT);


    } else if (direction == this->AV_FRONT_DOWN_RIGHT) {
        this->moveX(spriteNum, this->AV_RIGHT);
        this->moveY(spriteNum, this->AV_DOWN);
        this->moveZ(spriteNum, this->AV_FRONT);


    }

}

/*
 * Move sprite in direction of travel
 */
void CubeEngine::autoMoveSprites() {

    // get current time difference
    unsigned long curTimeStamp = millis();

    // flags which speeds can move
    bool canMove0 = curTimeStamp - this->AM_SPEED0_PERIOD > this->AM_SPEED0_DIF;
    bool canMove1 = curTimeStamp - this->AM_SPEED1_PERIOD > this->AM_SPEED1_DIF;
    bool canMove2 = curTimeStamp - this->AM_SPEED2_PERIOD > this->AM_SPEED2_DIF;
    bool canMove3 = curTimeStamp - this->AM_SPEED3_PERIOD > this->AM_SPEED3_DIF;
    bool canMove4 = curTimeStamp - this->AM_SPEED4_PERIOD > this->AM_SPEED4_DIF;
    bool canMove5 = curTimeStamp - this->AM_SPEED5_PERIOD > this->AM_SPEED5_DIF;
    bool canMove6 = curTimeStamp - this->AM_SPEED6_PERIOD > this->AM_SPEED6_DIF;

    // Cycle through sprites
    for (int i = SPRITE_SIZE; i >= 0; i--) {

        // Only proceed for sprites which can move
        if (this->getSpriteAttribute(i, this->AN_MOVE) == this->AV_NOMOVE) {
            continue;
        }

        // Get sprite speed and direction
        byte speed     = getSpriteAttribute(i, this->AN_SPEED);
        byte direction = getSpriteAttribute(i, this->AN_DIRECTION);

        // Check if this sprite can move at this speed
        if (canMove0 && (speed == this->AV_SPEED0)) {

            // Move the sprite
            this->moveSprite(i, direction);

        } else if (canMove1 && (speed == this->AV_SPEED1)) {
            this->moveSprite(i, direction);

        } else if (canMove2 && (speed == this->AV_SPEED2)) {
            this->moveSprite(i, direction);

        } else if (canMove3 && (speed == this->AV_SPEED3)) {
            this->moveSprite(i, direction);

        } else if (canMove4 && (speed == this->AV_SPEED4)) {
            this->moveSprite(i, direction);

        } else if (canMove5 && (speed == this->AV_SPEED5)) {
            this->moveSprite(i, direction);

        } else if (canMove6 && speed == this->AV_SPEED6) {
            this->moveSprite(i, direction);

        }

    }

    // Update timers
    if (canMove0) { this->AM_SPEED0_PERIOD = curTimeStamp; }

    if (canMove1) { this->AM_SPEED1_PERIOD = curTimeStamp; }

    if (canMove2) { this->AM_SPEED2_PERIOD = curTimeStamp; }

    if (canMove3) { this->AM_SPEED3_PERIOD = curTimeStamp; }

    if (canMove4) { this->AM_SPEED4_PERIOD = curTimeStamp; }

    if (canMove5) { this->AM_SPEED5_PERIOD = curTimeStamp; }

    if (canMove6) { this->AM_SPEED6_PERIOD = curTimeStamp; }

}

/*
 * Move sprite on x-axis
 */
void CubeEngine::moveX(int spriteNum, byte direction) {

    // get position and wrap attributes
    byte pos_x = this->getSpriteAttribute(spriteNum, AN_X);
    byte wrap = this->getSpriteAttribute(spriteNum, AN_WRAP);

    // move left
    if (direction == this->AV_LEFT) {
        if (pos_x > 0) {                                            // Move if able
            setSpriteAttribute(spriteNum, this->AN_X, pos_x - 1);
        } else if (pos_x == 0 && wrap == this->AV_WRAP) {           // Wrap if able
            setSpriteAttribute(spriteNum, this->AN_X, 5);
        }

    // move right
    } else if (direction == this->AV_RIGHT) {
        if (pos_x < 5) {
            setSpriteAttribute(spriteNum, this->AN_X, pos_x + 1);
        } else if (pos_x == 5 && wrap == this->AV_WRAP) {
            setSpriteAttribute(spriteNum, this->AN_X, 0);
        }
    }

}

/*
 * Move sprite on y-axis
 */
void CubeEngine::moveY(int spriteNum, byte direction) {

    // get position and wrap attributes
    byte pos_y = this->getSpriteAttribute(spriteNum, AN_Y);
    byte wrap = this->getSpriteAttribute(spriteNum, AN_WRAP);

    // move left
    if (direction == this->AV_UP) {
        if (pos_y < 5) {
            setSpriteAttribute(spriteNum, this->AN_Y, pos_y + 1);
        } else if (pos_y == 5 && wrap == this->AV_WRAP) {
            setSpriteAttribute(spriteNum, this->AN_Y, 0);
        }

    // move right
    } else if (direction == this->AV_DOWN) {
        if (pos_y > 0) {
            setSpriteAttribute(spriteNum, this->AN_Y, pos_y - 1);
        } else if (pos_y == 0 && wrap == this->AV_WRAP) {
            setSpriteAttribute(spriteNum, this->AN_Y, 5);
        }
    }

}

/*
 * Move sprite on z-axis
 */
void CubeEngine::moveZ(int spriteNum, byte direction) {

    // get position and wrap attributes
    byte pos_z = this->getSpriteAttribute(spriteNum, this->AN_Z);
    byte wrap = this->getSpriteAttribute(spriteNum, this->AN_WRAP);

    // move left
    if (direction == this->AV_FRONT) {
        if (pos_z > 0) {
            setSpriteAttribute(spriteNum, this->AN_Z, pos_z - 1);
        } else if (pos_z == 0 && wrap == this->AV_WRAP) {
            setSpriteAttribute(spriteNum, this->AN_Z, 5);
        }

    // move right
    } else if (direction == this->AV_BACK) {
        if (pos_z < 5) {
            setSpriteAttribute(spriteNum, this->AN_Z, pos_z + 1);
        } else if (pos_z == 5 && wrap == this->AV_WRAP) {
            setSpriteAttribute(spriteNum, this->AN_Z, 0);
        }
    }
}


/***********************************
 * END ENGINE SPECIFIC CODE
 **********************************/


/***********************************
 * BEGIN HARDWARE SPECIFIC CODE
 **********************************/

/* 
 * Set LED colour in the data array
 *
 * The co-ordinate system starts at (0,0,0) and increases
 * to (5,5,5). Co-ordinate (0,0,0) is the bottom-left LED at the
 * cube's front. Co-ordinate(5,5,5) is the top-right LED
 * at the cube's back.
 *
 * (y,z,x), where y = vertical, z = depth, x = horizontal
 *
 * This is the only function which updates the data array
 * after it's beein initialized by setup.
 */
void CubeEngine::setLED(int layerPos, int rowPos, int columnPos, byte colour) {

    // Do nothing for invalid co-ordinates
    if (layerPos > 5 || rowPos > 5 || columnPos > 5) {
        return;
    }

    /*
     * The below two formulas convert the cubes 3D structure (represented as
     * a three-point coordinate system) into the data arrays 2D structure.
     *
     * Each LED requires 2 bits to store it's state
     * Each layer (of the cube) requires 72 bits of storage (36 LEDs per layer, 2 bits storage each)
     * Each row required 6 * 2 = 12 bits of storage
     * Each each column (of a row) requires 2 bits of storage (the columns are limited to the row they're in)
     *
     * The index lets us know which element of the data array the LEDs bit code is in
     * The offset lets us know how far into the byte we need to go
     */
    int index  = (((72 * layerPos) + (rowPos * 12) + (columnPos * 2))  / 8);
    int offSet = ((rowPos * 12) + (columnPos * 2)) % 8;

    // Get the codes contained in data element
    byte codes = this->data[index];
    
    // Update the code
    switch (offSet) {
    
        // Offset 0
        case 0:
            codes = codes & B11111100;      // Clear old LED code
            codes = codes | (colour >> 6);  // Assign new code (and shift into position)
            break;
    
        // Offset 2
        case 2:
            codes = codes & B11110011;
            codes = codes | (colour >> 4);
            break;

        // Offset 4
        case 4:
            codes = codes & B11001111;
            codes = codes | (colour >> 2);
            break;
      
        // Offset 6
        case 6:
            codes = codes & B00111111;
            codes = codes | colour;
            break;
    
    }

    // Update the element with the new code
    this->data[index] = codes;

}

/*
 * Multiplexes the LEDs
 *
 * This function tries to be as fast as possible in order
 * to get a high refresh-rate.
 *
 * It favours direct port manipulation over digitalWrites
 * http://www.arduino.cc/en/Reference/PortManipulation
 *
 * Red LEDs can't coexist on the same layer as Blue and Green LEDs
 * at the same time so they must be displayed separately.
 */
void CubeEngine::mplex() {

    // Loop back to layer 0 if required
    if (this->layerCounter == 6) {
        this->layerCounter = 0;
    }

    // Set which elements of the data array the LED codes are found in
    int lower = (this->layerCounter * 72) / 8;
    int upper = lower + 8;

    // Turn off power to all layers (Digital Pins 2-7)
    PORTD = B00000000;

    // Prepare registers for data
    // Set latch pin (A1) to LOW
    PORTC = PORTC & B11111101;

    // These variables are used to manipulate the bits which
    // hold the colour codes in each byte
    // Bit-shifting the data array directly mutates it, which results in buggy behaviour
    byte colourCode, element;

    // Multiplex Green/Blue LEDs
    if (this->mplexCounter == 0) {

        // Load Blue/Green LEDs into registers
        for (int i = upper; i >= lower; i--) {

            // Get the current element
            element = this->data[i];

            // Cycle through the codes in each byte
            // This cycles by 2 since the state of each LED is encoded in 2 bits
            for (int j = 0; j < 8; j+=2) {

                // Get colour code
                colourCode = element << j;
                colourCode = colourCode >> 6;

                // push Off LED instead of Red            
                if (colourCode == this->REG_RED) {

                    // Set data pin (A2) to high
                    PORTC = PORTC | B00000100;

                    // Cycle clock pin (A3) three times
                    PORTC = PORTC | B00001000; // HIGH
                    PORTC = PORTC & B11110111; // LOW
                    PORTC = PORTC | B00001000;
                    PORTC = PORTC & B11110111; 
                    PORTC = PORTC | B00001000;
                    PORTC = PORTC & B11110111;

                // push Green LED
                } else if (colourCode == this->REG_GREEN) {

                    // Set data pin (A2) to HIGH
                    PORTC = PORTC | B00000100;

                    // Cycle clock pin (A3) one time 
                    PORTC = PORTC | B00001000;
                    PORTC = PORTC & B11110111;

                    // Set data pin (A2) to LOW
                    PORTC = PORTC & B11111011;

                    // Cycle clock pin (A3) one time 
                    PORTC = PORTC | B00001000;
                    PORTC = PORTC & B11110111;

                    // Set data pin (A2) to HIGH
                    PORTC = PORTC | B00000100;

                    // Cycle clock pin (A3) one time 
                    PORTC = PORTC | B00001000;
                    PORTC = PORTC & B11110111;

                // push Blue LED
                } else if (colourCode == this->REG_BLUE) {

                    // Set data pin (A2) to LOW
                    PORTC = PORTC & B11111011;

                    // Cycle clock pin (A3) one time 
                    PORTC = PORTC | B00001000;
                    PORTC = PORTC & B11110111;

                    // Set data pin (A2) to HIGH
                    PORTC = PORTC | B00000100;

                    // Cycle clock pin (A3) two times
                    PORTC = PORTC | B00001000;
                    PORTC = PORTC & B11110111;
                    PORTC = PORTC | B00001000;
                    PORTC = PORTC & B11110111;

                // push Off LED
                } else {

                    // Set data pin (A2) to high
                    PORTC = PORTC | B00000100;

                    // Cycle clock pin (A3) three times
                    PORTC = PORTC | B00001000;
                    PORTC = PORTC & B11110111;
                    PORTC = PORTC | B00001000;
                    PORTC = PORTC & B11110111; 
                    PORTC = PORTC | B00001000;
                    PORTC = PORTC & B11110111;

                }
            }
        }

        // Multiplex Red
        this->mplexCounter = 1;

    // Multiplex Red
    } else if (this->mplexCounter == 1) {

        // Load Red LEDs into registers
        for (int i = upper; i >= lower; i--) {

            // Get the current element
            element = this->data[i];

            // Cycle through the codes in each byte
            for (int j = 0; j < 8; j+=2) {

                // Get colour code
                colourCode = element << j;
                colourCode = colourCode >> 6;

                // Shift Red
                if (colourCode == this->REG_RED) {

                    // Set data pin (A2) to HIGH
                    PORTC = PORTC | B00000100;

                    // Cycle clock pin (A3) two times
                    PORTC = PORTC | B00001000;
                    PORTC = PORTC & B11110111;
                    PORTC = PORTC | B00001000;
                    PORTC = PORTC & B11110111; 

                    // Set data pin (A2) to LOW
                    PORTC = PORTC & B11111011;

                    // Cycle clock pin (A3) one time
                    PORTC = PORTC | B00001000;
                    PORTC = PORTC & B11110111;

                // push Off for all Blue/Green LEDs
                } else {

                    // Set data pin (A2) to high
                    PORTC = PORTC | B00000100;

                    // Cycle clock pin (A3) three times
                    PORTC = PORTC | B00001000;
                    PORTC = PORTC & B11110111;
                    PORTC = PORTC | B00001000;
                    PORTC = PORTC & B11110111; 
                    PORTC = PORTC | B00001000;
                    PORTC = PORTC & B11110111;

                }
            }
        }

        // Multiplex Blue/Green LEDs
        this->mplexCounter = 0;
    }

    // Signal end of data
    // Set latch pin (A1) to HIGH
    PORTC = PORTC | B00000010;

    // Supply power to the active layer
    switch (this->layerCounter) {
        case 0:
            PORTD = B00000100;  // 0
            break;
        case 1:
            PORTD = B00001000;
            break;
        case 2:
            PORTD = B00010000;
            break;
        case 3:
            PORTD = B00100000;
            break;
        case 4:
            PORTD = B01000000;
            break;
        case 5:
            PORTD = B10000000;  // 5
            break;
    }

    // Move to the next layer
    if (this->mplexCounter == 1) {
        this->layerCounter += 1;
    }

}

/* 
 * Ensures that the data array is set to 0 
 *
 * This sets all sprites to off
 */
void CubeEngine::killDataArray() {
    for (int i = this->DATA_SIZE; i >= 0; i--) {
        this->data[i] = 0;
    }
}

/*
 * Sets all registers to HIGH, which turns off the cube
 *
 * This pushes out 36 off LEDs
 */
void CubeEngine::killRegisters() {
    for (int i = 0; i <= 35; i++) {
        digitalWrite(this->dataPin, HIGH);
        digitalWrite(this->clockPin, HIGH);
        digitalWrite(this->clockPin, LOW); 
        digitalWrite(this->clockPin, HIGH);
        digitalWrite(this->clockPin, LOW); 
        digitalWrite(this->clockPin, HIGH);
        digitalWrite(this->clockPin, LOW); 
    }
}

/***********************************
 * END HARDWARE SPECIFIC CODE
 **********************************/



