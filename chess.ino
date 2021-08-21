#include <board.h>
#include <enum.h>
#include <event.h>
#include <move.h>
#include <piece.h>
#include <square.h>
#include <state.h>
#include <utility.h>

#include <LiquidCrystal.h>

const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// define pins
#define B_R A5
#define B_Y A4
#define B_G A3
#define W_R 11
#define W_Y 12
#define W_G 13

// function prototypes
void setupStateLEDs();
void updateStateLEDs();

State stateOfCurrPlayer();
void printBoardState();
void pollCurrEvent();

// MAIN CODE + VARIABLES
// variables to track board state
Board board;
Event prevEvent;
Event currEvent;
// variables to track game state
Player turn;
State BLACK_STATE;
State WHITE_STATE;
bool stateChanged;
PieceType promoType;
// variables to track move info
Move move;
Square prevBoard[9][9];
bool isCastle;

void setup() {  
  // put your setup code here, to run once:
  Serial.begin(9600);

  // set up game variables
  turn = white;
  BLACK_STATE = RED;
  WHITE_STATE = GREEN;
  stateChanged = false;
  promoType = empty;
  copyBoard(board.board, prevBoard);
  isCastle = false;
  setupStateLEDs();
  updateStateLEDs();

  lcd.begin(16, 2);
  lcd.clear();

}

void loop() {

  //lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  //lcd.print(millis() / 1000);
  
  // print board
  Serial.println("--------------------------------------------------------");
  printBoardState();
  Serial.println();

  // get next event;
  Serial.println("Enter event:");
  pollCurrEvent();
  currEvent.printSerial();
  Serial.println();

  // debugging output
  Serial.print("currEvent: ");
  currEvent.printSerial();
  Serial.print("prevEvent: ");
  prevEvent.printSerial();
  Serial.println();
  
  // update board
  board.update(currEvent);

  // update states
  State prevState = stateOfCurrPlayer();
  WHITE_STATE = nextState(WHITE_STATE, currEvent, prevEvent);
  BLACK_STATE = nextState(BLACK_STATE, currEvent, prevEvent);

  // check for castle
  if (stateOfCurrPlayer() == CASTLE2){
    isCastle = true;
  }
  
  // check for end of move
  if (WHITE_STATE == RED && BLACK_STATE == RED){
    // extract move info
    move.update(board.board, prevBoard, turn, isCastle);
    String algebraic = move.getLongAlgebraicNotation();
    String UCI = move.getUCINotation();
    Serial.println(algebraic);
    Serial.println(UCI);
    Serial.println();
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(algebraic);
    lcd.setCursor(0, 1);
    lcd.print(UCI);
    
    move.reset();
    copyBoard(board.board, prevBoard); // copy current board into prevBoard

    // switch turns
    if (turn == white){
      turn = black;
      BLACK_STATE = GREEN;
    }
    else if (turn == black){
      turn = white;
      WHITE_STATE = GREEN;
    }
  }
  updateStateLEDs();
  
  // update prevEvent
  stateChanged = (stateOfCurrPlayer() != prevState);
  updatePrevEvent(currEvent, prevEvent, stateOfCurrPlayer(), stateChanged);
}

/*

 /$$   /$$ /$$$$$$$$ /$$       /$$$$$$$  /$$$$$$$$ /$$$$$$$   /$$$$$$ 
| $$  | $$| $$_____/| $$      | $$__  $$| $$_____/| $$__  $$ /$$__  $$
| $$  | $$| $$      | $$      | $$  \ $$| $$      | $$  \ $$| $$  \__/
| $$$$$$$$| $$$$$   | $$      | $$$$$$$/| $$$$$   | $$$$$$$/|  $$$$$$ 
| $$__  $$| $$__/   | $$      | $$____/ | $$__/   | $$__  $$ \____  $$
| $$  | $$| $$      | $$      | $$      | $$      | $$  \ $$ /$$  \ $$
| $$  | $$| $$$$$$$$| $$$$$$$$| $$      | $$$$$$$$| $$  | $$|  $$$$$$/
|__/  |__/|________/|________/|__/      |________/|__/  |__/ \______/ 
                                                                      
 */

State stateOfCurrPlayer(){
    if (turn == black) return BLACK_STATE;
    else return WHITE_STATE;
}

void setupStateLEDs(){
  // white
  pinMode(W_G, OUTPUT); // green
  pinMode(W_Y, OUTPUT); // yellow
  pinMode(W_R, OUTPUT); // red
  //black
  pinMode(B_G, OUTPUT); // green
  pinMode(B_Y, OUTPUT); // yellow
  pinMode(B_R, OUTPUT); // red
}

void updateStateLEDs(){
  // set LED
  if (WHITE_STATE == GREEN){
    digitalWrite(W_G, HIGH);
    digitalWrite(W_Y, LOW);
    digitalWrite(W_R, LOW);
  }
  else if (WHITE_STATE == RED){
    digitalWrite(W_G, LOW);
    digitalWrite(W_Y, LOW);
    digitalWrite(W_R, HIGH);
  }
  else{
    digitalWrite(W_G, LOW);
    digitalWrite(W_Y, HIGH);
    digitalWrite(W_R, LOW);
  }
  
  if (BLACK_STATE == GREEN){
    digitalWrite(B_G, HIGH);
    digitalWrite(B_Y, LOW);
    digitalWrite(B_R, LOW);
  }
  else if (BLACK_STATE == RED){
    digitalWrite(B_G, LOW);
    digitalWrite(B_Y, LOW);
    digitalWrite(B_R, HIGH);
  }
  else{
    digitalWrite(B_G, LOW);
    digitalWrite(B_Y, HIGH);
    digitalWrite(B_R, LOW);
  }
}

void printBoardState(){
  board.printSerial();

  // print piece in hand
  if (prevEvent.action == lift){
    char c = prevEvent.piece.type;
    if (stateOfCurrPlayer() == PROMO3) c = promoType;
    if (prevEvent.piece.color == black) c += 32;
    Serial.println("Piece in hand: " + String(c));
    Serial.println();
  }
  Serial.println();

  // print info
  if (turn == white) Serial.println("Turn: white");
  else if (turn == black) Serial.println("Turn: black");
  Serial.println("white state: " + stateName(WHITE_STATE));
  Serial.println("black state: " + stateName(BLACK_STATE));
}

void pollCurrEvent(){
  currEvent = board.pollEvent(stateOfCurrPlayer(), prevEvent, turn);
  // check if event is promotion
  if (currEvent.promotion != empty){
    promoType = currEvent.promotion; // change this to piece in hand
  }
  // check if even
  if (stateOfCurrPlayer() == PROMO3){ // move into pollEvent
    currEvent.piece.type = promoType;
  }
}
