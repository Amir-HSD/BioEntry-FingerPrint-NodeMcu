#include <SPI.h>
#include <Wire.h>
#include <WiFiClient.h> 
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>    
#include <ArduinoJson.h>
#include <Adafruit_Fingerprint.h>
#include "config.h"

WiFiClientSecure client;


int getFingerprintIDez();

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println();

  //-----------initiate OLED display-------------
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
  //---------------------------------------------

  display.clearDisplay();
  display.drawBitmap(32, 0, HSD, FinPr_start_width, FinPr_start_height, WHITE);
  display.display();

  delay(2000);

  connectToWiFi();

  finger.begin(57600);
  Serial.println("\n\nAdafruit finger detect test");
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    display.clearDisplay();
    display.drawBitmap(34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    display.clearDisplay();
    display.drawBitmap(32, 0, FinPr_failed_bits, FinPr_failed_width, FinPr_failed_height, WHITE);
    display.display();
    while (1) {
      delay(1);
    }
  }
  
  finger.getTemplateCount();
  Serial.print("Sensor contains ");
  Serial.print(finger.templateCount);
  Serial.println(" templates");
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
  Serial.println("Waiting for request...");

}

uint16_t FingerFreeId(void) {
  uint16_t num = 0;

  while (num == 0) {
    finger.getTemplateCount();
    num = finger.templateCount + 1;
  }
  return num;
}

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

String Status = "Null";
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  if(Status == "S"){
    String Result = scanFingerprint();
    Serial.println(Result);
  }else if(Status == "A"){
    String Result = getFingerprintEnroll();
    Serial.println(Result);
  }else if(Status == "D"){
    String Result = deleteFingerprint();
    Serial.println(Result);
  }

  if (Serial.available() > 0) {
    char command = Serial.read();

    switch (command) {
      case 'S': // Scan
        Status = "S";
        Serial.println("command: S");
        break;
      case 'A': // Add
        Status = "A";
        Serial.println("command: A");
        break;
      case 'D': // Delete
        Status = "D";
        Serial.println("command: D");
        break;
      case 'N': // Disable
        Status = "Null";
        Serial.println("command: N");
        break;
      default:
        break;
    }
  }

  delay(1000);
}

String getFingerprintEnroll() {

  int p = -1;
  id = FingerFreeId();
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return "FINGERPRINT_IMAGEMESS";
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return "FINGERPRINT_PACKETRECIEVEERR";
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return "FINGERPRINT_FEATUREFAIL";
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return "FINGERPRINT_INVALIDIMAGE";
    default:
      Serial.println("Unknown error");
      return "Image2Tz_UnknownError";
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return "FINGERPRINT_IMAGEMESS";
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return "FINGERPRINT_PACKETRECIEVEERR";
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return "FINGERPRINT_FEATUREFAIL";
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return "FINGERPRINT_INVALIDIMAGE";
    default:
      Serial.println("Unknown error");
      return "Image2Tz_UnknownError";
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return "FINGERPRINT_PACKETRECIEVEERR";
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return "FINGERPRINT_ENROLLMISMATCH";
  } else {
    Serial.println("Unknown error");
    return "CreateModel_UnknownError";
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return "FINGERPRINT_PACKETRECIEVEERR";
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return "FINGERPRINT_BADLOCATION";
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return "FINGERPRINT_FLASHERR";
  } else {
    Serial.println("Unknown error");
    return "StoreModel_UnknownError";
  }
  Status = "Null";
  String Result = "Stored ID #";
  Result += finger.fingerID;
  return Result;
}

String scanFingerprint() {

  display.clearDisplay();
  display.drawBitmap(32, 0, FinPr_start_bits, FinPr_start_width, FinPr_start_height, WHITE);
  display.display();

  uint8_t p = finger.getImage();
  if (p == FINGERPRINT_OK) {
    //Serial.println("Image taken");
  } else if (p == FINGERPRINT_NOFINGER) {
    return "FINGERPRINT_NOFINGER";
  } else if (p == FINGERPRINT_IMAGEFAIL) {
    return "FINGERPRINT_IMAGEFAIL";
  } else {
    return "GetImage_UnknownError";
  }
  
  p = finger.image2Tz();
  if (p == FINGERPRINT_OK) {
    //Serial.println("Image converted");
  } else if (p == FINGERPRINT_IMAGEMESS) {
    return "FINGERPRINT_IMAGEMESS";
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    return "FINGERPRINT_PACKETRECIEVEERR";
  } else if (p == FINGERPRINT_FEATUREFAIL) {
    return "FINGERPRINT_FEATUREFAIL";
  } else if (p == FINGERPRINT_INVALIDIMAGE) {
    return "FINGERPRINT_INVALIDIMAGE";
  } else {
    return "Image2Tz_UnknownError";
  }

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    //Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    return "FINGERPRINT_PACKETRECIEVEERR";
  } else if (p == FINGERPRINT_NOTFOUND) {
    return "FINGERPRINT_NOTFOUND";
  } else {
    return "FingerSearch_UnknownError";
  }

  // Serial.print("Found ID #"); Serial.print(finger.fingerID);
  // Serial.print(" with confidence of "); Serial.println(finger.confidence);
  String Result = "Found ID #";
  Result += finger.fingerID;
  return Result;

}

String deleteFingerprint() {
  uint8_t p = -1;

  id = readnumber();

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    // Serial.println("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    return "FingerSearch_UnknownError";
  } else if (p == FINGERPRINT_BADLOCATION) {
    return "FingerSearch_UnknownError";
  } else if (p == FINGERPRINT_FLASHERR) {
    return "FingerSearch_UnknownError";
  } else {
    return "FingerSearch_UnknownError";
  }

  String Result = "Deleted ID #";
  Result += id;
  return Result;
}

void DisplayFingerprintID(int FingerID) {
  if (FingerID > 0) {
    display.clearDisplay();
    display.drawBitmap(34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
  } else if (FingerID == 0) {
    display.clearDisplay();
    display.drawBitmap(32, 0, FinPr_start_bits, FinPr_start_width, FinPr_start_height, WHITE);
    display.display();
  } else if (FingerID == -1) {
    display.clearDisplay();
    display.drawBitmap(34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
    display.display();
  } else if (FingerID == -2) {
    display.clearDisplay();
    display.drawBitmap(32, 0, FinPr_failed_bits, FinPr_failed_width, FinPr_failed_height, WHITE);
    display.display();
  }
}

void connectToWiFi() {
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(F("Connecting to \n"));
  display.setCursor(0, 50);
  display.setTextSize(2);
  display.print(ssid);
  display.drawBitmap(73, 10, Wifi_start_bits, Wifi_start_width, Wifi_start_height, WHITE);
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected");

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(8, 0);
  display.print(F("Connected \n"));
  display.drawBitmap(33, 15, Wifi_connected_bits, Wifi_connected_width, Wifi_connected_height, WHITE);
  display.display();

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
