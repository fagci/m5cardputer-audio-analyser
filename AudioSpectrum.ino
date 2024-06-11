#include <M5Cardputer.h>
#include <KickFFT.h>

static constexpr size_t SR = 36000;
static constexpr uint16_t BINS = 256;
static constexpr uint16_t FFT_SZ = BINS * 2;

static constexpr int32_t W = 240;
static constexpr int32_t H = 135;
static constexpr float scaleX = (float)W / BINS;

static constexpr uint8_t TEXT_COLOR = 0b1100;
static constexpr uint8_t BG_LINE_COLOR = 0b0010;
static constexpr uint8_t GRAPH_COLOR = 0b1110;

int16_t data[FFT_SZ] = { 0 };
uint32_t mag[BINS] = { 0 };
uint8_t screenData[W] = { 0 };

static M5GFX *display = &M5Cardputer.Display;
static LGFX_Sprite graph(display);

uint16_t hz2i(uint16_t hz) {
  return hz * FFT_SZ / SR;
}

void setup(void) {
  setCpuFrequencyMhz(80);
  M5Cardputer.begin();

  M5Cardputer.Speaker.end();
  M5Cardputer.Mic.begin();

  display->startWrite();

  graph.setColorDepth(4);

  graph.createSprite(W, H);

  graph.setTextSize(1);
  graph.setTextColor(TEXT_COLOR, 0);
  graph.setTextDatum(top_center);
}

inline void drawBg() {
  for (uint16_t i = 1; i <= SR / 2000; ++i) {
    uint16_t x = hz2i(i * 1000) * scaleX;
    graph.writeFastVLine(x, 12, H, BG_LINE_COLOR);
    if (i % 2) {
      graph.drawString((String)i, x, 0);
    }
  }
}

inline void analyze() {
  KickFFT<int16_t>::fft(SR, 0, SR / 2, FFT_SZ, data, mag);
  memset(screenData, 0, 240);

  for (int i = 0; i < BINS; ++i) {
    uint8_t xi = (int)(i * scaleX);
    uint32_t v = H * mag[i] / 65535;

    if (v > screenData[xi]) screenData[xi] = v;
  }
}

inline void drawGraph() {
  for (int x = 0; x < W; ++x) {
    graph.writeFastVLine(x, H - screenData[x], screenData[x], GRAPH_COLOR);
  }
  graph.pushSprite(0, 0);
}

void loop(void) {
  M5Cardputer.update();

  if (M5Cardputer.Mic.isEnabled()) {
    if (M5Cardputer.Mic.record(data, FFT_SZ, SR)) {
      analyze();
      graph.fillSprite(BLACK);
      drawBg();
      drawGraph();
    }
  }
}
