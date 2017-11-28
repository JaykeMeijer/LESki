void wifiInit() {
  //WIFI INIT
  //Serial.printf("Connecting to %s\n", ssid);
  //Wifi initialisation in progress
  WiFi.mode(WIFIMODE);
  WiFi.onEvent(WiFiEvent);
  if (WIFIMODE == WIFI_STA || WIFIMODE == WIFI_AP_STA) {
    WiFi.begin(ssid, password);
  }
  IPAddress ip(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(ap_name);
  wifiinitstarted = true;                                     //Flag wifi initialisation is started; Flag is set to false if connected, this prevent multiple start re-init (after disconnect event) if initialisation is already in progress
}

void wifiserverInit() {
  //Set OTA  handle
  server.on("/ota", HTTP_GET, handleOtabegin);
  
  //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
  });
  
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });
//
//  //get heap status, analog input value and all GPIO statuses in one json call
//  server.on("/all", HTTP_GET, []() {
//    String json = "{";
//    json += "\"heap\":" + String(ESP.getFreeHeap());
//    json += ", \"analog\":" + String(analogRead(A0));
//    json += "}";
//    server.send(200, "text/json", json);
//    json = String();
//  });
  server.begin();
  Serial.println("HTTP server started");
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event) {
  Serial.print("Wifi event occured :");
  Serial.println(event);
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      //When connected set
      Serial.print("WiFi connected! IP address: ");
      Serial.println(WiFi.localIP());

      if (MDNS.begin(host)) {
        Serial.println("wifiInit: MDNS responder started");
      }

      wifiserverInit();
      connected = true;

      MDNS.addService("http", "tcp", 80);
      Serial.println("wifiInit: MDNS http service added");

      wifiinitstarted = false;                                                                                 //Wifi initialisation finished
      //initializes the UDP state (only if NTP Needed!)
      //      udp.begin(WiFi.localIP(), udpPort);
      //      ntpInit();
      break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      connected = false;
      break;
  }
}

void handleOtabegin() {
  Serial.println("start OTA");
  server.send(200, "text/plain", "OK");
  otaInit();
  otaStart();
  otamode = true;                                                                                          //Ota mode enabled
}

