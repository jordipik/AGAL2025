//  (c) MARINA EYE-CAM TECHNOLOGIES 
//  16/7/2024  Carles Lorente 
// ------------------------------------------------------------------------
// CODIS DE RETORN DEL SERVEI WEB QUAN ES CONNECTA LA BASCULA PER INTERNET
// 0 = OK
// 1 = Error parser (general / malformat)
// 2 = Error idpes  (rang de pes diferent a la BBDD per aquest puesto)
// 3 = Error Weight (no te pes asignat)
// ------------------------------------------------------------------------

#include <WiFi.h>             // Libreria de connexió WIFI (per fer multi-wifi
#include <HTTPClient.h>       // Llibreria per poder connectar al servidor de BBDD
#include <FastLED.h>          // LLibreria per gestionar les matrisu de LEDs
#include "time.h"             // Llibreria per gestionar la data 
#include "caracters.h"        // Llibreria de Carles Lorente per fer el caracters
#include <WebServer.h>        // Llibreria per poder tenir el servidor web i actualitzar remotament
#include <Update.h>           // Llibreria que pujar el fitxer del programa i el carrega 

const String versio = "36-19/11/24";                                    // canviar per actualitzar versió a la caixa
const String urlh = "https://bascula.eye-cam.com/receive.php";       // la URL de enviament pesatges
const String urlm = "https://bascula.eye-cam.com/getidfrommac.php";  // la URL de registre de bàscula

const char* ntpServer = "pool.ntp.org";  // per agafar l'hora 
const long  gmtOffset_sec = 0;           // OFFSET de hores respcte e GMT
const int   daylightOffset_sec = 3600;   // 1 hora de diferencia 

int second;                // Variables de temps 
int minute;
int hour;
int day;
int month;
int year;
int weekday;
int fase = 0;             // FASES pesatge 0=(0Kg o neg) i no tara, 1= (0 o negatiu) i Tarat, 2 = Positu. ~ , 3= Pos. Estable y dentro de rango (posible caja), 4 Negatiu baixa i envia,  
int estav;                // variable de enviament estable           
long col;
long colorLED;
long colorLEDESTABLE;
struct tm timeinfo;
HTTPClient http;
WebServer server(80);
// Connexions WIFI definides
// -------------------------  Les WIFIS
const char* ssid[] ={
                      
                      "IPhone de Jordi",
                      "MOVISTAR_0F9D",
                     //"Bascula",
                     //"Vodafone-891C",
                     //"MOVISTAR_0F9D",
                     //"Bascula2",
                     };

// --------------------------- Els passwords
const char* password[] ={
                      "654321APB", // IPHONE JORDI
                      "FC4431DA50DF72FA9999", // CASA JORDI
                      //"Bascules#20#24", //
                      //"TrXT9TKafCgqtJsR",
                      //"FC4431DA50DF72FA9999",
                      //"Bascules#20#24",
                      };



const int pinp2 = 0; // Butó del Pin de test de pantalla y recarga

#define NUM_LEDS 64*1  // QUANTITAT DE LEDS TRICOLOR CONNECTATS
bool invert = false; //set true for snake display types,false for zig-zag type 
bool rotate = false; //set true for rotate 90º (and no invert)
unsigned int a = 64;
unsigned int tt = 0;
unsigned int netId = 0;
byte n = 0;
unsigned figu =0;
unsigned int count = 0;
unsigned int b = 0;
unsigned int ic = 0;
unsigned int fig = 0;
unsigned int nn = 0;
unsigned int tram = 0;
unsigned int dato = 0;
unsigned int dato2 = 0;
unsigned int hexString = 0;
unsigned int pesok = 0;
int enviado = 0;
int peso2 = 0;
int peso1 = 1;
int pesoEstable = 0;
int ultimoPesoInsta = 0;
int Nivel
 = 0;

//COLORES
long colorVerdeClaro = 0x000500;
long colorVerdeIntenso = 0x001500;
long colorRojoClaro = 0x050000;
long colorRojoIntenso = 0x150000;
long colorNaranjaClaro = 0x221100;
long colorNaranjaIntenso = 0x301500;
long colorAmarilloClaro = 0x050500;
long colorAmarilloIntenso = 0x151500;
long colorAzulClaro = 0x000010;
long colorAzulIntenso = 0x000020;

long colorMagentaClaro = 0x220022;
long colorMagentaIntenso = 0x440044;

int dif1 = 0;
int envia = 0;          // trama que envia la bàscula
int difer = 0;          // la diferencia en paso de 1/8 positiu o negatiu
String peso = "0000g";  // on enmagatzema el valor del pes
String pesov ="0000g";  // on enmagatzema el valor del pes per enviar quant toca
String ip="000.000.000.000";  // per guarda la IP quan arrenca
String cap ="---------------- MAC NO REGISTRADA --------------"; // el missatge a la pàgina de la caixa
String mac="00:00:00:00:00:00"; // per guradar la MAC quan arrenca
String nom="--";        // on guarda el nom de aquesta bascula a la BBDD ej: "15A"    
unsigned int pue =0;    // on guarda la id de la bàscuala a la BBDD
String ope ="-";      // operari (no es fa servir, però s'envia a la BBDD)
String fecha ="00/00/0000"; // pwer guardar la data 
String hora ="00:00:00";// hora(n
String milis="00";      // milisegons (no utilitzat)
String trama = "";      // trama que envia la bàscula
String trama2= "";      // trama de l'enviament anterior
unsigned int tip =0;    // tipus de pesada (0=OK Verda, 1=Exces Taronja, -1=Curta Vermell)
unsigned int minim =0;  // Pes minim seleccionat a la BBDD
unsigned int maxim =0;  // Pes màcim seleccionat a la BBDD
String zero = "";       // si el pes es cero el bit que marca
String envio ="";       // On s'enmagatzema la cadena d'enviament a la BBDD
unsigned int idp =0;    // IDP= Index De Pesada de la BBDD
unsigned tara = 0;      // valor de Tara 
unsigned taraGram = 0;  // bit de TaraGram 
unsigned int taraTemp = 0;  // bit de TaraGram 
unsigned vcero = 0;     // valor de cero
int estable = 0;        // bit d'estabale 
int estado = 0;         // Estat de la pesada
int estadoEstable= 0;   // Estado ESTABLE de la pesada
#define DATA_PIN 2      // Pin per on s'envia a la matrix les dades (sda)
#define CLOCK_PIN 13    // Pin per on s'envia a la matriu el clock (NO ES FA SERVIR al ws8262)
CRGB leds[NUM_LEDS];    // Es prepara l'espai pels LEDS
#define RXD2 16         // Defineix pin de entrada - Recepció RS232
#define TXD2 17         // Deifneix pin de sortida - Transmissió RS232 (no utilitzat)
HardwareSerial mySerial(2); // Defineix la UART 2 -- La RS-232 de la báscula

//--------------- Rutina d'arrencada (nomes un cop)
void setup() { 
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // Inicialitza la connexió amb la matriu de LEDs
  pinMode(pinp2, INPUT);  // Configura el pin del botó intern como entrada
  delay(250);
  mySerial.begin(38400,SERIAL_8N1,16,17); // Inicialitza la comunicació serie al pins definidos
  delay(250);                         //wait for wifi start 
  Serial.begin(115200);               // Velocitat del port serie de l'ordinador (per debug)
  Serial.println("->");             
  Serial.println("Iniciant...");
  memset(leds, 0, sizeof(leds));      // Esborra l'espai dels LEDS a zero
  pintaFig(versio[1],colorMagentaIntenso,0,1);       // Pinta en verd versió caracter dret 1 segon    
  pintaFig(versio[0],colorMagentaIntenso,4,1);       // Pinta en verd versió caracter esquerra 1 segon
  getSerial(); // segons + fasledshow + serial to led63
  Serial.print("Versió: ");
  Serial.print(versio[0]);
  Serial.println(versio[1]);
  
  int i=0;                            //  Variable d'us general a bucles
  int sent = 1;                       //  Marca per saber si ha enviat o no
  int wi = 0;                         //  Variable per fer la divisió de la barra d'avan´de connexió 
  delay(1000);                         //   Retard a l'inici per esperar a que tot arrenqui
  WiFi.begin(ssid[0], password[0]);   // Carrega la WIFI amb la primera opció i password
  memset(leds, 0, sizeof(leds));      // Esborra l'espai dels LEDS a zero
  pintaFig(netId+1,colorMagentaIntenso,2,0);     // Carrega la ID de la WIFI a la que connecta en blau 
  FastLED.show();                     // Ho pinta als leds
  Serial.printf("Connectanta a xarxa %s ", ssid[netId]);  // Ho treu pel port serie de l'ordinador (per debug)
  Serial.printf("Password %s ", password[netId]);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);                     // bucles de espera de 1/2 segon
      Serial.print(".");              // va treinet un punt pel port de l'ordinador (per debug)
      if (++wi %28 == 0) {            // Bucle compket 3 iteraccions de 14s
        netId = (wi / 28) % 3;        // Canvia de xarxa cada 28 intents (14 segons)
        memset(leds, 0, sizeof(leds));// esborra tots els valors dels leds a zero
        pintaFig(netId+1,colorMagentaIntenso,2,0);// posa les unitats en color nagenta (RRGGBB)
        getSerial(); // segons + fasledshow + serial to led63
        //FastLED.show();                // pinta el numero
        Serial.print("\nCanviant a la xarxa WiFi ");
        Serial.println(ssid[netId]);   // treu la id de la xarxa Wifi de conenxió pel port (per debug)
        Serial.printf("Password %s ", password[netId]);
        WiFi.begin(ssid[netId], password[netId]); // connecta ala niova WiFi
      }
      leds[63-i].setRGB(0,(i*10), 60); //pinta el troç de barra en verd-blavós que augmenta
      leds[63-i-1].setRGB( 0, 0, 0);   // esborra l'ultim led per quan va marxa enrere
      FastLED.show();                  // pinta la barra
      i = i + sent;                    // bucle de 7 leds de barra de connexió
      if (i == 0) sent = 1;            // si es zero canvia el sentit a positiu
      if (i == 7) sent = -1;           // si es 7 canvia el sentit a negatiu
  }
      leds[63-i].setRGB(0,100, 00);  // ja ha connectat i posa el led actual en verd
      leds[63-i-1].setRGB( 0, 0, 0); // Esborra l'ultim punt de la barra
      FastLED.show();                // Ho pinta 
  Serial.print(" CONNECTAT a la xarxa ");
  Serial.print(ssid[netId]);
  Serial.print(" amb la IP: ");
  Serial.println(WiFi.localIP()); // treu IP  a ordinador (per debug)
  Serial.println(WiFi.macAddress()); // treu MAC a ordinador (per degu)
  ip=WiFi.localIP().toString(); // guarda la IP de la placa per enviar-la
  mac = WiFi.macAddress();      // guarda la MAC de la wifi per enviar-la
  delay(500);                   // Retard de 1/2 segons
  memset(leds, 0, sizeof(leds));// esborra tots els valors dels leds a zero   
  // Agafa la data i hora del servidor NTP seleccionat
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); 
  printLocalTime();             // Imprimeix la data (per debug)
  montServer();                 // Monta el servidor web per actualitzacions
  mySerial.begin(38400,SERIAL_8N1,16,17); // Inicialitza la comunicació serie al pins definidos
  leepesoBBDD();                // llegeis dades de pes, si la MAC esta registrada
  mySerial.begin(38400,SERIAL_8N1,16,17); // Inicialitza la comunicació serie al pins definidos

  }


void loop() { 

  for (tt=0;tt<50;tt++) { //loop every 500 ms
  if (tt % 10 == 0) server.handleClient(); // pasa cada 100ms
  if (tip == 1 ) { gramZ3();} // si gramZ3
  if (tip == 2 ) { wunder();} // si Wunder
  if (tip == 98 ) { TestOffline();}
  
  delay(5); //50*10 = 500ms 
  }
  }

// RUTINAS 
void printLocalTime(){

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
    }
    Serial.println(&timeinfo, "%A, %B %D %Y %H:%M:%S");
    }

void pintaFig(int fig, CRGB col,int des, int asc) {
    if ( asc == 1 ) {
      fig = fig - 33; // for ascii conversión
    } else {
      fig = fig + 15; // for numeric conversión
    }
    //Serial.println(fig);
    CRGB miColor = col; // Utiliza el color pasado como parámetro
    for (nn = 0; nn < (NUM_LEDS / 64); nn++) {
        for (count = 0; count < 8; count++) {
            n = carac[7 - count + (fig * 8)];
            for ( b = 0; b < 8; b++) {
                ic = count;
                if ((b & 1) == 0 && invert == 1) ic = 7 - count; // Invierte columnas pares si está activado
                if (rotate == 0) ic = 7 - b; // Rotación si está desactivada

                int ledIndex = des; // Inicia con un desplazamiento base de -5
                if (rotate == 1) ledIndex += (8 * b) + ic + (nn * 64);
                if (rotate == 0) ledIndex += (8 * count) + ic + (nn * 64);
                
                // Solo asigna el color si el índice es válido (mayor o igual a 0)
                if (bitRead(n, 7-b) == 1 && ledIndex >= 0) {
                    leds[ledIndex] = miColor;
                }
            }
        }
    }
  }

void pintaptr4(char* ptr2,CRGB col,int del1, int del2){
    // ----------------------------------------------------------------------
    // Rutina que pinta les 4 xifres (2+2) a la matriu display amb un retard  
    // (4 caracters, color, retard 1ers 2 carac., retard 2ones 2 carac.) 
    //-----------------------------------------------------------------------
    memset(leds, 0, sizeof(leds));   // Esborra tot l'espai de la matriu de LEDs
    pintaFig(ptr2[1],col,0,1);       // caracter, color, posició (0=units 4=decs), 1 = Conevrsió a Ascii
    leds[3]=col;                     // posa el punt . separador
    pintaFig(ptr2[0],col,5,1);       // caracter, color, posició (0=units 4=decs), 1 = Conevrsió a Ascii
    FastLED.show();                  // Ho pinta 
    delay(del1*1000);                // Primer retard en segons
    memset(leds, 0, sizeof(leds));   // Esborra tot l'espai de la matriu de LEDs   
    pintaFig(ptr2[3],col,0,1);       // caracter, color, posició (0=units 4=decs), 1 = Conevrsió a Ascii       
    pintaFig(ptr2[2],col,4,1);       // caracter, color, posició (0=units 4=decs), 1 = Conevrsió a Ascii
    FastLED.show();                  // Ho pinta 
    delay(del2*1000);                // Segon retard
  }

void leepesoBBDD(){
  // ----------------------------------------------------------------------
  // Rutina que llegeix de BBDD el que hi ha configurat per aquesta bàscula  
  //-----------------------------------------------------------------------
  envio =urlm+"?mac="+mac+"&ip="+ip+"&ver="+versio;
  Serial.print("xxxxxx Envio consulta peso "); Serial.println(envio);
  http.begin(envio);
  //http.setTimeout(10000);
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println("-"+ payload + "-");
    if (payload != "KO") {
      Serial.println("PAYLOAD = OK");
      char charArray[50] ;
      payload.toCharArray(charArray, 50);
    // idpuesto,idpeso,peso,min,max,tipobas,Nombre 
      char *ptr = strtok(charArray, ",");
      pue= atoi(ptr);
      ptr = strtok(NULL, ","); if (ptr !=NULL) { idp = atoi(ptr);} else { idp =0; ptr="0000"; }  
      ptr = strtok(NULL, ","); if (ptr !=NULL) { pesok= atoi(ptr); } else { pesok = 0; ptr="0000"; }
      pintaptr4(ptr,colorRojoIntenso,2,1); //pinta ptr, color, delay 1, delay 2 (segs) 
      Serial.println("-------------->");
      Serial.println(versio);
      ptr = strtok(NULL, ","); if (ptr !=NULL) { minim = atoi(ptr); } else { minim = 0; ptr="0000"; }
      //pintaptr4(ptr,colorVerdeIntenso,2,1); //pinta ptr, color, delay 1, delay 2 (segs) 
      ptr = strtok(NULL, ","); if (ptr !=NULL) { maxim = atoi(ptr); } else { maxim = 0; ptr="0000"; } 
      //pintaptr4(ptr,colorNaranjaIntenso,2,1); //pinta ptr, color, delay 1, delay 2 (segs) 
      ptr = strtok(NULL, ","); if (ptr !=NULL) { tip = atoi(ptr); } else { tip = 0; } 
      ptr = strtok(NULL, ","); if (ptr !=NULL) { nom = ptr; } else { nom = "--"; }  //ptr = strtok(NULL, ",");
      Serial.println("-------------->");
      memset(leds, 0, sizeof(leds));
      pintaFig(nom[1],colorVerdeIntenso,0,1);
      pintaFig(nom[0],colorVerdeIntenso,4,1);
      getSerial(); // 2 segons + fasledshow + serial to led63
      cap ="Puesto: "+String(pue)+" / IDpeso: "+String(idp)+" / Pesok: "+String(pesok)+" / Minim: "+String(minim)+" / Maxim: "+String(maxim)+" / Tipus_Bascula: "+String(tip)+" / Lloc: "+nom;
      Serial.println(cap);
      } else {
        Serial.println("MAC SENSE REGISTRAR -----------");
        macError:                       // Bucle d'espera si la MAC no esta registrada a la BBDD
          server.handleClient();          // escolta el servei web per poder actualitzar tambe amb Mac error
          memset(leds, 0, sizeof(leds));  // esborra tots els LEDS
          pintaFig(mac[13],colorRojoIntenso,0,1); // pinta l'ultim caracter de la MAC
          pintaFig(mac[12],colorRojoIntenso,4,1); // pinta el penultim caracter de la MAC
          getSerial(); // 2 segons + fasledshow + serial to led63
          memset(leds, 0, sizeof(leds)); 
          pintaFig(mac[16],colorRojoIntenso,0,1); // pinta els dos darrers caracters de la MAC
          pintaFig(mac[15],colorRojoIntenso,4,1); // pinta el penultim caracter de la MAC
          getSerial();
          memset(leds, 0, sizeof(leds));
          pintaFig('*',colorRojoIntenso,0,1);     // pinta la 'X' de error MAC que es el "*"
          getSerial();
        goto macError; 
    } 
  } else {
    Serial.print("Error en la solicitud HTTP: ");
    Serial.println(httpResponseCode);
  }
  http.end();  // Finaliza la conexión
  Serial.println("COMPLETADA");
  //ESP.restart();
 
  }

void barreLetras(){
    // ----------------------------------------------------------------------
    // Rutina que mostra tots els caracters ASCii a la matriu secuencialment  
    //-----------------------------------------------------------------------
    figu++;
    memset(leds, 0, sizeof(leds));
    if (figu > 90) figu = 0;
    pintaFig(figu,0x440000,0,0); //units
    pintaFig(figu,0x003333,4,0); //decs
    FastLED.show();
  }



void montServer(){
  // ----------------------------------------------------------------------
  // Rutina que monta servidor WEB per poder rebre actulitzacions remotes  
  //-----------------------------------------------------------------------
  Serial.println("***-WEBSERVER-***");
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", "<table border='1' cellpadding='5' cellspacing='0'><td><font face='arial'color='darkblue'><b><i>ALENTIO</i></b> BASCULA &nbsp; &nbsp; &nbsp; &nbsp; Versi&oacute;: <b>"+versio+"</b></font></td></table><br>"+cap+"<br><form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
  });

  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.setDebugOutput(true);
      Serial.printf("Actualitzant: %s\n", upload.filename.c_str());
      
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Actualització complerta: %u\nRe-inicant...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
    yield();
  });

  server.begin();
 } 

void getSerial(){
    // ----------------------------------------------------------------------
    // Rutina que llegueix port de bàscula i pinta un led de colors si rep 
    //-----------------------------------------------------------------------
  for (tt=0;tt<400;tt++) {  // 2 segons = 400 * 5ms
    if (mySerial.available()) { // mira si recibe datos por el serie bascula y los pinta
      int dato = mySerial.read();
      if ( dato != dato2 ) {
        leds[63] = dato * 45000; // color del dato
      }
      dato2 = dato;
      }
      delay(5);
      if (tt % 10 == 0) FastLED.show();                       // pinta leds cada 50ms   
      if (tt % 20 == 0) server.handleClient();                // refresca clpasa cada 100ms
   }
 }


void gramZ3(){
    // ----------------------------------------------------------------------
    // Rutina que processa les dades serie de la báscula GRAM-Z3 
    //-----------------------------------------------------------------------
    if (mySerial.available()) {   
      //Serial.print ("Entra en Gram") ;                           // si el port es accesible fa tot un seguit de coses
      dato = mySerial.read(); 
      //Serial.println (dato) ;                                 // llegeix del port serie de la báscula 1 caracter
      
      if (dato == 3 ) {                                       // en el cas de la Z3 el 3 es el caracter de final de trama
        trama = trama + "03";                               // l'aqfegim a la trama (nomes per veure-la sencera)  
        tram = 0;
        

        if (trama2 != trama) { Serial.print(trama); Serial.print("     Fase: "); Serial.print(fase);Serial.print("    Peso: "); Serial.print(peso);Serial.print("  Tara: "); Serial.print(tara); Serial.print("  Estable: "); Serial.print(estable); Serial.print("  ");Serial.print ("  hexString: ");Serial.println (hexString);}
        CalculaTramaGram();
        ControlFases();
        trama2 =trama;
        trama = "";
        peso = "";
        

      } else {
        if (dato < 16) { zero = "0"; } else { zero = ""; }    // Si el caracter que ha rebut no es el de final de trama 
        trama = trama + zero +String(dato,HEX) +",";
        //Serial.print("Trama ") ;Serial.println (trama2) ;           // continua sumant per generar la trama complerta
        }
        
      if (dato != 0x20) {  // No te en compte els espais
        if (tram > 2 && tram < 13 ) peso = peso + char(dato);   // Si esta entre el caractee 2 y 12 son dades del pes
        //Serial.print ("Peso: ");Serial.println (peso) ;
        //Serial.print ("Tram: ");Serial.println (tram) ;
       }
      if (tram == 2 )                                         // Si es el segons caracter de trama son marques de bascula
        {
          tara = (0x2 & dato); if (tara == 2) tara = 1;       // mira si senyala el bit de Tara i activa la marca
          vcero = (0x8 & dato); if (vcero == 0x8) vcero = 1;  // mira si senyala el bit de pes zero i activa la marca
          estable = (0x40 & dato); if (estable == 0x40) estable = 1; //mira si senyala el bit de pes es estable i ho marca

          // hexString = (dato); 
          // //String hexString = "1A3F";
          
          // // unsigned int num = strtoul(hexString.c_str(), NULL, 16);
          // // printBinary(num);
          
          // taraGram = 1;       // mira si senyala el bit de Tara i activa la marca
          // estable = (0x40 & dato); if (estable == 0x40) estable = 1; //mira si senyala el bit de pes es estable i ho marca
          // // Serial.print ("Function GramZ3---TaraGram: ");Serial.print (taraGram);
          // // Serial.print ("  TaraTemp: ");Serial.print (taraTemp);
          // // Serial.print ("  Dato: ");Serial.print (dato);
          // // Serial.print ("  Estable: ");Serial.print (estable);
        }
      tram++;                                                 // incramenta el co,ptador de caracter de trama per saber per on va
      }   

      
      //pintaPeso();}                 // Si "peso" acaba en "g" vol dir que lectura bona i pinta display LEDs 
      //if ( tt %4 == 0) { pintaPeso(); }
 }


void wunder() {
  // ----------------------------------------------------------------------
  // Rutina que processa les dades serie de la báscula WUNDER JORDI
  //-----------------------------------------------------------------------

  bool wunderLeerTrama = false;
  if (mySerial.available()){
    //Serial.println("Serial available");
    dato = mySerial.read();  // read the received character
    if (dato == 0xA ) {
      trama = trama + ">";
      wunderLeerTrama = true;
      } 
    else {
      if (dato > 16) { trama = trama + (char)dato;}
      else { }
    } 
  }
    
    if (wunderLeerTrama){
      //Serial.println(trama);
      if (trama2 != trama) { Serial.print(trama); Serial.print("  Fase: "); Serial.print(fase);Serial.print("  Peso: "); Serial.print(peso2); Serial.println("  "); }
      //Serial.println(trama);

      CalculaTramaWunder();
      ControlFases();
      trama2 =trama;
      trama = "";
    }
         
 }


void ComprobarConfeccion(){
    // //ACTUALIZA AUTOMATICAMENTE LA CONFECCION DE LA BASE DE DATOS
    // PintaComprobacion();
    // envio =urlh+"?pue="+pue+"&ope="+String(ope)+"&idp="+String(idp)+"&est="+String(estadoEstable)+"&pes="+pesoEstable+"";
    // http.begin(envio);    
    // int httpResponseCode = http.GET();               // espera ka resposta del servidor
    //   if (httpResponseCode > 0) {                   // si la resposta es positiva  
    //     Serial.print("Comprobamos Confección: HTTP ");                        // treu el codi de resposta
    //     Serial.println(httpResponseCode);             // pel port de l'ordinador (per debug)
    //     String payload = http.getString();            // guarda la resposta a variable "payload"
    //     if (payload == "2")   {                       // si retorna  2es  que la ID de pes no es la actual
    //         Serial.print(payload);
    //         Serial.println(" ---- RECARREGA PESOS ----");         // treu pel port de l'ordinador (per debug)
    //         leepesoBBDD();                            // crida a la funció per reiniciar i carregar dades
    //       }
    //   }  else {                                      // SI negatiu es un error de connexió
    //   Serial.print("Codi de Error: ");             // treu el codi de error
    //   Serial.println(httpResponseCode);             // pel port de l'ordinador (per debug)
    //   Serial.println("Falla connexió :-(");          
    //   }    
    // http.end();      // Finalitza la conexió per alliberar recursos            

 }

void TestOffline(){
    // ----------------------------------------------------------------------
    // Routine that processes the data from text without online conection (Based on GramZ3)
    //-----------------------------------------------------------------------
    Serial.println("TestOffline");
     fase =2; estado =1; Nivel
     =0;
        for (int aa=0;aa<=28;aa++) {    //sizeof(tramaOFFLINE)
          trama = ""; //tramaOFFLINE[aa];
         

          if (trama2 != trama) { Serial.println(trama);}
          CalculaTramaWunder();
          ControlFases();
          trama2 =trama;
          delay (1000);
        }
        
        memset(leds, 0, sizeof(leds));
        delay (5000);
        FastLED.show();

 }

bool PesoEnRangoPosible (){
    bool pesoEnRangoPosible;
    if (peso2 >= (minim*0.8) && peso2 <= (maxim * 1.6)) {
      pesoEnRangoPosible = true;
    }else{
      pesoEnRangoPosible = false;
    }
    return pesoEnRangoPosible;
  }


void ControlFases(){
    switch (fase) 
      {
        case 0: //   FASE0 -  NO TARA 
          // statements
          CalculaTara();
          if ( tara == 1 ) { 
            fase = 1 ; 
            Serial.println("-----------Cambio a Fase 1 Y Comprobamos Confeccion"); 
            ComprobarConfeccion (); //COMPROBAMOS SI SE HA MODIFICADO LA CONFECCION} 
            } //PASAMOS A FASE 1
            else {PintaPantalla();}
          break;

        case 1: //  FASE1 - TARA HECHA - ESTAMOS EN ESPERA DE PESO
          // statements

          if ( peso2 > 0 ) {
             fase = 2 ;
             Serial.println("--------------Cambio a Fase 2");
              PintaPantalla();
              }
          else if ( peso2 < -1000){
            ComprobarConfeccion();
            fase = 0;
            //JORDI
            PintaPantalla();}
          else {
            PintaPantalla();}
          break;

        case 2://   FASE2 - TARA HECHA Y PESO DISTINTO DE CERO - ESTAMOS PESANDO
          // statements
          estado= CalculaEstado(peso2);
          CalculaNivel();
          if (estable==1 ) {
            if (PesoEnRangoPosible()) {
              pesoEstable = peso2;fase = 3; 
              estadoEstable = CalculaEstado(peso2);
              Serial.println("---------------Cambio a Fase 3"); 
              }
              else {
                if (peso2 ==0) {
                  fase = 1; 
                  estado= -1;
                  Serial.println("--------------Cambio a Fase 1");}
              }
            }else {  //NO ES ESTABLE
              // if (peso2 > (maxim+5000)){
              //   Serial.println("-----------Comprobamos Confeccion por sobrepeso"); 
              //   ComprobarConfeccion (); //COMPROBAMOS SI SE HA MODIFICADO LA CONFECCION} 
              // }
            }
          PintaPantalla();
          break;

        case 3: // FASE3  PESO ESTABLE Y DENTRO DE RANGOPOSIBLE - TENEMOS PESO ESTABBLE
          CalculaNivel();
          ultimoPesoInsta = peso2;



          if (estable==1 && PesoEnRangoPosible()) {
            pesoEstable = peso2;
            estadoEstable= CalculaEstado(peso2);
            }

          if (peso2 < 0.200){
            fase = 4; 
            Serial.println("----------------Cambio a Fase 4");
            } //PASAMOS A FASE 4 
            else{

                if (estable==1 && !PesoEnRangoPosible() && peso2 !=0) {
                  pesoEstable = 0; estadoEstable = 0;
                  estado= CalculaEstado(peso2);
                  fase = 2;
                  Serial.println("---------------Cambio a Fase 2");;
                  // Serial.print("Fase: ");
                  // Serial.println(fase);
                } //SI SE ESTABILIZA EL PESO FUERA DE RANGOPOSIBLE VUELVE A LA FASE 2
            }


          PintaPantalla();
          break;
          
        case 4:
          // statements
          //int pesoEnvio =  pesoEstable.toInt;///JORDI
          //Serial.print("xxxxxxxxxxxxAAxxxxxxxxxxxx Envioprevio "); Serial.println(envio);
          
          envio =urlh+"?pue="+pue+"&ope="+String(ope)+"&idp="+String(idp)+"&est="+String(estadoEstable)+"&pes="+pesoEstable+"";
          Serial.print("xxxxxxxxxxxxxxxxxxxxxxxx Envio: "); Serial.println(envio);
          Serial.print("xxxxxxxxxxxxxxxxxxxxxxxx PesoEstable:  "); Serial.println(pesoEstable);
          Serial.print("xxxxxxxxxxxxxxxxxxxxxxxx EstadoEstable:  "); Serial.println(estadoEstable);
          http.begin(envio);

          int httpResponseCode = http.GET();               // espera ka resposta del servidor
          if (httpResponseCode > 0) {                   // si la resposta es positiva  
            Serial.print("HTTP ");                        // treu el codi de resposta
            //Serial.println(httpResponseCode);             // pel port de l'ordinador (per debug)
            String payload = http.getString();            // guarda la resposta a variable "payload"
            if (payload == "2")   {                       // si retorna  2es  que la ID de pes no es la actual
                Serial.print(payload);
                Serial.print(" RECARREGA PESOS");         // treu pel port de l'ordinador (per debug)
                leepesoBBDD();                            // crida a la funció per reiniciar i carregar dades
              }
            } else {                                      // SI negatiu es un error de connexió
            Serial.print("Codi de Error: ");             // treu el codi de error
            Serial.println(httpResponseCode);             // pel port de l'ordinador (per debug)
            Serial.println("Falla connexió :-(");          
          }    
          http.end();      // Finalitza la conexió per alliberar recursos     


          pesoEstable = 0; 
          estadoEstable = 0;
          tara = 0; //PONEMOS TARA A 0
          fase = 0; 
          Serial.println("----------------Cambio a Fase 0");
          //PASAMOS A FASE 0hjjmc
          PintaPantalla();
          break;
          
      }
  }


bool CheckForDelivery(){//Deliver = Empujar la caja
    bool isDelivered = false;
    if (peso2 < 0.200){isDelivered = true; Serial.println("ENVIADO: PESO POR DEBAJO DE 200 GRAMOS");}
    // float saltoGramos = peso2 -ultimoPesoInsta;
    // float saltoPorCiento = saltoGramos / minim;

    return isDelivered;
  }


void CalculaTramaWunder(){
    //CALCULAR VALORES DE LA TRAMA WUNDER
    // ----------------------------------------------------------------------
    if  (trama.length() > 15) { // ST,GS,+000.320kg0
      if (trama.indexOf("kg") != -1){
        //Si la trama contiene un "+" el valor de peso es positivo
        if(trama.indexOf("+") != -1){
          peso = trama.substring(trama.indexOf("+") + 1, trama.indexOf("kg") + 2); // Extrae desde el carácter después de '+' hasta "kg"
          peso.replace(".","");
          peso.replace("k",""); //k
          peso2 = peso.toInt();
        }

        //Si la trama contiene un "-" el valor de peso es negativo
        else if(trama.indexOf("-") != -1){
          peso = trama.substring(trama.indexOf("-") + 1, trama.indexOf("kg") + 2); // Extrae desde el carácter después de '+' hasta "kg"
          peso.replace(".","");
          peso.replace("k",""); //k
          peso2 = -peso.toInt();
        }else{/*MostrarErrorDeLecturaDeTrama();*/}

        //Si la trama contiene "ST" el peso es estable, si no es inestable
        if (trama.indexOf("ST") != -1){ 
          estable = 1 ;
        }  else {estable = 0;}
      } 
      else 
      { 
        peso ="";/*MostrarErrorDeLecturaDeTrama();*/
      }
    // Serial.println("Funcion CalculaTramaWunder");
    // Serial.print("Peso: "); Serial.println(peso);
    // Serial.print("Peso2: "); Serial.println(peso2);
    }

    
  }

void CalculaTramaGram(){
    //CALCULAR VALORES DE LA TRAMA WUNDER
    // ----------------------------------------------------------------------
    // Serial.println("Funcion CalculaTramaGram: Inicio");
    // Serial.print("Peso: "); Serial.println(peso);
    // Serial.print("Peso2: "); Serial.println(peso2);

    peso.replace(".","");
    peso.replace("g","");
    peso.replace("k","");
    peso.replace("K","");
    peso2 = peso.toInt();
    if (trama.indexOf("-0") == -1){peso.replace("-","");}
    
      // if (trama.indexOf("g") != -1){
      //   //Si la trama contiene un "+" el valor de peso es positivo
      //   if(trama.indexOf("+") != -1){
      //     peso = trama.substring(trama.indexOf("+") + 1, trama.indexOf("g") + 2); // Extrae desde el carácter después de '+' hasta "kg"
      //     peso.replace(".","");
      //     peso2 = peso.toInt();
      //   }

      //   //Si la trama contiene un "-" el valor de peso es negativo
      //   else if(trama.indexOf("-") != -1){
      //     peso = trama.substring(trama.indexOf("-") + 1, trama.indexOf("g") + 2); // Extrae desde el carácter después de '+' hasta "kg"
      //     peso.replace(".","");
      //     peso2 = -peso.toInt();
      //   }else{/*MostrarErrorDeLecturaDeTrama();*/}

      // } 
      // else 
      // { 
      //   peso ="";/*MostrarErrorDeLecturaDeTrama();*/
      // }

      
    // Serial.println("Funcion CalculaTramaGram Final");
    // Serial.print("Peso: "); Serial.println(peso);
    // Serial.print("Peso2: "); Serial.println(peso2);
    
  }

void CalculaTara(){
    // ----------------------------------------------------------------------
    //CALCULAR TARA 
    // ----------------------------------------------------------------------

    if ( tip == 1 ) { 
      if ( peso2 == 0 && estable == 1 ) { tara = 1 ;}
      else {tara =0;}
    }

    if ( tip == 2 ) { 
      if ( peso2 == 0 && estable == 1 ) { tara = 1 ;}
      else {tara =0;}
    }

    //if (tara == 0) {Serial.print(" SIN TARA");}
  }
int CalculaEstado(int pesoTemp){
    int respuesta;
    // ----------------------------------------------------------------------
    //CALCULAR ESTADO
    // ----------------------------------------------------------------------
    if ( pesoTemp >= minim && pesoTemp <= maxim ) { respuesta = 0 ;} //VERDE
    if ( pesoTemp > maxim ) { respuesta = 1 ;} //AMARILLO 
    if ( pesoTemp < minim ) { respuesta = -1  ;} //ROJO

    return respuesta;
    // ----------------------------------------------------------------------
    //CALCULAR DIF
    // ----------------------------------------------------------------------
  }
void CalculaNivel(){
    float pesoOctavo = (float)pesok / 8 ;
    float numOctavos = (float)peso2 / (float)pesoOctavo;    
    if (numOctavos > 8.0 ) {numOctavos = 8.0; }
    if (numOctavos < 0.0 ) {numOctavos = 0.0; }
    Nivel= round(numOctavos);//COGEMOS EL NUMERO DE OCTAVOS 
 }

void PintaComprobacion(){
    memset(leds, 0, sizeof(leds));
    pintaFig('_',colorMagentaIntenso,2,1);
    FastLED.show();
  }

void PintaError(){
    memset(leds, 0, sizeof(leds));
    pintaFig('E',colorAzulIntenso,2,1);
    delay (500);
    FastLED.show();
  }

void PintaPantalla(){
  //Serial.printlnPantalla"); 
  if(fase == 0){ //  FASE 0   --------------------------------------- PINTA T
    memset(leds, 0, sizeof(leds));
    pintaFig('$',colorAmarilloIntenso,0,1);
  } 
  if(fase == 1){  //  FASE 1   --------------------------------------- NO PINTA NADA
    memset(leds, 0, sizeof(leds));
    pintaFig('_',colorAzulClaro,2,1);
  }
  if(fase == 2) {  //  FASE 2   ---------------------------------------PINTA RESULTAT ENVIAMENT  (VERMELL, GROC O VERD) PUEDE SER OTRA INTENSIDAD QUE LA FASE 3
    memset(leds, 0, sizeof(leds));
    //Serial.println ("-------------------Pinta Fase 2");
    if(estado == 0) { colorLED = colorVerdeClaro ;} //VERDE
    else if(estado == 1) { colorLED = colorNaranjaClaro ;} //AMARILLO
    else if(estado == -1) { colorLED = colorRojoClaro ;} //ROJO
    else {colorLED = colorAzulClaro ;} //AZUL en caso de que el estado no sea ni 0 ni 1 ni -1

    for (int ll=0;ll<Nivel
    *8;ll++) { leds[ll]=colorLED; }
  }
  if(fase == 3){ //  FASE 3   ---------------------------------------PINTA RESULTAT ENVIAMENT  (VERMELL, GROC O VERD) PUEDE SER OTRA INTENSIDAD QUE LA FASE 2
    //Peso estable y dentro de rango
    memset(leds, 0, sizeof(leds));
    //Serial.println ("-------------------Pinta Fase 3");
    if(estadoEstable == 0) { colorLEDESTABLE = colorVerdeIntenso ;} //VERDE
    else if(estadoEstable == 1) { colorLEDESTABLE = colorNaranjaIntenso ; } //AMARILLO
    else if(estadoEstable == -1) { colorLEDESTABLE = colorRojoIntenso ; } //ROJO
    else {colorLEDESTABLE = colorAzulClaro ;} //AZUL en caso de que el estadoEstable no sea ni 0 ni 1 ni -1


    for (int ll=0;ll<Nivel
    *8;ll++) { leds[ll]=colorLEDESTABLE; }

    // leds[0]=colorLED;
    // leds[1]=colorLED;
    // leds[2]=colorLED;
    // leds[8]=colorLED;
    // leds[9]=colorLED;
    // leds[10]=colorLED;
    // leds[16]=colorLED;
    // leds[17]=colorLED;
    // leds[18]=colorLED;
  }
  if(fase == 4){//  FASE 4   ---------------------------------------PINTA RESULTAT ENVIAMENT  (VERMELL, GROC O VERD)
    //Hemos empujado la caja y enviado el peso
    memset(leds, 0, sizeof(leds));                  
    //Serial.println ("-------------------Pinta Fase 4");
    if ( estadoEstable == 0  ) { pintaFig('&',colorVerdeIntenso,0,1); } 
    if ( estadoEstable == 1  ) { pintaFig('%',colorNaranjaIntenso,0,1); } 
    if ( estadoEstable == -1 ) { pintaFig('*',colorRojoIntenso,0,1); }
    FastLED.show();
    delay(2000);
  }


    FastLED.show();
     
  }
 
