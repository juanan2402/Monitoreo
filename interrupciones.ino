#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <esp_now.h>
#include <WiFi.h>

typedef struct struct_message {
  int id;
  float t;
  float h;
}struct_message;

struct_message myData;  
struct_message board1;
struct_message board2;
struct_message board3;
struct_message board4;
struct_message boardStruct[4] = {board1, board2, board3, board4};

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  
  memcpy(&myData, incomingData, sizeof(myData));
  // Update the structures with the new incoming data
  boardStruct[myData.id-1].t = myData.t;
  boardStruct[myData.id-1].h = myData.h;

}

LiquidCrystal_I2C lcd(0x3F, 16, 2);
const uint8_t ROWS = 4;
const uint8_t COLS = 4;
const String pass = "1234";
String input = "";
int intentos = 0;

char keys[ROWS][COLS] = {
  { '1', '2', '3', '<' },
  { '4', '5', '6', '=' },
  { '7', '8', '9', '>' },
  { ':', '0', ';', '?' }
}; 
uint8_t colPins[COLS] = {16, 4, 5, 15};
uint8_t rowPins[ROWS] = {23, 19, 18, 17};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

volatile int cnt1=0;
volatile int cnt2=0;
volatile int cnt3=0;
volatile int cnt4=0;

volatile bool isUpdated = false;

volatile unsigned long lastInterruptTime1 = 0;
volatile unsigned long lastInterruptTime2 = 0;
volatile unsigned long lastInterruptTime3 = 0;
volatile unsigned long lastInterruptTime4 = 0;
const unsigned long debounceDelay = 50; 

hw_timer_t *My_timer = NULL;
volatile bool enviarData = false;
bool acceso = false;
bool pararLect = false;

// Timer interrupt handler
void IRAM_ATTR onTimer() {
  enviarData = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(34, INPUT_PULLUP);
  pinMode(35, INPUT_PULLUP);
  pinMode(32, INPUT_PULLUP);
  pinMode(33, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(34), isr1, FALLING);
  attachInterrupt(digitalPinToInterrupt(35), isr2, FALLING);
  attachInterrupt(digitalPinToInterrupt(32), isr3, FALLING);
  attachInterrupt(digitalPinToInterrupt(33), isr4, FALLING);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Clave acceso:");
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  dht.begin();

  // Initialize timer
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 300000000, true);
  timerAlarmEnable(My_timer);
}

void loop() {
  char key = keypad.getKey();

  if (!acceso) {
    if (key) {
      if (key == '<') {
        // Clear the input
        input = "";
        lcd.setCursor(0, 1);
        lcd.print("                ");
      } else if (key == '>') {
        if (input == pass) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Bienvenido");
          delay(2000);
          lcd.clear();
          acceso = true;
          pararLect = false; 
          Serial.println("block");
        } else {
          intentos++;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Incorrecto");
          delay(2000);
          lcd.clear();
          if (intentos >= 3) {
            while (true){
              lcd.setCursor(0, 0);
            lcd.print("Bloqueado");
            delay(5000);
            lcd.clear();
            lcd.print("Clave acceso:");
            input="";
            intentos=0;
            break;
            
              }
          } else {
            lcd.setCursor(0, 0);
            lcd.print("Clave acceso:");
          }
          input = "";
        }
      } else {
        if (input.length() < 16) {
          input += key;
          lcd.setCursor(input.length() - 1, 1);
          lcd.print(key);
        }
      }
    }
  } else {
    if (key == ':') {
      
      pararLect = true;
      acceso = false;
      Serial.println("block");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Lectura detenida");
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Clave acceso:");
      input="";
    }

    if (!pararLect) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Leyendo datos...");
      lcd.setCursor(0, 1);
      lcd.print("* para apagar");
      if(enviarData){
      enviarData = false;

      float h = dht.readHumidity();
      float t = dht.readTemperature();
      


      Serial.print(t);
      Serial.print(",");
      Serial.print(h);
      Serial.print(",");
      Serial.print(boardStruct[0].t);
      Serial.print(",");
      Serial.print(boardStruct[0].h);
      Serial.print(",");
      Serial.print(boardStruct[1].t);
      Serial.print(",");
      Serial.print(boardStruct[1].h);
      Serial.print(",");
      Serial.print(boardStruct[2].t);
      Serial.print(",");
      Serial.print(boardStruct[2].h);
      Serial.print(",");
      Serial.print(boardStruct[3].t);
      Serial.print(",");
      Serial.println(boardStruct[3].h);
      } 
    }
  }
  if (isUpdated) {
    isUpdated = false; // Reset the flag

    // Adjust counters as needed
    if (cnt2 > cnt1) cnt2 = cnt1;
    if (cnt4 > cnt3) cnt4 = cnt3;

    // Print updated counters
    Serial.print(cnt1);
    Serial.print(",");
    Serial.print(cnt2);
    Serial.print(",");
    Serial.print(cnt3);
    Serial.print(",");
    Serial.println(cnt4);
  }
}
void isr1() {
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime1 > debounceDelay) {
    cnt1++;
    isUpdated = true;
    lastInterruptTime1 = currentTime;
  }
}

void isr2() {
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime2 > debounceDelay) {
    cnt2++;
    isUpdated = true;
    lastInterruptTime2 = currentTime;
  }
}

void isr3() {
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime3 > debounceDelay) {
    cnt3++;
    isUpdated = true;
    lastInterruptTime3 = currentTime;
  }
}

void isr4() {
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime4 > debounceDelay) {
    cnt4++;
    isUpdated = true;
    lastInterruptTime4 = currentTime;
  }
}
