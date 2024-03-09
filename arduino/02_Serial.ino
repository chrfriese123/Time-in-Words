#include "00_config.h"

void serial_setup() {
  Serial.begin(115200);
  Serial.println("\n\n");
}

void setup_header() {
  Serial.println(F("#####################################"));
  Serial.println(F("#  Textclock rev0.1                 #"));
  Serial.println(F("#                                   #"));
  Serial.println(F("#-----------------------------------#"));
  Serial.println(F("#  (c) 2023 Chr.Friese              #"));
  Serial.println(F("#####################################\n"));
  Serial.println(F("Running setup on"));
}
