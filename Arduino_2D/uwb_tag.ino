#include <SPI.h>
#include "DW1000Ranging.h"

#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 4

const uint8_t PIN_RST = 27; 
const uint8_t PIN_IRQ = 34; 
const uint8_t PIN_SS = 4; 

// Global variables to store distances
float distA1 = 0;
float distA2 = 0;
const float distBetweenAnchors = 0.7112; // 71.12 cm converted to meters

void setup() {
    Serial.begin(115200);
    delay(1000);

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);

    DW1000Ranging.attachNewRange(newRange);
    DW1000Ranging.attachNewDevice(newDevice);
    DW1000Ranging.attachInactiveDevice(inactiveDevice);

    DW1000Ranging.startAsTag("7D:00:22:EA:82:60:3B:9C", DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
}

void loop() {
    DW1000Ranging.loop();
}

void newRange() {
    uint16_t shortAddress = DW1000Ranging.getDistantDevice()->getShortAddress();
    float range = DW1000Ranging.getDistantDevice()->getRange() - 0.40;
    // Serial.print(DW1000Ranging.getDistantDevice()->getRange()); 

    // Store the distances based on Anchor ID
    if (shortAddress == 0x1786) {
        distA1 = range;
        Serial.print("Distance A1: "); Serial.println(distA1);
    } else if (shortAddress == 0x1787) {
        distA2 = range;
        Serial.print("Distance A2: "); Serial.println(distA2);
    } else {
        Serial.print(shortAddress); 
    }

    // Only calculate if we have readings from both
    // if (distA1 > 0 &&/distA2 > 0) {
       calculateTriangle(distA1, distA2, distBetweenAnchors);
    // }

}

void calculateTriangle(float a, float b, float c) {
    
    // Check if a triangle is physically possible (Triangle Inequality Theorem)
    // if ((a + b > c) && (a + c > b) && (b + c > a)) {
        // Law of Cosines: cos(A) = (b^2 + c^2 - a^2) / (2bc)
        float cosA = (pow(b, 2) + pow(c, 2) - pow(a, 2)) / (2 * b * c);
        float cosB = (pow(a, 2) + pow(c, 2) - pow(b, 2)) / (2 * a * c);
        float cosC = (pow(a, 2) + pow(b, 2) - pow(c, 2)) / (2 * a * b);

        // Convert to Angles (Radians to Degrees)
        float angleA = acos(cosA) * 180.0 / PI;
        // float angleB = acos(cosB) * 180.0 / PI;
        // float angleC = acos(cosC) * 180.0 / PI;


        // Serial.println("--- Triangle Geometry ---");
        // Serial.print("Sides: a="); Serial.print(a); 
        // Serial.print("m, b="); Serial.print(b); 
        // Serial.print("m, c="); Serial.print(c); Serial.println("m");

        Serial.println("--- Cartesian Coordinates ---");
        Serial.print("DistA: ");
        Serial.println(a); 

        Serial.print("DistB: ");
        Serial.println(b); 

        Serial.print("DistC: ");
        Serial.println(c); 

        float x = sin(angleA) * b; 
        float y = cos(angleA) * b;  

        Serial.print("x = "); 
        Serial.println(x); 

        Serial.print("y = "); 
        Serial.println(y); 

        Serial.print("Angles: A="); Serial.println(angleA); 
        // Serial.print("°, B="); Serial.print(angleB); 
        // Serial.print("°, C="); Serial.print(angleC); Serial.println("°");
        Serial.println("-------------------------");
    // } else {
        // Serial.println("Waiting for valid triangle coordinates...");
    // }
}

void newDevice(DW1000Device *device) {
    Serial.print("Device added: ");
    Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device *device) {
    Serial.print("Device lost: ");
    Serial.println(device->getShortAddress(), HEX);
}