#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include "MPU6050_tockn.h" // Thư viện MPU6050/MPU6500
#include "MAX30105.h"      // Thư viện MAX30102
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// Cấu hình
#define SDA_PIN 13
#define SCL_PIN 15
#define BUZZER_BUTTON_PIN 33

#define SCREEN_WIDTH 128 // Chiều rộng màn OLED
#define SCREEN_HEIGHT 64 // Chiều cao màn OLED
#define OLED_RESET -1    // Không sử dụng chân reset vật lý
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Thông tin WiFi và MQTT
const char *wifiSSID = "Quynh Chi";
const char *wifiPassword = "123456789";
const char *mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
const char *mqttTopic = "dulieu";

// Đối tượng WiFi và MQTT
WiFiClient espClient;
PubSubClient client(espClient);
Preferences preferences;

// Đối tượng cảm biến
MPU6050 mpu(Wire);
MAX30105 particleSensor;

String deviceID;
String patientID = "1";

void connectWiFi()
{
  Serial.println("Connecting to WiFi...");
  WiFi.begin(wifiSSID, wifiPassword);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT()
{
  while (!client.connected())
  {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESP32_Test_Client"))
    {
      Serial.println("Connected to MQTT Broker!");
    }
    else
    {
      Serial.print("Failed to connect. Error: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

String createJSONPayload(float temp, float accX, float accY, float accZ, float gyroX, float gyroY, float gyroZ, int bpm, int spo2)
{
  String json = "{";
  json += "\"deviceID\":\"" + deviceID + "\",";
  json += "\"patientID\":\"" + patientID + "\",";
  json += "\"temperature\":\"" + String(temp) + "\",";
  json += "\"acceleration\":{\"x\":" + String(accX) + ",\"y\":" + String(accY) + ",\"z\":" + String(accZ) + "},";
  json += "\"gyroscope\":{\"x\":" + String(gyroX) + ",\"y\":" + String(gyroY) + ",\"z\":" + String(gyroZ) + "},";
  json += "\"heartRate\":\"" + String(bpm) + "\",";
  json += "\"spo2\":\"" + String(spo2) + "\"";
  json += "}";
  return json;
}
void update_Oled(float temp, float accX, float accY, float accZ, float gyroX, float gyroY, float gyroZ, int spO2, int heartRate)
{
  display.clearDisplay();
  display.setCursor(0, 0);

  // Hiển thị nhiệt độ
  display.print("Temp: ");
  if (temp > -40 && temp < 85)
  {
    display.print(temp); // In giá trị float hợp lệ
  }
  else
  {
    display.print("N/A"); // In chuỗi "N/A"
  }
  display.println(" C");

  // Hiển thị gia tốc
  display.print("Acc: ");
  display.printf("X=%.2f Y=%.2f Z=%.2f\n", accX, accY, accZ);

  // Hiển thị con quay hồi chuyển
  display.print("Gyro: ");
  display.printf("X=%.2f Y=%.2f Z=%.2f\n", gyroX, gyroY, gyroZ);

  // Hiển thị nhịp tim
  display.print("Heart: ");
  if (heartRate > 0 && heartRate < 200)
  { // Nhịp tim trong khoảng hợp lý
    display.print(heartRate);
    display.println(" bpm");
  }
  else
  {
    display.println("N/A");
  }

  // Hiển thị SpO2
  display.print("SpO2: ");
  if (spO2 >= 70 && spO2 <= 100)
  { // SpO2 hợp lệ từ 70% - 100%
    display.print(spO2);
  }
  else
  {
    display.print("N/A");
  }
  display.println(" %");
  display.display(); // Cập nhật hiển thị
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  pinMode(BUZZER_BUTTON_PIN, OUTPUT);

  // Lấy địa chỉ MAC làm deviceID
  deviceID = WiFi.macAddress();

  // Lưu thông tin vào bộ nhớ flash
  preferences.begin("myApp", false);
  preferences.putString("deviceID", deviceID);
  preferences.putString("patientID", patientID);

  // Kết nối WiFi
  connectWiFi();

  // Cấu hình MQTT
  client.setServer(mqttServer, mqttPort);

  // Khởi động cảm biến MPU6050
  mpu.begin();
  mpu.calcGyroOffsets(true);
  Serial.println("MPU6050 ready!");

  // Khởi động cảm biến MAX30102
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD))
  {
    Serial.println("MAX30102 was not found. Please check wiring/power.");
    while (1)
      ;
  }
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x7F); // Đèn LED đỏ
  particleSensor.setPulseAmplitudeIR(0x7F);  // Đèn LED hồng ngoại

  Serial.println("MAX30102 initialized successfully.");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Địa chỉ I2C mặc định là 0x3C
    Serial.println("SSD1306 allocation failed");
    for (;;)
      ;
  }
  display.clearDisplay();
  display.setTextSize(1);      // Cỡ chữ 1
  display.setTextColor(WHITE); // Màu chữ trắng
  display.setCursor(0, 0);     // Vị trí bắt đầu
  display.println("OLED Initialized");
  display.display();
  delay(500);
}

void loop()
{
  if (!client.connected())
  {
    reconnectMQTT();
  }
  client.loop();

  // Đọc dữ liệu từ MPU6050
  mpu.update();
  float temp = mpu.getTemp();
  float accX = mpu.getAccX();
  float accY = mpu.getAccY();
  float accZ = mpu.getAccZ();
  float gyroX = mpu.getGyroX();
  float gyroY = mpu.getGyroY();
  float gyroZ = mpu.getGyroZ();

  // Đọc dữ liệu từ MAX30102
  long redValue = particleSensor.getRed(); // Giá trị đèn LED đỏ
  long irValue = particleSensor.getIR();   // Giá trị đèn LED hồng ngoại
  int spO2 = 0;
  int heartRate = 0;

  if (redValue < 50000 || irValue < 50000)
  { // Không có ngón tay hoặc tín hiệu yếu
    Serial.println("No finger detected.");
  }
  else
  {
    float ratio = (float)redValue / (float)irValue;
    spO2 = 100 - (17 * ratio / 100);
    heartRate = irValue / 2000.0;
  }
  // Cập nhật OLED với các giá trị
  update_Oled(temp, accX, accY, accZ, gyroX, gyroY, gyroZ, spO2, heartRate);

  // In dữ liệu ra Serial Monitor
  Serial.println("==============================");
  Serial.println("MPU6050 Data:");
  Serial.printf("Temp: %.2f C\n", temp);
  Serial.printf("Acc: X=%.2f Y=%.2f Z=%.2f\n", accX, accY, accZ);
  Serial.printf("Gyro: X=%.2f Y=%.2f Z=%.2f\n", gyroX, gyroY, gyroZ);

  Serial.println("MAX30102 Data:");
  if (heartRate > 0 && heartRate < 200)
  {
    Serial.printf("Heart Rate: %d bpm\n", heartRate);
  }
  else
    Serial.print("Heart Rate: N/A bpm\n");
  if (spO2 >= 70 && spO2 <= 100)
  {
    Serial.printf("SpO2: %d %%\n", spO2);
  }
  else
    Serial.print("SpO2: N/A %%\n");

  // Tạo payload JSON và gửi lên MQTT
  String data_payload = createJSONPayload(temp, accX, accY, accZ, gyroX, gyroY, gyroZ, heartRate, spO2);
  Serial.println(client.state());
  if (client.publish(mqttTopic, data_payload.c_str()))
  {
    Serial.println("Payload sent successfully!");
  }
  else
  {
    Serial.println("Failed to send payload.");
  }
  delay(5000); // Đợi 5 giây trước khi gửi dữ liệu tiếp theo
}
