/* 
* Matrix of Interruptors - it's recommended to try 2x2, 1x4, and 4x1, but this code supports up to 8x4 as written.
* At this point much of this code is pulled directly from AccordionMega's code.
*/

//This prototype currently supports 4 digital pins, 
//but you can add as many as you want by extending the two arrays below:
char DIGITAL_COLUMNS[] = { 9, 10, 11, 12 };
int KeysStatus[] = {  
  B0000000,
  B0000000,
  B0000000,
  B0000000
};

//this is the value of all pins from the analog port (Note: analog pins 7 and 6 don't exist on UNO)
//B00000000
//B76543210
byte reg_values = 0;

void setup()
{
  Serial.begin(9600);
  //Digital pins start turned off
  for (int i=0; i<sizeof(DIGITAL_COLUMNS);i++){ 
      pinMode(DIGITAL_COLUMNS[i],OUTPUT);
      digitalWrite(DIGITAL_COLUMNS[i], LOW);
  }

  //Arduino UNO has different port letters than the MEGA,
  //so this check is required for this code to be compatible with both boards.
  #if defined (__AVR_ATmega328P__)//UNO
    DDRC = B00000000;  // PortC as input (for Arduino Uno)
    PORTC = B11111111; // turn on pullup resistors
  #elif defined (__AVR_ATmega2560__)//MEGA
    DDRF = B00000000;  // PortF as input (for Arduino Mega)
    PORTF = B11111111; // turn on pullup resistors
  #endif
}

void loop()
{
  for (int i=0; i<sizeof(DIGITAL_COLUMNS);i++){ 
    //TODO: I wonder if we can replace this with a direct port write for better performance?
    digitalWrite(DIGITAL_COLUMNS[i], HIGH);
    delayMicroseconds(500);//This delay value has proven to work with the ITR9608
    
    #if defined (__AVR_ATmega328P__)
      reg_values = ~PINC;
    #elif defined (__AVR_ATmega2560__)
      reg_values = ~PINF;
    #endif
    
    digitalWrite(DIGITAL_COLUMNS[i], LOW);

    if (reg_values != KeysStatus[i]){
      if (reg_values > KeysStatus[i]){ //if it's greater, we're turning the note on; else, turning it off.
        check_key(reg_values ^ KeysStatus[i],i,true);  //using bit-wise OR to send modified bits only
      }
      else {
        check_key(reg_values ^ KeysStatus[i],i,false); //using bit-wise OR to send modified bits only
      }           
    }
  }
}

//Instead of iterating the array from 0-7, use a binary search to find the modified bits faster
void check_key(int reg, int group, boolean up){
   // saving 4 iterations, dividing byte by 2
   if (reg & 0xF0) {
     for(int i=0; i<4; i++){
       if ((reg >> 4+i) & 1){
         note_midi(group,i+4,up);
       }
     }
   }
   else if (reg & 0x0F) { 
    for(int i=0; i<4; i++){
       if ((reg >> i) & 1){
         note_midi(group,i,up);
       }
     }     
   }
}

//This is the mapping of pitches on the right hand of the accordion.
//Note: The right two most bits will always turn on for the UNO because those pins don't exist.
//You can stop this from happening by commenting out the right-most two values in the top row.
const char right_notes_midi_numbers[][8] = {
  {60,59,58,57,56,55,54,53},
  {68,67,66,65,64,63,62,61},
  {76,75,74,73,72,71,70,69},
  {84,83,82,81,80,79,78,77},
  {92,91,90,89,88,87,86,85},
  {0,0,0,0,0,0,0,93}
};

char notes_to_play;

//Set the state of the notes pressed and print data for the new note
void note_midi(int group, int position, boolean on){
  
  int pitch;
  String str_warn;
  byte midi_channel = 1;
  char midi_cmd;
  int midi_vel;

  pitch = right_notes_midi_numbers[group][position];

  if(on) {
    str_warn = "Note on ";
    midi_cmd = midi_channel | 0x90;
    KeysStatus[group] |= (1 << position);  //setting bit value
    Serial.print(" Changed Status: ");
    Serial.println(KeysStatus[group], BIN);
    midi_vel = 127;
  }
  else if(~on) {
    str_warn = "Note off ";
    midi_cmd = midi_channel | 0x80;
    KeysStatus[group] &= ~(1 << position);  //setting bit value
    Serial.print(" Changed Status: ");
    Serial.println(KeysStatus[group], BIN);
    midi_vel = 0;
  }
    
  Serial.print(str_warn);
  Serial.print(pitch,DEC);
  Serial.print(" Vel: ");
  Serial.println(midi_vel);
  Serial.print(" Group: ");
  Serial.println(group);
  Serial.print(" Position: ");
  Serial.println(position);
  
  notes_to_play = right_notes_midi_numbers[group][position];
  if (notes_to_play){
    //This is where we'd print the MIDI Note On/Off command with the ArduinoMIDI library
    //See the Opto-Interruptor MIDI prototype or the final project for more info.
    
    //Serial1.print(midi_cmd, BYTE);
    //Serial1.print(notes_to_play, BYTE);
    //Serial1.print(midi_vel, BYTE);
    Serial.print(" ");
    Serial.println(notes_to_play,DEC);
  }

}
