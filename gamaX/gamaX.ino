#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <LiquidCrystal_I2C.h>

//  Network related variables
const char* ssid = "networkName"; //esp and local machine must be in same network if you run localhost!
const char* password = "password";
int x=1;
unsigned long previousMillis = 0;
unsigned long interval = 8000;
bool rec = false;

const char* serverURL = "http://domain.com/index.php";
const char* apiKey = "a"; // Replace with your API key

//  LCD related variables
LiquidCrystal_I2C lcd(0x27, 16, 2);

//  Sensor related variables
MAX30105 particleSensor;
#define MAX_BRIGHTNESS 255
uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
int32_t bufferLength; //data length
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid
byte pulseLED = 2; //Must be on PWM pin
byte readLED = 13; //Blinks with each data read

//     Button related variables
const int wakeupPin = 25;
const int sleepPin = 33;
RTC_DATA_ATTR int bootCount = 0;

void setup()
{
  Serial.begin(115200); // initialize serial communication at 115200 bits per second:
  bootCount++;
  Serial.print("Boot number:");
  Serial.println(bootCount);
  pinMode(sleepPin, INPUT);
  pinMode(wakeupPin, INPUT);
  Serial.println("Rise and shine");
  esp_sleep_enable_ext0_wakeup((gpio_num_t) wakeupPin, HIGH);
  delay(300);
  WiFi.mode(WIFI_STA); // Set the ESP32 to work as station (connects to router or hotspot)
  WiFi.begin(ssid, password); // Connect to the network using it's network name and password
  Serial.println("Connecting to WiFi network...");

  pinMode(pulseLED, OUTPUT);
  pinMode(readLED, OUTPUT);

  // Initialize the i2c lcd
  lcd.begin(16, 2); //initialize the lcd with 16 columns and 2 rows
  lcd.init();
  lcd.backlight();

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }

  byte ledBrightness = 140; //Options: 0=Off to 255=50mA
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 200; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 16384; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
}

void status() {
  switch (WiFi.status()) {
    case WL_CONNECTED:
      Serial.println("Connected");
      break;
    case WL_IDLE_STATUS:
      Serial.println("Idle");
      break;
    case WL_CONNECT_FAILED:
      Serial.println("Connection Failed");
      break;
    case WL_DISCONNECTED:
      Serial.println("Disconnected");
      break;
    default:
      Serial.println("Unknown");
      break;
  }
}
void printAddress(){
  Serial.print("Network Name (SSID): ");
  Serial.println(WiFi.SSID());
  Serial.print("Connection Status: ");
  status();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  x=x-1;
}
void handler(){
  unsigned long currentMillis = millis(); //Used to keep track of the current time in milliseconds each time the loop() is executed.
  if( (WiFi.status()!=WL_CONNECTED)&&(currentMillis-previousMillis>=interval) ){
    //If the WiFi.status value is not WL_CONNECTED and 5 seconds passed, reconnect!
    rec=true;
    Serial.println("Attempting to connect to a WiFi network...");
    Serial.print("Connection status: ");
    WiFi.disconnect();
    WiFi.reconnect();
    status();
    previousMillis = currentMillis; //Update the previousMillis value, so that it functions properly during the next loop cycle.
    }
    if( (WiFi.status()==WL_CONNECTED) && (rec==true) ){
    Serial.println("Successfully, Reconnected!");
    rec=false;
    }
}
void loop() {
  if(x!=0 && WiFi.status() == WL_CONNECTED){
    printAddress();
    }
    
    bufferLength = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps
  //read the first 3. samples, and determine the signal range
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false) //do we have new data?
      particleSensor.check(); //Check the sensor for new data

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample(); //We're finished with this sample so move to next sample
  }

  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
  while (1)
  {
    handler();
    if(digitalRead(sleepPin)==HIGH){
      Serial.print("Good night!");
      delay(300); // we add this comment so it prints good night then sleep. else it sleeps instantly!
      esp_deep_sleep_start();
      }
    //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    //take 25 sets of samples before calculating the heart rate.
    for (byte i = 75; i < 100; i++)
    {
      while (particleSensor.available() == false) //do we have new data?
        particleSensor.check(); //Check the sensor for new data

      digitalWrite(readLED, !digitalRead(readLED)); //Blink onboard LED with every data read

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); //We're finished with this sample so move to next sample

      //send samples and calculation result to terminal program through UART


      // Print the spo2 and hr values on the serial monitor
      // Clear the lcd
      lcd.clear();
      // Set the cursor to the first row and first column
      lcd.setCursor(0, 0);
      // Print the spo2 value on the lcd
      if(validSPO2==0 || validHeartRate==0 ){
        lcd.print("Place your index");
      }
      else{
      lcd.print("SPO2: ");
      lcd.print(spo2);
      lcd.print("%");
      // Set the cursor to the second row and first column
      lcd.setCursor(0, 1);
      // Print the hr value on the lcd
      lcd.print("HR: ");
      lcd.print(heartRate);
      }

    }

    //After gathering 25 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  // Create the HTTP client object
  HTTPClient http;

  // Create the POST request data
  String postData = "spo2=" + String(spo2) + "&heartRate=" + String(heartRate) + "&api_key=" + apiKey; //Prepare the payload
  // Send the POST request
  http.begin(serverURL); // serverURL is the destination to which the message should be delivered. 
                        //  In our case it's the php script hosted on a web server.
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(postData); //When the message is sent, the server returns a value
                                              //that indicates the status of this attempt "200" is successful operation
  
  // Print the response code
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  
  http.end();

  // Wait for a while before sending the next reading
  delay(1000);  
  }
}