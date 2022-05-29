# smart-lamp.ino
Created for 6.08: Embedded Systems at MIT.

# Video

Demo: <a href="https://youtu.be/6P7QhrRaGo4" target="_blank">https://youtu.be/6P7QhrRaGo4</a>

# Specifications

Below is how I addressed each specification:

## User Control

**Requirement:**

* The user of the system should control the color/brightness of the smart lamp using a web app. This piece of your system should consist of a user-facing control rendered in HTML (which you can have your Python scripts return).

**Implementation:**

On the server is a Python script `smart_lamp_web.py` that returns the HTML for a minimal user interface upon receiving a GET request. 

## Lamp Control

**Requirement:**

* Upon pushing "Submit" this HTML form will send a POST to the exact same spot from where it was originally served (by a GET request in the browser).
* The lamp should respond to color changes from the web app within one second of them being submitted. It should also maintain those colors even if nobody is actively using the web app. A database is therefore required.

**Implementation:**

After the user sets the desired RGB values in the HTML form, they submit the form and send a POST request to the server script `smart_lamp_web.py`. The script then inputs the form responses into a database.

Every 500 ms, the ESP32 makes a GET request to the server script `smart_lamp.py`, which returns a tuple containing the most recent RGB values inputted into the database. I parse the response buffer using the `strtok` function to separate each color channel.
I then employ the built-in PWM functionality on the ESP32 by associating three PWM modules with the IO pins of the LEDs. The PWM resolution is set at 8 bits per color so that each color channel has 256 possible brightnesses.

# Other Thoughts

The LED does not immediately provide a balanced RGB spectrum. The green LED tends to overpower the red channel, which becomes evident
when mixing the two colors. To calibrate the system, I run all three LEDs at 50% duty cycle and place a white paper slip over the LED. I decrease the duty cycle values of the green and blue channels
until the previous color tints that shone through the paper are no longer visible.

# Code Appendix

**ESP32**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include <WiFi.h> //Connect to WiFi Network
  #include <SPI.h>
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

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Respond to GET requests from ESP32**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
import sqlite3

smart_lamp_db = '/var/jail/home/sophiez/smart_lamp.db'

def request_handler(request):

    conn = sqlite3.connect(smart_lamp_db)
    c = conn.cursor()  # move cursor into database (allows us to execute commands)
    c.execute('''CREATE TABLE IF NOT EXISTS smart_lamp_table (red int, green int, blue int);''') # run a CREATE TABLE command
    things = c.execute('''SELECT * FROM smart_lamp_table''')
    
    for x in things:
        color = (x[0], x[1], x[2])

    return color
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Handle HTML form requests**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
import sqlite3

smart_lamp_db = '/var/jail/home/sophiez/smart_lamp.db'

def request_handler(request):

    if request['method'] == 'GET':
        return f"""<!DOCTYPE html>
        <html>
        <body>

        <h1>Lamp Control</h1>
        <form action="/sandbox/sc/sophiez/smart_lamp_web.py" method="post">
        <label for="red">Red (between 0 and 255):</label>
        <input type="range" id="red" name="red" min="0" max="255">
        <br>
        <label for="green">Green (between 0 and 255):</label>
        <input type="range" id="green" name="green" min="0" max="255">
        <br>
        <label for="blue">Blue (between 0 and 255):</label>
        <input type="range" id="blue" name="blue" min="0" max="255">
        <br>
        <input type="submit">
        </form>

        </body>
        </html>"""

    else:
        conn = sqlite3.connect(smart_lamp_db)
        c = conn.cursor()  # move cursor into database (allows us to execute commands)
        c.execute('''CREATE TABLE IF NOT EXISTS smart_lamp_table (red int, green int, blue int);''') # run a CREATE TABLE command
        c.execute('''INSERT into smart_lamp_table VALUES (?,?,?);''', (request['form']['red'], request['form']['green'], request['form']['blue']))
        conn.commit() # commit commands
        conn.close() # close connection to database

        return (request['form']['red'], request['form']['green'], request['form']['blue'])

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

<style class="fallback">body{visibility:hidden}</style><script>markdeepOptions={tocStyle:'medium'};</script>
<!-- Markdeep: --><script src="https://casual-effects.com/markdeep/latest/markdeep.min.js?" charset="utf-8"></script>
