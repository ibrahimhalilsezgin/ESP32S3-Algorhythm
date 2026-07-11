#include <Arduino.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "img_converters.h"
#include "config.h"
#include "wifi_manager.h"
#include "generators/base_generator.h"
#include "generators/plasma_gen.h"
#include "generators/fractal_gen.h"
#include "generators/noise_gen.h"
#include "generators/automata_gen.h"
#include "generators/pixel_art_gen.h"

// Globals
WifiManager wifi;
WebSocketsClient ws;
bool wsConnected = false;
bool registered = false;

// Frame buffers (PSRAM)
uint16_t* frameBuffer = nullptr;  // RGB565
uint8_t*  rgbBuffer   = nullptr;  // RGB888 (JPEG input)
uint8_t*  jpegBuffer  = nullptr;  // JPEG output

// Generators
BaseGenerator* generators[5];
int generatorCount = 5;
int currentGen = 0;
uint32_t frameNum = 0;
unsigned long lastSwitch = 0;
unsigned long lastFrame = 0;

void switchGenerator() {
    currentGen = random(0, generatorCount);
    Serial.printf("Generator degisti: %s\n", generators[currentGen]->name());
}

// RGB565 → RGB888 dönüşüm
void rgb565ToRgb888(uint16_t* src, uint8_t* dst, int w, int h) {
    for (int i = 0; i < w * h; i++) {
        uint16_t px = src[i];
        dst[i * 3 + 0] = (px >> 8) & 0xF8;  // R
        dst[i * 3 + 1] = (px >> 3) & 0xFC;  // G
        dst[i * 3 + 2] = (px << 3) & 0xF8;  // B
    }
}

// JPEG encode
int encodeJpeg(uint8_t* rgb, int w, int h, uint8_t* out, int outSize) {
    uint8_t* outBuf = nullptr;
    size_t outLen = 0;
    
    bool success = fmt2jpg(rgb, w * h * 3, w, h, PIXFORMAT_RGB888, JPEG_QUALITY, &outBuf, &outLen);
    
    if (!success || outLen == 0 || !outBuf) {
        Serial.println("JPEG encode basarisiz!");
        if (outBuf) free(outBuf);
        return -1;
    }
    
    int bytesToWrite = (outLen > (size_t)outSize) ? outSize : outLen;
    memcpy(out, outBuf, bytesToWrite);
    free(outBuf);
    
    return bytesToWrite;
}

void wsEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            Serial.println("WebSocket baglandi");
            wsConnected = true;
            registered = false;
            {
                // Register mesajı gönder
                JsonDocument doc;
                doc["type"] = "register";
                doc["chip_id"] = wifi.chipId();
                doc["token"] = DEVICE_TOKEN;
                doc["resolution"] = String(FRAME_WIDTH) + "x" + String(FRAME_HEIGHT);
                doc["generator"] = generators[currentGen]->name();
                String msg;
                serializeJson(doc, msg);
                ws.sendTXT(msg);
                Serial.println("Register gonderildi");
            }
            break;
        case WStype_DISCONNECTED:
            Serial.println("WebSocket koptu");
            wsConnected = false;
            registered = false;
            break;
        case WStype_TEXT:
            {
                JsonDocument doc;
                deserializeJson(doc, payload, length);
                const char* msgType = doc["type"];
                if (msgType && strcmp(msgType, "registered") == 0) {
                    registered = true;
                    Serial.println("Kayit onaylandi, frame gonderme basliyor");
                } else if (msgType && strcmp(msgType, "error") == 0) {
                    Serial.printf("Sunucu hatasi: %s\n", doc["message"].as<const char*>());
                }
            }
            break;
        default:
            break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== ESP32-S3 Generative Art Streamer ===");

    // PSRAM kontrol
    if (!psramFound()) {
        Serial.println("HATA: PSRAM bulunamadi!");
        while(1) delay(1000);
    }
    Serial.printf("PSRAM: %d bytes\n", ESP.getFreePsram());

    // Buffer'ları PSRAM'da oluştur
    size_t pixelCount = FRAME_WIDTH * FRAME_HEIGHT;
    frameBuffer = (uint16_t*)ps_malloc(pixelCount * 2);        // RGB565
    rgbBuffer   = (uint8_t*) ps_malloc(pixelCount * 3);        // RGB888
    jpegBuffer  = (uint8_t*) ps_malloc(pixelCount * 2);        // JPEG (en kötü durum)

    if (!frameBuffer || !rgbBuffer || !jpegBuffer) {
        Serial.println("HATA: Buffer allocation basarisiz!");
        while(1) delay(1000);
    }

    // Generator'ları oluştur
    generators[0] = new PlasmaGen();
    generators[1] = new FractalGen();
    generators[2] = new NoiseGen();
    generators[3] = new AutomataGen();
    generators[4] = new PixelArtGen();
    currentGen = random(0, generatorCount);
    Serial.printf("Baslangic generator: %s\n", generators[currentGen]->name());

    // WiFi bağlan
    wifi.begin(WIFI_SSID, WIFI_PASS);

    // WebSocket bağlan
    ws.begin(WS_HOST, WS_PORT, WS_PATH);
    ws.onEvent(wsEvent);
    ws.setReconnectInterval(3000);

    lastSwitch = millis();
    lastFrame = millis();
}

void loop() {
    ws.loop();
    wifi.reconnectIfNeeded();

    unsigned long now = millis();

    // Generator otomatik değiştir
    if (now - lastSwitch > AUTO_SWITCH_INTERVAL_MS) {
        switchGenerator();
        lastSwitch = now;
        // Sunucuya bildir
        if (wsConnected && registered) {
            JsonDocument doc;
            doc["type"] = "generator_change";
            doc["generator"] = generators[currentGen]->name();
            String msg;
            serializeJson(doc, msg);
            ws.sendTXT(msg);
        }
    }

    // Frame üret ve gönder
    unsigned long frameInterval = 1000 / TARGET_FPS;
    if (wsConnected && registered && (now - lastFrame >= frameInterval)) {
        lastFrame = now;

        // Frame üret
        generators[currentGen]->generate(frameBuffer, FRAME_WIDTH, FRAME_HEIGHT, frameNum);
        frameNum++;

        // RGB565 → RGB888
        rgb565ToRgb888(frameBuffer, rgbBuffer, FRAME_WIDTH, FRAME_HEIGHT);

        // JPEG encode
        int jpegLen = encodeJpeg(rgbBuffer, FRAME_WIDTH, FRAME_HEIGHT,
                                  jpegBuffer, FRAME_WIDTH * FRAME_HEIGHT * 2);
        if (jpegLen > 0) {
            ws.sendBIN(jpegBuffer, jpegLen);
            if (frameNum % 30 == 0) {
                Serial.printf("Frame %d: %d bytes JPEG (%s)\n",
                    frameNum, jpegLen, generators[currentGen]->name());
            }
        }
    }
}
