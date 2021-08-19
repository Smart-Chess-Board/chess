#include <board.h>
#include <enum.h>
#include <event.h>
#include <piece.h>
#include <square.h>
#include <state.h>
#include <utility.h>

Board board;
Event prevEvent;
Event currEvent;
Player turn = white;
State BLACK_STATE = RED;
State WHITE_STATE = GREEN;
PieceType promoSelection = empty;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

  // print board
  Serial.print("----------------------------");
  Serial.println("----------------------------");
  board.printSerial();

  // print piece in hand
  if (prevEvent.action == lift){
    char c = prevEvent.piece.type;
    if (WHITE_STATE == PROMO3 || BLACK_STATE == PROMO3) c = promoSelection;
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
  Serial.println();
  Serial.println("Enter event:");
  currEvent = board.pollEvent(prevEvent, turn);
  currEvent.printSerial();
  Serial.println();

  // debugging output
  Serial.print("currEvent: ");
  currEvent.printSerial();
  Serial.print("prevEvent: ");
  prevEvent.printSerial();
  Serial.println();

  if (currEvent.promotion != empty){
    promoSelection = currEvent.promotion;
  }
  
  // perform updates
  if (WHITE_STATE == PROMO3 || BLACK_STATE == PROMO3){
    currEvent.piece.type = promoSelection;
  }
  board.update(currEvent);
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

  if (currEvent.isNullEvent == false){ // need better code to decide when to update prevEvent
    prevEvent = currEvent;
  }

}
