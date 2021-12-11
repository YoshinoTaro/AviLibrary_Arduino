# AviLibrary_Arduino
Simple AVI Library for Arduino and Spresense. This library could be used for Arduino based boards, but please note that the sample codes are written for Sony Spresense, and I have not checked the library with other Arduino-based board.

## Support functions
This library supports not only a camcorder function but also a timelapse function. If you specify fps (frame per second) when the recording function, you can make a timelapse movie. For example, if you capture images in 10 fps when you specify 20 fps, you can see the double speed movie. Or if you specify 5 fps, you can see the half-speed movie. 

Since this library is based on Motion JPEG format, the sound recording function is not supported.

## Camcorder example
```
void setup() {
  Serial.begin(115200);
  theCamera.begin();
  while (!theSD.begin()) { Serial.println("insert SD card"); }

  // Setup the still picture parameters of Spresense Camera
  // The image format must be JPEG
  theCamera.setStillPictureImageFormat(
     CAM_IMGSIZE_QUADVGA_H, CAM_IMGSIZE_QUADVGA_V, CAM_IMAGE_PIX_FMT_JPG);

  theSD.remove(filename);
  aviFile = theSD.open(filename, FILE_WRITE);

  // Initialize AVI library
  theAvi.begin(aviFile, CAM_IMGSIZE_QUADVGA_H, CAM_IMGSIZE_QUADVGA_V);
  theAvi.startRecording();
}

void loop() {
  static uint32_t start_time = millis();
  CamImage img = theCamera.takePicture();

  // Add a frame. The image format must be JPEG
  theAvi.addFrame(img.getImgBuff(), img.getImgSize());

  uint32_t duration = millis() - start_time;
  if (duration > recording_time_in_ms) {
    // Stop AVI library
    theAvi.endRecording();
    theAvi.end();
    theCamera.end();
    while (true) {}
  }
}
```

## Timelapse example
```
void setup() {
  Serial.begin(115200);
  theCamera.begin();
  while (!theSD.begin()) { Serial.println("insert SD card"); }

  // Setup the still picture parameters of Spresense Camera
  // The image format must be JPEG
  theCamera.setStillPictureImageFormat(
     CAM_IMGSIZE_QUADVGA_H, CAM_IMGSIZE_QUADVGA_V, CAM_IMAGE_PIX_FMT_JPG);

  theSD.remove(filename);
  aviFile = theSD.open(filename, FILE_WRITE);

  // Initialize AVI library
  theAvi.begin(aviFile, CAM_IMGSIZE_QUADVGA_H, CAM_IMGSIZE_QUADVGA_V);
  // Start the timelapse specifying fps
  theAvi.startTimelapse(target_fps);
}

void loop() {
  static uint32_t start_time = millis();
  CamImage img = theCamera.takePicture();
  // Add a timelapse frame. The image format must be JPEG
  theAvi.addTimelapseFrame(img.getImgBuff(), img.getImgSize());

  uint32_t duration = millis() - start_time;
  if (duration > recording_time_in_ms) {
    // Stop AVI library
    theAvi.endTimelapse();
    theAvi.end();
    theCamera.end();
    while (true) {}
  }
}
```

## Low power camera example
This example is a low power camera using Spresense. Spresense supoprts a LowPower library enabling deep sleep controled by timer or interrupts.
```
void setup() {
  // Initialize LowPower Library
  LowPower.begin();
  RTC.begin();

  // get boot cause to check whether this boot is caused by the alarm
  bootcause_e bc = LowPower.bootCause();

  Serial.begin(115200);
  theCamera.begin();
  while (!theSD.begin()) { Serial.println("insert SD card"); }

  // Setup the still picture parameters of Spresense Camera
  // The image format must be JPEG
  theCamera.setStillPictureImageFormat(
     CAM_IMGSIZE_QUADVGA_H, CAM_IMGSIZE_QUADVGA_V, CAM_IMAGE_PIX_FMT_JPG);

  // In the case of power on / reset, the parameters are set to the initial value.
  if (bc != DEEP_RTC && bc != DEEP_OTHERS) {
    theSD.remove(filename);
    rec_frame = 0;
    movi_size = 0;
    file_size = 0; 
  } else {
    EEPROM.get(rec_frame_address, rec_frame);  // memory the recorded frame count
    EEPROM.get(movi_size_address, movi_size);  // memory the size of the video
    EEPROM.get(file_size_address, file_size);  // memory the file size
  }

  aviFile = theSD.open(filename, FILE_WRITE);

  // Initialize AVI library
  theAvi.begin(aviFile, CAM_IMGSIZE_QUADVGA_H, CAM_IMGSIZE_QUADVGA_V);
  // set parameters got from EEPROM
  theAvi.setTotalFrame(rec_frame);
  theAvi.setFileSize(file_size);
  theAvi.setMovieSize(movi_size); 
  // Start the timelapse specifying fps
  theAvi.startTimelapse(target_fps);
}

void loop() {
  CamImage img = theCamera.takePicture();
  // Add a timelapse frame. The image format must be JPEG
  theAvi.addTimelapseFrame(img.getImgBuff(), img.getImgSize());
  // end the recording immediately
  theAvi.endTimelapse();
  theAvi.end();
  theCamera.end();

  // memory the updated parameters
  EEPROM.put(rec_frame_address, theAvi.getTotalFrame());
  EEPROM.put(movi_size_address, theAvi.getMovieSize());
  EEPROM.put(file_size_address, theAvi.getFileSize());

  // finish the task when the total frame is over the specified number
  if (theAvi.getTotalFrame() > recording_total_frames) {
    while (true) {}
  }
  // power off until  the time specified by sleep_time
  LowPower.deepSleep(sleep_time);
}
```
