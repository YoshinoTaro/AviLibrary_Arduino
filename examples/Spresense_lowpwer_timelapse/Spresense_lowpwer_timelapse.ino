/**
 * Example code using Simple AVI library for Arduino
 * LGPL version 2.1 Copyright 2021 Yoshino Taro
 */

#include <Camera.h>
#include <SDHCI.h>
#include <LowPower.h>
#include <RTC.h>
#include <EEPROM.h>
#include "AviLibrary.h"

SDClass theSD;
File aviFile;

AviLibrary theAvi;

String filename = "movie.avi";

const uint32_t recording_total_frames = 30;
const uint32_t target_fps = 5;
const uint32_t sleep_time = 2; /* sec */
const uint32_t rec_frame_address = 0;
const uint32_t movi_size_address = rec_frame_address + sizeof(uint32_t);
const uint32_t file_size_address = movi_size_address + sizeof(uint32_t);

uint32_t rec_frame = 0;
uint32_t movi_size = 0;
uint32_t file_size = 0;

void setup() {
  LowPower.begin();
  RTC.begin();
  bootcause_e bc = LowPower.bootCause();
  
  Serial.begin(115200);

  theCamera.begin();
  while (!theSD.begin()) { Serial.println("insert SD card"); }

  theCamera.setStillPictureImageFormat(
     CAM_IMGSIZE_QUADVGA_H,
     CAM_IMGSIZE_QUADVGA_V,
     CAM_IMAGE_PIX_FMT_JPG);

  /* check wheter this recording is for the first time */
  if (bc != DEEP_RTC && bc != DEEP_OTHERS) {
    theSD.remove(filename);
    rec_frame = 0;
    movi_size = 0;
    file_size = 0; 
  } else {
    EEPROM.get(rec_frame_address, rec_frame);
    EEPROM.get(movi_size_address, movi_size);
    EEPROM.get(file_size_address, file_size);
  }

  Serial.println("rec_frame : " + String(rec_frame));
  Serial.println("movi_size : " + String(movi_size));
  Serial.println("file_size : " + String(file_size));

  aviFile = theSD.open(filename, FILE_WRITE);
  theAvi.begin(aviFile, CAM_IMGSIZE_QUADVGA_H, CAM_IMGSIZE_QUADVGA_V);
  theAvi.setTotalFrame(rec_frame);
  theAvi.setFileSize(file_size);
  theAvi.setMovieSize(movi_size); 

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

  theAvi.endTimelapse();
  theAvi.end();
  theCamera.end();
  Serial.println("Movie saved");
  Serial.println(" File size (kB): " + String(theAvi.getFileSize()));
  Serial.println(" Captured Frame: " + String(theAvi.getTotalFrame())); 
  Serial.println(" Duration (sec): " + String(theAvi.getDuration()));
  Serial.println(" Frame per sec : " + String(theAvi.getFps()));
  Serial.println(" Max data rate : " + String(theAvi.getMaxDataRate()));

  EEPROM.put(rec_frame_address, theAvi.getTotalFrame());
  EEPROM.put(movi_size_address, theAvi.getMovieSize());
  EEPROM.put(file_size_address, theAvi.getFileSize());
  
  if (theAvi.getTotalFrame() > recording_total_frames) {
    while (true) {
      digitalWrite(LED0, HIGH);
      delay(100);
      digitalWrite(LED0, LOW);
      delay(100);
    }
  }

  LowPower.deepSleep(sleep_time);
}
