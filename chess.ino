#include <board.h>
#include <enum.h>
#include <event.h>
#include <piece.h>
#include <square.h>
#include <utility.h>

Board board;
Event event(white, lift, knight, white, D, 3);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  board.printSerial();
  Serial.println();
  Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:
  board.printSerial();
  Serial.println();
  Serial.println();
  event.printSerial();
  Serial.print(event.rank);
  delay(10000);
}
