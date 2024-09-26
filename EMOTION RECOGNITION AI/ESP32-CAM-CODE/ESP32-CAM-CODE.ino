/*CODE FOR ESP32-CAM*/

/*
 * ESP32-CAM Web Server
 * This code sets up an ESP32-CAM module to serve JPEG images at different resolutions via a web server.
 * 
 * Libraries:
 * - WebServer: For creating the web server.
 * - WiFi: For WiFi connectivity.
 * - esp32cam: For interfacing with the ESP32-CAM module.
 * 
 * Configuration:
 * - WiFi SSID and password are set as constants.
 * - Static IP address configuration.
 * - Camera resolutions for low, medium, and high quality images.
 * 
 * Functionality:
 * - The web server handles requests for images at different resolutions:
 *   - /cam-lo.jpg: Low resolution (320x240).
 *   - /cam-mid.jpg: Medium resolution (350x530).
 *   - /cam-hi.jpg: High resolution (800x600).
 * - The camera is initialized with high resolution by default.
 * 
 * Usage:
 * - Connect to the ESP32-CAM's WiFi network.
 * - Access the camera images through the specified URLs.
 * 
 * Detailed Function Descriptions:
 * - serveJpg(): Captures a frame from the camera and sends it as a JPEG image in the HTTP response.
 * - handleJpgLo(): Sets the camera to low resolution and serves a JPEG image.
 * - handleJpgMid(): Sets the camera to medium resolution and serves a JPEG image.
 * - handleJpgHi(): Sets the camera to high resolution and serves a JPEG image.
 * 
 * Setup Process:
 * - Initialize the Serial monitor for debugging.
 * - Configure the camera with the specified settings.
 * - Set up WiFi connection with the specified SSID and password.
 * - Attempt to configure a static IP address.
 * - Start the web server and define URL handlers for different resolutions.
 * 
 * Main Loop:
 * - Continuously handle incoming web server requests.
 * 
 * Note:
 * - Ensure the esp32cam library is downloaded and installed: https://github.com/yoursunny/esp32cam
 * - Adjust the WiFi SSID, password, and IP settings as per your network configuration.
 */
 
/*SELECT BOARD AI Thinkar ESP32-CAM*/

#include <WebServer.h>
#include <WiFi.h>
#include <esp32cam.h>  // Download This Library: https://github.com/yoursunny/esp32cam

const char* WIFI_SSID = "ESP32";        // SSID SET AS PAR ESP CODE 
const char* WIFI_PASS = "password";     // PASS SET AS PAR ESP CODE 

IPAddress local_IP(192, 168, 4, 5);     // Set your desired static IP address
IPAddress gateway(192, 168, 4, 1);      // Set your network gateway (usually your router's IP)
IPAddress subnet(255, 255, 255, 0);     // Set your network subnet
IPAddress primaryDNS(8, 8, 8, 8);       // Optional: Set your primary DNS
IPAddress secondaryDNS(8, 8, 4, 4);     // Optional: Set your secondary DNS

WebServer server(80);

static auto loRes = esp32cam::Resolution::find(320, 240);
static auto midRes = esp32cam::Resolution::find(350, 530);
static auto hiRes = esp32cam::Resolution::find(800, 600);

void serveJpg() {
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
}

void handleJpgLo() {
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}

void handleJpgHi() {
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}

void handleJpgMid() {
  if (!esp32cam::Camera.changeResolution(midRes)) {
    Serial.println("SET-MID-RES FAIL");
  }
  serveJpg();
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }
  
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  
  // Try to configure a static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("URLs to access the camera:");
  Serial.println("  /cam-lo.jpg");
  Serial.println("  /cam-hi.jpg");
  Serial.println("  /cam-mid.jpg");

  server.on("/cam-lo.jpg", handleJpgLo);
  server.on("/cam-hi.jpg", handleJpgHi);
  server.on("/cam-mid.jpg", handleJpgMid);

  server.begin();
}

void loop() {
  server.handleClient();
}
