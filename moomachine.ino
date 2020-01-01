/*
 *
 * MooMachine:  Basic Program for MooMachine, both Editor and Interter
 * Version 0.9
 *
 */
 
#include <EEPROM.h>
 
//Program-wide Variables
#define DEBOUNCEDELAY 10


//Use C++ Defines for Switch and LED Definitions

//Lets start with output LEDs...will be doing 22-52 (even...matches Arduino Mega)
#define D0 22
#define D1 24
#define D2 26
#define D3 28

#define A0 30
#define A1 32
#define A2 34
#define A3 36
#define A4 38
#define A5 40
#define A6 42
#define A7 44
#define A8 46
#define A9 48

#define WAIT 43
#define HLD 45

#define MEMR 47
#define INP 49
#define MI 51
#define OUT 53
#define WO 52
#define INT 50


//Switches (input) will be 23-53 (odd)
#define S0 23
#define S1 25
#define S2 27
#define S3 29
#define S4 31
#define S5 33
#define S6 35
#define S7 37
#define S8 39
#define S9 41


//Input ports, which will use the PWM capabile ports (for now...)

#define RUN 2
#define STEP 3
#define EXAMINE 4
#define EXAMINENXT 5
#define DEPOSIT 6
#define DEPOSITNXT 7

#define RESET 8
#define CLR 9
#define SAVE 10
#define LOAD 11
#define AUX 12


//Static Arrays for Data/Address LEDs and Switches Addresses

byte DataLEDAddr[] = {D0, D1, D2, D3};
int DataLEDAddrSize=4;
byte AddressLEDAddr[] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9};
int AddressLEDAddrSize=10;
byte IndicatorLEDAddr[] = {WAIT, HLD, MEMR, INP, MI, OUT, WO, INT};
int IndicatorLEDAddrSize = 8;
byte SwitchAddr[] = {S0, S1, S2, S3, S4, S5, S6, S7, S8, S9};
int SwitchAddrSize=10;
byte SwitchCmdAddr[] = {RUN, STEP, EXAMINE, EXAMINENXT, DEPOSIT, DEPOSITNXT, RESET, CLR, SAVE, LOAD, AUX};
int SwitchCmdAddrSize = 11;
int SwitchPosition[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int SwitchPositionSize = 10;

//Program Variables
//  Assume 512 instructions (using bytes, although just 4-bit used internally)
//  Assumes 512 memory positions (bytes)
int instructionArray[1024];
int instructionArraySize = 1024;
int memoryArray[1024];
int memoryArraySize = 1024;

//position, represented by int
int instructionPointer = 0;
int memoryPointer = 0;

//Register

int registerValue;
//registerValue=0;


void setup() {
  //Setup LED Pins
  for (byte i=0;i<DataLEDAddrSize;i++) {
    pinMode(DataLEDAddr[i], OUTPUT);
  } 
  for (byte i=0;i<AddressLEDAddrSize;i++) {
    pinMode(AddressLEDAddr[i], OUTPUT);
  } 
  for (byte i=0;i<IndicatorLEDAddrSize;i++) {
    pinMode(AddressLEDAddr[i], OUTPUT);
  } 

 
 
 
 
  //INPUT PINS
  for (byte i=0;i<SwitchAddrSize;i++) {
    //Setup Switch PINS, setting internal pullup resistors
    pinMode(SwitchAddr[i], INPUT);
    digitalWrite(SwitchAddr[i], HIGH);
  }
  for (byte i=0;i<SwitchCmdAddrSize;i++) {
    //Setup Switch PINS, setting internal pullup resistors
    pinMode(SwitchCmdAddr[i], INPUT);
    digitalWrite(SwitchCmdAddr[i], HIGH);
  }
 
  // Fill Instruction Arrays with "13"s (invalid instructions...just in case)

  for (int i=0;i<instructionArraySize;i++) {
    //Setup Switch PINS, setting internal pullup resistors
    instructionArray[i] = 13;
  }


  //Serial Debug
 
    //Setup Serial Communication
   Serial.begin(9600);
  
  
   Serial.println("Setup Done...Ready for Commands");
 
 
}


void loop() {
  //Determine if RUN has been selected
  if (debounceRead(RUN) == LOW) {
   //Serial.println("RUN");
    //Run Routine
    mooRun();
  } else {
   //Serial.println("EDIT");
    //Edit mode..
    mooEdit();
  }
 
 
}


/*
 *
 * mooRun:  Run single step for program in memory
 *
 */
void mooRun() {

    //First, clear data display
     for (byte i=0;i<DataLEDAddrSize;i++) {
        digitalWrite(DataLEDAddr[i], LOW);
      }     
 
    int mooRunReturn;
   
    mooRunReturn = mooExec(instructionArray[instructionPointer]);
 
    if (mooRunReturn != 0) {
       errorHandler();  
     
    } else {
 
      instructionPointer++;
     
    }




 
}
   
   
// mooEdit: Perform any Edits that need to be made (plus update lights)

void mooEdit() {

  //Read Address/Data Switch Positions...since we need accurate switch positions for the rest of this routine
  readSwitch();

  //If Deposit or DepostNext are selected, then run deposit routine
  if (debounceRead(DEPOSIT) == LOW) { 
    //Wait for HIGH (switch to return)
       Serial.println("DEPOSIT");
    while (debounceRead(DEPOSIT) == LOW) {
       true;
    }

    readInst();

    displayDataInstLed();
   
  } else if (debounceRead(DEPOSITNXT) == LOW) {
    //Wait for HIGH (switch to return)
       Serial.println("DEPOSITNXT");
    while (debounceRead(DEPOSITNXT) == LOW) {
        true;
    }
   
    readInst();

    //Increment Instruction Pointer
    instructionPointer++;
   
    displayDataInstLed();
    displayDataAddrLed();
   
  } else if (debounceRead(EXAMINE) == LOW) {
           Serial.println("EXAMINE");
    //Wait for HIGH (switch to return)
    while (debounceRead(EXAMINE) == LOW) {
      true;
    }
   
    readAddr();
   
    displayDataAddrLed();
    displayDataInstLed();
   
  } else if (debounceRead(EXAMINENXT) == LOW) {
       Serial.println("EXAMINENXT");

    //Wait for HIGH (switch to return)
    while (debounceRead(EXAMINENXT) == LOW) {
       true;
    }

    //Increment instruction pointer by one
    instructionPointer++;
   
    //readAddr();
   
    displayDataAddrLed();
    displayDataInstLed();
   
  } else if (debounceRead(RESET) == LOW) {
       Serial.println("RESET");
    //Wait for HIGH (switch to return)
    while (debounceRead(RESET) == LOW) {
       true;
    }
    //Set instruction and memory pointer to address 0
    instructionPointer=0;
    memoryPointer=0;
    displayDataInstLed();
    displayDataAddrLed();

  } else if (debounceRead(CLR) == LOW) {
    Serial.println("CLEAR");
    //Wait for HIGH (switch to return)
    while (debounceRead(CLR) == LOW) {
       true;
    }

    //Clear memory
    clearMemory();
    displayDataInstLed();
    displayDataAddrLed();

  } else if (debounceRead(SAVE) == LOW) {
    Serial.println("SAVE");
    //Wait for HIGH (switch to return)
    while (debounceRead(SAVE) == LOW) {
       true;
    }
   
    saveToROM();
 
//Temporary change to test load switch
  } else if (debounceRead(LOAD) == LOW) {
    Serial.println("LOAD");
    //Wait for HIGH (switch to return)
    while (debounceRead(LOAD) == LOW) {
       true;
    }
   
    //TODO: Wirte routine for Load
    loadFromROM();
   
  }
}


/*
 *  Main Functions
 *
 */


//Read Switch Positions
void readSwitch() {
  for (byte i=0;i<SwitchAddrSize;i++) {
     SwitchPosition[i] = debounceRead(SwitchAddr[i]);
  }
 
}


//Clear memory, both instructions and memory
void clearMemory() {
    for (int i=0; i<instructionArraySize; i++) {
       //Assign null to all instruction positions
       instructionArray[i]=NULL;
    }
    for (int i=0; i<memoryArraySize; i++) {
       //Assign null to all memory positions
       memoryArray[i]=NULL;
    }
} 


// Read an instruction from the switch array, and set the current instruction bit
void readInst() {
    int totalValue=0;
    int currentBitValue=1;
    for (int pin=0; pin<4; pin++) {
        if (SwitchPosition[pin] == LOW) {
           totalValue = totalValue + currentBitValue;
        }
        currentBitValue = currentBitValue*2;
    }
    instructionArray[instructionPointer] = totalValue;
    Serial.print("Instruction: ");
    Serial.println(totalValue);
}

// Read an address from the switch array, and set the current pointer
void readAddr() {
    int totalValue=0;
    int currentBitValue=1;
    for (int pin=0; pin<10; pin++) {
        if (SwitchPosition[pin] == LOW) {
           totalValue = totalValue + currentBitValue;
        }
        currentBitValue = currentBitValue*2;
    }
    instructionPointer = totalValue;

    Serial.print("Address: ");
    Serial.println(totalValue);
}


/*
 *
 *  Low-level Functions
 *
 *
 */


//Update all Datadisplay LEDs to match switch LEDs
//**TODO**: Rewirte to includate data at current instruction pointer!
void displayDataInstLed() {

    if (instructionArray[instructionPointer] == 13) {
        Serial.println("Display NULL VALUE (13) ");
        for (byte i=0;i<4;i++) {
          digitalWrite(DataLEDAddr[i],LOW); // send 0         
        }
    } else {
    Serial.print("Display Data: ");
    Serial.println(instructionArray[instructionPointer]);
    //Heavily modified from http://www.arduino.cc/en/Tutorial/BitMask...although I'm purposly going through the bitmask, and using the size of the
    //Data instruction 
    int mask = 0001;
    for (byte i=0;i<4;i++) {
      if (instructionArray[instructionPointer] & mask){ // if bitwise AND resolves to true
         digitalWrite(DataLEDAddr[i],HIGH); // send 1
      }
      else{ //if bitwise and resolves to false
          digitalWrite(DataLEDAddr[i],LOW); // send 0
      }
      mask <<= 1;
     
    }
   
    }
}


void displayDataAddrLed() {
  //TODO:  there will be an issue, with a 10-bit address pointer.  Probably will have to do something about that mask
  //Display the address of the "instructionPointer" itself..., taken from displayDataInstLed
    Serial.print("Display Data Address: ");
    Serial.println(instructionPointer);


     int mask = 00000001;
     for (byte i=0;i<AddressLEDAddrSize;i++) {
      if (instructionPointer & mask){ // if bitwise AND resolves to true
         digitalWrite(AddressLEDAddr[i],HIGH); // send 1
      }
      else{ //if bitwise and resolves to false
          digitalWrite(AddressLEDAddr[i],LOW); // send 1
      }
      mask <<= 1;
     
    }

}


//DebounceRead...simple routine to debounce a switch
// Input: Pin Number
// Output:  HIGH or LOW
//TODO: Rewrite to be more like Playground example (as this routine will hold up everything else while attempting to debounce...and
//     takes 5ms per read, or 95ms for all switches (1/10th of sec).  Should keep state and delay in global array

int debounceRead(int pinNum) {
    //Simple routine...read the switch, wait
    int state1;
    int state2;
    state1 = digitalRead(pinNum);
    delay(DEBOUNCEDELAY);
    state2 = digitalRead(pinNum);
    while (state1 != state2) {
        delay(DEBOUNCEDELAY);
        state1 = state2;
        state2 = digitalRead(pinNum);
    }
    return state2;
}

// Instruction Run

int mooExec(int instruction) {

  switch( instruction )
    {
    // moo: Search in reverse for MOO command
    case 0:
        {
            Serial.print("Running instruction 0 from Address: ");
            Serial.println(instructionPointer);
            if( instructionPointer == 0 )
                return 1;

            instructionPointer--;    // skip previous command.
            int level = 1;
            while( level > 0 )
            {
                if( instructionPointer == 0 )
                    break;

                instructionPointer--;
           
                if( instructionArray[instructionPointer] == 0 )
                    level++;
                else
                if( instructionArray[instructionPointer] == 7 )  // look for MOO
                    level--;
            }

            if( level != 0 )
                return 1;

            //TEMP ADD:  If successful, I need to decrease pointer by one, so that mail routine will increment by one
            instructionPointer--;
            
            break;  //All was successful, if we reached this point
        }
   
    // mOo: Move memory position back one
    case 1:
            Serial.print("Running instruction 1 from Address: ");
            Serial.println(instructionPointer);
        if( memoryPointer == 0 )
            return 1;
        else
            memoryPointer--;
        break;
       
        Serial.print("Current memory pointer: ");
        Serial.println(memoryPointer);

    // moO: Move memory position up one
    case 2:
            Serial.print("Running instruction 2 from Address: ");
            Serial.println(instructionPointer);
        memoryPointer++;
        //If memory instruction is at end of array
        if( memoryPointer >= memoryArraySize)
        {
            memoryPointer = memoryArraySize;
            memoryPointer--;
        }
        Serial.print("Current memory pointer: ");
        Serial.println(memoryPointer);

        break;
   
    // mOO: Execute block in memory as if it were an instruction
    case 3:
            Serial.print("Running instruction 3 from Address: ");
            Serial.println(instructionPointer);
        if( memoryArray[memoryPointer] == 3 )
            return 1;
       
        mooExec(memoryArray[memoryPointer]);
       
        break;
   
    // Moo: Input/output.  Of memory block is 0, read in value, else write one out
    case 4:
            Serial.print("Running instruction 4 from Address: ");
            Serial.println(instructionPointer);
        if( memoryArray[memoryPointer] != 0 ) {
            // Output Routine
            displayOutput(memoryArray[memoryPointer]);
        }
        else
        {
            //Input Routine
            memoryArray[memoryPointer] = readInput();
        }
        break;
   
    // MOo: Decrement value in memory block by one
    case 5:
            Serial.print("Running instruction 5 from Address: ");
            Serial.println(instructionPointer);
        memoryArray[memoryPointer] = memoryArray[memoryPointer]-1;
        Serial.print("Decrease memory pointer ");
        Serial.print(memoryPointer);
        Serial.print(" to value ");
        Serial.println(memoryArray[memoryPointer]);

        break;
   
    // MoO: Increment value in memory block by one
    case 6:
            Serial.print("Running instruction 6 from Address: ");
            Serial.println(instructionPointer);
            memoryArray[memoryPointer] = memoryArray[memoryPointer]+1;
        Serial.print("Increase memory pointer ");
        Serial.print(memoryPointer);
        Serial.print(" to value ");
        Serial.println(memoryArray[memoryPointer]);


        break;

    // MOO:  If memory value is zero, then skip next command and search for moo command.  If not zero, then continue executing
    case 7:
            Serial.print("Running instruction 7 from Address: ");
            Serial.println(instructionPointer);
        if( memoryArray[memoryPointer] == 0 )
        {
            Serial.println("7: Zero Pointer Encountered");
           int level = 1;
            int prev = 0;
            instructionPointer++;      // have to skip past next command when looking for next moo.
            if( instructionArray[instructionPointer] == 13 )
                break;
            while( level > 0 )
            {
                prev = instructionPointer;
                instructionPointer++;
               
                if( instructionArray[instructionPointer] == 13 )
                    break;
               
                if( instructionArray[instructionPointer] == 7 )
                    level++;
                else if( instructionArray[instructionPointer] == 0 )    // look for moo command.
                {
                    level--;
                    if( prev == 7 )
                        level--;
                }
            }
            if( level != 0 )
                return 1;
        }
        break;
   
    // OOO: Set current memory block to 0
    case 8:
             Serial.print("Running instruction 8 from Address: ");
            Serial.println(instructionPointer);
        memoryArray[memoryPointer] = 0;
        break;

    // MMM: If register has value, put into memory and clear the register, otherwise copy current memory block into register
    case 9:
            Serial.print("Running instruction 9 from Address: ");
            Serial.println(instructionPointer);

        if( registerValue == 0 ) {
           
            registerValue = memoryArray[memoryPointer];
           
        } else {
             memoryArray[memoryPointer] = registerValue; 
             registerValue=0;
        }
        break;

    // OOM: Print current memory as integer (does the same thing as  3 in this case)
    case 10:
            Serial.print("Running instruction 10 from Address: ");
            Serial.println(instructionPointer);

            displayOutput(memoryArray[memoryPointer]);
            break;
   
    // oom: Read an integer (does the same thing as 3 pretty much)
    case 11:
            Serial.print("Running instruction 11 from Address: ");
            Serial.println(instructionPointer);
            memoryArray[memoryPointer] = readInput();
            break;
       

    // bad stuff
    default:
        return 1;
    };
   
    return 0;

}

/*
 * errorHandler:  Some bad happened, so flash Error light, and wait until Run/Stop Switch is in "Stop"
 *
 */


void errorHandler() {
  //Dump debugging information

  Serial.println("Error Occured!");
  Serial.print("Error: instructionPointer: ");
  Serial.println(instructionPointer);
  Serial.print("Error: instruction: ");
  Serial.println(instructionArray[instructionPointer]);

  Serial.print("Error: memoryPointer: ");
  Serial.println(memoryPointer);
  Serial.print("Error: memory: ");
  Serial.println(memoryArray[memoryPointer]);

  //LIght up HLD light (probably should be HLT)
  digitalWrite(HLD,HIGH);
 
  //Wait for Run switch to be turned off (Loop forever until done)
  while (debounceRead(RUN) == LOW) {
     true;
  }
 
  //Turn off HLD light
  digitalWrite(HLD,LOW); 
 
}

//Read Input...read switch configuration, and return the integer.  Only reads "ASCII" value (first 8 bits)

int readInput() {
    //Indicate input is needed...flash input light three times, then leave one
      digitalWrite(INP,HIGH);
      delay(100);
      digitalWrite(INP,LOW);
      delay(100);
      digitalWrite(INP,HIGH);
      delay(100);
      digitalWrite(INP,LOW);
      delay(100);
      digitalWrite(INP,HIGH);
      delay(100);

    //Wait for DEP Toggle.  Loop forever until switch goes low
    while (DEPOSIT == HIGH) {
       true;
    }


    int totalValue=0;
    int currentBitValue=1;
    for (int pin=0; pin<8; pin++) {
        if (SwitchPosition[pin] == LOW) {
           totalValue = totalValue + currentBitValue;
        }
        currentBitValue = currentBitValue*2;
    }
    Serial.print("readInput: ");
    Serial.println(totalValue); 
    digitalWrite(INP,LOW); 


    return totalValue;

}

// displayOutput.  Display the integer passed to the routine
void displayOutput(int displayValue) {
     Serial.print("Display Value: ");
     Serial.println(displayValue);

    //Indicate input is needed...flash input light three times, then leave one
      digitalWrite(OUT,HIGH);
      delay(100);
      digitalWrite(OUT,LOW);
      delay(100);
      digitalWrite(OUT,HIGH);
      delay(100);
      digitalWrite(OUT,LOW);
      delay(100);
      digitalWrite(OUT,HIGH);
      delay(100);
 

     int mask = 00000001;
     for (byte i=0;i<AddressLEDAddrSize;i++) {
      if (displayValue & mask){ // if bitwise AND resolves to true
         digitalWrite(AddressLEDAddr[i],HIGH); // send 1
      }
      else{ //if bitwise and resolves to false
          digitalWrite(AddressLEDAddr[i],LOW); // send 1
      }
      mask <<= 1;
     
    }
   
    //Wait for EXAMING switch to toggle before continuoing
    while (EXAMINE == HIGH) {
       true;
    }
   
    while (EXAMINE == LOW) {
       true;
    }

      digitalWrite(OUT,LOW);
 
}

void saveToROM() {
   //Determine memory "position"

    int totalValue=0;
    int currentBitValue=1;
    for (int pin=0; pin<2; pin++) {
        if (SwitchPosition[pin] == LOW) {
           totalValue = totalValue + currentBitValue;
        }
        currentBitValue = currentBitValue*2;
    }
    int memoryStart;
    memoryStart = totalValue * 1024;

    Serial.print("Saving to EERPOM in location: ");
    Serial.println(memoryStart);
  
    //Clear EEPROM
    int clearStart = memoryStart;
    for (int i = clearStart; i < clearStart+1024; i++)
      EEPROM.write(i, 0);

    //Save current program to eeprom
    int activeProgCounter = 0;
    for (int i = memoryStart; i < memoryStart+1024; i++) {
      EEPROM.write(i, instructionArray[activeProgCounter]);
      activeProgCounter++;
    } 

    Serial.println("Done Saving"); 
}

void loadFromROM() {
       //Determine memory "position"
    int totalValue=0;
    int currentBitValue=1;
    for (int pin=0; pin<2; pin++) {
        if (SwitchPosition[pin] == LOW) {
           totalValue = totalValue + currentBitValue;
        }
        currentBitValue = currentBitValue*2;
    }
    int memoryStart;
    memoryStart = totalValue * 1024;

    Serial.print("Load from memory position: ");
    Serial.println(memoryStart);

    //Clear memroy
    clearMemory();
   
    //Load memory from EEPROM
    int activeProgCounter = 0;
    for (int i = memoryStart; i < memoryStart+1024; i++) {
      instructionArray[activeProgCounter] = EEPROM.read(i);
      activeProgCounter++;
    } 
   
   
    //Setup pointers to beginning
    instructionPointer=0;
    memoryPointer=0;
    displayDataInstLed();
    displayDataAddrLed();

    Serial.println("Done Loading");
}
