#ifndef SERVIDOR_H
	#define SERVIDOR_H
    #include <WiFi.h>
    #include <AsyncTCP.h>
    #include <ESPAsyncWebServer.h>
    
    
    #define DEBUGSERVIDOR


    const char* PARAM_INPUT_1 = "output";
    const char* PARAM_INPUT_2 = "state";

    const int Rele0 = 26;
    const int Rele1 = 27;
    const int Rele2 = 25;


    const char index_html[] PROGMEM = R"rawliteral(
        <!DOCTYPE HTML><html>
        <head>
          <title>ESP Web Server</title>
          <meta name="viewport" content="width=device-width, initial-scale=1">
          <link rel="icon" href="data:,">
          <style>
            html {font-family: Arial; display: inline-block; text-align: center;}
            h2 {font-size: 3.0rem;}
            p {font-size: 3.0rem;}
            body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
            .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
            .switch input {display: none}
            .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
            .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
            input:checked+.slider {background-color: #b30000}
            input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
          </style>
        </head>
        <body>
          <h2>Campanario</h2>
          %BUTTONPLACEHOLDER%
        <script>function toggleCheckbox(element) {
          var xhr = new XMLHttpRequest();
          if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
          else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
          xhr.send();
        }
        </script>
        </body>
        </html>
        )rawliteral";

    AsyncWebServer server(80);
    WiFiClient client;


    void ServidorOn(void);
    String ProcesaBotones(const String& var);
    String EstadoSalida(int output);

    /**
     * @brief Devuelve el estado de un pin de salida en formato de texto.
     * 
     * Esta función verifica el estado de un pin de salida digital especificado
     * y retorna "checked" si el pin está en estado alto (HIGH), o una cadena vacía ("")
     * si el pin está en estado bajo (LOW).
     * 
     * @param output El número del pin de salida digital que se desea verificar.
     * @return String "checked" si el pin está en estado alto, o una cadena vacía ("") si está en estado bajo.
     */
    String EstadoSalida(int output)
    {
        if(digitalRead(output)){
          return "checked";
        }
        else {
          return "";
        }
      }

    /**
     * @brief Genera un conjunto de botones HTML dinámicos basados en el estado de los relés.
     * 
     * Esta función procesa una variable de entrada y, si coincide con "BUTTONPLACEHOLDER",
     * genera un conjunto de botones HTML con interruptores (switches) que representan 
     * el estado de tres campanas (Campana 1, Campana 2, Campana 3). Cada botón incluye 
     * un evento `onchange` que llama a la función JavaScript `toggleCheckbox` y utiliza 
     * el estado actual de los relés (Rele0, Rele1, Rele2) para determinar si el interruptor 
     * debe estar activado o desactivado.
     * 
     * @param var Una referencia constante a un objeto String que contiene la variable a procesar.
     * @return Un objeto String que contiene el código HTML generado si `var` es "BUTTONPLACEHOLDER".
     *         Si no, devuelve una cadena vacía.
     */
    String ProcesaBotones(const String& var)
    {
        if(var == "BUTTONPLACEHOLDER"){
          String buttons = "";
          buttons += "<h4>Campana 1</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"26\" " + EstadoSalida(Rele0) + "><span class=\"slider\"></span></label>";
          buttons += "<h4>Campana 2</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"27\" " + EstadoSalida(Rele1) + "><span class=\"slider\"></span></label>";
          buttons += "<h4>Campana 3</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"25\" " + EstadoSalida(Rele2) + "><span class=\"slider\"></span></label>";
          return buttons;
        }
        return String();
    }
    void ServidorOn(void)
    {
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send_P(200, "text/html", index_html, ProcesaBotones);
        });
        server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
            String Mensaje1;
            String Mensaje2;
            // GET input1 value on <ESP_IP>/update?output=<Mensaje1>&state=<Mensaje2>
            if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
              Mensaje1 = request->getParam(PARAM_INPUT_1)->value();
              Mensaje2 = request->getParam(PARAM_INPUT_2)->value();
              digitalWrite(Mensaje1.toInt(), Mensaje2.toInt());
            }
            else {
              Mensaje1 = "No message sent";
              Mensaje2 = "No message sent";
            }
            Serial.print("GPIO: ");
            Serial.print(Mensaje1);
            Serial.print(" - Set to: ");
            Serial.println(Mensaje2);
            request->send(200, "text/plain", "OK");
          });
        // Iniciar el servidor
        server.begin();
        #ifdef DEBUGSERVIDOR
            Serial.println("Servidor HTTP iniciado en el puerto 80.");
            Serial.println("Servidor HTTP iniciado. Esperando conexiones...");
        #endif
    }
    
     
   

#endif