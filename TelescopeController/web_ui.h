#pragma once
#include <WebServer.h>
#include "celestial.h"

void webSetup(WebServer& server, ObserverSite& site);
void webLoopExtras();  // reserved
