#include <board.h>
#include <enum.h>
#include <square.h>

Board board;

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
  delay(10000);
}
