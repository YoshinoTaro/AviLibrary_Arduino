/**
 * Example code using Simple AVI library for Arduino
 * LGPL version 2.1 Copyright 2021 Yoshino Taro
 */
 
#include <Camera.h>
#include <SDHCI.h>
#include "AviLibrary.h"

SDClass theSD;
File aviFile;

AviLibrary theAvi;

String filename = "movie.avi";

const static uint32_t recording_time_in_ms = 10000;  /* msec */
const uint32_t target_fps = 20;

void setup() {
  Serial.begin(115200);
  theCamera.begin();
  while (!theSD.begin()) { Serial.println("insert SD card"); }

  theCamera.setStillPictureImageFormat(
     CAM_IMGSIZE_QUADVGA_H,
     CAM_IMGSIZE_QUADVGA_V,
     CAM_IMAGE_PIX_FMT_JPG);

  theSD.remove(filename);
  aviFile = theSD.open(filename, FILE_WRITE);

  theAvi.begin(aviFile, CAM_IMGSIZE_QUADVGA_H, CAM_IMGSIZE_QUADVGA_V);

  Serial.println("Start timelapse...");
  theAvi.startTimelapse(target_fps);
}

void loop() {
  static uint32_t start_time = millis();
  CamImage img = theCamera.takePicture();
  if (!img.isAvailable()) {
    Serial.println("fail to take a picture");
    return;
  }

  theAvi.addTimelapseFrame(img.getImgBuff(), img.getImgSize());

  uint32_t duration = millis() - start_time;
  if (duration > recording_time_in_ms) {
    theAvi.endTimelapse();
    theAvi.end();
    theCamera.end();
    Serial.println("Movie saved");
    Serial.println(" File size (kB): " + String(theAvi.getFileSize()));
    Serial.println(" Captured Frame: " + String(theAvi.getTotalFrame())); 
    Serial.println(" Duration (sec): " + String(theAvi.getDuration()));
    Serial.println(" Frame per sec : " + String(theAvi.getFps()));
    Serial.println(" Max data rate : " + String(theAvi.getMaxDataRate()));
    while (true) {
      digitalWrite(LED0, HIGH);
      delay(100);
      digitalWrite(LED0, LOW);
      delay(100);
    }
  }
}
