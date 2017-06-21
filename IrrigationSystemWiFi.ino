
#define STANDALONE

#include <RTClib.h>
#include <RTC_DS3231.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager



#define STATE_ON        0
#define STATE_OFF       1
#define STATE_FORCED_ON 2

void page_index();
void page_get_info();
void page_start_manual();
void page_set_start();


WiFiClient client;
ESP8266WebServer server(80); // Start server on port 80 (default for a web-browser, change to your requirements, e.g. 8080 if your Router uses port 80
// To access server from the outsid of a WiFi network e.g. ESP8266WebServer server(8266); and then add a rule on your Router that forwards a
// connection request to http://your_network_ip_address:8266 to port 8266 and view your ESP server from anywhere.
// Example http://g6ejd.uk.to:8266 will be directed to http://192.168.0.40:8266 or whatever IP address your router gives to this server
//String Argument_Name, Clients_Response;

//Global variable
const int relayPin = 6;
// Use pin 2 as wake up pin and change state pin

volatile int state = STATE_ON;
String 		 state_string = "";

#ifdef STANDALONE
  RTC_Millis RTC;
#else
  RTC_DS3231 RTC;
#endif

// support for read date
DateTime currentDateTimeRead;
// start irrigation variables
int start_hour = 19;
int start_minute = 0;
// irrigation duration variables
int duration_hours = 0;
int duration_minutes = 30;

void setup() 
{
  Serial.begin(115200);
  //WiFiManager intialisation. Once completed there is no need to repeat the process on the current board
  WiFiManager wifiManager;
  // New OOB ESP8266 has no Wi-Fi credentials so will connect and not need the next command to be uncommented and compiled in, a used one with incorrect credentials will
  // so restart the ESP8266 and connect your PC to the wireless access point called 'ESP8266_AP' or whatever you call it below in ""
  // wifiManager.resetSettings(); // Command to be included if needed, then connect to http://192.168.4.1/ and follow instructions to make the WiFi connection
  // Set a timeout until configuration is turned off, useful to retry or go to sleep in n-seconds
  wifiManager.setTimeout(180);
  //fetches ssid and password and tries to connect, if connections succeeds it starts an access point with the name called "ESP8266_AP" and waits in a blocking loop for configuration
  if(!wifiManager.autoConnect("ESP8266_AP")) 
  {
    Serial.println("failed to connect and timeout occurred");
    delay(3000);
    ESP.reset(); //reset and try again
    delay(5000);
  }

// At this stage the WiFi manager will have successfully connected to a network, or if not will try again in 180-seconds
//----------------------------------------------------------------------------------------------------------------------
  Serial.println("WiFi connected..");
  server.begin(); Serial.println("Webserver started..."); // Start the webserver
  Serial.print("Use this URL to connect: http://");// Print the IP address
  Serial.print(WiFi.localIP());Serial.println("/");
  // Next define what the server should do when a client connects
  server.on("/", page_index); // The client connected with no arguments e.g. http:192.160.0.40/
  server.on("/index", page_index);
  server.on("/getInfo", page_get_info);
  server.on("/startManual", page_start_manual);
  server.on("/setStart", page_set_start);


    //--------RTC SETUP ------------
  #ifndef STANDALONE
  Wire.begin();
  RTC.begin();
  // set the correct time
  if (! RTC.isrunning()) 
  {
    lcd.print(" RTC is NOT run");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  #endif
  DateTime now = RTC.now();
  DateTime compiled = DateTime(__DATE__, __TIME__);
  
  if (now.unixtime() < compiled.unixtime()) 
  {
    //Serial.println("RTC is older than compile time! Updating");
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}


void loop() 
{

  switch(state)
  {
  case STATE_ON:
    // read the current time
    currentDateTimeRead = RTC.now();
    int mins, hours;
    // set the time elapsed from the start of irrigation
    hours = currentDateTimeRead.hour() - start_hour;
    mins = currentDateTimeRead.minute() - start_minute;
    
    unsigned int start_time_min;
    unsigned int duration_min;
    unsigned int current_time_min;
    
    start_time_min = start_hour * 60 + start_minute;
    duration_min = duration_hours * 60 + duration_minutes;
    current_time_min = currentDateTimeRead.hour() * 60 + currentDateTimeRead.minute();

    // check if enable or not the irrigation
    //if(((hours <= duration_hours) && (hours >= 0)) && ((mins < duration_minutes) && (mins >= 0)))
    if((start_time_min < current_time_min) && ((start_time_min + duration_min) > current_time_min))
    {
      digitalWrite(relayPin, LOW);
      state_string = "STATE_ON - relay off";

     //Serial.println("STATE ON -- relay off");
    }
    else
    {
      digitalWrite(relayPin, HIGH);
      state_string = "STATE_ON - relay on";
      //Serial.println("STATE ON -- relay on");
    }
  break;
  case STATE_OFF:
    digitalWrite(relayPin, LOW);
	state_string = "STATE_OFF";
    //Serial.println("STATE OFF -- relay off");
  break;
  case STATE_FORCED_ON:
      digitalWrite(relayPin, HIGH);
      state_string = "STATE_FORCED_ON";
      //Serial.println("STATE FORCED ON -- relay on");
  default:
    digitalWrite(relayPin, LOW);
    //Serial.println("STATE default -- relay off");
  }

    server.handleClient();

}

////////////////////////
// page_get_info()
////////////////////////
void page_get_info()
{

  Serial.println("get info");
  String page_out = "";
  page_out += "<!doctype html>";
  page_out += "<html>";
  page_out += "<head>";
  page_out += "<meta charset='utf-8'>";
  page_out += "<title>Get Info</title>";
  page_out += "<style>";
  page_out += "body";
  page_out += "{";
  page_out += "   background-color: #FFFFFF;";
  page_out += "   color: #000000;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "   line-height: 1.1875;";
  page_out += "   margin: 0;";
  page_out += "   padding: 0;";
  page_out += "}";
  page_out += "a";
  page_out += "{";
  page_out += "   color: #0000FF;";
  page_out += "   text-decoration: underline;";
  page_out += "}";
  page_out += "a:visited";
  page_out += "{";
  page_out += "   color: #800080;";
  page_out += "}";
  page_out += "a:active";
  page_out += "{";
  page_out += "   color: #FF0000;";
  page_out += "}";
  page_out += "a:hover";
  page_out += "{";
  page_out += "   color: #0000FF;";
  page_out += "   text-decoration: underline;";
  page_out += "}";
  page_out += "#wb_f_getInfo";
  page_out += "{";
  page_out += "   background-color: #F0FFFF;";
  page_out += "   background-image: none;";
  page_out += "   border: 0px #000000 solid;";
  page_out += "}";
  page_out += "#l_startOre";
  page_out += "{";
  page_out += "   border: 0px #CCCCCC solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: #FAFAD2;";
  page_out += "   background-image: none;";
  page_out += "   color :#000000;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "   padding: 4px 4px 4px 4px;";
  page_out += "   text-align: left;";
  page_out += "   vertical-align: middle;";
  page_out += "}";
  page_out += "#wb_t_startOre";
  page_out += "{";
  page_out += "   background-color: transparent;";
  page_out += "   background-image: none;";
  page_out += "   border: 0px #C0C0C0 solid;";
  page_out += "   padding: 0;";
  page_out += "   margin: 0;";
  page_out += "   text-align: left;";
  page_out += "}";
  page_out += "#wb_t_startOre div";
  page_out += "{";
  page_out += "   text-align: left;";
  page_out += "}";
  page_out += "#l_startMinuti";
  page_out += "{";
  page_out += "   border: 0px #CCCCCC solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: #FFFACD;";
  page_out += "   background-image: none;";
  page_out += "   color :#000000;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "   padding: 4px 4px 4px 4px;";
  page_out += "   text-align: left;";
  page_out += "   vertical-align: middle;";
  page_out += "}";
  page_out += "#wb_t_startMinuti";
  page_out += "{";
  page_out += "   background-color: transparent;";
  page_out += "   background-image: none;";
  page_out += "   border: 0px #C0C0C0 solid;";
  page_out += "   padding: 0;";
  page_out += "   margin: 0;";
  page_out += "   text-align: left;";
  page_out += "}";
  page_out += "#wb_t_startMinuti div";
  page_out += "{";
  page_out += "   text-align: left;";
  page_out += "}";
  page_out += "#wb_t_durata";
  page_out += "{";
  page_out += "   background-color: transparent;";
  page_out += "   background-image: none;";
  page_out += "   border: 0px #C0C0C0 solid;";
  page_out += "   padding: 0;";
  page_out += "   margin: 0;";
  page_out += "   text-align: left;";
  page_out += "}";
  page_out += "#wb_t_durata div";
  page_out += "{";
  page_out += "   text-align: left;";
  page_out += "}";
  page_out += "#l_durata";
  page_out += "{";
  page_out += "   border: 0px #FFFFFF solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: #FFA07A;";
  page_out += "   background-image: none;";
  page_out += "   color :#000000;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "   padding: 4px 4px 4px 4px;";
  page_out += "   text-align: left;";
  page_out += "   vertical-align: middle;";
  page_out += "}";
  page_out += "#l_currentState";
  page_out += "{";
  page_out += "   border: 0px #CCCCCC solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: #00FF7F;";
  page_out += "   background-image: none;";
  page_out += "   color :#000000;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "   padding: 4px 4px 4px 4px;";
  page_out += "   text-align: left;";
  page_out += "   vertical-align: middle;";
  page_out += "}";
  page_out += "#wb_t_currentState";
  page_out += "{";
  page_out += "   background-color: transparent;";
  page_out += "   background-image: none;";
  page_out += "   border: 0px #C0C0C0 solid;";
  page_out += "   padding: 0;";
  page_out += "   margin: 0;";
  page_out += "   text-align: left;";
  page_out += "}";
  page_out += "#wb_t_currentState div";
  page_out += "{";
  page_out += "   text-align: left;";
  page_out += "}";
  page_out += "#b_back";
  page_out += "{";
  page_out += "   border: 1px #2E6DA4 solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: #3370B7;";
  page_out += "   background-image: none;";
  page_out += "   color: #FFFFFF;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "}";
  page_out += "#Line1";
  page_out += "{";
  page_out += "   color: #708090;";
  page_out += "   background-color: #708090;";
  page_out += "   border-width: 0;";
  page_out += "   margin: 0;";
  page_out += "   padding: 0;";
  page_out += "}";
  page_out += "#l_curretnTime";
  page_out += "{";
  page_out += "   border: 0px #CCCCCC solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: #00FF7F;";
  page_out += "   background-image: none;";
  page_out += "   color :#000000;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "   padding: 4px 4px 4px 4px;";
  page_out += "   text-align: left;";
  page_out += "   vertical-align: middle;";
  page_out += "}";
  page_out += "#wb_t_currentTime";
  page_out += "{";
  page_out += "   background-color: transparent;";
  page_out += "   background-image: none;";
  page_out += "   border: 0px #C0C0C0 solid;";
  page_out += "   padding: 0;";
  page_out += "   margin: 0;";
  page_out += "   text-align: left;";
  page_out += "}";
  page_out += "#wb_t_currentTime div";
  page_out += "{";
  page_out += "   text-align: left;";
  page_out += "}";
  page_out += "#b_updateInfo";
  page_out += "{";
  page_out += "   border: 1px #2E6DA4 solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: #3370B7;";
  page_out += "   background-image: none;";
  page_out += "   color: #FFFFFF;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "}";
  page_out += "</style>";
  page_out += "</head>";
  page_out += "<body>";
  page_out += "<div id='wb_f_getInfo' style='position:absolute;left:0px;top:0px;width:256px;height:204px;z-index:11;'>";
  page_out += "<form name='Form1' method='post' action='' enctype='text/plain' id='f_getInfo'>";
  page_out += "<label for='' id='l_startOre' style='position:absolute;left:8px;top:4px;width:83px;height:16px;line-height:16px;z-index:0;'>Start Ore:</label>";
  page_out += "<label for='l_startOre' id='l_startMinuti' style='position:absolute;left:8px;top:32px;width:82px;height:16px;line-height:16px;z-index:1;'>Start Minuti:</label>";
  page_out += "<div id='wb_t_startOre' style='position:absolute;left:103px;top:8px;width:150px;height:21px;z-index:2;'>";
  page_out += "<div style='font-family:Arial;font-size:13px;line-height:16px;color:#000000;'>";
  page_out += "<div>" + String(start_hour) + "</div>";
  page_out += "</div>";
  page_out += "</div>";
  page_out += "<div id='wb_t_startMinuti' style='position:absolute;left:102px;top:36px;width:150px;height:17px;z-index:3;'>";
  page_out += "<div style='font-family:Arial;font-size:13px;line-height:16px;color:#000000;'>";
  page_out += "<div>" + String(start_minute) + "</div>";
  page_out += "</div>";
  page_out += "</div>";
  page_out += "<div id='wb_t_durata' style='position:absolute;left:103px;top:77px;width:149px;height:19px;z-index:4;'>";
  page_out += "<div style='font-family:Arial;font-size:13px;line-height:16px;color:#000000;'>";
  page_out += "<div>" + String(duration_hours) + ":" + String(duration_minutes) + "</div>";
  page_out += "</div>";
  page_out += "</div>";
  page_out += "<label for='l_startOre' id='l_currentState' style='position:absolute;left:6px;top:129px;width:84px;height:16px;line-height:16px;z-index:5;'>Current State:</label>";
  page_out += "<div id='wb_t_currentState' style='position:absolute;left:103px;top:133px;width:149px;height:19px;z-index:6;'>";
  page_out += "<div style='font-family:Arial;font-size:13px;line-height:16px;color:#000000;'>";
  page_out += "<div>" + state_string + "</div>";
  page_out += "</div>";
  page_out += "</div>";
  page_out += "<label for='l_startOre' id='l_durata' style='position:absolute;left:6px;top:73px;width:84px;height:16px;line-height:16px;z-index:7;'>Durata (min):</label>";
  page_out += "<label for='l_startOre' id='l_curretnTime' style='position:absolute;left:6px;top:168px;width:84px;height:16px;line-height:16px;z-index:8;'>Current Time:</label>";
  page_out += "<div id='wb_t_currentTime' style='position:absolute;left:103px;top:172px;width:149px;height:19px;z-index:9;'>";
  page_out += "<div style='font-family:Arial;font-size:13px;line-height:16px;color:#000000;'>";
  page_out += "<div>" + String(RTC.now().hour()) + ":" + String(RTC.now().minute()) + "</div>";
  page_out += "</div>";
  page_out += "</div>";
  page_out += "</form>";
  page_out += "</div>";
  page_out += "<a href='/index'><input type='button' id='b_back' name='' value='Back' style='position:absolute;left:7px;top:217px;width:97px;height:26px;z-index:12;'></a>";
  page_out += "<hr id='Line1' style='position:absolute;left:5px;top:111px;width:251px;height:4px;z-index:13;'>";
  page_out += "<a href='/getInfo'><input type='button' id='b_updateInfo' name='' value='Update Info' style='position:absolute;left:154px;top:217px;width:97px;height:26px;z-index:14;'>";
  page_out += "</body>";
  page_out += "</html>";

  server.send(200, "text/html", page_out); // Send a response to the client asking for input

  state = STATE_ON;
}

////////////////////////
// page_index()
////////////////////////
void page_index()
{

  Serial.println("index");
  String page_out = "";
  page_out += "<!doctype html>";
  page_out += "<html>";
  page_out += "<head>";
  page_out += "<meta charset='utf-8'>";
  page_out += "<title>Untitled Page</title>";
  page_out += "<meta name='generator' content='Quick 'n Easy Web Builder - http://www.quickandeasywebbuilder.com'>";
  page_out += "<style>";
  page_out += "body";
  page_out += "{";
  page_out += "   background-color: #FFFFFF;";
  page_out += "   color: #000000;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "   line-height: 1.1875;";
  page_out += "   margin: 0;";
  page_out += "   padding: 0;";
  page_out += "}";
  page_out += "a";
  page_out += "{";
  page_out += "   color: #0000FF;";
  page_out += "   text-decoration: underline;";
  page_out += "}";
  page_out += "a:visited";
  page_out += "{";
  page_out += "   color: #800080;";
  page_out += "}";
  page_out += "a:active";
  page_out += "{";
  page_out += "   color: #FF0000;";
  page_out += "}";
  page_out += "a:hover";
  page_out += "{";
  page_out += "   color: #0000FF;";
  page_out += "   text-decoration: underline;";
  page_out += "}";
  page_out += "#pb_getInfo";
  page_out += "{";
  page_out += "   border: 1px #2E6DA4 solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: #3370B7;";
  page_out += "   background-image: none;";
  page_out += "   color: #FFFFFF;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "}";
  page_out += "#pb_startManual";
  page_out += "{";
  page_out += "   border: 1px #2E6DA4 solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: #3370B7;";
  page_out += "   background-image: none;";
  page_out += "   color: #FFFFFF;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "}";
  page_out += "#pb_setStart";
  page_out += "{";
  page_out += "   border: 1px #2E6DA4 solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: #3370B7;";
  page_out += "   background-image: none;";
  page_out += "   color: #FFFFFF;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "}";
  page_out += "</style>";
  page_out += "</head>";
  page_out += "<body>";
  page_out += "<a href='/getInfo'><input type='button' id='pb_getInfo' name='' value='Get Info' style='position:absolute;left:8px;top:8px;width:188px;height:40px;z-index:1;'></a>";
  page_out += "<a href='/startManual'><input type='button' id='pb_startManual' name='' value='Start Manual' style='position:absolute;left:8px;top:56px;width:188px;height:40px;z-index:2;'></a>";
  page_out += "<a href='/setStart'><input type='button' id='pb_setStart' name='' value='Set Start' style='position:absolute;left:9px;top:104px;width:188px;height:40px;z-index:3;'></a>";
  page_out += "</body>";
  page_out += "</html>";

    server.send(200, "text/html", page_out); // Send a response to the client asking for input

  state = STATE_ON;
}

////////////////////////
// page_set_start()
////////////////////////
void page_set_start()
{

  Serial.println("set start");
  String page_out = "";

  page_out += "<!doctype html>";
  page_out += "<html>";
  page_out += "<head>";
  page_out += "<meta charset='utf-8'>";
  page_out += "<title>Set Start</title>";
  page_out += "<style>";
  page_out += "body";
  page_out += "{";
  page_out += "   background-color: #FFFFFF;";
  page_out += "   color: #000000;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "   line-height: 1.1875;";
  page_out += "   margin: 0;";
  page_out += "   padding: 0;";
  page_out += "}";
  page_out += "a";
  page_out += "{";
  page_out += "   color: #0000FF;";
  page_out += "   text-decoration: underline;";
  page_out += "}";
  page_out += "a:visited";
  page_out += "{";
  page_out += "   color: #800080;";
  page_out += "}";
  page_out += "a:active";
  page_out += "{";
  page_out += "   color: #FF0000;";
  page_out += "}";
  page_out += "a:hover";
  page_out += "{";
  page_out += "   color: #0000FF;";
  page_out += "   text-decoration: underline;";
  page_out += "}";
  page_out += "#wb_Form1";
  page_out += "{";
  page_out += "   background-color: #F4F4F4;";
  page_out += "   background-image: none;";
  page_out += "   border: 0px #000000 solid;";
  page_out += "}";
  page_out += "#l_statMinuti";
  page_out += "{";
  page_out += "   border: 0px #CCCCCC solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: transparent;";
  page_out += "   background-image: none;";
  page_out += "   color :#000000;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "   padding: 4px 4px 4px 4px;";
  page_out += "   text-align: left;";
  page_out += "   vertical-align: middle;";
  page_out += "}";
  page_out += "#le_startMinuti";
  page_out += "{";
  page_out += "   border: 1px #CCCCCC solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: #FFFFFF;";
  page_out += "   background-image: none;";
  page_out += "   color :#000000;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "   padding: 4px 4px 4px 4px;";
  page_out += "   text-align: left;";
  page_out += "   vertical-align: middle;";
  page_out += "}";
  page_out += "#le_startMinuti:focus";
  page_out += "{";
  page_out += "   border-color: #66AFE9;";
  page_out += "   -webkit-box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102, 175, 233, 0.60);";
  page_out += "   -moz-box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102, 175, 233, 0.60);";
  page_out += "   box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102, 175, 233, 0.60);";
  page_out += "   outline: 0;";
  page_out += "}";
  page_out += "#l_startOre";
  page_out += "{";
  page_out += "   border: 0px #CCCCCC solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: transparent;";
  page_out += "   background-image: none;";
  page_out += "   color :#000000;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "   padding: 4px 4px 4px 4px;";
  page_out += "   text-align: left;";
  page_out += "   vertical-align: middle;";
  page_out += "}";
  page_out += "#le_startOre";
  page_out += "{";
  page_out += "   border: 1px #CCCCCC solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: #FFFFFF;";
  page_out += "   background-image: none;";
  page_out += "   color :#000000;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "   padding: 4px 4px 4px 4px;";
  page_out += "   text-align: left;";
  page_out += "   vertical-align: middle;";
  page_out += "}";
  page_out += "#le_startOre:focus";
  page_out += "{";
  page_out += "   border-color: #66AFE9;";
  page_out += "   -webkit-box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102, 175, 233, 0.60);";
  page_out += "   -moz-box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102, 175, 233, 0.60);";
  page_out += "   box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102, 175, 233, 0.60);";
  page_out += "   outline: 0;";
  page_out += "}";
  page_out += "#l_durata";
  page_out += "{";
  page_out += "   border: 0px #CCCCCC solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: transparent;";
  page_out += "   background-image: none;";
  page_out += "   color :#000000;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "   padding: 4px 4px 4px 4px;";
  page_out += "   text-align: left;";
  page_out += "   vertical-align: middle;";
  page_out += "}";
  page_out += "#le_durata";
  page_out += "{";
  page_out += "   border: 1px #CCCCCC solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: #FFFFFF;";
  page_out += "   background-image: none;";
  page_out += "   color :#000000;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "   padding: 4px 4px 4px 4px;";
  page_out += "   text-align: left;";
  page_out += "   vertical-align: middle;";
  page_out += "}";
  page_out += "#le_durata:focus";
  page_out += "{";
  page_out += "   border-color: #66AFE9;";
  page_out += "   -webkit-box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102, 175, 233, 0.60);";
  page_out += "   -moz-box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102, 175, 233, 0.60);";
  page_out += "   box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102, 175, 233, 0.60);";
  page_out += "   outline: 0;";
  page_out += "}";
  page_out += "#b_set";
  page_out += "{";
  page_out += "   border: 1px #2E6DA4 solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: #3370B7;";
  page_out += "   background-image: none;";
  page_out += "   color: #FFFFFF;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "}";
  page_out += "#b_back";
  page_out += "{";
  page_out += "   border: 1px #2E6DA4 solid;";
  page_out += "   -moz-border-radius: 4px;";
  page_out += "   -webkit-border-radius: 4px;";
  page_out += "   border-radius: 4px;";
  page_out += "   background-color: #3370B7;";
  page_out += "   background-image: none;";
  page_out += "   color: #FFFFFF;";
  page_out += "   font-family: Arial;";
  page_out += "   font-weight: normal;";
  page_out += "   font-size: 13px;";
  page_out += "}";
  page_out += "</style>";
  page_out += "<SCRIPT TYPE='text/JavaScript'>";
  page_out += " function validateHh(inputField) {";
  page_out += "   var x = parseInt(inputField.value, 10);";
  page_out += "   if(x >= 0 && x <= 24)";
  page_out += "   {";
  page_out += "     document.getElementById('b_set').disabled = false; ";
  page_out += "     return true;";
  page_out += "   }";
  page_out += "   else";
  page_out += "   {";
  page_out += "     document.getElementById('b_set').disabled = true;";
  page_out += "     alert('Inserisci correttamente start ore');";
  page_out += "     return false;";
  page_out += "   }";
  page_out += " }";
  page_out += " function validateMm(inputField) {";
  page_out += "   var x = parseInt(inputField.value, 10);";
  page_out += "   if(x >= 0 && x <= 60)";
  page_out += "   {";
  page_out += "     document.getElementById('b_set').disabled = false; ";
  page_out += "     return true;";
  page_out += "   }";
  page_out += "   else";
  page_out += "   {";
  page_out += "     document.getElementById('b_set').disabled = true; ";
  page_out += "     alert('Inserisci correttamente start minuti');";
  page_out += "     return false;";
  page_out += "   }";
  page_out += " }";
  page_out += "   function validateDurata(inputField) {";
  page_out += "   var x = parseInt(inputField.value, 10);";
  page_out += "   if(x > 0 && x <= 360)";
  page_out += "   {";
  page_out += "     document.getElementById('b_set').disabled = false; ";
  page_out += "     return true;";
  page_out += "   }";
  page_out += "   else";
  page_out += "   {";
  page_out += "     document.getElementById('b_set').disabled = true; ";
  page_out += "     alert('Inserisci correttamente durata');";
  page_out += "     return false;";
  page_out += "   }";
  page_out += " }";
  page_out += "";
  page_out += "</SCRIPT>";
  page_out += "</head>";
  page_out += "<body>";
  page_out += "<div id='wb_Form1' style='position:absolute;left:0px;top:0px;width:316px;height:160px;z-index:8;'>";
  page_out += "<form name='set_start' method='post' action='' enctype='text/plain' id='Form1'>";
  page_out += "<label for='le_startMinuti' id='l_statMinuti' style='position:absolute;left:13px;top:50px;width:73px;height:17px;line-height:17px;z-index:0;'>Start Minuti</label>";
  page_out += "<input type='text' id='le_startOre' style='position:absolute;left:96px;top:17px;width:190px;height:15px;line-height:15px;z-index:1;' name='le_startOre' value='' spellcheck='false' onchange='validateHh(this);'>";
  page_out += "<label for='le_durata' id='l_durata' style='position:absolute;left:10px;top:82px;width:73px;height:17px;line-height:17px;z-index:2;'>Durata (min)</label>";
  page_out += "<input type='text' id='le_durata' style='position:absolute;left:96px;top:82px;width:190px;height:15px;line-height:15px;z-index:3;' name='le_durata' value='' spellcheck='false' onchange='validateDurata(this);'>";
  page_out += "<input type='text' id='le_startMinuti' style='position:absolute;left:96px;top:49px;width:190px;height:15px;line-height:15px;z-index:4;' name='le_startMinuti' value='' spellcheck='false' onchange='validateMm(this);'>";
  page_out += "<input type='submit' id='b_set' name='' value='Set' style='position:absolute;left:202px;top:121px;width:97px;height:26px;z-index:5;'>";
  page_out += "<label for='le_startOre' id='l_startOre' style='position:absolute;left:17px;top:16px;width:73px;height:17px;line-height:17px;z-index:6;'>Start Ore</label>";
  page_out += "</form>";
  page_out += "</div>";
  page_out += "<a href='/index'><input type='button' id='b_back' name='' value='Back' style='position:absolute;left:12px;top:169px;width:97px;height:26px;z-index:9;'></a>";
  page_out += "</body>";
  page_out += "</html>";


  server.send(200, "text/html", page_out); // Send a response to the client asking for input
  if (server.args() > 0 ) 
  { // Arguments were received
    for ( uint8_t i = 0; i < server.args(); i++ ) 
    {
	   String Argument_Name = server.argName(i);
	   String client_response = server.arg(i);
	   Serial.print(Argument_Name); // Display the argument
      if (Argument_Name == "le_startOre") 
      {
        Serial.print(" Input received was: ");
        Serial.println(client_response);

        start_hour = client_response.toInt();
      }
      if (Argument_Name == "le_startMinuti") 
      {
        Serial.print(" Input received was: ");
        Serial.println(client_response);

        start_minute = client_response.toInt();
      }
      if (Argument_Name == "le_durata") 
      {
        Serial.print(" Input received was: ");
        Serial.println(client_response);

        duration_hours = (client_response.toInt()) / 60;
        duration_minutes = (client_response.toInt()) % 60;
        
      }
    }
  }

  state = STATE_OFF;
}


////////////////////////
// page_start_manual()
////////////////////////
void page_start_manual()
{

	Serial.println("set manual");
	String page_out = "";

	page_out += "<!doctype html>";
	page_out += "<html>";
	page_out += "<head>";
	page_out += "<meta charset='utf-8'>";
	page_out += "<title>Start Manual</title>";
	page_out += "<style>";
	page_out += "body";
	page_out += "{";
	page_out += "   background-color: #FFFFFF;";
	page_out += "   color: #000000;";
	page_out += "   font-family: Arial;";
	page_out += "   font-weight: normal;";
	page_out += "   font-size: 13px;";
	page_out += "   line-height: 1.1875;";
	page_out += "   margin: 0;";
	page_out += "   padding: 0;";
	page_out += "}";
	page_out += "a";
	page_out += "{";
	page_out += "   color: #0000FF;";
	page_out += "   text-decoration: underline;";
	page_out += "}";
	page_out += "a:visited";
	page_out += "{";
	page_out += "   color: #800080;";
	page_out += "}";
	page_out += "a:active";
	page_out += "{";
	page_out += "   color: #FF0000;";
	page_out += "}";
	page_out += "a:hover";
	page_out += "{";
	page_out += "   color: #0000FF;";
	page_out += "   text-decoration: underline;";
	page_out += "}";
	page_out += "#wb_Form1";
	page_out += "{";
	page_out += "   background-color: #F4F4F4;";
	page_out += "   background-image: none;";
	page_out += "   border: 0px #000000 solid;";
	page_out += "}";
	page_out += "#b_irrigationOn";
	page_out += "{";
	page_out += "   border: 1px #2E6DA4 solid;";
	page_out += "   -moz-border-radius: 4px;";
	page_out += "   -webkit-border-radius: 4px;";
	page_out += "   border-radius: 4px;";
	page_out += "   background-color: #3370B7;";
	page_out += "   background-image: none;";
	page_out += "   color: #FFFFFF;";
	page_out += "   font-family: Arial;";
	page_out += "   font-weight: normal;";
	page_out += "   font-size: 13px;";
	page_out += "}";
	page_out += "#b_stopIrrigation";
	page_out += "{";
	page_out += "   border: 1px #2E6DA4 solid;";
	page_out += "   -moz-border-radius: 4px;";
	page_out += "   -webkit-border-radius: 4px;";
	page_out += "   border-radius: 4px;";
	page_out += "   background-color: #3370B7;";
	page_out += "   background-image: none;";
	page_out += "   color: #FFFFFF;";
	page_out += "   font-family: Arial;";
	page_out += "   font-weight: normal;";
	page_out += "   font-size: 13px;";
	page_out += "}";
	page_out += "#l_irrigationState";
	page_out += "{";
	page_out += "   border: 0px #CCCCCC solid;";
	page_out += "   -moz-border-radius: 4px;";
	page_out += "   -webkit-border-radius: 4px;";
	page_out += "   border-radius: 4px;";
	page_out += "   background-color: #FFFFFF;";
	page_out += "   background-image: none;";
	page_out += "   color :#000000;";
	page_out += "   font-family: Arial;";
	page_out += "   font-weight: normal;";
	page_out += "   font-size: 13px;";
	page_out += "   padding: 4px 4px 4px 4px;";
	page_out += "   text-align: left;";
	page_out += "   vertical-align: middle;";
	page_out += "}";
	page_out += "#wb_t_irrigationState";
	page_out += "{";
	page_out += "   background-color: transparent;";
	page_out += "   background-image: none;";
	page_out += "   border: 0px #C0C0C0 solid;";
	page_out += "   padding: 0;";
	page_out += "   margin: 0;";
	page_out += "   text-align: left;";
	page_out += "}";
	page_out += "#wb_t_irrigationState div";
	page_out += "{";
	page_out += "   text-align: left;";
	page_out += "}";
	page_out += "#l_currentTime";
	page_out += "{";
	page_out += "   border: 0px #CCCCCC solid;";
	page_out += "   -moz-border-radius: 4px;";
	page_out += "   -webkit-border-radius: 4px;";
	page_out += "   border-radius: 4px;";
	page_out += "   background-color: #FFFFFF;";
	page_out += "   background-image: none;";
	page_out += "   color :#000000;";
	page_out += "   font-family: Arial;";
	page_out += "   font-weight: normal;";
	page_out += "   font-size: 13px;";
	page_out += "   padding: 4px 4px 4px 4px;";
	page_out += "   text-align: left;";
	page_out += "   vertical-align: middle;";
	page_out += "}";
	page_out += "#wb_t_currentTime";
	page_out += "{";
	page_out += "   background-color: transparent;";
	page_out += "   background-image: none;";
	page_out += "   border: 0px #C0C0C0 solid;";
	page_out += "   padding: 0;";
	page_out += "   margin: 0;";
	page_out += "   text-align: left;";
	page_out += "}";
	page_out += "#wb_t_currentTime div";
	page_out += "{";
	page_out += "   text-align: left;";
	page_out += "}";
	page_out += "#b_back";
	page_out += "{";
	page_out += "   border: 1px #2E6DA4 solid;";
	page_out += "   -moz-border-radius: 4px;";
	page_out += "   -webkit-border-radius: 4px;";
	page_out += "   border-radius: 4px;";
	page_out += "   background-color: #3370B7;";
	page_out += "   background-image: none;";
	page_out += "   color: #FFFFFF;";
	page_out += "   font-family: Arial;";
	page_out += "   font-weight: normal;";
	page_out += "   font-size: 13px;";
	page_out += "}";
	page_out += "#b_update";
	page_out += "{";
	page_out += "   border: 1px #2E6DA4 solid;";
	page_out += "   -moz-border-radius: 4px;";
	page_out += "   -webkit-border-radius: 4px;";
	page_out += "   border-radius: 4px;";
	page_out += "   background-color: #3370B7;";
	page_out += "   background-image: none;";
	page_out += "   color: #FFFFFF;";
	page_out += "   font-family: Arial;";
	page_out += "   font-weight: normal;";
	page_out += "   font-size: 13px;";
	page_out += "}";
	page_out += "#wb_Form2";
	page_out += "{";
	page_out += "   background-color: #F4F4F4;";
	page_out += "   background-image: none;";
	page_out += "   border: 0px #000000 solid;";
	page_out += "}";
	page_out += "</style>";
	page_out += "</head>";
	page_out += "<body>";
	page_out += "<div id='wb_Form1' style='position:absolute;left:0px;top:0px;width:305px;height:67px;z-index:3;'>";
	page_out += "<form name='Form1' method='post' action='' enctype='text/plain' id='Form1'>";
	page_out += "<input type='submit' id='b_irrigationOn' name='' value='Start Irrigation' style='position:absolute;left:69px;top:12px;width:151px;height:46px;z-index:0;'>";
	page_out += "</form>";
	page_out += "</div>";
	page_out += "<label for='' id='l_irrigationState' style='position:absolute;left:15px;top:151px;width:93px;height:19px;line-height:19px;z-index:4;'>Irrigation State:</label>";
	page_out += "<div id='wb_t_irrigationState' style='position:absolute;left:124px;top:155px;width:181px;height:19px;z-index:5;'>";
	page_out += "<div style='font-family:Arial;font-size:13px;line-height:16px;color:#000000;'>";
	page_out += "<div>" + state_string + </div>";
	page_out += "</div>";
	page_out += "</div>";
	page_out += "<label for='' id='l_currentTime' style='position:absolute;left:15px;top:183px;width:93px;height:19px;line-height:19px;z-index:6;'>Current Time:</label>";
	page_out += "<div id='wb_t_currentTime' style='position:absolute;left:124px;top:187px;width:181px;height:19px;z-index:7;'>";
	page_out += "<div style='font-family:Arial;font-size:13px;line-height:16px;color:#000000;'>";
	page_out += "<div>" + String(RTC.now().hour()) + ":" + String(RTC.now().minute()) + "</div>";
	page_out += "</div>";
	page_out += "</div>";
	page_out += "<input type='button' id='b_back' name='' value='Back' style='position:absolute;left:6px;top:242px;width:97px;height:26px;z-index:8;'>";
	page_out += "<input type='button' id='b_update' name='' value='Update' style='position:absolute;left:206px;top:242px;width:97px;height:26px;z-index:9;'>";
	page_out += "<div id='wb_Form2' style='position:absolute;left:0px;top:67px;width:305px;height:67px;z-index:10;'>";
	page_out += "<form name='Form2' method='post' action='' enctype='text/plain' id='Form2'>";
	page_out += "<input type='submit' id='b_stopIrrigation' name='' value='Stop Irrigation' style='position:absolute;left:67px;top:12px;width:151px;height:46px;z-index:1;'>";
	page_out += "</form>";
	page_out += "</div>";
	page_out += "</body>";
	page_out += "</html>";

     server.send(200, "text/html", page_out); // Send a response to the client asking for input
  if (server.args() > 0 ) 
  { // Arguments were received
    for ( uint8_t i = 0; i < server.args(); i++ ) 
    {
	   String Argument_Name = server.argName(i);
	   String client_response = server.arg(i);
	   Serial.print(Argument_Name); // Display the argument

      if (Argument_Name == "b_irrigationOn") 
      {
        Serial.println(" b_irrigationOn ");
        state = STATE_FORCED_ON;
        // e.g. range_maximum = server.arg(i).toInt(); // use string.toInt() if you wanted to convert the input to an integer number
        // e.g. range_maximum = server.arg(i).toFloat(); // use string.toFloat() if you wanted to convert the input to a floating point number
     }
      if (Argument_Name == "b_stopIrrigation") 
      {
        Serial.println(" b_stopIrrigation ");
        state = STATE_OFF;
     }
    }
  }
}
