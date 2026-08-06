#pragma once
#include <Arduino.h>
#include "telescope.h"

class OledScreen {
public:
  bool begin(int sda, int scl, int addr);
  void render(const ScopeStatus& st, const String& apIp, int clients);
  bool ok() const { return ok_; }
private:
  bool ok_ = false;
};

extern OledScreen oledScreen;
