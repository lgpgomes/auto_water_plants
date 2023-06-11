#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <TimeAlarms.h>

#define ON LOW
#define OFF HIGH
#define RELAY_PUMP 5
#define LED_BOARD 2

const char* ssid = "INTELBRAS";
const char* password = "samsung+aquario";

unsigned long momentStartWater;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

String getContentType(String filename);
bool handleFileRead(String path);

int setupSchedule(String dayWeek, String hourSchedule) {
  String hour = hourSchedule.substring(0, 2);
  String minute = hourSchedule.substring(3, 5);
  String second = hourSchedule.substring(6, 8);
  int alarmId = Alarm.alarmRepeat(hour.toInt(), minute.toInt(), second.toInt(), water);
  return alarmId;
}

void water() {
  DynamicJsonDocument doc(5120);
  String dataJson = readJsonFile();
  deserializeJson(doc, dataJson);
  int intervalWater = doc["variables"]["intervalWater"];

  startWater();
  Alarm.timerOnce(intervalWater, endWater);
}

void setValues() {
  DynamicJsonDocument doc(5120);
  String inputJson = readJsonFile();
  deserializeJson(doc, inputJson);

  String intervalWater = server.arg("intervalWater");
  String flowRate = server.arg("flowRate");

  if (intervalWater != "")
  {
    doc["variables"]["intervalWater"] = intervalWater.toInt();
  }
  else if (flowRate != "")
  {
    doc["variables"]["flowRate"] = flowRate.toInt();
  }

  String outputJson;
  serializeJson(doc, outputJson);

  server.send(200, "text/plane", "");
  updateData(true, outputJson);
}

void setSchedules() {
  DynamicJsonDocument doc(5120);
  String inputJson = readJsonFile();
  deserializeJson(doc, inputJson);

  String hourSchedule = server.arg("hour");
  String dayWeekSchedule = server.arg("dayWeek");
  String idToRemove = server.arg("idToRemove");
  int dataSize = doc["hours"].size();

  if (hourSchedule != "" && dayWeekSchedule != "" && dataSize < 7) {
    doc["hours"][dataSize]["id"] = setupSchedule(dayWeekSchedule, hourSchedule);
    doc["hours"][dataSize]["hour"] = hourSchedule;
    doc["hours"][dataSize]["dayWeek"] = dayWeekSchedule;
  }

  if (idToRemove != "") {
    Alarm.free(idToRemove.toInt());
    for (int i = 0; i < doc["hours"].size(); i++) {
      if (doc["hours"][i]["id"] == idToRemove.toInt()) {
        doc["hours"].remove(i);
      }
    }
  }

  String outputJson;
  serializeJson(doc, outputJson);

  server.send(200, "text/plane", outputJson);
  updateData(true, outputJson);
}

void updateData(int useWebsocket, String dataJson) {
  if (useWebsocket == 1) webSocket.broadcastTXT(dataJson);
  else server.send(200, "text/plane", dataJson);
  saveDataJson(dataJson);
}

void waterPlants() {
  String power = server.arg("power");
  if (power == "true") startWater();
  else if (power == "false") endWater();
  server.send(200, "text/plane", "");
}

void startWater() {
  DynamicJsonDocument doc(5120);
  String dataJson = readJsonFile();
  deserializeJson(doc, dataJson);

  if (doc["states"]["isWatering"] == 0) {
    digitalWrite(RELAY_PUMP, ON);
    digitalWrite(LED_BOARD, ON);
    momentStartWater = millis();
    doc["states"]["isWatering"] = 1;
    String out;
    serializeJson(doc, out);
    updateData(1, out);
  }
}

void endWater() {
  DynamicJsonDocument doc(5120);
  String dataJson = readJsonFile();
  deserializeJson(doc, dataJson);

  if (doc["states"]["isWatering"] == 1) {
    digitalWrite(RELAY_PUMP, OFF);
    digitalWrite(LED_BOARD, OFF);

    long elapsedTime = millis() - momentStartWater;
    int timeCount = doc["data"]["timeCount"];


    int waterCountSun = doc["data"]["waterCount"]["sunday"];
    int waterCountMon = doc["data"]["waterCount"]["monday"];
    int waterCountTue = doc["data"]["waterCount"]["tuesday"];
    int waterCountWed = doc["data"]["waterCount"]["wednesday"];
    int waterCountThu = doc["data"]["waterCount"]["thursday"];
    int waterCountFri = doc["data"]["waterCount"]["friday"];
    int waterCountSat = doc["data"]["waterCount"]["saturday"];
    int waterCount = doc["data"]["waterCount"]["total"];

    timeCount += elapsedTime;
    waterCount++;

    if (weekday() == 1) {
      waterCountSun++;
      doc["data"]["waterCount"]["sunday"] = waterCountSun;
    }
    else if (weekday() == 2) {
      waterCountMon++;
      doc["data"]["waterCount"]["monday"] = waterCountMon;
    }
    else if (weekday() == 3) {
      waterCountTue++;
      doc["data"]["waterCount"]["tuesday"] = waterCountTue;
    }
    else if (weekday() == 4) {
      waterCountWed++;
      doc["data"]["waterCount"]["wednesday"] = waterCountWed;
    }
    else if (weekday() == 5) {
      waterCountThu++;
      doc["data"]["waterCount"]["thursday"] = waterCountThu;
    }
    else if (weekday() == 6) {
      waterCountFri++;
      doc["data"]["waterCount"]["friday"] = waterCountFri;
    }
    else if (weekday() == 7) {
      waterCountSat++;
      doc["data"]["waterCount"]["saturday"] = waterCountSat;
    }
    doc["data"]["timeCount"] = timeCount;
    doc["data"]["waterCount"]["total"] = waterCount;
    doc["states"]["isWatering"] = 0;

    String out;
    serializeJson(doc, out);
    updateData(1, out);
  }
}

void resetJson() {
  DynamicJsonDocument doc(5120);
  String dataJson = readJsonFile();
  deserializeJson(doc, dataJson);

  doc["data"]["timeCount"] = 0;
  doc["data"]["waterCount"]["total"] = 0;
  doc["data"]["waterCount"]["sunday"] = 0;
  doc["data"]["waterCount"]["monday"] = 0;
  doc["data"]["waterCount"]["tuesday"] = 0;
  doc["data"]["waterCount"]["wednesday"] = 0;
  doc["data"]["waterCount"]["thursday"] = 0;
  doc["data"]["waterCount"]["friday"] = 0;
  doc["data"]["waterCount"]["saturday"] = 0;

  String out;
  serializeJson(doc, out);
  server.send(200, "text/plane", "");
  updateData(1, out);
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".json")) return "application/json";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  Serial.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}

void getHour() {
  Serial.print("\nSyncing time");
  configTime(0, 0, "pool.ntp.org");
  while (time(nullptr) < 100000)
  {
    delay(500);
    Serial.print(".");
  }
  setTime(time(nullptr));
  adjustTime(-3 * 3600);
  char timeBuffer[32];
  sprintf(timeBuffer, " %d:%d:%d\n", hour(), minute(), second());
  Serial.print(timeBuffer);
}

void connectToWifi() {
  Serial.print("\nConnecting to Wifi ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  digitalWrite(LED_BOARD, OFF);
  Serial.print("\nConnected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

String readJsonFile() {
  File file = SPIFFS.open("/data.json", "r");
  if (!file)
  {
    Serial.println("Failed to open config file");
  }
  String jsonDataFromFile = file.readString();
  file.close();
  return jsonDataFromFile;
}

bool saveDataJson(String dataJson) {
  File file = SPIFFS.open("/data.json", "w");
  if (!file) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  file.print(dataJson);
  file.close();
  return true;
}

void sleepMode () {
  Serial.println("Deep Sleep Started - 12h");
  ESP.deepSleep(43200e6);
}

void setupSleepMode() {
  Alarm.alarmRepeat(21, 30, 0, sleepMode);
}

void setupAllAlarms() {
  DynamicJsonDocument doc(5120);

  String dataJson = readJsonFile();
  deserializeJson(doc, dataJson);

  int data = doc["hours"].size();
  for (int i = 0; i < data; i++) {
    String hourSetup = doc["hours"][i]["hour"];
    String dayWeek = doc["hours"][i]["dayWeek"];
    setupSchedule(dayWeek, hourSetup);
  }
}

void setup() {
  Serial.begin(115200);
  SPIFFS.begin();
  webSocket.begin();

  pinMode(RELAY_PUMP, OUTPUT);
  pinMode(LED_BOARD, OUTPUT);

  digitalWrite(RELAY_PUMP, OFF);
  digitalWrite(LED_BOARD, ON);

  DynamicJsonDocument doc(5120);

  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "404: Not Found");
  });
  server.on("/waterPlants", waterPlants);
  server.on("/updateData", []() {
    updateData(false, readJsonFile());
  });
  server.on("/updateSchedules", setSchedules);
  server.on("/updateVariables", setValues);
  server.on("/resetJson", resetJson);

  server.serveStatic("/", SPIFFS, "/", "max-age=86400");

  connectToWifi();
  getHour();
  readJsonFile();
  setupSleepMode();
  setupAllAlarms();

  server.begin();
}

void loop() {
  server.handleClient();
  Alarm.delay(1);
  webSocket.loop();
}
