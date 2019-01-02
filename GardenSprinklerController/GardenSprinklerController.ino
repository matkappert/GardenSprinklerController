
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include "Settings.h"
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>

// Set web server port number to 80
WiFiServer server(80);
int output_state[outputs];
long int output_timmer[outputs];
bool this_on = true;
bool this_off = false;

/*
  ------------------------------------------------------------------------------------------------------------------
  HTML headers and styling crap, don't bother looking at any of this.
  ------------------------------------------------------------------------------------------------------------------
*/
const String HTML_response = "HTTP/1.1 200 OK\r\nContent-type:text/html\r\nConnection: close\r\n";
const String HTML_meta =  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> \r\n <meta http-equiv=\"refresh\" content=\"10; url=/\"> \r\n <link rel=\"icon\" href=\"data:,\">";
const String HTML_css =  "<style>  \r\n html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;} \r\n .button { background-color: #56ac6c; border: none; color: white; padding: 16px 40px; min-width:300px; min-height:50px;  text-decoration: none; font-size: 100px; margin: 2px; cursor: pointer;} \r\n .off {background-color: #E67373;} \r\n </style> \r\n";
const String HTML_header =  "<!DOCTYPE html> <html> \n\r <head> \r\n" + HTML_meta + HTML_css + "\r\n </head> \r\n";

// Assign output variables to GPIO pins


void setup() {
  Serial.begin(115200);


  /*
    ------------------------------------------------------------------------------------------------------------------
    Loop though all the pins and set state to low.
    ------------------------------------------------------------------------------------------------------------------
  */
if (output_invert){
   this_on = false;
   this_off = true;
}

  for (int this_button = 0; this_button < outputs; this_button++) { // loop though all the buttons
    pinMode(output_pins[this_button], OUTPUT); // set this button as an output
    digitalWrite(output_pins[this_button], LOW); // set this output to off
    output_state[this_button] = this_off; // turns the state of the button off
  }


  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;


  if (!enable_dhcp)
    wifiManager.setSTAStaticIPConfig(IP_address, dns, subnet);
  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect( (const char *)project_name.c_str());
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

  // if you get here you have connected to the WiFi
  Serial.println("Connected.");

  if (!MDNS.begin((const char *)project_name.c_str())) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }

  server.begin();

  ArduinoOTA.setHostname((const char *)project_name.c_str());
  ArduinoOTA.begin();


}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    String header; // Variable to store the HTTP request
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        //  Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {

            client.println(HTML_response); // prints the html server response header
            client.println(HTML_header); // print the html header



            // Web Page Heading
            client.println("<body><h1>" + project_name + "</h1>");


            for (int this_button = 0; this_button < outputs; this_button++) { // loop though all the buttons
              /*
                ------------------------------------------------------------------------------------------------------------------
                check if the url page is "set on" or "set off" link
                ------------------------------------------------------------------------------------------------------------------
              */
              if (header.indexOf("GET /" + output_names[this_button] + "/set/on") >= 0) {
                output_state[this_button] = this_on; // turns the state of the button on
                output_timmer[this_button] = millis(); // the current time of the button press
                Serial.println("Button \"" + output_names[this_button] + "\" is now On.");
              }
              else if (header.indexOf("GET /" + output_names[this_button] + "/set/off") >= 0) {
                output_state[this_button] = this_off; // turns the state of the button off
                Serial.println("Button \"" + output_names[this_button] + "\" is now Off.");
              }


              /*
                ------------------------------------------------------------------------------------------------------------------
                display the current state of this button
                ------------------------------------------------------------------------------------------------------------------
              */
              client.println("<p>" + output_names[this_button] + "</p>");  // display this button name
              if (output_state[this_button]) { // if this button is on
                client.println("<p><a href=\"/" + output_names[this_button] + "/set/off\"><button class=\"button\">"  + humanReadableTimeTill(output_durations[this_button], output_timmer[this_button]) + "</button></a></p>");
              } else {// else this button is off
                client.println("<p><a href=\"/" + output_names[this_button] + "/set/on\"><button class=\"button off\">OFF</button></a></p>");
              }

            }



            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }

    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }


  /*
    ------------------------------------------------------------------------------------------------------------------
    check the state of the buttons and timmers
    ------------------------------------------------------------------------------------------------------------------
  */
  for (int this_button = 0; this_button < outputs; this_button++) { // loop though all the buttons
    if (output_state[this_button]) { // if this button is state is on
      if (millis() - output_timmer[this_button] >= (output_durations[this_button] * 60000 )) { // if this button turn on time is more then the duration
        output_state[this_button] = this_off; // turns the state of the button off
        digitalWrite(output_pins[this_button], LOW);// turns the output off
      } else {
        digitalWrite(output_pins[this_button], HIGH);// turns the output on
      }
    } else { // else this button is state is off
      digitalWrite(output_pins[this_button], LOW); // turns the output off
    }
  }



  ArduinoOTA.handle();
  yield();
}


/*
  ------------------------------------------------------------------------------------------------------------------
  turn milliseconds to a human readable time string.
  ------------------------------------------------------------------------------------------------------------------
*/
String humanReadableTimeTill(int durations, long int timmer) {
  int raw =  (durations * 60000) - (millis() - timmer); // milliseconds till end of timmer
  if ((durations * 60000) - raw >= 5000) { // Show the timmer or show text after button press
    int min = raw / 60000; // minutes left = milliseconds ⁒ (number of milliseconds in a minute)
    int sec = (raw / 1000) % 60; // seconds left = (milliseconds ⁒ (number of milliseconds in a seccond)) round over ever 60 seccond
    String rtn = String(min) + ":"; // make a string with the minutes and colon (":")
    if (sec < 10) // if the seconds is between 0-9
      rtn += "0"; // and a "0" infront on the seconds; looks better
             rtn += String(sec); // add seconds to the string
    return rtn;
  } else { // Show the timmer or show text after button press
    return "ON";  // shows the text
  }

}
