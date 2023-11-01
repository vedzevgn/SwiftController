#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BluetoothSerial.h>
#include <BleKeyboard.h>
#include <EEPROM.h>
#include "GyverTimer.h"
#include "icons.h"

#define DEVICE_INFO "SwiftController;S/N092300001"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
BleKeyboard keyboard("SwiftController", "Higlight House", 100);

GTimer connectionTimer(MS);

//static NimBLEUUID bmeServiceUUID("91bad492-b950-4226-aa2b-4ede9fa42f59");
//static NimBLEUUID stringUUID("cba1d466-344c-4be3-ab3f-189f80dd7518");

#define ICON_HEIGHT 60
#define ICON_WIDTH 60

/*bool connectToServer(BLEAddress pAddress) {
   BLEClient* pClient = BLEDevice::createClient();
 
  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to server");
 
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(bmeServiceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(bmeServiceUUID.toString().c_str());
    return (false);
  }
 
  stringCharacteristic = pRemoteService->getCharacteristic(stringUUID);

  if (stringeCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID");
    return false;
  }
  Serial.println(" - Found our characteristics");
 
  stringCharacteristic->registerForNotify(stringNotifyCallback);
  return true;
}*/





#define MAX_KEY_LENGTH 16
#define MAX_VALUE_LENGTH 32
#define MAX_ENTRIES 10

struct KeyValuePair {
  char key[MAX_KEY_LENGTH];
  char value[MAX_VALUE_LENGTH];
};

class KeyValueCollection {
public:
  KeyValueCollection() {
    EEPROM.begin(sizeof(KeyValuePair) * MAX_ENTRIES);
    loadCollection();
  }

  void add(const char* key, const char* value) {
    if (size >= MAX_ENTRIES) {
      return;
    }

    for (int i = 0; i < size; i++) {
      if (strcmp(entries[i].key, key) == 0) {
        strncpy(entries[i].value, value, MAX_VALUE_LENGTH);
        saveCollection();
        return;
      }
    }

    KeyValuePair entry;
    strncpy(entry.key, key, MAX_KEY_LENGTH);
    strncpy(entry.value, value, MAX_VALUE_LENGTH);

    entries[size] = entry;
    size++;

    saveCollection();
  }

  const char* get(const char* key) {
    for (int i = 0; i < size; i++) {
      if (strcmp(entries[i].key, key) == 0) {
        return entries[i].value;
      }
    }

    return nullptr;
  }

private:
  KeyValuePair entries[MAX_ENTRIES];
  int size = 0;

  void loadCollection() {
    for (int i = 0; i < MAX_ENTRIES; i++) {
      EEPROM.get(i * sizeof(KeyValuePair), entries[i]);
      if (entries[i].key[0] == '\0') {
        break;
      }
      size++;
    }
  }

  void saveCollection() {
    for (int i = 0; i < size; i++) {
      EEPROM.put(i * sizeof(KeyValuePair), entries[i]);
    }
    EEPROM.commit();
  }
};

KeyValueCollection collection;

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(10);
  Serial.println(DEVICE_INFO);
  connectionTimer.setInterval(500);
  Serial.println("Starting SwiftController");
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.drawBitmap((display.width() - ICON_WIDTH) / 2, (display.height() - ICON_HEIGHT) / 2, logo, ICON_WIDTH, ICON_HEIGHT, 1);
  display.display();
  keyboard.begin();
  Serial.println("Ready");
  display.clearDisplay();
}

void loop() {
  if(connectionTimer.isReady()) Serial.println(DEVICE_INFO);
  if (Serial.available() > 0) {
    char received[MAX_KEY_LENGTH];
    int len = Serial.readBytesUntil('\n', received, MAX_KEY_LENGTH - 1);
    received[len] = '\0';
    parseString(received);
  }
}

void executeShortcut(String str) {
  int len = str.length();
  for (int i = 0; i < len; i++) {
    if (str.charAt(i) == 'c' && str.charAt(i + 1) == 't' && str.charAt(i + 2) == 'r' && str.charAt(i + 3) == 'l') {
      keyboard.press(KEY_LEFT_CTRL);
      Serial.print("CTRL ");
      i += 3;
    } else if (str.charAt(i) == 'u' && str.charAt(i + 1) == 'p' && str.charAt(i + 2) == 'a' && str.charAt(i + 3) == 'r' && str.charAt(i + 4) == 'r') {
      Serial.print("UPARR ");
      keyboard.press(KEY_UP_ARROW);
      i += 3;
    } else if (str.charAt(i) == 'd' && str.charAt(i + 1) == 'a' && str.charAt(i + 2) == 'r' && str.charAt(i + 3) == 'r') {
      Serial.print("DARR ");
      keyboard.press(KEY_DOWN_ARROW);
      i += 3;
    } else if (str.charAt(i) == 'l' && str.charAt(i + 1) == 'a' && str.charAt(i + 2) == 'r' && str.charAt(i + 3) == 'r') {
      Serial.print("LARR ");
      keyboard.press(KEY_LEFT_ARROW);
      i += 3;
    } else if (str.charAt(i) == 'r' && str.charAt(i + 1) == 'a' && str.charAt(i + 2) == 'r' && str.charAt(i + 3) == 'r') {
      Serial.print("RARR ");
      keyboard.press(KEY_RIGHT_ARROW);
      i += 3;
    } else if (str.charAt(i) == 'g' && str.charAt(i + 1) == 'u' && str.charAt(i + 2) == 'i') {
      keyboard.press(KEY_LEFT_GUI);
      Serial.print("GUI ");
      i += 3;
    } else if (str.charAt(i) == 'a' && str.charAt(i + 1) == 'l' && str.charAt(i + 2) == 't') {
      keyboard.press(KEY_LEFT_GUI);
      Serial.print("ALT ");
      i += 3;
    } else if (isAlpha(str.charAt(i))) {
      Serial.print(str.charAt(i));
      keyboard.press(str.charAt(i));
    }
  }
  keyboard.releaseAll();
  str = "";
}


void parseString(String str) {
  int screen, icon;
  int pressed;
  String shortcut;

  if (str == "sendConnect") {
    Serial.println(DEVICE_INFO);
    delay(100);
    Serial.println(DEVICE_INFO);
  } else {
    if (sscanf(str.c_str(), "s%di%d", &screen, &icon) == 2) {
      changeIcon(screen, icon);

    } else if (sscanf(str.c_str(), "p%d", &pressed) == 1) {
      String pressedS = String(pressed);
      executeShortcut(collection.get(pressedS.c_str()));

    } else {
      String input = str;
      if (input.startsWith("s") && input.indexOf("sc") != -1) {
        int keyStartIndex = input.indexOf("s") + 1;
        int keyEndIndex = input.indexOf("sc");
        int valueStartIndex = input.indexOf("sc") + 2;

        if (keyStartIndex < keyEndIndex && keyEndIndex < valueStartIndex) {
          String key = input.substring(keyStartIndex, keyEndIndex);
          String value = input.substring(valueStartIndex);

          collection.add(key.c_str(), value.c_str());

          Serial.print("Screen: ");
          Serial.print(key.c_str());
          Serial.print(" Shortcut: ");
          Serial.print(value.c_str());
          return;
        }
      }
    }
  }
}

void changeIcon(int screenNumber, int iconNumber) {
  Serial.print("Changing: ");
  Serial.print(screenNumber);
  Serial.print(" Icon: ");
  Serial.println(iconNumber);
  drawIcon(getIcon(iconNumber));
}


const unsigned char* ICONS_LIST[] = {copy_ico, paste_ico, mute_ico, volumeup, volumedown, pause_ico, play, backward, forward, screenshot, search, moon, lock};

const unsigned char* getIcon(int number) {
  if (number >= 0 && number < sizeof(ICONS_LIST) / sizeof(ICONS_LIST[0])) {
    return ICONS_LIST[number];
  } else {
    return 0;
  }
}

void drawIcon(const unsigned char* icon) {
  display.clearDisplay();
  display.drawBitmap((display.width() - ICON_WIDTH) / 2, (display.height() - ICON_HEIGHT) / 2, icon, ICON_WIDTH, ICON_HEIGHT, 1);
  display.display();
}