//GRUPO 4
//BARCAROLO, MORAT Y RESNIK

#include <U8g2lib.h>
#include "DHT.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

/* ----------------------------------------------------------------------------------------------------- */

// Definicion de constructores y variables globales

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

/* ----------------------------------------------------------------------------------------------------- */
void printBMP_OLED(void);
void printBMP_OLED2(void) ;
#define BOTON1 34
#define BOTON2 35
#define P1 0
#define P2 1
#define RST 20
#define BC1 7
#define BC2 8
#define BC3 9
#define BC4 10
#define BC5 11
#define ESPERAFINAL 12
#define AUMENTAR 4
#define RESTAR 5
int estado = RST;
#define LED 25
#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float temp;
int valorU = 23;
int millis_valor;
int millis_actual;
int millis_boton;
int millis_actual2;

const char* ssid = "MECA-IoT";
const char* password = "IoT$2025";

#define BOTtoken "7561786153:AAEAnTbyt_XnvsfXFY1onCdNb3hJCKMGF_o"
#define CHAT_ID "7389596977"


WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

TaskHandle_t Task1;
TaskHandle_t Task2;



void setup() {
  pinMode(LED, OUTPUT);
  pinMode(BOTON1, INPUT_PULLUP);
  pinMode(BOTON2, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println(F("DHTxx test!"));
  u8g2.begin();
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  bot.sendMessage(CHAT_ID, "hola", "");


  xTaskCreatePinnedToCore(
    Task1code,   /* Task function. */
    "Task1",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */
  delay(500);

  xTaskCreatePinnedToCore(
    Task2code,   /* Task function. */
    "Task2",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task2,      /* Task handle to keep track of created task */
    1);          /* pin task to core 1 */
  delay(500);
}


void Task1code( void * pvParameters ) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  for (;;) {
    millis_actual = millis();

    if (temp >= valorU) {
      digitalWrite(LED, HIGH);
      if (millis_actual - millis_valor >= 5000) {
        bot.sendMessage(CHAT_ID, "La temperatura supera el umbral", "");
        millis_valor = millis_actual;
      }
    } else {
      digitalWrite(LED, LOW);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  // Recibir actualizaciones de Telegram
  TelegramUpdate updates = bot.getUpdates(); // Get the updates from the Telegram API
  if (updates.isValid()) {
    // Procesar las actualizaciones recibidas
    for (int i = 0; i < updates.updatesCount; ++i) {
      millis_actual2 = millis();
      if (millis_actual2 - millis_valor >= 5000) {

        const auto& update = updates[i];

        // Verificar si el mensaje es de un usuario (no de un bot)
        if (update.message->from.id != 0) {
          String text = update.message->text; // Get the text message
          Serial.print("Mensaje recibido: ");
          Serial.println(text);

          // Realizar acciones según el texto recibido
          if (text.equals("TA")) {
            bot.sendMessage(CHAT_ID, temp, "");
            // digitalWrite(PIN_LED, HIGH);
          }
        }
      }
    }
  }



  void Task2code( void * pvParameters ) {
    Serial.print("Task2 running on core ");
    Serial.println(xPortGetCoreID());

    for (;;) {
      millis_actual = millis();

      if (millis_actual - millis_valor >= 5000) {
        temp = dht.readTemperature();
        if (isnan(temp)) {
          Serial.println(F("Failed to read from DHT sensor!"));
        }
        millis_valor = millis_actual;
      }
      switch (estado) {
        case RST:

          millis_valor = millis();
          estado = P1;
          break;

        case P1:

          printBMP_OLED();
          if (digitalRead(BOTON1) == LOW) {
            estado = BC1;
          }
          break;

        case BC1:

          if (digitalRead(BOTON1) == HIGH) {
            estado = BC2;
            millis_boton = 0;
          }
          break;
        case BC2:

          if (digitalRead(BOTON2) == LOW) {
            estado = BC3;
          }
          if (millis_boton - millis_valor >= 5000) {
            estado = P1;
          }
          break;
        case BC3:

          if (digitalRead(BOTON2) == HIGH) {
            estado = BC4;
          }
          if (millis_boton - millis_valor >= 5000) {
            estado = P1;
          }
          break;
        case BC4:

          if (digitalRead(BOTON1) == LOW) {
            estado = BC5;
          }
          if (millis_boton - millis_valor >= 5000) {
            estado = P1;
          }
          break;
        case BC5:

          if (digitalRead(BOTON1) == HIGH) {
            estado = P2;
          }
          if (millis_boton - millis_valor >= 5000) {
            estado = P1;
          }
          break;
        case P2:

          printBMP_OLED2();
          if (digitalRead(BOTON1) == LOW) {
            estado = AUMENTAR;
          }
          if (digitalRead(BOTON2) == LOW) {
            estado = RESTAR;
          }

          break;
        case ESPERAFINAL:

          if (digitalRead(BOTON1) == HIGH && digitalRead(BOTON2) == HIGH) {
            estado = P1;
          }

          break;

        case AUMENTAR:


          if (digitalRead(BOTON1) == HIGH) {
            valorU = valorU + 1;
            estado = P2;
          }
          if (digitalRead(BOTON1) == LOW && digitalRead(BOTON2) == LOW) {
            estado = ESPERAFINAL;
          }
          break;

        case RESTAR:

          if (digitalRead(BOTON2) == HIGH) {
            valorU = valorU - 1;
            estado = P2;
          }
          if (digitalRead(BOTON1) == LOW && digitalRead(BOTON2) == LOW) {
            estado = ESPERAFINAL;
          }

          break;
      }

      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
  }


  void loop() {

  }

  void printBMP_OLED(void) {
    char stringU[5];
    char stringtemp[5];
    u8g2.clearBuffer();          // clear the internal memory
    u8g2.setFont(u8g2_font_t0_11b_tr); // choose a suitable font
    sprintf (stringtemp, "%.2f" , temp); ///convierto el valor float a string
    sprintf (stringU, "%d" , valorU); ///convierto el valor float a string
    u8g2.drawStr(0, 35, "T. Actual:");
    u8g2.drawStr(60, 35, stringtemp);
    u8g2.drawStr(90, 35, "°C");
    u8g2.drawStr(0, 50, "V. Umbral:");
    u8g2.drawStr(60, 50, stringU);
    u8g2.drawStr(75, 50, "°C");
    u8g2.sendBuffer();          // transfer internal memory to the display
  }

  void printBMP_OLED2(void) {
    char stringU[5];
    u8g2.clearBuffer();          // clear the internal memory
    sprintf (stringU, "%d" , valorU);
    u8g2.setFont(u8g2_font_t0_11b_tr); // choose a suitable font
    u8g2.drawStr(0, 50, "V. Umbral:");
    u8g2.drawStr(60, 50, stringU);
    u8g2.drawStr(75, 50, "°C");
    u8g2.sendBuffer();          // transfer internal memory to the display
  }
