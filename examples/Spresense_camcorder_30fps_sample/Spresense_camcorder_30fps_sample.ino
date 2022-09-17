#include <Camera.h>
#include <SDHCI.h>
#include "AviLibrary.h"

SDClass theSD;
File aviFile;

AviLibrary theAvi;

String filename = "movie.avi";

/*-----------------------------------------------------*/
/* NOTICE!! : To achive 30 fps                         */ 
/* (1) need to use KIOXIA 32GB SD card                 */
/* (2) The SD card must be just after physical format  */
/*-----------------------------------------------------*/
void CamCB(CamImage img) {
  const static uint32_t recording_time_in_ms = 10000;  /* msec */
  static uint32_t start_time = millis();

  if (img.isAvailable()) {
   uint32_t duration = millis() - start_time;
    if (duration > recording_time_in_ms) {
      theAvi.endRecording();
      theAvi.end();
      Serial.println("Movie saved");
      Serial.println(" Movie width:    " + String(theAvi.getWidth()));
      Serial.println(" Movie height:   " + String(theAvi.getHeight()));
      Serial.println(" File size (kB): " + String(theAvi.getFileSize()));
      Serial.println(" Captured Frame: " + String(theAvi.getTotalFrame())); 
      Serial.println(" Duration (sec): " + String(theAvi.getDuration()));
      Serial.println(" Frame per sec : " + String(theAvi.getFps()));
      Serial.println(" Max data rate : " + String(theAvi.getMaxDataRate()));
      theCamera.end();
      while (true) {
        digitalWrite(LED0, HIGH);
        delay(100);
        digitalWrite(LED0, LOW);
        delay(100);
      }
    } else {  
      theAvi.addFrame(img.getImgBuff(), img.getImgSize());
    }
  }
}


void setup() {
  Serial.begin(115200);
  while (!theSD.begin()) { Serial.println("insert SD card"); }

  const int buff_num = 2;
  theCamera.begin(buff_num, CAM_VIDEO_FPS_30 
    ,CAM_IMGSIZE_QVGA_H ,CAM_IMGSIZE_QVGA_V ,CAM_IMAGE_PIX_FMT_JPG ,3);

  theSD.remove(filename);
  aviFile = theSD.open(filename, FILE_WRITE);

  theAvi.begin(aviFile, CAM_IMGSIZE_QVGA_H, CAM_IMGSIZE_QVGA_V);
  theCamera.startStreaming(true, CamCB);

  Serial.println("Start recording...");
  theAvi.startRecording();
}

void loop() {}
