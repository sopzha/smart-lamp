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
