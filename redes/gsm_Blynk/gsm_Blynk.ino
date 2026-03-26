// Este exemplo foi baseado na documentação da placa da LilyGo

//Pinos do Modem e Serial
#define UART_BAUD           115200
#define MODEM_TX            27
#define MODEM_RX            26
#define MODEM_PWRKEY        4
#define MODEM_DTR           32
#define MODEM_RI            33
#define MODEM_FLIGHT        25
#define MODEM_STATUS        34
#define BAT_ADC             35

#define SD_MISO             2 //SPI
#define SD_MOSI             15 //SPI
#define SD_SCLK             14 //SPI
#define SD_CS               13 //SPI

// Outros pinos
#define LED_PIN             23 //Led para sinalização
#define RELE1_PIN           18
#define RELE2_PIN           19
#define SDA_PIN             21 //I2C para o Sensor
#define SCL_PIN             22 //I2C para o Sensor

// Defines usados para identificação do dispositivo no Blynk
#define BLYNK_TEMPLATE_ID "TMPL27iW3NNBO"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "S1Kwmamil5dJggp7liFTsO0qeJq7pZMp"

// Seleção do MODEM
#define TINY_GSM_MODEM_SIM7600

// Seleciona a serial usada para DEBUG (para Serial Monitor, default baud rate 115200)
#define SerialMon Serial

// Define a serial para envio dos comandos AT para o módulo (MODEM)
// Use Hardware Serial no Mega, Leonardo, Micro
// no ESP, Serial1 fica nos pinos RX1=9 e TX1=10 (usados pela PSRAM)
// usar outros para evitar conflitos (MODEM_TX = 27 e MODEM_RX = 26)
#define SerialAT Serial1 //Serial do Modem (Serial1) como SerialAT


// O intervalo padrão do heartbeat para GSM é de 60
// Se você quiser anular este valor, descomente e defina esta opção:
// #define BLINK_HEARTBEAT 30




#include <TinyGsmClient.h> //Biblioteca do Modem
#include <BlynkSimpleTinyGSM.h> //Biblioteca do Blynk
#include "Wire.h" //Biblioteca do I2C
#include "SHT2x.h" //Bibliteca para utilização do sensor
//https://www.arduinolibraries.info/libraries/sht2x


BlynkTimer timer; //timer para execução periódica do Blynk

//Você deve obter o Auth Token no aplicativo do Blink
//Vá para o project Settings (icone da engrenagem)
char auth[] = BLYNK_AUTH_TOKEN; //Token de autorização da plataforma Blynk
char apn[] = "INTERNET.BR"; //APN para acesso a rede
char user[] = "";
char pass[] = "";

bool reply = false; //sinalizador de resposta do Blink
float temperatura = 0.0; //Armazena a temperatura
float umidade = 0.0; //Armazena a umidade
boolean estadoRele1 = false; //Armazena o estado do rele 1
boolean estadoRele2 = false; //Armazena o estado do rele 2
boolean estadoLed = false; //Armazena o estado do Led




SHT2x htu21d; //Objeto que representa o sensor

//Veja todos os comandos AT, se desejar
//#Define DUMP_AT_COMMANDS
#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif


void setup()
{
    // Set console baud rate
    SerialMon.begin(115200);
    //delay(10);
    setupPinos();
    // Set GSM module baud rate
    SerialAT.begin(UART_BAUD, SERIAL_8N1, MODEM_RX, MODEM_TX);
    // Inicia o sensor
    setupHTU();
    //Inicia o MoDem
    setupMoDem();
    //Inicia o Blynk
    Blynk.begin(auth, modem, apn, user, pass);
    // Define a função que deve ser chamada a cada 5 segundos
    timer.setInterval(5000L, sendSensor);
}

void loop()
{

    Blynk.run(); // Função necessária do Blink
    timer.run(); // reativa o timer (GSM) caso já tenha disparado
}


void setupPinos(){
    // define a função dos pinos utilizados
    pinMode(RELE1_PIN, OUTPUT);
    pinMode(RELE2_PIN, OUTPUT);
    pinMode(MODEM_PWRKEY, OUTPUT);
    pinMode(MODEM_FLIGHT, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    Serial.println("Funções dos pinos definidas.");

    // Inicia os relés em desligados (Atenção para lógica invertida do módulo)
    estadoRele1 = HIGH;
    estadoRele2 = HIGH;
    estadoLed = LOW;
    digitalWrite(RELE1_PIN, estadoRele1);
    digitalWrite(RELE2_PIN, estadoRele2);
    digitalWrite(LED_PIN, estadoLed);
    Serial.println("Estados dos pinos definidos.");

}






void setupHTU()
{
  htu21d.begin();
  Serial.println("Sensor HTU21D iniciado.");
}

// Executa a consulta às medidas disponíveis no sensor
void leHTU21()
{
  // Inicia o sensor
  htu21d.read();
  temperatura = htu21d.getTemperature();
  umidade = htu21d.getHumidity();
}












void setupMoDem()
{
  //O sinal de ativação PWRKEY do Modem deve ser definido, senão ele não responderá aos comandos AT
  digitalWrite(MODEM_PWRKEY, LOW);
  delay(500); //delay necessário
  digitalWrite(MODEM_PWRKEY, HIGH);
  //MODEM_FLIGHT Define o modo de funcionamento do MODEM (modo de vôo)
  //deve ser ajustado para nível alto
  digitalWrite(MODEM_FLIGHT, HIGH);

  //O reinicio leva muito tempo, pode-se chamar init() ao invés de restart
  Serial.println("Iniciando o MODEM...");
  //  if (!modem.restart()) {
  //     Serial.println("Failed to restart modem, attempting to continue without restarting");
  //}
  if (modem.init()) {
    Serial.println("Falha ao iniciar o MODEM");
  }
  //obtem o nome do MoDem
  String resp = modem.getModemName();
  Serial.println("Nome do MODEM: " + resp);
  delay(1000);
}



//envia dados do sensor para o Blynk
void sendSensor()
{
  leHTU21(); //obtem temperatura e umidade

  //envia as variáveis para serial (se desejável)
  Serial.print("Temp: "); Serial.println(temperatura);
  Serial.print("Umid: "); Serial.println(umidade);

  //Se os valores não são numéricos, não envia para o Blink
  if (isnan(temperatura) || isnan(umidade))
  {
    Serial.println("Falha obtendo dados do HTU21D!");
    return;
  }
  //Envia os valores para Blink
  // Por favor não enviar mais de 10 valores por segundo
  Blynk.virtualWrite(V0, temperatura);
  Blynk.virtualWrite(V1, umidade);
}





BLYNK_WRITE(V2) { //Executa a escrita no pino virtual 2 (LED)
  if(param.asInt()==0)
  {
   Serial.println("led off");
   estadoLed = LOW;
   digitalWrite(LED_PIN, estadoLed);
   Blynk.logEvent("LED_OFF");//Sending Events
  }
  else{
    Serial.println("led on");
    estadoLed = HIGH;
    digitalWrite(LED_PIN, estadoLed);
    Blynk.logEvent("LED_ON");//Sending Events
  }
}










BLYNK_WRITE(V3) { //Executa a escrita no pino virtual 3 (Rele1)
  if(param.asInt()==0)
  {
   Serial.println("rele1 off");
   estadoRele1 = LOW;
   Blynk.logEvent("rele1_OFF");//Sending Events
  }
  else{
    Serial.println("rele1 on");
    estadoRele1 = HIGH;
     Blynk.logEvent("rele1_ON");//Sending Events
  }
  digitalWrite(RELE1_PIN, !estadoRele1); //O módulo relé tem lógica invertida
}











BLYNK_WRITE(V4) { //Executa a escrita no pino virtual 4 (Rele2)
  if(param.asInt()==0)
  {
   Serial.println("rele2 off");
   estadoRele2 = LOW;
   Blynk.logEvent("rele2_OFF");//Sending Events
  }
  else{
    Serial.println("rele2 on");
    estadoRele2 = HIGH;
     Blynk.logEvent("rele2_ON");//Sending Events
  }
  digitalWrite(RELE2_PIN, !estadoRele2); //O módulo relé tem lógica invertida
}

//Sincronizando o estado de saída com o aplicativo na inicialização
BLYNK_CONNECTED() //Ocorre ao conectar com a plataforma
{
  sendSensor();//Sincroniza dados do sensor

  Blynk.syncVirtual(V2); // fará com que BLYNK_WRITE(V2) seja executado
  Blynk.syncVirtual(V3); // fará com que BLYNK_WRITE(V3) seja executado
  Blynk.syncVirtual(V4); // fará com que BLYNK_WRITE(V4) seja executado
}
