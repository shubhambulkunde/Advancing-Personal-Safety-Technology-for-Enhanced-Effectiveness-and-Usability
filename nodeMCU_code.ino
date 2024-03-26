#include <SoftwareSerial.h>
#include <ArduinoJson.h>  // Include the ArduinoJson library for JSON parsing

class Location {
private:
  String latitude;
  String longitude;
public:
  Location(String lat, String lon) {
    latitude = lat;
    longitude = lon;
  }
  void setLatitude(String lat) {
    latitude = lat;
  }
  void setLongitude(String lat) {
    latitude = lat;
  }
  String getLatitude() {
    return latitude;
  }
  String setLongitude() {
    return longitude;
  }
};
Location deviceLocation("0.0", "0.0");

class Contact {
public:
  String name;
  int number;
  Contact(String n, int no) {
    name = n;
    number = no;
  }
};

Contact emergencyContact("Contact1", 123456789);
String SOS = "0";
// RX, TX
SoftwareSerial sim800l(12, 13);
SoftwareSerial mySerial(5, 4);

class GSMModule {
private:
  String apn;
public:
  GSMModule(String apn = "airtelgprs.com") {
    this->apn = apn;
    sim800l.begin(9600);
    //"Initializing SIM800L...
    sim800l.println("AT");

    // Replace "your_apn", "your_username", "your_password" with your actual APN, username, and password
    sim800l.println("AT+CGDCONT=1,\"\",\"" + apn + "\"\r");

    //"Connecting to GPRS..."
    sim800l.println("AT+CGATT=1");

    //"Connecting to the Internet..."
    sim800l.println("AT+CSTT=\"" + apn + "\",\"\",\"\"");

    //"Bringing up the wireless connection...");
    sim800l.println("AT+CIICR");
  }

  void sendSMS(String message, Contact recipient) {

    //"Setting SMS mode..."
    sim800l.println("AT+CMGF=1");

    // Replace "recipient_number" with the recipient's phone number
    sim800l.print("AT+CMGS=\"" + String(recipient.number) + "\"\r");

    // Replace "your_message" with the message you want to send
    sim800l.print(message);

    // End the SMS message with Ctrl+Z (ASCII code 26)
    sim800l.print((char)26);
  }
};

class FirebaseManager {
private:
  String APIKey = "-------";
  String RTDBUrl = "https://personalsafety3-default-rtdb.firebaseio.com/";
  String bucket = "S1";
public:
  void parseFirebaseResponse(String response) {
    // Use ArduinoJson library to parse the JSON response
    DynamicJsonDocument jsonDocument(1024);  // Adjust the size based on your expected JSON response size
    deserializeJson(jsonDocument, response);

    // Extract and store the data as needed
    emergencyContact.number = jsonDocument["phone"];
    SOS = jsonDocument["SOS"];
    //Serial.print("Received data: ");
    //Serial.println(key);
  }

  void sendATCommand(const char* command, int timeout) {
    sim800l.println(command);  // This line sends the specified AT command to the SIM800L module.
    delay(timeout);            //This line introduces a delay to wait for the specified timeout period
    /*while (sim800l.available()) {
      Serial.write(sim800l.read());
    }*/
  }

  void writeToDatabase(String tag, String data) {
    //"Connecting to Firebase...");

    // Construct the Firebase URL
    String firebaseURL = RTDBUrl + "//" + bucket + ".json?auth=" + APIKey;

    sim800l.print("AT+HTTPINIT\r");
    delay(1000);

    sim800l.print("AT+HTTPPARA=\"CID\",1\r");
    delay(1000);

    sim800l.print("AT+HTTPPARA=\"URL\",\"" + RTDBUrl + "\"\r");
    delay(1000);

    //Serial.println("Setting up HTTP POST request...");
    sim800l.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r");
    delay(1000);

    // Replace "your_json_data" with the actual JSON payload you want to send
    String jsonData = "{\"" + tag + "\":\"" + data + "\"}";
    sim800l.print("AT+HTTPDATA=" + String(jsonData.length()) + ",10000\r");
    delay(1000);
    sim800l.print(jsonData);
    delay(1000);

    //Serial.println("Sending HTTP POST request...");
    sendATCommand("AT+HTTPACTION=1", 5000);

    //Serial.println("Reading response...");
    sendATCommand("AT+HTTPREAD", 2000);

    //Serial.println("Closing HTTP connection...");
    sendATCommand("AT+HTTPTERM", 1000);
  }

  void readFromDatabase() {
    // Implement reading data from Firebase
    //Serial.println("Connecting to Firebase...");

    // Construct the Firebase URL
    String firebaseURL = RTDBUrl + "//" + bucket + "//" + ".json?auth=" + APIKey;

    sim800l.print("AT+HTTPINIT\r");
    delay(1000);

    sim800l.print("AT+HTTPPARA=\"CID\",1\r");
    delay(1000);

    sim800l.print("AT+HTTPPARA=\"URL\",\"" + firebaseURL + "\"\r");
    delay(1000);

    //Serial.println("Sending HTTP GET request...");
    sendATCommand("AT+HTTPACTION=0", 5000);

    String response = "";
    while (sim800l.available()) {
      response += sim800l.readString();
    }

    // Print the entire response for debugging
    //Serial.println("Full Response:");
    //Serial.println(response);

    //Serial.println("Parsing and storing data...");
    parseFirebaseResponse(response);

    //Serial.println("Closing HTTP connection...");
    sendATCommand("AT+HTTPTERM", 1000);

    //return response;
  }
};

class EmergencyServicesNotifier {
private:
  GSMModule gsmModule;

public:
  void notifyEmergencyServices() {
    //oye pols aa gai pols wee woo wee woo...
  }

  void notifyEmergencyContacts() {
    gsmModule.sendSMS("Send Help At : www.google.com/maps/place/" + deviceLocation.getLatitude() + "+" + deviceLocation.getLatitude(), emergencyContact);
  }
};

void splitString(const String& input, char separator, String parts[], int maxParts) {
  int partIndex = 0;
  int startIndex = 0;
  int endIndex = input.indexOf(separator);

  while (endIndex != -1 && partIndex < maxParts - 1) {
    parts[partIndex] = input.substring(startIndex, endIndex);
    startIndex = endIndex + 1;
    endIndex = input.indexOf(separator, startIndex);
    partIndex++;
  }

  // Handle the last part of the string
  if (partIndex < maxParts) {
    parts[partIndex] = input.substring(startIndex);
    partIndex++;
  }

  // Fill the remaining parts with an empty string
  for (; partIndex < maxParts; partIndex++) {
    parts[partIndex] = "";
  }
}

unsigned long sendDataPrevMillis = 0;

FirebaseManager fb;

EmergencyServicesNotifier emergencyAlert;

void setup() {
  Serial.begin(115200);
  mySerial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // Main loop code
  String inputString = mySerial.readStringUntil('\r');

  if ((((millis() - sendDataPrevMillis) > 15000) || (sendDataPrevMillis == 0)) && (inputString.length() > 3)) {

    sendDataPrevMillis = millis();
    fb.readFromDatabase();
    // Define an array to store the parts
    String parts[3];

    switch (inputString.charAt(0)) {
      case 's':
        // Call the splitString function
        splitString(inputString.substring(1), '.', parts, 3);
        fb.writeToDatabase("iot", inputString);
        
        if(SOS.equals("1") || parts["2"].equals("1")){
            emergencyAlert.notifyEmergencyContacts();
        }

        break;

      case 'g':
        // Call the splitString function
        splitString(inputString.substring(1), ',', parts, 2);
        fb.writeToDatabase("lat", parts[0]);
        fb.writeToDatabase("lon", parts[1]);
        deviceLocation.setLatitude(parts[0]);
        deviceLocation.setLongitude(parts[1]);
        break;
    }
  }


}
