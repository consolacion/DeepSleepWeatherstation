void setupWiFi(void)
{
  Serial.println("WiFi setup");
  WiFi.begin(ssid, password);
  Serial.println();

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    count++;
    Serial.print("Tries ");
    Serial.println(count);
    if (count >= 10 && (digitalRead(SLEEPPIN) ) == LOW) {
      Serial.println("Back to sleep, try again in 30 sec ");
      count = 0;
      ESP.deepSleep(30 * 1000000);
    }
  }
  Serial.println("WiFi ready!");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*
    if (MDNS.begin("ESP8266")) {
      Serial.println("MDNS responder started");
    }
  */
}
