// ESP8266_ILI9341_permanent_openweathermap_time_alarm
// Version Board: ESP8266 3.0.1
// Version Library:
// Adafruit_ILI9341.h : 1.0.6
// Arduino_JSON.h: 0.2.0
// NTPClient by Fabrice: 3.2.1
// 2024 Coding by Vinh Cao
//
// pins:
// TFT SPI ILI9341 - ESP8266 
// VCC  ------------ 3V3
// GND  ------------ GND 
// CS   ------------ D2
// RST  ------------ D3
// D/C  ------------ D4
// MOSI ------------ D7  
// SCK  ------------ D5       
// BL   ------------ 3V3

   #include <Adafruit_GFX.h>                                                                    // Adafruit graphics library
   #include <Adafruit_ILI9341.h>                                                                // Adafruit ILI9341 TFT library
   #define TFT_CS      D2                                                                       // TFT CS  pin is connected to NodeMCU pin D2
   #define TFT_RST     D3                                                                       // TFT RST pin is connected to NodeMCU pin D3
   #define TFT_DC      D4                                                                       // TFT DC  pin is connected to NodeMCU pin D4
   #define TFT_SCK     D5                
   #define TFT_MOSI    D7

   Adafruit_ILI9341 tft = Adafruit_ILI9341 (TFT_CS, TFT_DC, TFT_RST);                           // constructor
 
   #define BLACK       0x0000                                                                   // standard color scheme
   #define BLUE        0x001F
   #define RED         0xF800
   #define GREEN       0x07E0
   #define CYAN        0x07FF
   #define MAGENTA     0xF81F
   #define YELLOW      0xFFE0
   #define WHITE       0xFFFF
   #define SCALE0      0xC655                                                                   // accent color for unused rainbow scale segments                                   
   #define SCALE1      0x5DEE                                                                   // accent color for unused rainbow scale segments         
   #define SCALE2      0x10CE                                                                   // blue used as fill in baro panel
      
   #define RED2RED     0                                                                        // rainbow scale ring meter color scheme
   #define GREEN2GREEN 1
   #define BLUE2BLUE   2
   #define BLUE2RED    3
   #define GREEN2RED   4
   #define RED2GREEN   5

   #define DEG2RAD 0.0174532925   


   #include <ESP8266WiFi.h>
   #include <ESP8266WebServer.h>
   #include <ESP8266HTTPClient.h>
   #include <WiFiClient.h>
   #include <Arduino_JSON.h>
   #include <Fonts/FreeMono9pt7b.h>
   #include <NTPClient.h>
  #include <WiFiUdp.h>
   const char* ssid = "VC Analog 2";                                                                  // your wifi SSID here
   const char* password = "12356789";                                                   // your wifi password here 
   String openWeatherMapApiKey = "48f28ad53b9e7d78735b2e7ee328789d";                                    // your OpenWeatherMap APIkey here  
   String city = "Tinh Quang Nam";                                                                      // replace with your city's name
   String countryCode = "vn";                                                                   // replace with your country identification
   String jsonBuffer;

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// for a final application, check the API call limits per hour/minute to avoid getting blocked/banned
   unsigned long lastTime = 0;
// timer set to 10 minutes (600000)
   unsigned long timerDelay = 30000;                                                           // currently one initial call and then every ten minutes 
// set timer to 30 seconds (30000)                                                              // handy during the testing phase of your project
// unsigned long timerDelay = 30000;

   int delayTime = 500;
                                                       
// joint variables
   float temp_01=10;
   float hum_01;
   int   hum_02; 
   int   press_02 = 960;                                                                        // necessary for initial dynamic baro panel
   float w_speed_01;
   float w_speed_02;
   float press_01;
   

// rainbow scale ring meter variables
   uint32_t runTime = -99999;                                                                   // time for next update
   int reading = 10;                                                                            // value to be displayed in circular scale
   int tesmod = 0;
   int r_gauge_pos_x;                                                                           // these two variables govern the position
   int r_gauge_pos_y;                                                                           // of the square + gauge on the display
   int ringmeter_radius = 65;                                                                   // governs diameter of rainbow gauge
   char* ringlabel[] = {"","*C","%","mBar"};                                                    // some custom labels
   float temp_rainbowgauge;  
   int t;                                                                                       // x-position of numeric in rainbow scale center                                                                 // governs position of numerical output rainbow scale

// small needle humidity meter 
   int   j;   
   float center_x1;                                                                             // center x of edge markers humidity gauge 
   float center_y1;                                                                             // center y of edge markers humidity gauge         
   int   radius_s = 65;                                                                         // for scale markers
   int   needle_length = 45;                                                                    // gauge needle length
   int   edgemarkerLength = 5;                                                                  // edge marker length               
   float edge_x1, edge_y1, edge_x1_out, edge_y1_out;   
   float angle_needle = 0;
   float needle_x, needle_y;                                                                        
   float needle_x_old, needle_y_old;
   float angle_circle = 0;
   int   pivot_x;                                                                               // pivot coordinates needle of small gauge hum panel
   int   pivot_y;

   int   static_num_panel_x;
   int   static_num_panel_y;
   int   baro_panel_x;
   int   baro_panel_y;
   float barval;   

   int   p1_x, p1_y, p1_x_old, p1_y_old;                                                        // barometer panel polygon points
   int   p2_x, p2_y, p2_x_old, p2_y_old;
   int   p3_x, p3_y, p3_x_old, p3_y_old;
   int   p4_x, p4_y, p4_x_old, p4_y_old;

   float compassPivot_x;                                                                        // wind compass direction pointer
   float compassPivot_y;  
   float c_x1, c_x2,c_x3, c_x4;
   float c_y1, c_y2,c_y3, c_y4;
   float c_x1_old,c_x2_old,c_x3_old, c_x4_old;
   float c_y1_old,c_y2_old,c_y3_old, c_y4_old;    
   float wind_dir_01;
   int   wind_dir_02;
   float compass_angle;
   int   compass_r = 22;
   char* sector [] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};                              // wind sector labels
   int   h; 

   float max_temp_today;                                                                        // mini max temp panel 
   float min_temp_today;
   int   minmax_panel_x; 
   int   minmax_panel_y; 
   int   t_meter_increase;
   int   t_meter_increase_old;
 
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
ESP8266WebServer server(80);
//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

// Variables for storing alarm time
int alarmHour = -1;
int alarmMinute = -1;

#define BUZZER_PIN D8
// Variables for storing alarm time
int State = LOW; 
unsigned long previousMillis = 0;
void handleRoot() {
  String html = "<html><body><h1>Set Alarm</h1>";
  html += "<form action=\"/setAlarm\" method=\"GET\">";
  html += "Hour: <input type=\"number\" name=\"hour\" min=\"0\" max=\"23\"><br>";
  html += "Minute: <input type=\"number\" name=\"minute\" min=\"0\" max=\"59\"><br>";
  html += "<input type=\"submit\" value=\"Set Alarm\">";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleSetAlarm() {
  if (server.hasArg("hour") && server.hasArg("minute")) {
    alarmHour = server.arg("hour").toInt();
    alarmMinute = server.arg("minute").toInt();
    server.send(200, "text/html", "<html><body><h1>Alarm Set</h1><p>Bao thuc da duoc dat thanh cong!</p><a href=\"/\">Back</a></body></html>");
  } else {
    server.send(400, "text/html", "<html><body><h1>Invalid Request</h1><a href=\"/\">Back</a></body></html>");
  }
}
// Set your Static IP address
IPAddress local_IP(192, 168, 1, 50);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional
void setup() {
    pinMode(BUZZER_PIN, OUTPUT);
   Serial.begin(115200);
     // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
   WiFi.begin (ssid, password);
   Serial.println ("Connecting");

   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
   }
   Serial.println("");
   Serial.print("Connected to WiFi network with IP Address: ");
   Serial.println(WiFi.localIP());

  //  TIME ########################3
// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(25200); //GMT +7 Vietnam

  //TIME ////////////////
 
   Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");
  
   tft.begin ();
   tft.setRotation (3);
   tft.fillScreen (BLACK);

   draw_all_panel_frames (); 
   title_panel_text();
   static_data_in_numeric_window ();
   static_data_in_rainbow_panel ();
   humidity_static ();
   compassGauge ();
   rainbowScaleMeter ();                                                                      
   build_dynamic_baropanel ();                                                                  // to start dynamic baropanel
   static_minmax_panel ();

   Serial.println ("First run to get data. Next update according to timerDelay");
   Serial.println ();
   run_the_weatherstation (); // we do this here once to get the station up and running. next update: see timerDelay
  // Initialize web server
  server.on("/", handleRoot);
  server.on("/setAlarm", handleSetAlarm);
  server.begin();
}

void loop() {
   timeClient.update();
  server.handleClient();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();

  char hourStr[3];
  char minuteStr[3];
  char secondStr[3];

  sprintf(hourStr, "%02d", currentHour);
  sprintf(minuteStr, "%02d", currentMinute);
  sprintf(secondStr, "%02d", currentSecond);
   
  tft.setTextColor(YELLOW, BLACK);
  tft.setFont(&FreeMono9pt7b);
  tft.setTextSize(1);

   tft.fillRect (static_num_panel_x+60,static_num_panel_y+57,70,15,BLACK); 
   tft.setCursor (static_num_panel_x+60,static_num_panel_y+71); 
  //  if (press_02 <1000) tft.setCursor (static_num_panel_x+65,static_num_panel_y+71); 
   tft.print (alarmHour); 

      tft.fillRect (static_num_panel_x+85,static_num_panel_y+57,70,15,BLACK); 
   tft.setCursor (static_num_panel_x+85,static_num_panel_y+71); 
  //  if (press_02 <1000) tft.setCursor (static_num_panel_x+65,static_num_panel_y+71); 
   tft.print (":"); 


      tft.fillRect (static_num_panel_x+100,static_num_panel_y+57,70,15,BLACK); 
   tft.setCursor (static_num_panel_x+100,static_num_panel_y+71); 
   tft.print (alarmMinute); 

  // Display hour
  tft.fillRect(static_num_panel_x + 10, static_num_panel_y + 123, 40, 15, ILI9341_BLACK);
  tft.setCursor(static_num_panel_x + 10, static_num_panel_y + 135);
  tft.print(hourStr);

  // Display colon
  tft.fillRect(static_num_panel_x + 40, static_num_panel_y + 123, 10, 15, ILI9341_BLACK);
  tft.setCursor(static_num_panel_x + 40, static_num_panel_y + 135);
  tft.print(":");

  // Display minute
  tft.fillRect(static_num_panel_x + 60, static_num_panel_y + 123, 40, 15, ILI9341_BLACK);
  tft.setCursor(static_num_panel_x + 60, static_num_panel_y + 135);
  tft.print(minuteStr);

  // Display colon
  tft.fillRect(static_num_panel_x + 90, static_num_panel_y + 123, 10, 15, ILI9341_BLACK);
  tft.setCursor(static_num_panel_x + 90, static_num_panel_y + 135);
  tft.print(":");

  // Display second
  tft.fillRect(static_num_panel_x + 110, static_num_panel_y + 123, 40, 15, ILI9341_BLACK);
  tft.setCursor(static_num_panel_x + 110, static_num_panel_y + 135);
  tft.print(secondStr);
  // Check if the current time matches the alarm time
  if (currentHour == alarmHour && currentMinute == alarmMinute) {
 
  unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 500) {
      previousMillis = currentMillis;
      if (State == LOW)
        State = HIGH;  // Đổi trạng thái
      else
        State = LOW;   // Đổi trạng thái
      digitalWrite(BUZZER_PIN, State);
    }


  }
    else digitalWrite(BUZZER_PIN, LOW);

 

   if ((millis() - lastTime) > timerDelay)                                                      // check WiFi connection status
      {
       if(WiFi.status()== WL_CONNECTED)
         {     
          run_the_weatherstation ();        
         }   
       else
         {
          Serial.println("WiFi Disconnected");
         }
         
         lastTime = millis();
      }

}


// ######################################################################################
// #  functions section                                                                 # 
// ######################################################################################

// ######################################################################################
// #  static - draw all panel frames                                                    #
// ######################################################################################

void draw_all_panel_frames (){ 

   baro_panel_x = 162;
   baro_panel_y = 156;
   minmax_panel_x = 242; 
   minmax_panel_y  = 156;
   
   tft.drawRoundRect (  3,   3, 156,  34, 4, GREEN);                                            // title panel frame
   tft.drawRoundRect (  3,  40, 156, 113, 4, GREEN);                                            // left upper big panel frame
   tft.drawRoundRect (162,   3, 156, 150, 4, GREEN);                                            // right upper big panel frame 
   tft.drawRoundRect (  3, 156,  77,  78, 4, GREEN);                                            // extreme left lowerpanel frame    
   tft.drawRoundRect ( 83, 156,  76,  78, 4, GREEN);                                            // left-of-middle lower panel frame      
   tft.drawRoundRect (baro_panel_x, baro_panel_y,  77,  78, 4, GREEN);                          // right-of-middle lower panel frame
   tft.drawRoundRect  (minmax_panel_x, minmax_panel_y, 77,  78, 4, GREEN);                      // extreme right lower window e                              
}


// ######################################################################################
// # static data in the title panel                                                     #
// ######################################################################################

void title_panel_text(){

   tft.setTextColor (SCALE0,BLACK);
   tft.setFont(&FreeMono9pt7b);
   tft.setTextSize (1);
   tft.setCursor (10,19);
   tft.print ("Vinh Cao");
   tft.setFont(0);
   tft.setCursor (10,25);
   tft.print ("Location: Quang Nam");   
}


// ######################################################################################
// # static data in the numeric panel                                                   #
// ######################################################################################

void static_data_in_numeric_window (){
    
   static_num_panel_x = 10;                                                                     // all coordinates in this panel relative to these two
   static_num_panel_y = 5;

   tft.setTextColor (CYAN,BLACK);
   tft.setFont(&FreeMono9pt7b); 
   tft.setTextSize (1);   
   tft.setCursor (static_num_panel_x-1,static_num_panel_y+50);
   tft.print ("192.168.1.50");
   tft.setCursor (static_num_panel_x,static_num_panel_y+71);
   tft.print ("Alarm");
   tft.setCursor (static_num_panel_x,static_num_panel_y+92);
   tft.print ("hum");
   tft.setCursor (static_num_panel_x,static_num_panel_y+114);
   tft.print ("wind");
  //  tft.setCursor (static_num_panel_x,static_num_panel_y+136);
  //  tft.print ("Time:");

   tft.setFont(0);
  //  tft.setCursor (static_num_panel_x+123,static_num_panel_y+42);
  //  tft.print (char(247));                                                                       // degree character
  //  tft.print ("C"); 
  //  tft.setCursor (static_num_panel_x+123,static_num_panel_y+65); 
  //  tft.print ("mB");
   tft.setCursor (static_num_panel_x+123,static_num_panel_y+85);
   tft.print ("%");
   tft.setCursor (static_num_panel_x+123,static_num_panel_y+106);
   tft.print ("m/s");
}

 
// ######################################################################################
// # dynamic data in the numeric panel                                                  #
// ######################################################################################

void print_dynamic_data_to_display (){

   static_num_panel_x = 10;                                                                     // all coordinates in this panel relative to these two
   static_num_panel_y = 5;
   
   tft.setTextColor (WHITE,BLACK);
   tft.setFont(&FreeMono9pt7b); 
   tft.setTextSize (1);
  
  //  tft.fillRect (static_num_panel_x+50,static_num_panel_y+39,70,15,BLACK);  
  //  tft.setCursor (static_num_panel_x+60,static_num_panel_y+50);  
  //  tft.print (temp_rainbowgauge,1);

  //  tft.fillRect (static_num_panel_x+50,static_num_panel_y+57,70,15,BLACK); 
  //  tft.setCursor (static_num_panel_x+60,static_num_panel_y+71); 
  //  if (press_02 <1000) tft.setCursor (static_num_panel_x+65,static_num_panel_y+71); 
  //  tft.print (press_02,0); 

   tft.fillRect (static_num_panel_x+60,static_num_panel_y+80,50,15,BLACK);  
   tft.setCursor (static_num_panel_x+70,static_num_panel_y+92);  
   tft.print (hum_02,0);        
 
   tft.fillRect (static_num_panel_x+55,static_num_panel_y+100,65,15,BLACK);  
   tft.setCursor (static_num_panel_x+60,static_num_panel_y+112);   
   tft.print (w_speed_02,1);
}


// ######################################################################################
// #  dynamic temperature rainbow scale meter                                           #
// ######################################################################################

void  rainbowScaleMeter (){
   
   if (millis () - runTime >= 100)                                                              // originally 500 = delay                                       
     {                                    
      runTime = millis ();
      if( tesmod==0)
        {
         reading =  99;
        }
      if( tesmod==1)
        {
         reading = temp_rainbowgauge*2;                                                         // important: here ring is seeded with value      
        } 
 
       r_gauge_pos_x = 161;                                                                     // these two variables govern the position
       r_gauge_pos_y = 5;                                                                       // of the square + gauge on the display = upper left corner

     ringMeter (reading,0,100, (r_gauge_pos_x+15),(r_gauge_pos_y+17),ringmeter_radius,ringlabel[0],GREEN2RED);    
     tesmod = 1;
     }
}


// ######################################################################################
// #  rainbow scale: draw the rainbox ring meter, returns x coord of righthand side     #
// ######################################################################################

int ringMeter(int value,int vmin,int vmax,int x,int y,int r, char *units, byte scheme){
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option
 
   x += r; y += r;                                                                              // calculate coordinates of center of ring
   int w = r / 3;                                                                               // width of outer ring is 1/4 of radius
   int angle = 150;                                                                             // half the sweep angle of the meter (300 degrees)
   int v = map (value, vmin, vmax, -angle, angle);                                              // map the value to an angle v
   byte seg = 3;                                                                                // segments are 3 degrees wide = 100 segments for 300 degrees
   byte inc = 6;                                                                                // draw segments every 3 degrees, increase to 6 for segmented ring
   int colour = BLUE;                                                                           // variable to save "value" text color from scheme and set default
 
 
   for (int i = -angle+inc/2; i < angle-inc/2; i += inc)                                        // draw color blocks every increment degrees
     {           
      float sx = cos((i - 90) * DEG2RAD);                                                       // calculate pair of coordinates for segment start
      float sy = sin((i - 90) * DEG2RAD);
      uint16_t x0 = sx * (r - w) + x;
      uint16_t y0 = sy * (r - w) + y;
      uint16_t x1 = sx * r + x;
      uint16_t y1 = sy * r + y;
    
      float sx2 = cos((i + seg - 90) * DEG2RAD);                                                // calculate pair of coordinates for segment end
      float sy2 = sin((i + seg - 90) * DEG2RAD);
      int x2 = sx2 * (r - w) + x;
      int y2 = sy2 * (r - w) + y;
      int x3 = sx2 * r + x;
      int y3 = sy2 * r + y;

      if (i < v) 
         {                                                                                      // fill in coloured segments with 2 triangles
          switch (scheme)
             {
              case 0: colour = RED; break;                                                      // fixed color
              case 1: colour = GREEN; break;                                                    // fixed color
              case 2: colour = BLUE; break;                                                     // fixed colour
              case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break;                   // full spectrum blue to red
              case 4: colour = rainbow(map(i, -angle, angle, 70, 127)); break;                  // green to red (high temperature etc)
              case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break;                  // red to green (low battery etc)
              default: colour = BLUE; break;                                                    // fixed color
             }
              tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
              tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
             }
      else                                                                                      // fill in blank segments
             {
              tft.fillTriangle(x0, y0, x1, y1, x2, y2, SCALE1);                                 // color of the unoccupied ring scale 
              tft.fillTriangle(x1, y1, x2, y2, x3, y3, SCALE0);                                 // color of the unoccupied ring scale
             }
         }
   t=197;
   if (temp_rainbowgauge<-9.9) t = 197;
   if (temp_rainbowgauge>-9.9) t = 202;  
   if (temp_rainbowgauge > 0 ) t = 214;
   if (temp_rainbowgauge >9.9) t = 206;
   tft.fillRect (t,y,65,15,BLACK);
   tft.setFont(&FreeMono9pt7b); 
   tft.setTextSize (1); 
   tft.setCursor (t+15,y+10);  
   tft.setTextColor (GREEN);
           
   tft.print (temp_rainbowgauge,1);
   return (temp_rainbowgauge);
}


// ######################################################################################
// #  rainbow scale: 16-bit rainbow color mixer                                         #
// ######################################################################################

unsigned int rainbow (byte value) {                                                             // value is expected to be in range 0-127
                                                                                                // value is converted to a spectrum color from 0 = blue through to 127 = red
   byte red = 0;                                                                                // red is the top 5 bits of a 16 bit colour value
   byte green = 0;                                                                              // green is the middle 6 bits
   byte blue = 0;                                                                               // blue is the bottom 5 bits
   byte quadrant = value / 32;

   if (quadrant == 0)
     {
      blue = 31;
      green = 2 * (value % 32);
      red = 0;
     }
   if (quadrant == 1)
     {
      blue = 31 - (value % 32);
      green = 63;
      red = 0;
     }
   if (quadrant == 2)
     {
      blue = 0;
      green = 63;
      red = value % 32;
     }
   if (quadrant == 3)
     {
      blue = 0;
      green = 63 - 2 * (value % 32);
      red = 31;
     }
   return (red << 11) + (green << 5) + blue;
}



// ######################################################################################
// # static data in the rainbow scale panel                                             #
// ######################################################################################

 void static_data_in_rainbow_panel (){

   r_gauge_pos_x = 161;                                                                         // all x-y positions in this panel relative to these
   r_gauge_pos_y = 5;  
   
   tft.setCursor (r_gauge_pos_x+10,r_gauge_pos_y+10);  
   tft.print ("temp");                                                                      
   tft.setCursor (r_gauge_pos_x+130,r_gauge_pos_y+8);  
   tft.print (char(247));                                                                       // degree character
   tft.setTextSize (2);
   tft.print ("C");   
   tft.setTextSize (1); 
   tft.setCursor (r_gauge_pos_x+55,r_gauge_pos_y+133);  
   tft.print ("0");
   tft.setCursor (r_gauge_pos_x+97,r_gauge_pos_y+133);                                          // 50 is top rainbow scale temperature 
   tft.print ("50");
}


// ######################################################################################
// # rainbow scale: return a value in range -1 to +1 for a given phase angle (degrees)  #
// ######################################################################################

float sineWave(int phase) {
  
   return sin(phase * 0.0174532925);
}


// ######################################################################################
// #  wind compass - static part                                                        #
// ######################################################################################

void compassGauge (){

   compassPivot_x = 1;                                                                          // these coordinates determine the position of wind compass and everything in it
   compassPivot_y = 155;

   tft.drawCircle ((compassPivot_x+40), (compassPivot_y+40), (compass_r+11), BLUE);             // outer circle compass 
   tft.drawCircle ((compassPivot_x+40), (compassPivot_y+40), (compass_r+2), CYAN);              // inner circle compass - pointer fits in here
   tft.setTextSize (1);
   tft.setCursor ((compassPivot_x+38), (compassPivot_y+4));
   tft.setTextColor (WHITE, BLACK);
   tft.print ("N");
   tft.setCursor ((compassPivot_x+38), (compassPivot_y+69));
   tft.print ("S");
   tft.setCursor ((compassPivot_x+71), (compassPivot_y+35));
   tft.print ("E");
   tft.setCursor ((compassPivot_x+5), (compassPivot_y+35));
   tft.print ("W"); 
   tft.setCursor ((compassPivot_x+6), (compassPivot_y+3));
   tft.setTextSize (1);  
   tft.print ("wi");
   tft.setCursor ((compassPivot_x+64), (compassPivot_y+3));
   tft.print ("nd");  
}


// ######################################################################################
// #  wind compass - dynamic wind compass pointer                                       #
// ######################################################################################

void compassPointer (){                       
                                                   
   tft.drawLine (c_x1, c_y1, c_x2, c_y2, BLACK); 
   tft.fillTriangle (c_x2, c_y2 ,c_x3, c_y3, c_x4, c_y4,BLACK);                                 // remove old compass pointer by overwriting in white
   compass_angle = ((wind_dir_02+90)*DEG2RAD);

   c_x1 = ((compassPivot_x+40) + ((compass_r) * cos (compass_angle - 180*DEG2RAD)));            // x1 is shaft foot point at edge circle
   c_y1 = ((compassPivot_y+40) + ((compass_r) * sin (compass_angle - 180*DEG2RAD)));                 

   c_x2 = ((compassPivot_x+40) + ((compass_r-30) * cos (compass_angle - 180*DEG2RAD)));         // x2 is shaft point at arrow tip
   c_y2 = ((compassPivot_y+40) + ((compass_r-30) * sin (compass_angle - 180*DEG2RAD)));                 
 
   c_x3 = ((compassPivot_x+40) + ((compass_r-12) * cos (compass_angle -90*DEG2RAD)));           // arrowhead corner point
   c_y3 = ((compassPivot_y+40) + ((compass_r-12) * sin (compass_angle -90*DEG2RAD)));                 

   c_x4 = ((compassPivot_x+40) + ((compass_r-12) * cos (compass_angle - 270*DEG2RAD)));         // arrowhead corner point
   c_y4 = ((compassPivot_y+40) + ((compass_r-12) * sin (compass_angle - 270*DEG2RAD)));                 

   c_x1_old = c_x1; c_x2_old = c_x2; c_x3_old = c_x3;   c_x4_old = c_x4;
   c_y1_old = c_y1; c_y2_old = c_y2; c_y3_old = c_y3;   c_y4_old = c_y4; 

   tft.drawLine (c_x1, c_y1, c_x2, c_y2, CYAN);
   tft.fillTriangle (c_x2, c_y2 ,c_x3, c_y3, c_x4, c_y4, CYAN);                                 // print the new pointer to display
}


// ######################################################################################
// #  wind sector calculator                                                            #
// ######################################################################################

void windSectorReporter (){

   h = 0;
   if (wind_dir_02 <22.5) h = 0;
   if (wind_dir_02> 22.5) h = 1; 
   if (wind_dir_02> 67.5) h = 2;
   if (wind_dir_02>112.5) h = 3;
   if (wind_dir_02>157.5) h = 4;  
   if (wind_dir_02>202.5) h = 5;
   if (wind_dir_02>247.5) h = 6;
   if (wind_dir_02>292.5) h = 7; 
   if (wind_dir_02>337.5) h = 8; 
}


// ######################################################################################
// #  humidity needle meter panel - static part                                         #       // in right-of-middle lower window
// ######################################################################################

void humidity_static (){

// these four coordinates determine everything in the humidity meter panel 
   pivot_x   = 90;                                                                              // pivot coordinates needle of humidity meter
   pivot_y   = 225;       
   center_x1 = 82;                                                                              // center x of edge markers humidity gauge - different from pivot needle!
   center_y1 = 230;   
   needle_x_old = pivot_x;                                                                      // remember
   needle_y_old = pivot_y;                                                                      // remember

   tft.setCursor ((pivot_x+36), (pivot_y-65));                                                  // display percent sign in  small gauge scale
   tft.print ("%");
   tft.setCursor ((pivot_x+46), (pivot_y-65));  
   tft.print ("hum");
   tft.fillCircle (pivot_x, pivot_y, 2, MAGENTA);                                               // pivot needle middle small gauge                                                                         // arbitrary seeding temp - avoids drawing black line from position 0.0
   tft.setCursor ((pivot_x+46),(pivot_y-3));                        
   tft.setTextSize (1);  
   tft.print ("40");                                                                            // scale starts with 40% and ends with 100%
   tft.setCursor ((pivot_x-4),(pivot_y-56));   
   tft.print ("100");    
   tft.setTextSize (2);

   j = 270;                                                                                     // start point of cirle segment
   do {
       angle_circle = (j* DEG2RAD);                                                             // angle expressed in radians - 1 degree = 0,01745331 radians      

       edge_x1 = (center_x1+4 + (radius_s*cos (angle_circle)));                                 // scale - note the 4 pixels offset in x      
       edge_y1 = (center_y1 + (radius_s*sin (angle_circle)));                                   // scale
         
       edge_x1_out = (center_x1+4 + ((radius_s+edgemarkerLength)*cos (angle_circle)));          // scale - note the 4 pixels offset in x   
       edge_y1_out = (center_y1 + ((radius_s+edgemarkerLength)*sin (angle_circle)));            // scale
        
       tft.drawLine (edge_x1, edge_y1, edge_x1_out, edge_y1_out,MAGENTA); 
       j = j+6; 
   } 
   while (j<356);                                                                               // end of circle segment
}


// ######################################################################################
// #   humidity needle meter - dynamic part = needle movement control                   #
// ######################################################################################

void needle_meter (){                                                                         
          
   tft.drawLine (pivot_x, pivot_y, needle_x_old, needle_y_old, 0);                              // remove old needle by overwritig in black

   angle_needle = (420*DEG2RAD - 1.5*hum_02*DEG2RAD);                                           // contains a 1.5 stretch factor to expand 60 percentage points over 90 degrees of scale

   if (angle_needle > 6.28) angle_needle = 6.28;                                                // prevents the needle from ducking below horizontal    
   needle_x = (pivot_x + ((needle_length)*cos (angle_needle)));                                 // calculate x coordinate needle point
   needle_y = (pivot_y + ((needle_length)*sin (angle_needle)));                                 // calculate y coordinate needle point
   needle_x_old = needle_x;                                                                     // remember previous needle position
   needle_y_old = needle_y;

   tft.drawLine (pivot_x, pivot_y, needle_x, needle_y,MAGENTA); 
   tft.fillCircle (pivot_x, pivot_y, 2, MAGENTA);                                               // restore needle pivot
}


// ######################################################################################
// #  dynamic barometer panel                                                           #
// ######################################################################################

void build_dynamic_baropanel (){                                                                // fills two triangles to create 4-point polygon
   
   barval = 1050-press_02;                                                                      // based on baro pressure 

   p1_x = ((baro_panel_x+2)+ ((100-barval)/100*72));                                            // p1-p2-p3 corner points for first triangle
   p1_y = ((baro_panel_y+75)- ((100-barval)/100*73));                                           // p1-p4-p3 corner points for second triangle
   p2_x = baro_panel_x+3;  
   p2_y = baro_panel_y+74; 
   p3_x = baro_panel_x+75; 
   p3_y = baro_panel_y+74; 
   p4_x = baro_panel_x+75; 
   p4_y = p1_y;  

   p1_x_old = p1_x;                                                                             // remember previous positions of points
   p1_y_old = p1_y;
   p2_x_old = p2_x;
   p2_y_old = p2_y;
   p3_x_old = p3_x;
   p3_y_old = p3_y;    
   p4_x_old = p4_x;
   p4_y_old = p4_y; 
   
   tft.fillTriangle (p1_x, p1_y, p2_x, p2_y, p3_x, p3_y, SCALE2);                               // make new first triangle
   tft.fillTriangle (p1_x, p1_y, p4_x, p4_y, p3_x, p3_y, SCALE2);                               // make new second triangle
   tft.drawLine (p1_x, p1_y,  p4_x, p4_y, CYAN);                                                // draw horizontal level line      

   tft.drawLine (baro_panel_x+4, baro_panel_y+75, baro_panel_x+74, baro_panel_y+75, SCALE2);    // correct triangles base
   tft.drawLine (baro_panel_x+4, baro_panel_y+76, baro_panel_x+72, baro_panel_y+76, SCALE2);    // correct triangles-base
   tft.drawRoundRect (baro_panel_x, baro_panel_y,  77,  78, 4, GREEN);                          // correct right-of-middle lower panel frame
   
   tft.drawLine (baro_panel_x+2, baro_panel_y+75, (baro_panel_x+75), (baro_panel_y+2), CYAN);   // redraw line
   tft.fillCircle (p1_x, p1_y,3,RED);

   tft.setFont(0);
   tft.setTextColor (YELLOW,BLACK);
   tft.setCursor (baro_panel_x+5, baro_panel_y+4);
   tft.print ("baro ");
   tft.setTextColor (WHITE,BLACK);
   tft.setCursor (baro_panel_x+32, baro_panel_y+4); 
       if (press_02>999) tft.print ("high"); else tft.print ("low ");
}


// ######################################################################################
// # refresh the dynamic barometer panel                                                #
// ######################################################################################


void refresh_dynamic_baropanel (){

  tft.fillCircle   (p1_x_old, p1_y_old, 3, BLACK);                                              // erase previous position of red circle
  tft.fillTriangle (p1_x_old, p1_y_old, p2_x_old, p2_y_old, p3_x_old, p3_y_old, BLACK);         // remove previous first triangle
  tft.fillTriangle (p1_x_old, p1_y_old, p4_x_old, p4_y_old, p3_x_old, p3_y_old, BLACK);         // remove previous second triangle 
  tft.drawLine (p1_x_old, p1_y_old,  p4_x_old, p4_y_old, BLACK);                                // remove previous horizontal level line  
  tft.drawLine (baro_panel_x+2, baro_panel_y+75, (baro_panel_x+75), (baro_panel_y+2), CYAN);    // draw new diagonal line

  tft.drawLine (baro_panel_x+4, baro_panel_y+75, baro_panel_x+74, baro_panel_y+75, SCALE2);     // correct triangles base
  tft.drawLine (baro_panel_x+4, baro_panel_y+76, baro_panel_x+72, baro_panel_y+76, SCALE2);     // correct triangles-base
  tft.drawRoundRect  (minmax_panel_x, minmax_panel_y, 77,  78, 4, GREEN);                       // extreme right lower window                                 
}



// ######################################################################################
// # static data in the min-max temp panel                                              #
// ######################################################################################

   void static_minmax_panel (){

   tft.drawRoundRect (minmax_panel_x+8, minmax_panel_y+6, 7, 60, 4, WHITE);                     // 'thermometer'
   tft.fillCircle (minmax_panel_x+11, minmax_panel_y+63, 6, RED);  
   tft.drawCircle (minmax_panel_x+11, minmax_panel_y+63, 7, WHITE);

   tft.setTextSize (1);
   tft.setCursor (minmax_panel_x+22,minmax_panel_y+8); 
   tft.print ("Max temp");
   tft.setCursor (minmax_panel_x+64,minmax_panel_y+23); 
   tft.print ("C");
 
   tft.setCursor (minmax_panel_x+22,minmax_panel_y+40); 
   tft.print ("Min temp");
   tft.setCursor (minmax_panel_x+64,minmax_panel_y+55); 
   tft.print ("C");

   t_meter_increase = 46;                                                                       // absolute vertical pixel range of 'mercury' in thermometer simulation
}


// ######################################################################################
// # dynamic data in the min-max temp panel                                             #
// ######################################################################################

   void dynamic_minmax_panel (){

   tft.fillRect (minmax_panel_x+22,minmax_panel_y+23, 40,7,BLACK);
   tft.setTextSize (1);
   if (max_temp_today >=  0.0) tft.setCursor (minmax_panel_x+44,minmax_panel_y+23); 
   if (max_temp_today >= 10.0) tft.setCursor (minmax_panel_x+38,minmax_panel_y+23); 
   if (max_temp_today <  -0.0) tft.setCursor (minmax_panel_x+38,minmax_panel_y+23); 
   if (max_temp_today <  -9.9) tft.setCursor (minmax_panel_x+32,minmax_panel_y+23); 
   tft.print (max_temp_today,1);  
 
   tft.fillRect (minmax_panel_x+22,minmax_panel_y+55, 40,7,BLACK);  
   if (min_temp_today >=  0.0) tft.setCursor (minmax_panel_x+44,minmax_panel_y+55); 
   if (min_temp_today >= 10.0) tft.setCursor (minmax_panel_x+38,minmax_panel_y+55); 
   if (min_temp_today <  -0.0) tft.setCursor (minmax_panel_x+38,minmax_panel_y+55); 
   if (min_temp_today <  -9.9) tft.setCursor (minmax_panel_x+32,minmax_panel_y+55);  
   tft.print (min_temp_today,1);   
  
   t_meter_increase_old = t_meter_increase;  
   t_meter_increase = ((max_temp_today/33)*46);                                                 // empirically determined dynamic range of 'mercury' in the graphical thermometer
   
   if (t_meter_increase <1)  t_meter_increase = 1;
   if (t_meter_increase >45) t_meter_increase = 45;                                             // barrier against overflowing of graphical thermometer 
   
   tft.fillRect (minmax_panel_x+9,minmax_panel_y+57, 5, -t_meter_increase_old, BLACK);
   tft.fillRect (minmax_panel_x+9,minmax_panel_y+57, 5, -t_meter_increase, RED);  
}


// ######################################################################################
// # run the weather station - get json string and parse                                #
// ######################################################################################


void run_the_weatherstation (){
          
          
          String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
      
          jsonBuffer = httpGETRequest(serverPath.c_str());
          Serial.println(jsonBuffer);
          JSONVar myObject = JSON.parse(jsonBuffer);
               double temp_01     = (myObject ["main"]["temp"]); 
               double hum_01      = (myObject ["main"]["humidity"]); 
               double press_01    = (myObject ["main"]["pressure"]); 
               double w_speed_01  = (myObject ["wind"]["speed"]);   
               double wind_dir_01 = (myObject ["wind"]["deg"]);          
  
         JSONVar minmax_temp = JSON.parse(jsonBuffer);
               double min_temp = (minmax_temp ["main"]["temp_min"]);
               double max_temp = (minmax_temp ["main"]["temp_max"]);
 
  
          if (JSON.typeof(myObject) == "undefined")                                             // JSON.typeof(jsonVar) can be used to get the type of the var
            {
             Serial.println("Parsing input failed!");
             return;
             }
    
          Serial.println ("*******************************************");  
          Serial.print ("JSON object = ");
          Serial.println (myObject);
          Serial.println ("*******************************************");    
          Serial.println ("extracted from JSON object:");        
          Serial.print ("temperature:    ");    
          Serial.print (myObject["main"]["temp"]);
          Serial.println (" *K");       
          Serial.print ("pressure:       ");
          Serial.print (myObject["main"]["pressure"]);
          Serial.println (" mB"); 
          Serial.print ("humidity:       ");
          Serial.print(myObject["main"]["humidity"]);
          Serial.println (" %");  
          Serial.print ("wind speed:     ");
          Serial.print (myObject["wind"]["speed"]);
          Serial.println (" m/s");  
          Serial.print("wind direction: ");
          Serial.print (myObject["wind"]["deg"]);
          Serial.println (" degrees");                 
                                                                                                                            
          temp_rainbowgauge = temp_01-273;                                                      // convert temp from Kelvin to Celsius
          hum_02 = hum_01;                                                                      // place humidity value in a transferable variable
          press_02 = press_01;                                                                  // place air pressure value in a transferable variable
          w_speed_02 = w_speed_01;                                                              // place wind speed value in a transferable variable

          max_temp_today = max_temp-273;
          min_temp_today = min_temp-273;
      
          Serial.print("min temp today: ");
          Serial.print (min_temp_today,1);
          Serial.println (" *C");         
          Serial.print("max temp today: ");
          Serial.print (max_temp_today,1);                  
          Serial.println (" *C");   
          
          wind_dir_02 = wind_dir_01;
          windSectorReporter ();
          compassPointer ();  
          rainbowScaleMeter ();
          print_dynamic_data_to_display ();
          refresh_dynamic_baropanel();
          build_dynamic_baropanel (); 
          needle_meter ();  
          dynamic_minmax_panel (); 
}


// ######################################################################################
// # http GET request module                                                            #
// ######################################################################################

String httpGETRequest(const char* serverName) {

   WiFiClient client;
   HTTPClient http;
      
   http.begin(client, serverName);                                                              // your IP address with path or Domain name with URL path 
    
   int httpResponseCode = http.GET();                                                           // send HTTP POST request  
   String payload = "{}"; 
  
   if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
   }
   else {
     Serial.print("Error code: ");
     Serial.println(httpResponseCode);
   }

   http.end();                                                                                  // free resources
   return payload;  
}
