/**
 * Simple AVI library for Arduino
 * LGPL version 2.1 Copyright 2021 Yoshino Taro
 */

#include "AviLibrary.h"

/* constructor */
AviLibrary::AviLibrary():
    m_initialized(false)
  , m_width(0)
  , m_height(0)
  , m_frames(0)
  , m_total_size(0)
  , m_movi_size(0)
  , m_fps(0)
  , m_us_per_frame(0)
  , m_max_bytes_per_sec(0)
  , m_duration_sec(0.0)
  , m_rec_mode(AVI_REC_NONE)
  , m_aviFile(NULL) {}

/* destructor */
AviLibrary::~AviLibrary() {
  if (m_initialized || m_aviFile != NULL) 
    m_aviFile.close();
}

bool AviLibrary::begin(File& aviFile, uint16_t width, uint16_t height) {
  if (aviFile == NULL || width == 0 || height == 0) return false;
  m_aviFile = aviFile;
  m_width = width;
  m_height = height;
  m_initialized = true;
  return true;
}

bool AviLibrary::writeHeader() {
  if (!m_initialized || m_frames != 0) return false;
  m_avi_header[64] = (uint8_t)( m_width  & 0x0f );
  m_avi_header[65] = (uint8_t)((m_width  & 0xf0) >> 8);
  m_avi_header[68] = (uint8_t)( m_height & 0x0f );
  m_avi_header[69] = (uint8_t)((m_height & 0xf0) >> 8);
  m_aviFile.write(m_avi_header, AVI_OFFSET);
  return true;
}

bool AviLibrary::startRecording() {
  if (!m_initialized || !m_aviFile) return false;
  writeHeader();
  m_rec_mode = AVI_REC_MOVIE;
  m_start_time = millis();
  return true;
}

bool AviLibrary::addFrame(char* ImgBuff, uint32_t imgSize) {
  if (!m_initialized || !ImgBuff || !imgSize) return false;

  add_frame(ImgBuff, imgSize);

  m_duration_sec = (millis() - m_start_time) / 1000.0f;
  float fps_in_float = m_frames / m_duration_sec;
  float us_per_frame_in_float = 1000000.0f / fps_in_float;
  m_fps = round(fps_in_float);
  m_us_per_frame =round(us_per_frame_in_float);
  m_total_size = m_movi_size + 12*m_frames + 4;
  m_max_bytes_per_sec = m_movi_size * m_fps / m_frames;
}


bool AviLibrary::endRecording() {
  if (!m_initialized || !m_aviFile) return false;
  write_parameters();
}


bool AviLibrary::startTimelapse(uint8_t target_fps) {
  if (!m_initialized || !target_fps) return false;
  m_fps = target_fps;
  m_rec_mode = AVI_REC_TIMELAPSE;
  writeHeader();
  return true;
}

bool AviLibrary::addTimelapseFrame(char* ImgBuff, uint32_t imgSize) {
  if (!m_initialized || !ImgBuff || !imgSize) return false;

  add_frame(ImgBuff, imgSize);

  m_duration_sec = (float)m_frames / m_fps;
  m_us_per_frame = round(1000000.0f / m_fps);
  m_total_size = m_movi_size + 12*m_frames + 4;
  m_max_bytes_per_sec = m_movi_size * m_fps / m_frames;
}

bool AviLibrary::endTimelapse() {
  if (!m_initialized || !m_aviFile) return false;
  write_parameters();
}


AVI_REC_MODE AviLibrary::getRecMode() {
  return m_rec_mode;
}

void AviLibrary::end() {
  m_aviFile.close();
  m_initialized = false;
  m_aviFile = NULL;
  m_rec_mode = AVI_REC_NONE;
}


void AviLibrary::uint32_write_to_aviFile(uint32_t v) {
  char value = v % 0x100;
  m_aviFile.write(value);  v = v >> 8; 
  value = v % 0x100;
  m_aviFile.write(value);  v = v >> 8;
  value = v % 0x100;
  m_aviFile.write(value);  v = v >> 8; 
  value = v;
  m_aviFile.write(value); 
}

void AviLibrary::add_frame(char* img, uint32_t img_size) {
  m_aviFile.seek(m_aviFile.size());
  m_aviFile.write("00dc", 4);
  uint32_write_to_aviFile(img_size);
  m_aviFile.write(img, img_size);
  m_movi_size += img_size;
  ++m_frames;  
}

void AviLibrary::write_parameters() {
  /* overwrite riff file size */
  m_aviFile.seek(0x04);
  uint32_write_to_aviFile(m_total_size);

  /* overwrite hdrl */
  /* hdrl.avih.us_per_frame */
  m_aviFile.seek(0x20);
  uint32_write_to_aviFile(m_us_per_frame);
  m_aviFile.seek(0x24);
  uint32_write_to_aviFile(m_max_bytes_per_sec);

  /* hdrl.avih.tot_frames */
  m_aviFile.seek(0x30);
  uint32_write_to_aviFile(m_frames);
  m_aviFile.seek(0x84);
  uint32_write_to_aviFile(m_fps);

  /* hdrl.strl.list_odml.frames */
  m_aviFile.seek(0xe0);
  uint32_write_to_aviFile(m_frames);
  m_aviFile.seek(0xe8);
  uint32_write_to_aviFile(m_movi_size);  
}
