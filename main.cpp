#include <SPI.h>
#include <MFRC522.h>
#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>
#include <TouchScreen.h>

MCUFRIEND_kbv tft;
// Configuracion de lectores
#define SS_1  53
#define RST_1 31
MFRC522 rfid1(SS_1, RST_1);
#define SS_2  33
#define RST_2 35
MFRC522 rfid2(SS_2, RST_2);

// Definición de colores 
#define BLACK       0x0000
#define BLUE        0x001F
#define RED         0xF800
#define GREEN       0x07E0
#define YELLOW      0xFFE0
#define WHITE       0xFFFF
#define DARKGRAY    0x31E6
#define LIGHTGRAY   0xC618
#define GRAY        0x8410

// Pines touch 
#define XP 6
#define XM A2
#define YP A1
#define YM 7

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define MINPRESSURE 10
#define MAXPRESSURE 1000

struct Animal
{
  String uidStr;      
  int corralID;       
  String nombre;      
  bool registrado;    
  int estadoColor;    // 1 = Verde  2 = Rojo
  int drawX, drawY;   
};

Animal listaAnimales[20];
int totalAnimales = 0;

int activeFolder = 1;         // Solo corral1 o corral2 activos
bool modoTeclado = false;     
bool regSeleccionandoCirculo = false; 
int animalAElegir = -1;       
String inputNombre = "";      

void setup() 
{
  SPI.begin();
  rfid1.PCD_Init();
  rfid2.PCD_Init();

  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(1); // 480x320
  tft.fillScreen(WHITE);

  drawUI();
}

void loop() 
{
  // Detección de toques
  TSPoint p = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) 
  {
    int x = map(p.y, 958, 168, 0, 480);
    int y = map(p.x, 172, 930, 0, 320);
    x = constrain(x, 0, 479);
    y = constrain(y, 0, 319);

    manejarToqueTactil(x, y); 
  }

  // Lectura del lector rfid1
  if (rfid1.PICC_IsNewCardPresent() && rfid1.PICC_ReadCardSerial()) 
  {
    String currentUID = obtenerUID(rfid1);
    procesarTarjetaRFID(currentUID, 1);
    rfid1.PICC_HaltA();
    rfid1.PCD_StopCrypto1();
  }

  // Lectura del lector rfid2
  if (rfid2.PICC_IsNewCardPresent() && rfid2.PICC_ReadCardSerial()) {
    String currentUID = obtenerUID(rfid2);
    procesarTarjetaRFID(currentUID, 2);
    rfid2.PICC_HaltA();
    rfid2.PCD_StopCrypto1();
  }
}

String obtenerUID(MFRC522 &lector) {
  String uidStr = "";
  for (byte i = 0; i < lector.uid.size; i++) {
    if (lector.uid.uidByte[i] < 0x10) uidStr += "0";
    uidStr += String(lector.uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();
  return uidStr;
}

//interfaz grafica
void drawUI() {
  tft.fillScreen(WHITE);
  drawHeader();
  drawFolders();
  
  if (modoTeclado) {
    drawTecladoNumerico();
  } else {
    drawCorralContent();
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 480, 40, LIGHTGRAY);
  tft.drawRect(0, 0, 480, 40, BLACK);
  
  tft.setTextSize(2);
  tft.setTextColor(BLACK);
  
  tft.fillRect(20, 8, 120, 32, WHITE);
  tft.drawRect(20, 8, 120, 32, BLACK);
  tft.setCursor(35, 16);
  tft.println("VACA " + String(activeFolder));

  uint16_t colorRegBtn = regSeleccionandoCirculo ? GREEN : LIGHTGRAY;
  tft.fillRect(360, 8, 100, 32, colorRegBtn);
  tft.drawRect(360, 8, 100, 32, BLACK);
  tft.setTextColor(BLACK);
  tft.setCursor(390, 16);
  tft.println("Reg");
}

void drawFolders() {
  tft.fillRect(0, 40, 140, 280, DARKGRAY);
  tft.drawFastVLine(140, 40, 280, BLACK);

  // carpeta1
  tft.fillRect(10, 60, 120, 75, (activeFolder == 1 && !modoTeclado) ? WHITE : LIGHTGRAY);
  tft.drawRect(10, 60, 120, 75, BLACK);
  tft.setTextColor(BLACK);
  tft.setTextSize(1);
  tft.setCursor(50, 95); tft.println("Corral 1");

  // carpeta2
  tft.fillRect(10, 155, 120, 75, (activeFolder == 2 && !modoTeclado) ? WHITE : LIGHTGRAY);
  tft.drawRect(10, 155, 120, 75, BLACK);
  tft.setCursor(50, 190); tft.println("Corral 2");
}

void drawCorralContent() {
  tft.fillRect(141, 41, 339, 279, WHITE);

  int xOffset = 180;
  int yOffset = 90;
  int count = 0;

  for (int i = 0; i < totalAnimales; i++) {
    if (listaAnimales[i].corralID == activeFolder) {
      listaAnimales[i].drawX = xOffset;
      listaAnimales[i].drawY = yOffset;

      uint16_t colorBorde = GRAY;
      if (listaAnimales[i].registrado) {
        colorBorde = (listaAnimales[i].estadoColor == 1) ? GREEN : RED;
      }

      tft.fillCircle(xOffset, yOffset, 30, colorBorde);
      tft.drawCircle(xOffset, yOffset, 30, colorBorde);
      tft.drawCircle(xOffset, yOffset, 25, colorBorde);

      tft.setTextSize(1);
      tft.setTextColor(BLACK);
      tft.setCursor(xOffset - 12, yOffset - 4);
      tft.println(listaAnimales[i].registrado ? listaAnimales[i].nombre : "????");

      xOffset += 90;
      count++;
      if (count % 3 == 0) {
        xOffset = 180;
        yOffset += 80;
      }
    }
  }

  if (regSeleccionandoCirculo) {
    tft.setTextSize(1);
    tft.setTextColor(RED);
    tft.setCursor(150, 48);
    tft.println("Toca un circulo gris (????) para registrarlo");
  }
}

void drawTecladoNumerico() {
  tft.fillRect(141, 41, 339, 279, WHITE);

  tft.fillRect(160, 55, 180, 35, WHITE);
  tft.drawRect(160, 55, 180, 35, BLACK);
  tft.setTextSize(2);
  tft.setTextColor(BLACK);
  tft.setCursor(170, 65);
  tft.println(inputNombre);

  tft.fillRoundRect(350, 55, 110, 35, 6, GREEN);
  tft.drawRoundRect(350, 55, 110, 35, 6, BLACK);
  tft.setCursor(390, 65);
  tft.println("OK");

  char teclas[3][3] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'}
  };

  int startX = 180;
  int startY = 110;
  for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 3; c++) {
      int bx = startX + (c * 75);
      int by = startY + (r * 60);
      tft.fillRoundRect(bx, by, 65, 50, 6, LIGHTGRAY);
      tft.drawRoundRect(bx, by, 65, 50, 6, BLACK);
      tft.setCursor(bx + 25, by + 18);
      tft.setTextSize(2);
      tft.println(teclas[r][c]);
    }
  }
}

//logica de procesamiento 
void procesarTarjetaRFID(String uid, int corralDestino) {
  int index = -1;
  for (int i = 0; i < totalAnimales; i++) {
    if (listaAnimales[i].uidStr == uid) {
      index = i;
      break;
    }
  }

  if (index != -1) {
    // La tarjeta ya existe en el sistema
    if (listaAnimales[index].corralID != corralDestino) {
      // Si se pasó por el lector del otro corral, se mueve de corral
      listaAnimales[index].corralID = corralDestino;
    } else {
      // Si se pasa de nuevo por el mismo lector, alterna su estado de color (Verde/Rojo)
      if (listaAnimales[index].registrado) {
        listaAnimales[index].estadoColor = (listaAnimales[index].estadoColor == 1) ? 2 : 1;
      }
    }
  } else {
    // Si la tarjeta es totalmente nueva, se añade directamente al corral del lector que la leyó
    if (totalAnimales < 20) {
      listaAnimales[totalAnimales] = {uid, corralDestino, "????", false, 0, 0, 0};
      totalAnimales++;
    }
  }
  
  modoTeclado = false;
  regSeleccionandoCirculo = false;
  drawUI();
}

void manejarToqueTactil(int x, int y) {
  if (y < 40) {
    if (x >= 20 && x <= 140) {
      modoTeclado = false;
      regSeleccionandoCirculo = false;
      drawUI();
    } 
    else if (x >= 360 && x <= 460) {
      regSeleccionandoCirculo = !regSeleccionandoCirculo; 
      modoTeclado = false;
      drawHeader();
      if(!modoTeclado) drawCorralContent();
    }
    return;
  }

  // Toques en la barra 
  if (x < 140 && y >= 40) {
    if (y >= 60 && y <= 135) activeFolder = 1;
    else if (y >= 155 && y <= 230) activeFolder = 2;
    
    modoTeclado = false;
    regSeleccionandoCirculo = false;
    drawUI();
    return;
  }

  if (modoTeclado) {
    if (x >= 350 && x <= 460 && y >= 55 && y <= 90) {
      if (inputNombre != "" && animalAElegir != -1) {
        listaAnimales[animalAElegir].nombre = inputNombre;
        listaAnimales[animalAElegir].registrado = true;
        listaAnimales[animalAElegir].estadoColor = 1; 
      }
      inputNombre = "";
      animalAElegir = -1;
      modoTeclado = false;
      drawUI();
      return;
    }

    char teclas[3][3] = { {'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'} };
    int startX = 180;
    int startY = 110;
    for (int r = 0; r < 3; r++) {
      for (int c = 0; c < 3; c++) {
        int bx = startX + (c * 75);
        int by = startY + (r * 60);
        if (x >= bx && x <= bx + 65 && y >= by && y <= by + 50) {
          if (inputNombre.length() < 4) {
            inputNombre += teclas[r][c];
            drawTecladoNumerico();
          }
        }
      }
    }
    return;
  }

  if (regSeleccionandoCirculo && !modoTeclado) {
    for (int i = 0; i < totalAnimales; i++) {
      if (listaAnimales[i].corralID == activeFolder) {
        int distanciaX = x - listaAnimales[i].drawX;
        int distanciaY = y - listaAnimales[i].drawY;
        if (distanciaX * distanciaX + distanciaY * distanciaY <= 35 * 35) {
          animalAElegir = i;
          regSeleccionandoCirculo = false;
          modoTeclado = true; 
          inputNombre = "";
          drawUI();
          break;
        }
      }
    }
  }
}
