#include <WiFi.h>

const char *ssid = "yourssidherechangeit"; // change this to the SSID of your network
const char *password = "word"; // and change this to the password of your network
bool pinState = false;

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <style>
    body,
    html {
      margin: 0;
      padding: 0;
      height: 100%;
      width: 100%;
      display: flex;
      align-items: center;
      justify-content: center;
      background-color: #1e1e1e;
      transition: background-color 0.5s ease-in-out;
      font-family: sans-serif;
    }

    .container {
      display: flex;
      justify-content: center;
      align-items: center;
      width: 100%;
    }

    .switch {
      font-size: 3vw;
      position: relative;
      display: inline-block;
      width: 8em;
      height: 4em;
      user-select: none;
    }

    .switch .cb {
      opacity: 0;
      width: 0;
      height: 0;
    }

    .toggle {
      position: absolute;
      cursor: pointer;
      width: 100%;
      height: 100%;
      background-color: #373737;
      border-radius: 0.2em;
      transition: 0.4s;
      text-transform: uppercase;
      font-weight: 700;
      overflow: hidden;
      box-shadow:
        -0.3em 0 0 0 #373737,
        -0.3em 0.3em 0 0 #373737,
        0.3em 0 0 0 #373737,
        0.3em 0.3em 0 0 #373737,
        0 0.3em 0 0 #373737;
    }

    .toggle>.left,
    .toggle>.right {
      position: absolute;
      display: flex;
      width: 50%;
      height: 88%;
      background-color: #f3f3f3;
      align-items: center;
      justify-content: center;
      transform-origin: right;
      transform-style: preserve-3d;
      transition: all 150ms;
      font-size: 0.5em;
    }

    .toggle>.left {
      color: #373737;
      left: 0;
      bottom: 0;
      transform: rotateX(10deg);
    }

    .left::before,
    .right::before,
    .left::after,
    .right::after {
      position: absolute;
      content: "";
      width: 100%;
      height: 100%;
    }

    .left::before {
      background-color: rgb(206, 206, 206);
      transform-origin: center left;
      transform: rotateY(90deg);
    }

    .left::after {
      background-color: rgb(112, 112, 112);
      transform-origin: center bottom;
      transform: rotateX(90deg);
    }

    .toggle>.right {
      color: rgb(206, 206, 206);
      right: 0;
      bottom: 0;
      transform: rotateX(10deg) rotateY(-45deg);
      transform-origin: left;
    }

    .right::before {
      background-color: rgb(206, 206, 206);
      transform-origin: center right;
      transform: rotateY(-90deg);
    }

    .right::after {
      background-color: rgb(112, 112, 112);
      transform-origin: center bottom;
      transform: rotateX(90deg);
    }

    .switch input:checked+.toggle>.left {
      transform: rotateX(10deg) rotateY(45deg);
      color: rgb(206, 206, 206);
    }

    .switch input:checked+.toggle>.right {
      transform: rotateX(10deg) rotateY(0deg);
      color: #487bdb;
    }

    .left,
    .right {
      user-select: none;
    }
    body.true-bg {
    background-color: #fff3b0; 
    }
  </style>
</head>
<body>
  <label class="switch">
    <input class="cb" type="checkbox" />
    <span class="toggle">
      <span class="left">off</span>
      <span class="right">on</span> 
    </span>
  </label>
  <script>

  const bodyElement = document.body;
  document.querySelector('.cb').addEventListener('change', function() {
    if (this.checked) {
      fetch('/H');
      bodyElement.classList.add('true-bg');  
    } else {
      fetch('/L');
      bodyElement.classList.remove('true-bg');
    }
  });

  
  window.onload = function() {

    fetch('/state')  
      .then(response => response.text())
      .then(data => {
        const switchElement = document.querySelector('.cb');
        if (data === 'ON') {
          switchElement.checked = true;
          bodyElement.classList.add('true-bg');
        } else {
          switchElement.checked = false;
          bodyElement.classList.remove('true-bg');
        }
      });
  }
</script>
</body>
</html>
)rawliteral";



NetworkServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(21, OUTPUT); 
  //pinMode(5, OUTPUT);
  delay(10);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  server.begin();
}

void loop() {
  NetworkClient client = server.accept();  
  //digitalWrite(5, HIGH);

  if (client) {                     
    Serial.println("New Client."); 
    String currentLine = "";       
    while (client.connected()) {    
      if (client.available()) {     
        char c = client.read();     
        Serial.write(c);            
        if (c == '\n') {            

          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print(htmlPage);
            client.println();

          
            break;
          } else {  
            currentLine = "";
          }
        } else if (c != '\r') { 
          currentLine += c;      
        }

        if (currentLine.endsWith("GET /H")) {
          digitalWrite(21, HIGH);
          pinState = true;
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(21, LOW);  
          pinState = false;
        }
        if (currentLine.endsWith("GET /state")) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/plain");
          client.println();
          client.println(pinState ? "ON" : "OFF");  
        }
      }
    }
    client.stop();
  }
}
