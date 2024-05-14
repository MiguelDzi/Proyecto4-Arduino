#include <SPI.h> // Incluye la librería SPI para la comunicación SPI
#include <Ethernet.h> // Incluye la librería Ethernet para la comunicación Ethernet
#include <SD.h> // Incluye la librería SD para interactuar con la tarjeta SD

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Dirección MAC para la conexión Ethernet
byte ip[] = { 192, 168, 1, 70 }; // Dirección IP estática para el dispositivo
EthernetServer server(80); // Crea un servidor Ethernet en el puerto 80

char readString[100]; // Arreglo de caracteres para almacenar los datos recibidos
int ledRed = 6; // Pin digital para el LED rojo
int ledGreen = 2; // Pin digital para el LED verde
char Analog[4]; // Arreglo de caracteres para almacenar el valor analógico del LED rojo
char leitura[5]; // Arreglo de caracteres para almacenar el valor del potenciómetro

File dataFile; // Variable para manejar el archivo en la tarjeta SD

void setup() {
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.begin(9600);

  // Inicializa la comunicación con la tarjeta SD
  if (!SD.begin(4)) { // El pin 4 se usa para la comunicación SPI con la tarjeta SD
    Serial.println("Error al inicializar la tarjeta SD!");
    return;
  }

  // Abre el archivo de datos en modo escritura
  dataFile = SD.open("data.txt", FILE_WRITE);
  if (!dataFile) {
    Serial.println("Error al abrir el archivo de datos!");
    return;
  }
}

void loop() {
  EthernetClient client = server.available(); // Espera a que haya un cliente disponible para atender
  if (client) { // Verifica si hay un cliente conectado
    while (client.connected()) { // Procesa la solicitud del cliente mientras esté conectado
      if (client.available()) { // Verifica si hay datos disponibles para leer del cliente
        char c = client.read(); // Lee un carácter del cliente

        if (strlen(readString) < 100) { // Verifica si el tamaño de la cadena recibida es menor a 100 caracteres
          strncat(readString, &c, 1); // Concatena el carácter leído a la cadena "readString"
        }

        if (c == '\n' ) { // Verifica si se ha recibido un salto de línea, indicando el final de la solicitud del cliente
          guardarDatosEnSD(); // Guarda los datos en la tarjeta SD
          ledVerde(); // Controla el LED verde
          lerPot(); // Lee el valor del potenciómetro
          ledVermelho(); // Controla el LED rojo

          client.println(F("HTTP/1.1 200 OK"));
          client.println(F("Content-Type: text/html"));
          client.println();

          client.println(F("<!DOCTYPE html>"));
          client.println(F("<head><meta charset=utf-8><title>Arduino / HTML</title>"));
          client.println(F("<style>"));
          client.println(F("body { font-family: montserrat, arial, verdana; height: 1000%; background: linear-gradient(rgba(196, 102, 0, 0.6), rgba(155, 89, 182, 0.6)); display: flex; justify-content: center; align-items: center; }"));
          client.println(F("#msform { width: 350px; text-align: center; position: relative; }"));
          client.println(F("#msform fieldset { background: white; border: 0 none; border-radius: 3px; box-shadow: 0 0 15px 1px rgba(0, 0, 0, 0.4); padding: 20px 30px; box-sizing: border-box; width: 80%; margin: 0 auto; position: relative; }"));
          client.println(F("#msform input, #msform textarea { padding: 15px; border: 1px solid #ccc; border-radius: 3px; margin-bottom: 10px; width: calc(100% - 30px); box-sizing: border-box; font-family: montserrat; color: #2C3E50; font-size: 13px; }"));
          client.println(F("#msform .action-button { width: 100px; background: #27AE60; font-weight: bold; color: white; border: 0 none; border-radius: 1px; cursor: pointer; padding: 10px; margin: 10px 5px; text-decoration: none; font-size: 14px; }"));
          client.println(F("#msform .action-button:hover, #msform .action-button:focus { box-shadow: 0 0 0 2px white, 0 0 0 3px #27AE60; }"));
          client.println(F(".fs-title { font-size: 15px; text-transform: uppercase; color: #2C3E50; margin-bottom: 10px; }"));
          client.println(F(".fs-subtitle { font-weight: normal; font-size: 13px; color: #666; margin-bottom: 20px; }"));
          client.println(F("#led-form { display: flex; align-items: center; justify-content: space-between; flex-direction: column; }"));
          client.println(F("#led-form h3 { margin-top: 0; }")); // Estilo para la palabra "LED ROJO"
          client.println(F("#led-form form { display: flex; flex-direction: column; align-items: center; }")); // Alineación vertical de los elementos del formulario LED ROJO
          client.println(F("#led-form form h3 { text-align: center; }")); // Centrar la palabra "LED ROJO"
          client.println(F("#led-form form div { display: flex; align-items: center; }")); // Estilo para alinear "Encender" y "Apagar" con el botón "Enviar"
          client.println(F("#led-form form div input[type='radio'] { margin-right: 5px; }")); // Espacio entre los botones "Encender" y "Apagar"
          client.println(F("</style>"));
          client.println(F("</head>"));
          client.println(F("<body>")); // Centra verticalmente y limita al tamaño de la pantalla
          client.println(F("<div id='msform'>")); // Crea un contenedor principal y centra verticalmente
          client.println(F("<h2 style='color:red; margin-bottom: 3px;'>CONTROLADOR DE ARDUINO</h2>")); // Ajusta el margen inferior para separar del siguiente elemento

          client.println(F("<div style='margin-top: 30px;'>")); // Abre un div adicional para contener los elementos dentro del borde
          client.println(F("<form>")); // Centra los elementos del formulario verticalmente
          client.println(F("<h3 class='fs-title'>SALIDA</h3>")); // Elimina el margen izquierdo y ajusta el texto centrado
          client.println(F("<h4 style='color:blue;'>LED AZUL</h4>Valor Analógico (entre 0 e 255):"));
          client.println(F("<input type='number' id='v' name='v' min='0' max='255' style='width: 80px;'>")); // Ajusta el margen inferior para separar del botón
          client.println(F("<input class='action-button' type='submit' value='Enviar'>"));
          client.println(F("</form></div>")); // Cierra el div del formulario y el div contenedor

          client.println(F("<div style='margin-top: 30px;'>"));
          client.println(F("<form id='led-form' style='text-align: center;'>")); // Establece el formulario centrado
          client.println(F("<h3 class='fs-title' style='color:red;'>LED ROJO</h3>"));
          client.println(F("<div>")); // Abrimos un contenedor div para los botones de Encender y Apagar
          client.println(F("<div style='display: inline-block; margin-right: 10px;'><input type='radio' id='Encender' name='LED' value='Encender'><label for='Encender'>Encender</label></div>")); // Botón de Encender en línea
          client.println(F("<div style='display: inline-block;'><input type='radio' id='Apagar' name='LED' value='Apagar'><label for='Apagar'>Apagar</label></div>")); // Botón de Apagar en línea
          client.println(F("</div>")); // Cerramos el contenedor div para los botones de Encender y Apagar
          client.println(F("<br>")); // Salto de línea
          client.println(F("<input class='action-button' type='submit' value='Enviar' style='margin-top: 10px;'>")); // Botón "Enviar" con margen superior
          client.println(F("</form></div>"));


          client.println(F("<div style='margin-top: 30px;'>")); // Abre un div adicional para contener los elementos dentro del borde
          client.println(F("<form>")); // Centra los elementos del formulario verticalmente
          client.println(F("<h3 class='fs-title'>ENTRADAS</h3>")); // Ajusta el margen izquierdo y ajusta el texto centrado
          client.print(F("Valor do Potenciômetro : "));
          client.println(leitura);
          client.println(F("<br><br>F5/Update -> Para actualizarlo:")); // Aquí incluimos la oración antes del botón Update
          client.println(F("<input class='action-button' type='submit' value='Update'>")); // El botón Update ahora está en una línea separada
          client.println(F("</form><br></div>")); // Cerramos el formulario y añadimos un salto de línea antes de cerrar el div contenedor



          client.println(F("</div>")); // Cierra el contenedor principal
          client.println(F("</body>"));
          client.println(F("</html>"));

          delay(1); // Pequeña pausa
          client.stop(); // Cierra la conexión con el cliente
          readString[0] = '\0'; // Reinicia la cadena "readString"
        }
      }
    }
  }
}

void guardarDatosEnSD() {
  // Abre el archivo "data.txt" en modo escritura y maneja los errores
  dataFile = SD.open("data.txt", FILE_WRITE);
  if (dataFile) {
    if (strstr(readString, "?v=") != NULL) { // Verifica si se recibió un valor analógico para el LED azul
      char* firstEqual = strchr(readString, '=');
      if (firstEqual != NULL) {
        char Analog[4];
        strncpy(Analog, firstEqual + 1, 3); // Extrae el valor analógico del LED azul de la cadena recibida
        Analog[3] = '\0'; // Agrega el carácter nulo al final de la cadena
        int AnalogValue = atoi(Analog); // Convierte la cadena a un entero
        // Guarda el valor analógico del LED azul en el archivo
        dataFile.print(F("Led azul = Valor analógico puesto = "));
        dataFile.println(AnalogValue);
      }
    }

    // Verifica si se recibió una solicitud para encender o apagar el LED rojo
    if (strstr(readString, "LED=Encender") != NULL) {
      dataFile.println(F("Led rojo = Encendido")); // Guarda el estado del LED rojo en el archivo
    } else if (strstr(readString, "LED=Apagar") != NULL) {
      dataFile.println(F("Led rojo = Apagado")); // Guarda el estado del LED rojo en el archivo
    }

    // Guarda el valor del potenciómetro en el archivo
    dataFile.print(F("Valor del potenciómetro = "));
    dataFile.println(leitura);

    dataFile.close(); // Cierra el archivo
  } else {
    Serial.println(F("Error al abrir el archivo data.txt")); // Imprime un mensaje de error
  }
}

void ledVerde() {
  if (strstr(readString, "LED=Encender") != NULL) digitalWrite(ledGreen, HIGH); // Enciende el LED verde si se recibió la solicitud
  if (strstr(readString, "LED=Apagar") != NULL) digitalWrite(ledGreen, LOW); // Apaga el LED verde si se recibió la solicitud
}

void lerPot() {
  int sensorReading = analogRead(0); // Lee el valor del potenciómetro
  itoa(sensorReading, leitura, 10); // Convierte el valor a una cadena y lo guarda en "leitura"
}

void ledVermelho() {
  if (strstr(readString, "?v=") != NULL) { // Verifica si se recibió un valor analógico para el LED rojo
    char* firstEqual = strchr(readString, '=');
    if (firstEqual != NULL) {
      strncpy(Analog, firstEqual + 1, 3); // Extrae el valor analógico del LED rojo de la cadena recibida
      Analog[3] = '\0'; // Agrega el carácter nulo al final de la cadena
      int AnalogValue = atoi(Analog); // Convierte la cadena a un entero
      analogWrite(ledRed, AnalogValue); // Asigna el valor analógico al LED rojo
    }
  }
}
