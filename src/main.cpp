#include "template.h"

void setup() {
  Serial.begin(115200);
  initWiFi();

  if (SPIFFS.begin()) {
    Serial.println(F("File System mounted"));
  } else {
    Serial.println(F("Failed to mount file system"));
  }

    // Activate mDNS this is used to be able to connect to the server
  // with local DNS hostmane esp8266.local
  if (MDNS.begin(device_name)) {
    Serial.println("MDNS responder started");
  }

  serverRouter();
  // Handle Web Server Events
  events.onConnect(eventConnectController);
  server.addHandler(&events);
  server.begin();
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}

void loop() {
  //MDNS.update();
  if ((millis() - lastTime) > timerDelay) {
    getSensorReadings();
    // Serial.printf("Temperature = %.2f ºC \n", temperature);
    // Serial.printf("Humidity = %.2f \n", humidity);
    // Serial.printf("Pressure = %.2f hPa \n", pressure);
    // Serial.println();

    // Send Events to the Web Server with the Sensor Readings
    events.send("ping","ping",millis());
    // events.send(String(temperature).c_str(),"temperature",millis());
    // events.send(String(humidity).c_str(),"humidity",millis());
    // events.send(String(pressure).c_str(),"pressure",millis());
    
    lastTime = millis();
  }
}

bool handleFileRead(AsyncWebServerRequest *request, String path) {
  Serial.print(F("handleFileRead: "));
  Serial.println(path);
  if (!is_authenticated(request)) {
    Serial.println(F("Go on not login!"));
    path = "/login.html";
  } else {
    if (path.endsWith("/")) path += F("home.html"); // If a folder is requested, send the index file
  }
  String contentType = getContentType(path);              // Get the MIME type
  String pathWithGz = path + F(".gz");
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){   // If the file exists, either as a compressed archive, or normal
    bool gzipped = false;
    if(SPIFFS.exists(pathWithGz)) {                         // If there's a compressed version available
      path += F(".gz");                                     // Use the compressed version
      gzipped = true;
    }
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, contentType);
    if (gzipped){
      response->addHeader("Content-Encoding", "gzip");
    }
    Serial.print("Real file path: ");
    Serial.println(path);
    request->send(response);
    return true;
  }
  Serial.println(String(F("\tFile Not Found: ")) + path);
  return false;                     // If the file doesn't exist, return false
}

void eventConnectController(AsyncEventSourceClient *client) {
  if(client->lastId()){
    Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
  }
  // send event with message "hello!", id current millis
  // and set reconnect delay to 1 second
  client->send("hello!", NULL, millis(), 2000);
}

void notFoundController(AsyncWebServerRequest *request) {
  Serial.println(F("On not found"));
  if (!handleFileRead(request, request->url())){                  // send it if it exists
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += request->url();
    message += "\nMethod: ";
    message += (request->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += request->args();
    message += "\n";
    for (uint8_t i = 0; i < request->args(); i++) {
      message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
    }
    request->send(404, "text/plain", message);
  }
}

void homeController(AsyncWebServerRequest *request) {
  if (!is_authenticated(request)) {
    Serial.println(F("Go on not login!"));
    AsyncWebServerResponse *response = request->beginResponse(301); //Sends 301 redirect
    response->addHeader("Location", "/login?msg=Please login to visit home page&to-page=home");
    response->addHeader("Cache-Control", "no-cache");
    response->addHeader("Set-Cookie", "ESPSESSIONID=0");
    request->send(response);
    return;
  }
  String message;
  request->send(SPIFFS, "/home.html", getContentType("home.html"), false);
}

void passwordController(AsyncWebServerRequest *request) {
  if (request->url().indexOf("/reset-pass") != -1) {

  }
  else if (request->url().indexOf("/change-pass") != -1) {

  }
}

void loginController(AsyncWebServerRequest *request) {
  Serial.println("Handle login");
  String msg;
  if (request->method() == HTTP_GET) {
    request->send(SPIFFS, "/login.html", getContentType("login.html"), false);
    return ;
  }
  else if (request->method() == HTTP_POST) {
    if (request->hasHeader("Cookie")) {
      // Print cookies
      Serial.print("Found cookie: ");
      String cookie = request->header("Cookie");
      Serial.println(cookie);
    }
    if (request->hasArg("username") && request->hasArg("password")) {
      Serial.print("Found parameter: ");
      if (request->arg("username") == String(www_uname) && request->arg("password") == String(www_pass)) {
        AsyncWebServerResponse *response = request->beginResponse(301); //Sends 301 redirect
        response->addHeader("Location",  request->hasArg("activenav") ? "/?activenav="+request->arg("activenav") : "/");
        response->addHeader("Cache-Control", "no-cache");
        String token = sha1(String(www_uname) + ":" + String(www_pass) + ":" + request->client()->remoteIP().toString());
        Serial.print("Token: ");
        Serial.println(token);
        response->addHeader("Set-Cookie", "ESPSESSIONID=" + token);
        request->send(response);
        lastSession = millis();
        Serial.println("Log in Successful");
        return;
      }
      msg = "Wrong username/password! try again.";
      Serial.println("Log in Failed");
      AsyncWebServerResponse *response = request->beginResponse(301); //Sends 301 redirect
      response->addHeader("Location", "/login?msg=" + msg);
      response->addHeader("Cache-Control", "no-cache");
      request->send(response);
      return;
    }
  }
}

/**
* Manage logout (simply remove correct token and redirect to login form)
*/
void logoutController(AsyncWebServerRequest *request) {
  Serial.println("Disconnection");
  AsyncWebServerResponse *response = request->beginResponse(301); //Sends 301 redirect
  response->addHeader("Location", "/login?msg=User disconnected");
  response->addHeader("Cache-Control", "no-cache");
  response->addHeader("Set-Cookie", "ESPSESSIONID=0");
  request->send(response);
  return;
}

/**
 *      Controller for handling static server resources
 */
void resourceController(AsyncWebServerRequest * request) {
  if (request->method() == HTTP_GET) {
    if (request->url() == "/background.jpg") {
      request->send(SPIFFS, "/background.jpg", getContentType("/background.jpg"));
    }
    else if (request->url() == "/login.css") {
      request->send(SPIFFS, "/login.css", getContentType("/login.css"));
    }
    else if (request->url() == "/login.js") {
      request->send(SPIFFS, "/login.js", getContentType("/login.js"));
    }
    else if (request->url() == "/home.css") {
      request->send(SPIFFS, "/home.css", getContentType("/home.css"));
    }
    else if (request->url() == "/home.js") {
      request->send(SPIFFS, "/home.js", getContentType("/home.js"));
    }
  }
}

/**
 *      Controller for fetching sms and errors table data from DB
 */
void tabelsDataController(AsyncWebServerRequest * request) {
  if (request->method() == HTTP_GET) {
    
  }
}

/**
 *     Controller for handling static file uploads
 */
void uploadFileController(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    Serial.printf("Upload started: %s\n", filename.c_str());
    // open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open("/"+filename, "w");
  }

  if (len) {
    // stream the incoming chunk to the opened file
    Serial.printf("%s", (const char*)data);
    request->_tempFile.write(data, len);
  }

  if (final) {
    Serial.printf("Upload finised: %s (%u)\n", filename.c_str(), index+len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    AsyncWebServerResponse *response = request->beginResponse(301); //Sends 301 redirect
    response->addHeader("Location", "/?active-nav=ota");
    response->addHeader("Cache-Control", "no-cache");
    response->addHeader("Set-Cookie", "ESPSESSIONID=0");
    request->send(response);
    return;
  }
}

void authenticateUpload(AsyncWebServerRequest *request) {
  if (!is_authenticated(request)) {
    Serial.println(F("Go on not login!"));
    AsyncWebServerResponse *response = request->beginResponse(301); //Sends 301 redirect
    response->addHeader("Location", "/login?msg=Please login to upload a file&to-page=home&active-nav=ota");
    response->addHeader("Cache-Control", "no-cache");
    response->addHeader("Set-Cookie", "ESPSESSIONID=0");
    request->send(response);
    return;
  }
  request->send(200);
  return;
}

//Check if header is present and correct
bool is_authenticated(AsyncWebServerRequest *request) {
  Serial.println("Enter is_authenticated");
  if (request->hasHeader("Cookie") && (millis() - lastSession) < SESSION_TIMEOUT) {
    Serial.print("Found cookie: ");
    String cookie = request->header("Cookie");
    Serial.println(cookie);
    String token = sha1(String(www_uname) + ":" +
    String(www_pass) + ":" +
    request->client()->remoteIP().toString());
    //  token = sha1(token);
    if (cookie.indexOf("ESPSESSIONID=" + token) != -1) {
      Serial.println("Authentication Successful");
      lastSession = millis();
      return true;
    }
  }
  Serial.println("Authentication Failed");
  return false;
}

void serverRouter() {
  // External rest end point (out of authentication)
  server.on("/", HTTP_GET, homeController);
  server.on("/login", HTTP_GET, loginController);
  server.on("/login", HTTP_POST, loginController);
  server.on("/logout", HTTP_GET, logoutController);  // Handle Web Server
  server.on("/background.jpg", HTTP_GET, resourceController);
  server.on("/login.css", HTTP_GET, resourceController);
  server.on("/login.js", HTTP_GET, resourceController);
  server.on("/home.css", HTTP_GET, resourceController);
  server.on("/home.js", HTTP_GET, resourceController);
  server.on("/uploadbin", HTTP_POST, authenticateUpload, uploadFileController);
  server.on("/api/table-data", HTTP_GET, tabelsDataController);
  Serial.println(F("Go on not found!")); 

  server.onNotFound(notFoundController);
  //Serial.println(F("Set cache!"));
  // Serve a file with no cache so every tile It's downloaded
  //server.serveStatic("/configuration.json", SPIFFS, "/configuration.json", "no-cache, no-store, must-revalidate");
  // Server all other page with long cache so browser chaching they
  // Comment this line for esp8266
  //#ifdef ESP32
  //server.serveStatic("/", SPIFFS, "/", "max-age=31536000");
  //#endif
}

String getContentType(String filename) {
  if (filename.endsWith(F(".htm"))) return F("text/html");
  else if (filename.endsWith(F(".html"))) return F("text/html");
  else if (filename.endsWith(F(".css"))) return F("text/css");
  else if (filename.endsWith(F(".js"))) return F("application/javascript");
  else if (filename.endsWith(F(".json"))) return F("application/json");
  else if (filename.endsWith(F(".png"))) return F("image/png");
  else if (filename.endsWith(F(".gif"))) return F("image/gif");
  else if (filename.endsWith(F(".jpg"))) return F("image/jpeg");
  else if (filename.endsWith(F(".jpeg"))) return F("image/jpeg");
  else if (filename.endsWith(F(".ico"))) return F("image/x-icon");
  else if (filename.endsWith(F(".xml"))) return F("text/xml");
  else if (filename.endsWith(F(".pdf"))) return F("application/x-pdf");
  else if (filename.endsWith(F(".zip"))) return F("application/x-zip");
  else if (filename.endsWith(F(".gz"))) return F("application/x-gzip");
  else if (filename.endsWith(F(".bin")) || filename.endsWith(F(".hex"))) return F("application/x-binary");
  return F("text/plain");
}

void getSensorReadings(){
  temperature = random(1, 100)/ 100.0F;
  // Convert temperature to Fahrenheit
  //temperature = 1.8 * bme.readTemperature() + 32;
  humidity = random(1, 100)/ 100.0F;
  pressure = random(1, 100)/ 100.0F;
}
#ifdef ESP32
String sha1(String payloadStr){
    const char *payload = payloadStr.c_str();
 
    int size = 20;
 
    byte shaResult[size];
 
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA1;
 
    const size_t payloadLength = strlen(payload);
 
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char *) payload, payloadLength);
    mbedtls_md_finish(&ctx, shaResult);
    mbedtls_md_free(&ctx);
 
    String hashStr = "";
 
    for(uint16_t i = 0; i < size; i++) {
        String hex = String(shaResult[i], HEX);
        if(hex.length() < 2) {
            hex = "0" + hex;
        }
        hashStr += hex;
    }
 
    return hashStr;
}
#endif

// Initialize WiFi
void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }
    Serial.println(WiFi.localIP());
}

String processor(const String& var){
  getSensorReadings();
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(temperature);
  }
  else if(var == "HUMIDITY"){
    return String(humidity);
  }
  else if(var == "PRESSURE"){
    return String(pressure);
  }
  return String();
}

String getRandomString(int lent) {
  char numbs[] = "123456789";
  char caps[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  char smalls[] = "abcdefghijklmnopqrstuvwxyz";
  String text;
  for (int i = 0; i < lent; i++) {
    int num = random(1, 3);
    switch (num)
    {
    case 1:
      text.concat(numbs[random(1, 9)]);
      break;
    case 2:
      text.concat(caps[random(1, 26)]);
      break;
    case 3:
      text.concat(smalls[random(1, 26)]);
      break;
    default:
      break;
    }
  }
  return text;
}


/***********************************************************************************
 *  SQLite DB functions here
 * ********************************************************************************/

const char* data = "Callback function called";
static int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   Serial.printf("%s: ", (const char*)data);
   for (i = 0; i<argc; i++){
       Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   Serial.printf("\n");
   return 0;
}

int openDb(char *filename, sqlite3 **db) {
   int rc = sqlite3_open(filename, db);
   if (rc) {
       Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
       return rc;
   } else {
       Serial.printf("Opened database successfully\n");
   }
   return rc;
}

int db_exec(sqlite3 *db, const char *sql) {
   Serial.println(sql);
   long start = micros();
   int rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   if (rc != SQLITE_OK) {
       Serial.printf("SQL error: %s\n", zErrMsg);
       sqlite3_free(zErrMsg);
   } else {
       Serial.printf("Operation done successfully\n");
   }
   Serial.print(F("Time taken:"));
   Serial.println(micros()-start);
   return rc;
}

// if (items.length > 4) {
//     items_array = items[3].split("+04")

//     items[3] = items_array[0]
//     items[4] = items_array[1]

    
//     smsDataSet = JSON.parse(localStorage.getItem('smsData'))
//     localStorage.removeItem('smsData')
//     sms.push(e.data.id)
//     sms.push(items[1].replace('\"', ''))
//     sms.push(items[2].replace('\"', ''))
//     sms.push('20'+ items[3].replace('-', ''))
//     sms.push(items[4].replace('\"', ''))
//     smsDataSet.push(sms)
//     localStorage.setItem('smsData', JSON.stringify(smsDataSet))
//     smsTable.row.add(sms).draw()
// }