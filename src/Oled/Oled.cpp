// #include <SPI.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
// #include "Oled.h"
// #include "../variables.h"

// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 32
// #define OLED_RESET -1
// #define SCREEN_ADDRESS 0x3C // Địa chỉ I2C của màn hình OLED

// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// extern String deviceID;
// extern String patientID;
// extern long irBuffer;
// void setup_Oled() {
//     if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { 
//       Serial.println(F("SSD1306 allocation failed"));
//       for(;;); // Don't proceed, loop forever
//     }
//     // Khởi tạo màn hình LED
//     display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
//     // Xóa màn hình
//     display.clearDisplay();
    
//     // Cài đặt kích thước và kiểu font chữ
//     display.setTextSize(1);
//     display.setTextColor(SSD1306_WHITE);
//         // // Hiển thị ID bệnh nhân và ID thiết bị lên màn hình
//     display.setCursor(0, 0);
//     display.print("ID: ");
//     display.println(patientID);
//     display.print("Device:");
//     display.println(deviceID);
//     display.print("initialize screen");
    
//     // Cập nhật màn hình
//     display.display();    
// }

// void update_Oled(long irValue, float spO2, float hrValue){
//     display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
//     // Xóa màn hình
//     display.clearDisplay();
    
//     // Cài đặt kích thước và kiểu font chữ
//     display.setTextSize(1);
//     display.setTextColor(SSD1306_WHITE);
//         // // Hiển thị ID bệnh nhân và ID thiết bị lên màn hình
//     display.setCursor(0, 0);
//     display.print("patientID: ");
//     display.println(patientID);
//     display.print("ID: ");
//     display.println(deviceID);
//     if (irValue < 50000) {
//         display.print(" No finger?");
//     }else{
//         display.print("Heart Rate: ");
//         display.print(String(hrValue));
//         display.print("bpm\n");
//         display.print("SpO2: ");
//         display.print(String(spO2));
//         display.print("%");
//     }
//     // Cập nhật màn hình
//     display.display();    
// }
