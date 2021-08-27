#ifdef ESP32
#include <WiFi.h>
#include <AsyncTcp.h>
#include <mbedtls/md.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>
#include <vfs.h>
#include <Hash.h>
#include <FS.h>
#endif
#include <sqlite3.h>
#include <ESPAsyncWebServer.h>

#define SESSION_TIMEOUT 1000*60*10
/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED false

// Replace with your network credentials
const char* ssid = "Basttyy";
const char* password = "6yrbz5iz";
const char* www_uname = "admin";
const char* www_pass = "upass";
const char* device_name = "simbank1";
String session_key(6);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

sqlite3 *db1;
sqlite3 *db2;
char *zErrMsg = 0;
int rc;
char *sql;

// Timer variables
unsigned long lastTime = 0;  
unsigned long timerDelay = 500;
unsigned long lastSession = 0;

float temperature;
float humidity;
float pressure;

void serverRouting();
void getSensorReadings();
void initWiFi();
void eventConnectController(AsyncEventSourceClient *client);
void notFoundController(AsyncWebServerRequest *request);
void homeController(AsyncWebServerRequest *request);
void passwordController(AsyncWebServerRequest *request);
void loginController(AsyncWebServerRequest *request);
void logoutController(AsyncWebServerRequest *request);
void resourceController(AsyncWebServerRequest *request);
void tabelsDataController(AsyncWebServerRequest *request);
bool is_authenticated(AsyncWebServerRequest *request);
void serverRouter();
String getContentType(String filename);
String processor(const String& var);
String getRandomString(int lent=6);

static int callback(void *data, int argc, char **argv, char **azColName);
int openDb(char *filename, sqlite3 **db);
int db_exec(sqlite3 *db, const char *sql);

#ifdef ESP32
String sha1(String payloadStr);
#endif