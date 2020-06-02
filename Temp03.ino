#include "SSD1306.h"
#include <OneWire.h>
#include <DS18B20.h>
#include <ESP8266WiFi.h>  // Importa a Biblioteca ESP8266WiFi
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient

// Setup display SSD 1306

#define SDA D1 //5
#define SCL D2 //4
SSD1306 display(0x3c, SDA, SCL);

// Setup sensor DS18B20
OneWire oneWire(D5);   // pino onde estah o sensor DS18B20
DS18B20 sensor(&oneWire);

// WIFI
const char* SSID = "Galia"; // rede
const char* PASSWORD = "1231231231"; // Senha da rede WI-FI
  
// MQTT
const char* MQTT_BROKER = "mqtt.eclipse.org"; //URL do broker MQTT que se deseja utilizar
int PORTA_BROKER = 1883; // Porta do Broker MQTT
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient

#define TOPICO_SUBSCRIBE "cmd"       //tópico MQTT de escuta
#define TOPICO_PUBLISH   "temp/aquario/camarao"    //tópico MQTT de envio de informações para Broker
#define ID_MQTT  "ControleAlice"          //id mqtt (para identificação de sessão)

//-----------------------------------------------------------------------------------

//Prototypes
float pegaTemperatura();
void telaTemperatura(float t);
void initWiFi();
void reconectWiFi();
String ip2Str(IPAddress ip);
void telaWiFi();
void initMQTT();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void enviaTemperaturaMQTT(float t);
void verificaConexoesMQTT(void);

//----------------------------------------------------------------------------------

//Funcao: converte um endereco IP em string.
//Parametros: ip do tipo IPAddress
//Retorno: strint com o endereco IP
String ip2Str(IPAddress ip){
  String s="";
  for (int i=0; i<4; i++) {
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  }
  return s;
}

//Funcao: inicializa e conecta-se na rede WI-FI desejada
//Parametros: nenhum
//Retorno: nenhum
void initWiFi() {
    display.clear();
    display.setFont(ArialMT_Plain_24);
    display.drawString(5, 0, ">>"+(String)SSID+"<<");
    display.display();
    delay(10);     
    reconectWiFi();
}

//Funcao: reconecta-se ao WiFi
//Parametros: nenhum
//Retorno: nenhum
void reconectWiFi() {
  int cc = 5;
    
  display.setFont(ArialMT_Plain_16);
    
  //se jah estáah conectado a rede WI-FI, nada eh feito. 
  //Caso contrario, são efetuadas tentativas de conexao
  if (WiFi.status() == WL_CONNECTED)
    return;
         
  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
     
  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        display.drawString(cc, 30, ".");
        display.display();
        cc+=3;
  }   
  telaWiFi();
}

//----------------------------------------------------------------------------------

//Funcao: inicializa parametros de conexao MQTT(endereço do 
//        broker, porta e seta função de callback)
//Parametros: nenhum
//Retorno: nenhum
void initMQTT(){
  MQTT.setServer(MQTT_BROKER, PORTA_BROKER);   //informa qual broker e porta deve ser conectado
  MQTT.setCallback(mqtt_callback);            //atribui funcaoo de callback 
} 
 
//Funcao: função de callback 
//Parametros: padroes da biblioteca MQTT.
//Retorno: nenhum
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
 
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
   
    //toma acao dependendo da string recebida:
    //verifica se deve colocar nivel alto de tensão na saída D0:
    //IMPORTANTE: o Led já contido na placa eh acionado com logica invertida (ou seja,
    //enviar HIGH para o output faz o Led apagar / enviar LOW faz o Led acender)
    if (msg.equals("L"))
    {
        digitalWrite(LED_BUILTIN, LOW);        
    }
 
    //verifica se deve colocar nivel alto de tensao na saída D0:
    if (msg.equals("D"))
    {
        digitalWrite(LED_BUILTIN, HIGH);
    }
     
}

//Funcao: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
//        em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
//Parametros: nenhum
//Retorno: nenhum
void reconnectMQTT() {
  if (MQTT.connect(ID_MQTT)) {
    // Conectado com sucesso ao broker MQTT
    MQTT.subscribe(TOPICO_SUBSCRIBE); 
  } 
  else {
    display.clear();    
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 0, "!");
    display.setFont(ArialMT_Plain_16);
    display.drawString(5, 30, ip2Str(WiFi.localIP()));
    display.display();
    delay(2000);
  }      
}
   
//Funcao: verifica o estado das conexões WiFI e ao broker MQTT. 
//        Em caso de desconexão (qualquer uma das duas), a conexão
//        é refeita.
//Parametros: nenhum
//Retorno: nenhum
void verificaConexoesMQTT(void) {
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
}
 
//Funcao: envia ao Broker o estado atual do output 
//Parametros: temperatura a ser enviada
//Retorno: nenhum
void enviaTemperaturaMQTT(float t){
  String s;

  s = (String)t;
  MQTT.publish(TOPICO_PUBLISH,(char*) s.c_str());
  delay(1000);
}

//----------------------------------------------------------------------------------

// Faz a leitura do sensor DS18B20
float pegaTemperatura(){
  float t;
  
  sensor.requestTemperatures();
  while (!sensor.isConversionComplete());
  t = sensor.getTempC();
  return t;
}

// Tela para mostrar dados da conexao WiFi.
void telaWiFi(){
  String  ipStr;
  
  display.clear();    
  display.setFont(ArialMT_Plain_24);
  display.drawString(5, 0, ">>"+(String)SSID+"<<");
  ipStr = ip2Str(WiFi.localIP());
  display.setFont(ArialMT_Plain_16);
  display.drawString(10, 30, ipStr);
  display.display();
  delay(7000);
}

// Tela para mostrar temperatura
void telaTemperatura(float t){
  String strTemp;

  display.clear();
  strTemp = (String) t;
  display.setFont(ArialMT_Plain_16);
  display.drawString(15, 4, "Temperatura");
 
  display.setFont(ArialMT_Plain_24);
  display.drawString(20, 30, strTemp+ " C");
 
  display.display();
}

//-----------------------------------------------------------------------------------

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // Inicia o pino LED_BUILTIN como output
  digitalWrite(LED_BUILTIN, LOW);   // LED on >> inicio do setup
  
  // Configura display
  display.init();
  display.flipScreenVertically();
  display.clear();

  // Configura DS18B20 
  sensor.begin();
  sensor.setResolution(12);
  sensor.requestTemperatures();
  while (!sensor.isConversionComplete());

  // Configura Wifi
  initWiFi();

  // Configura MQTT
  initMQTT();

  digitalWrite(LED_BUILTIN, HIGH);   // LED off >> fim setup
}

 
void loop() { 
  float t;
  
  verificaConexoesMQTT();
  t = pegaTemperatura();
  telaTemperatura(t);
  enviaTemperaturaMQTT(t);
 
  delay(5000);
}
