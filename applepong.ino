
#include <SoftwareSerial.h>

// Pins
#define ledPin_player1 12 
#define ledPin_player2 13 
#define inPin_player1  6
#define inPin_player2 7
#define enablePin  10   // Connects to the RFID's ENABLE pin
#define rxPin      0  // Serial input (connects to the RFID's SOUT pin)
#define txPin      1  // Serial output (unused)

// Globals
#define BUFSIZE     11  // Size of receive buffer (in bytes) (10-byte unique ID + null character)
#define RFID_START  0x0A  // RFID Reader Start and Stop bytes
#define RFID_STOP   0x0D

// Player Scores, and values
#define MAX_POINTS 5
int val_player1 = 0;
int val_player2 = 0;
int score_player1 = 0;
int score_player2 = 0;

// Debouncing vars
int DebounceTimeLimit = 300; 
int totDebounceTime = 0; 
int lastTimeButtonPushed = 0; 

boolean game_over = false; 

// set up a new serial port
SoftwareSerial rfidSerial =  SoftwareSerial(rxPin, txPin);

void setup()
{
  // Input pins
  pinMode(inPin_player1, INPUT);
  pinMode(inPin_player2, INPUT);
  pinMode(rxPin, INPUT);
    
  // Output pins
  pinMode(ledPin_player1, OUTPUT);
  pinMode(ledPin_player2, OUTPUT);
  pinMode(enablePin, OUTPUT);

  //digitalWrite(enablePin, HIGH);  // disable RFID Reader
  
  // setup Arduino Serial Monitor
  Serial.begin(9600);
  while (!Serial);   // wait until ready
  Serial.println("\n\nParallax RFID Card Reader");
  
  // set the baud rate for the SoftwareSerial port
  rfidSerial.begin(2400);

  digitalWrite(enablePin, LOW);   // enable the RFID Reader
  Serial.println("\n\n RFID Card Reader enabled");
  
  Serial.flush();   // wait for all bytes to be transmitted to the Serial Monitor
}

void loop()
{
  
  readPlayersRFID();
  
  val_player1 = digitalRead(inPin_player1);
  val_player2 = digitalRead(inPin_player2);
  //Serial.println(val);
  //delay(500);
  
  if (val_player1 == LOW & (game_over == false)) // Player1 Pushed the button
  { 
    if (lastTimeButtonPushed)
    {
      lastTimeButtonPushed = millis(); 
      score_player1++; 
    }
    else
    {
      totDebounceTime = millis() - lastTimeButtonPushed; 
      lastTimeButtonPushed = millis(); 
      if (totDebounceTime > DebounceTimeLimit)
      {
       score_player1++; 
      }
    }
    
    //ledBlink(score_player1, ledPin_player1);
    //Serial.println(score);
    //delay(500);
    //digitalWrite(ledPin, HIGH); // LED OFF
    //delay(500);
  }
  else if (val_player2 == LOW & (game_over == false)) // Player1 Pushed the button
  {
    score_player2++; 
    //ledBlink(score_player2, ledPin_player2);
  }
  else
  {
    digitalWrite(ledPin_player1, LOW); // LED ON
    digitalWrite(ledPin_player2, LOW);
    //Serial.println(val);
  }
  
  if (score_player1 > MAX_POINTS || score_player2 > MAX_POINTS)
  {
    game_over = true; 
  }
  
  delay (300);
  
  Serial.print("Score: ");
  Serial.print(score_player1);
  Serial.print("\t");
  Serial.print(score_player2);
  Serial.println("");
}

// Function reads RFID of player1 and player2. Player1 can be predefined as left player, and player2 as right player
void readPlayersRFID()
{ 
  char rfidData[BUFSIZE];  // Buffer for incoming data
  char player1Data[BUFSIZE]; // RFID of player 1
  char offset = 0;         // Offset into buffer
  rfidData[0] = 0;         // Clear the buffer  
  boolean player1_signedin = false; 
  boolean player2_signedin = false; 
  int i = 0; 
  
  // If RFID is disabled, return
  if (digitalRead(enablePin) == HIGH )
  {
    Serial.println("RFID disabled. Return");
    return; 
  }
  while (1)
  {
    if (rfidSerial.available() > 0 && !(player1_signedin == true && player2_signedin == true)) // If there are any bytes available to read, then the RFID Reader has probably seen a valid tag
    {
      rfidData[offset] = rfidSerial.read();  // Get the byte and store it in our buffer
      Serial.println(rfidData[offset]);
      //Serial.println("\n");
      if (rfidData[offset] == RFID_START)    // If we receive the start byte from the RFID Reader, then get ready to receive the tag's unique ID
      {
        Serial.println("start start");
        Serial.println(rfidData[offset]);
        Serial.println("start end");
        offset = -1;     // Clear offset (will be incremented back to 0 at the end of the loop)
      }
      else if (rfidData[offset] == RFID_STOP)  // If we receive the stop byte from the RFID Reader, then the tag's entire unique ID has been sent
      {
        Serial.println("stop start");
        Serial.println(rfidData[offset]);
        Serial.println("stop end");
        rfidData[offset] = 0; // Null terminate the string of bytes we just received
        
        if (player1_signedin == false)
        {
          player1_signedin = true; // Player1 Signed in
          Serial.print("Player1 ID: ");
          Serial.println(rfidData); 
          for (i = 0; i < BUFSIZE; i++)
          {
            player1Data[i] = rfidData[i];
          }
        }
        else if (player1_signedin == true && player2_signedin == false)
        {
          // Match with previous stored data and determine if it's new player data
          for (i = 0; i < BUFSIZE; i++)
          {
            if(player1Data[i] != rfidData[i])
            {
              player2_signedin = true;
              Serial.print("Player2 ID: ");
              Serial.println(rfidData); 
              break; 
            }
          }
        }   
        
        // Break out of the loop only when both of players have tagged in
        if(player1_signedin == true && player2_signedin == true)
        {
          Serial.println("Breaking out of while loop"); 
          break;  // Break out of while loop        
        }  
      }
          
      offset++;  // Increment offset into array
      if (offset >= BUFSIZE) offset = 0; // If the incoming data string is longer than our buffer, wrap around to avoid going out-of-bounds
    }
       
  }
  
  digitalWrite(enablePin, HIGH);   // disable the RFID Reader
  Serial.println("\n\n RFID Card Reader Disabled");
}

// This function would blink player LED n times. n is player's score  
void ledBlink(int score, int led_player)
{
   int i;
   for (i = score; i > 0; i-- )
   {
     digitalWrite(led_player, LOW); // LED OFF
     delay(500);
     
     digitalWrite(led_player, HIGH); // LED OFF
     delay(500);
   }
}
