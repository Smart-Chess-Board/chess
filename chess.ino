#include <board.h>
#include <enum.h>
#include <event.h>
#include <piece.h>
#include <square.h>
#include <state.h>
#include <utility.h>

//#define LED

Board board;
Event prevEvent;
Event currEvent;
Player turn = white;
State BLACK_STATE = RED;
State WHITE_STATE = GREEN;
bool stateChanged = false;
PieceType promoSelection = empty;

// function prototypes
State stateOfCurrPlayer();
void setupStateLEDs();
void updateStateLEDs();
void printBoardState();
void pollCurrEvent();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
#ifdef LED
  setupStateLEDs();
#endif
}

void loop() {
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
  
  if (WHITE_STATE == RED && BLACK_STATE == RED){
    if (turn == white){
      turn = black;
      BLACK_STATE = GREEN;
    }
    else if (turn == black){
      turn = white;
      WHITE_STATE = GREEN;
    }
  }
#ifdef LED
  setupStateLEDs();
#endif

  // update prevEvent
  stateChanged = (stateOfCurrPlayer() != prevState);
  updatePrevEvent(currEvent, prevEvent, stateOfCurrPlayer(), stateChanged);

}

/*
 _   _  _____ _     ______ ___________  _____ 
| | | ||  ___| |    | ___ \  ___| ___ \/  ___|
| |_| || |__ | |    | |_/ / |__ | |_/ /\ `--. 
|  _  ||  __|| |    |  __/|  __||    /  `--. \
| | | || |___| |____| |   | |___| |\ \ /\__/ /
\_| |_/\____/\_____/\_|   \____/\_| \_|\____/ 

 */

State stateOfCurrPlayer(){
    if (turn == black) return BLACK_STATE;
    else return WHITE_STATE;
}

void setupStateLEDs(){
  // white
  pinMode(13, OUTPUT); // green
  pinMode(12, OUTPUT); // yellow
  pinMode(11, OUTPUT); // red
  //black
  pinMode(6, OUTPUT); // green
  pinMode(5, OUTPUT); // yellow
  pinMode(4, OUTPUT); // red
}

void updateStateLEDs(){
  // set LED
  if (WHITE_STATE == GREEN){
    digitalWrite(13, HIGH);
    digitalWrite(12, LOW);
    digitalWrite(11, LOW);
  }
  else if (WHITE_STATE == RED){
    digitalWrite(13, LOW);
    digitalWrite(12, LOW);
    digitalWrite(11, HIGH);
  }
  else{
    digitalWrite(13, LOW);
    digitalWrite(12, HIGH);
    digitalWrite(11, LOW);
  }

  if (BLACK_STATE == GREEN){
    digitalWrite(6, HIGH);
    digitalWrite(5, LOW);
    digitalWrite(4, LOW);
  }
  else if (BLACK_STATE == RED){
    digitalWrite(6, LOW);
    digitalWrite(5, LOW);
    digitalWrite(4, HIGH);
  }
  else{
    digitalWrite(6, LOW);
    digitalWrite(5, HIGH);
    digitalWrite(4, LOW);
  }
}

void printBoardState(){
  board.printSerial();

  // print piece in hand
  if (prevEvent.action == lift){
    char c = prevEvent.piece.type;
    if (stateOfCurrPlayer() == PROMO3) c = promoSelection;
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
    promoSelection = currEvent.promotion; // change this to piece in hand
  }
  // check if even
  if (stateOfCurrPlayer() == PROMO3){ // move into pollEvent
    currEvent.piece.type = promoSelection;
  }
}
