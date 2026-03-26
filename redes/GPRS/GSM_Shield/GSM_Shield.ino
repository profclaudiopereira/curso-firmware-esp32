#include <SoftwareSerial.h>
#include <GSM.h>

// Define os pinos para a comunicação serial com o shield
#define SIM900_TX_PIN 9
#define SIM900_RX_PIN 10

// Define o número de telefone para ligar
#define TELEFONE "+5581999255443" // Substitua pelo número desejado

// Cria um objeto para a comunicação serial
SoftwareSerial serialSIM900(SIM900_TX_PIN, SIM900_RX_PIN);

void setup() {
  // Inicia a comunicação serial com o computador
  Serial.begin(9600);

  // Inicia a comunicação serial com o shield
  serialSIM900.begin(9600);

  Serial.println("Iniciando...");
  delay(1000);

  // Envia comandos para inicializar o shield
  enviarComando("AT", 1000);
  enviarComando("ATE0", 1000); // Desativa eco
  enviarComando("AT+CPIN?", 1000); // Verifica se o SIM está desbloqueado
  enviarComando("AT+CREG?", 1000); // Verifica o registro na rede
}

void loop() {
  // Faz a chamada
  fazerChamada(TELEFONE);
  delay(20000); // Aguarda 20 segundos
  desligarChamada();
  delay(5000);
}

// Função para enviar comandos AT e verificar a resposta
String enviarComando(String comando, int tempoEspera) {
  String resposta = "";
  serialSIM900.println(comando);
  delay(tempoEspera);
  while (serialSIM900.available()) {
    resposta += (char)serialSIM900.read();
  }
  Serial.print("Comando: ");
  Serial.println(comando);
  Serial.print("Resposta: ");
  Serial.println(resposta);
  return resposta;
}

// Função para fazer a chamada
void fazerChamada(String numero) {
  enviarComando("ATD" + numero + ";", 1000);
}

// Função para desligar a chamada
void desligarChamada() {
  enviarComando("ATH;", 1000);
}