#include <FirebaseESP32.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include <string>

// Defina as suas credenciais do Firebase
#define FIREBASE_HOST "https://automacaoa3-30cf0-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "AIzaSyCNQAkW_15jHJl74APyAnfmRvjFYksTI7Q"

// Defina as suas credenciais de rede Wi-Fi
#define WIFI_SSID "Sandro"
#define WIFI_PASSWORD "sandro12"

// Pino do LED
#define LED_PIN 25// living_room
#define LED_PIN2 21 // bedroom
#define LED_PIN3 33 // bathroom
#define LED_PIN4 32 // kitchen


int rele = 15;

// Pinos do sensor ultrassônico
#define TRIG_PIN 26
#define ECHO_PIN 27


// Distância máxima permitida
#define MAX_DISTANCE 16 // Em centímetros
#define MIN_DISTANCE 3

// Objeto do sensor ultrassônico
int distance;
long duration;

// Pino do servo
#define SERVO_PIN 13
Servo myServo;

FirebaseData ledStatusData;
FirebaseData firebaseData;

// Inicialização da conexão Wi-Fi
void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.println(".");
  }

  Serial.println();
  Serial.println("Conectado à rede Wi-Fi!");
}

// Inicialização do Firebase
void setupFirebase() {
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

// Configuração inicial
void setup() {
  Serial.begin(9600);
  setupWiFi();
  setupFirebase();

  // Define os pinos dos leds
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(LED_PIN3, OUTPUT);
  pinMode(LED_PIN4, OUTPUT);

  pinMode(rele, OUTPUT);
  digitalWrite(rele, HIGH);

  // Define os pinos do sensor ultrassônico como entrada/saída
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Define o pino do servo motor como saída
  myServo.attach(SERVO_PIN);
  
  // Define a posição inicial do servo motor
  myServo.write(0);

}

void handleLight(String room, int pin) {
  if (Firebase.getBool(ledStatusData, "rooms/" + room + "/light/status")) {
    bool ledStatus = ledStatusData.boolData();
    digitalWrite(pin, ledStatus ? HIGH : LOW);

    if (ledStatus = HIGH) {
      Serial.println("Luz " + room + " ligada");
    } else {
      Serial.println("Luz " + room + " desligada");
    }
  }
}

// Loop principal
void loop() {
  // Cria um objeto FirebaseData
  FirebaseData data;


  int porcentagem;
  // Realiza a leitura da distância do sensor ultrassônico
  porcentagem = getUltrasonicDistance();
  
  // Envia a distância para o Firebase
  sendDistanceToFirebase(porcentagem);

  handleLight("living_room", LED_PIN);
  handleLight("bedroom", LED_PIN2);
  handleLight("bathroom", LED_PIN3);
  handleLight("kitchen", LED_PIN4);



  // Obtém o valor booleano da porta no Firebase
  if (Firebase.getBool(data, "rooms/living_room/door/status")) {
    bool doorStatus = data.boolData();

    // Verifica o valor lido
    if (doorStatus) {
      // Abre a porta
      myServo.write(90);
      Serial.println("Porta aberta");
    } else {
      // Fecha a porta
      myServo.write(0);
      Serial.println("Porta fechada");
    }
  } else {
    myServo.write(0);
  }

  if (porcentagem  <= 50) {
    digitalWrite(rele, HIGH);
    Serial.println("Rele acionado");
  } else if (porcentagem <= 90 ) {
    digitalWrite(rele, LOW);
    Serial.println("Rele Desativado");
  }
  Serial.print("Nivel: ");
  Serial.print(porcentagem);
  Serial.println(" %");
  delay(500);
}

// Função para obter a distância do sensor ultrassônico
int getUltrasonicDistance() {
  // Limpa o pino de trigger
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Envia um pulso de 10 microssegundos no pino de trigger
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Mede a duração do pulso de eco
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calcula a distância em centímetros
  int distance = duration * 0.034 / 2;
  
  // Comverte em porcentagem
  int porcentagem = map(distance, 16, 3, 0, 100);

  return porcentagem;
}

// Função para enviar a distância para o Firebase
void sendDistanceToFirebase(int porcentagem) {

  // Envia o valor da distância para o Firebase
  Firebase.setInt(firebaseData, "rooms/bathroom/water_tank/level", porcentagem);
}

