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
#define W_R A5
#define W_Y A4
#define W_G A3
#define B_R 11
#define B_Y 12
#define B_G 13

// function prototypes
void setupStateLEDs();
void updateStateLEDs();
void showLastMove(String uci);
void showAIMove(String uci);

State stateOfCurrPlayer();
void printBoardState();
void pollCurrEvent();
void sendMoveToPi(const String& uci);

// MAIN CODE + VARIABLES
// variables to track board state
Board board;
Event prevEvent;
Event currEvent;

// variables to track game state
Player turn = white;
State BLACK_STATE = RED;
State WHITE_STATE = GREEN;
bool stateChanged = false;
PieceType promoType = empty;

// variables to track move info
Move move;
Square prevBoard[9][9];
bool isCastle = false;

// variables for chess AI
bool isWhiteAI;
bool isBlackAI;

void setup() {  
  // put your setup code here, to run once:
  Serial.begin(9600);

  // set up game variables
  copyBoard(board.board, prevBoard);
  setupStateLEDs();
  updateStateLEDs();

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Last move: ");
  lcd.setCursor(0, 1);
  lcd.print("AI move: ");

  // set up game state
  // TODO: this should be done with switches
  String input = getInputFromPi();
  if (input == "y" || input == "yes") isWhiteAI = false;    
  else isWhiteAI = true;
  
  input = getInputFromPi();
  if (input == "y" || input == "yes")isBlackAI = false;    
  else isBlackAI = true;
  
  if (isWhiteAI){
    String bestMove = getInputFromPi();
    showAIMove(bestMove);
  }
}

void loop() {
  // print board
  Serial.println("--------------------------------------------------------");
  printBoardState();

  // get next event;
  Serial.println("Enter event:");
  //pollCurrEvent();
  getEventFromPi();
  currEvent.printSerial();
  
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
    sendMoveToPi(UCI);
    
    Serial.println(algebraic);
    Serial.println(UCI);
    showLastMove(UCI);
    
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
    updateStateLEDs();
    
    // request move from stockfish if applicable
    if ((turn == white && isWhiteAI) || (turn == black && isBlackAI)){
      String bestMove = getInputFromPi();
      showAIMove(bestMove);
    }
  }
  else{
    updateStateLEDs();
  }
  
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

void showLastMove(String uci){
  lcd.setCursor(11, 0);
  lcd.print(uci);
}
void showAIMove(String uci){
  lcd.setCursor(11, 1);
  lcd.print(uci);
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

void getEventFromPi(){
  String input = getInputFromPi();
  currEvent = board.getEvent(input, stateOfCurrPlayer(), prevEvent, turn);
  // check if event is promotion
  if (currEvent.promotion != empty){
    promoType = currEvent.promotion; // change this to piece in hand
  }
  // check if even
  if (stateOfCurrPlayer() == PROMO3){ // move into pollEvent
    currEvent.piece.type = promoType;
  }
}

void sendMoveToPi(const String& uci){
  Serial.print("M: ");
  Serial.println(uci);
}

String getInputFromPi(){
  while (!Serial.available());
  String ret = Serial.readString();
  Serial.println("ACK: " + ret);
  return ret;
}
