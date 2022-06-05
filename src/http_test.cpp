

// #include <WiFi.h>
// #include <WiFiMulti.h>
// #include <HTTPClient.h>
// char kHostname[50];
// #define USE_SERIAL Serial
// WiFiMulti wifiMulti;

// #include <Adafruit_AHTX0.h>
// Adafruit_AHTX0 aht;
// sensors_event_t humidity_event, temp_event;
// float degreesF = 0;
// float humidity = 0;

// void setup()
// {

//   Serial.begin(9200);

//   wifiMulti.addAP("Pixel_7807", "ethanalllowercase");
//   wifiMulti.addAP("UCInet Mobile Access", "");
//   wifiMulti.addAP("Packet_Dispensary", "passwordalllowercase");

//   if (!aht.begin())
//   {
//     Serial.println("Could not find AHT? Check wiring");
//     while (1)
//       vTaskDelay(10);
//   }
//   Serial.println("AHT10 or AHT20 found");
// }

// void loop()
// {

//   aht.getEvent(&humidity_event, &temp_event);
//   degreesF = 32 + (temp_event.temperature * 1.8);
//   humidity = humidity_event.relative_humidity;

//   // wait for WiFi connection
//   if ((wifiMulti.run() == WL_CONNECTED))
//   {

//     HTTPClient http;

//     USE_SERIAL.print("[HTTP] begin...\n");
//     // configure traged server and url
//     sprintf(kHostname, "http://192.168.125.145:5000/?temp=%.1f&humidity=%.2f", degreesF, humidity);

//     http.begin(kHostname); // HTTP

//     USE_SERIAL.print("[HTTP] GET...\n");
//     // start connection and send HTTP header
//     int httpCode = http.GET();

//     // httpCode will be negative on error
//     if (httpCode > 0)
//     {
//       // HTTP header has been send and Server response header has been handled
//       USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

//       // file found at server
//       if (true)
//       {
//         String payload = http.getString();
//         USE_SERIAL.println(payload);
//       }
//     }
//     else
//     {
//       USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
//     }

//     http.end();
//   }

//   delay(5000);
// }