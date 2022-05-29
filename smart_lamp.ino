#include <wifi.h> //Connect to WiFi Network
  #include <spi.h>
  #include<math.h>
  #include<string.h>
  
  const int BLUE_LED = 32;
  const int RED_LED = 13;
  const int GREEN_LED = 33;
  
  const uint32_t PWM_CHANNEL_BLUE = 0;
  const uint32_t PWM_CHANNEL_RED = 1;
  const uint32_t PWM_CHANNEL_GREEN = 2;
  
  //char network[] = "EECS_Labs";  //SSID for 6.08 Lab
  char network[] = "MIT";
  char password[] = ""; //Password for 6.08 Lab
  
  //Some constants and some resources:
  const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
  const uint16_t IN_BUFFER_SIZE = 3500;
  const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
  char request[IN_BUFFER_SIZE];
  char response[OUT_BUFFER_SIZE];
  
  uint32_t request_timer; // ensure we request every 500 ms
  
  int red;
  int green;
  int blue;
  
  void setup() {
    Serial.begin(115200); //for debugging if needed.
    WiFi.begin(network, password); //attempt to connect to wifi
    uint8_t count = 0; //count used for Wifi check times
    Serial.print("Attempting to connect to ");
    Serial.println(network);
    while (WiFi.status() != WL_CONNECTED && count < 12) {
      delay(500);
      Serial.print(".");
      count++;
    }
    delay(2000);
    if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
      Serial.println("CONNECTED!");
      Serial.printf("%d:%d:%d:%d (%s) (%s)\n", WiFi.localIP()[3], WiFi.localIP()[2],
                    WiFi.localIP()[1], WiFi.localIP()[0],
                    WiFi.macAddress().c_str() , WiFi.SSID().c_str());    delay(500);
    } else { //if we failed to connect just Try again.
      Serial.println("Failed to Connect :/  Going to restart");
      Serial.println(WiFi.status());
      ESP.restart(); // restart the ESP (proper way)
    }
  
    //create pwm channel, @1000 Hz, with 8 bits of precision
    //link each pwm channel to corresponding led pin
    ledcSetup(PWM_CHANNEL_BLUE, 1000, 8);
    ledcAttachPin(BLUE_LED, PWM_CHANNEL_BLUE);
    ledcSetup(PWM_CHANNEL_RED, 1000, 8);
    ledcAttachPin(RED_LED, PWM_CHANNEL_RED);
    ledcSetup(PWM_CHANNEL_GREEN, 1000, 8);
    ledcAttachPin(GREEN_LED, PWM_CHANNEL_GREEN);
  
    request_timer = millis();
  }
  
  void loop() {
    if (millis() - request_timer >= 500) { // Make POST request to server every 500 ms
      char request_body[100];
      request[0] = '\0';
      int offset = 0;
      offset += sprintf(request + offset, "GET /sandbox/sc/sophiez/smart_lamp.py HTTP/1.1\r\n");
      offset += sprintf(request + offset, "Host: 608dev-2.net\r\n\r\n");
      do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
      Serial.println("RESPONSE");
      Serial.println(response);
      request_timer = millis();
  
      // Get RGB values from tuple
      char* token = strtok(response, "(), ");
      int count = 1;
      while (token != NULL) {
        count++;
        if (count == 2) {
          red = atoi(token);
        }
        if (count == 3) {
          green = atoi(token);
        }
        if (count == 4) {
          blue = atoi(token);
        }
        token = strtok(NULL, "(), ");
      }
  
    }
  
    // Update LED
    ledcWrite(PWM_CHANNEL_RED, red);
    ledcWrite(PWM_CHANNEL_GREEN, 0.75*green);
    ledcWrite(PWM_CHANNEL_BLUE, 0.85*blue);
  }
