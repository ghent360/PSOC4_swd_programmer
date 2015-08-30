#include "stdafx.h"
#include "HexFileParser.h"

#include <algorithm>
#include <fstream>
#include <iostream>

HexFileParser::HexFileParser() : baseAddress_(0)
{
}

HexFileParser::~HexFileParser(void)
{
  clean();
}

void HexFileParser::clean()
{
  for(std::list<Record>::iterator it = content_.begin(); it != content_.end(); ++it)
  {
    if (it->data_)
    {
      delete [] it->data_;
    }
  }
  content_.clear();
}

bool HexFileParser::parse(const std::string& fileName)
{
  std::ifstream hexFileInput;
  hexFileInput.open(fileName.c_str());
  if (!hexFileInput.good())
  {
    std::cout << "Error opening input file: " << fileName << std::endl;
    return false;
  }
  clean();
  return process(hexFileInput);
}

bool HexFileParser::compareAddress(const Record& first, const Record& second)
{
  return first.adress_ < second.adress_;
}

bool HexFileParser::process(std::istream& inputFile)
{
  baseAddress_ = 0;
  while(!inputFile.eof() && inputFile.good())
  {
    std::string line;
    inputFile >> line;
    if (!processLine(line))
    {
      return false;
    }
  }
  content_.sort(compareAddress);
  return true;
}

bool HexFileParser::processLine(const std::string& line)
{
  if (line[0] != ':')
  {
    std::cerr << "Line does not start with ':'" << std::endl << line << std::endl;
    return false;
  }
  unsigned char lineSize = parseHexByte(line, 1);
  unsigned int addr = parseHexWord(line, 3);
  unsigned char recordType = parseHexByte(line, 7);
  if (recordType == 4)
  {
    baseAddress_ = parseHexWord(line, 9) << 16;
  }
  else if (recordType == 0)
  {
    Record r;
    r.adress_ = baseAddress_ + addr;
    r.size_ = lineSize;
    r.data_ = new unsigned char[lineSize];
    for(int idx = 0; idx < lineSize; idx++)
    {
      r.data_[idx] = parseHexByte(line, 9 + idx*2);
    }
    content_.push_back(r);
  }
  else if (recordType == 1)
  {
    // Ignore EOF record.
  }
  else
  {
    std::cerr << "Unknown record type" << recordType << std::endl << line << std::endl;
    return false;
  }
  return true;
}

static unsigned char hexValue(char c)
{
  switch(c)
  {
  case '0':
    return 0;
  case '1':
    return 1;
  case '2':
    return 2;
  case '3':
    return 3;
  case '4':
    return 4;
  case '5':
    return 5;
  case '6':
    return 6;
  case '7':
    return 7;
  case '8':
    return 8;
  case '9':
    return 9;
  case 'A':
  case 'a':
    return 10;
  case 'B':
  case 'b':
    return 11;
  case 'C':
  case 'c':
    return 12;
  case 'D':
  case 'd':
    return 13;
  case 'E':
  case 'e':
    return 14;
  case 'F':
  case 'f':
    return 15;
  }
  throw "Invalid character";
}

unsigned char HexFileParser::parseHexByte(const std::string& line, int idx)
{
  char c1, c2;
  c1 = line[idx];
  c2 = line[idx+1];
  return (hexValue(c1) << 4) | hexValue(c2);
}

unsigned int HexFileParser::parseHexWord(const std::string& line, int idx)
{
  unsigned char b1, b2;
  b1 = parseHexByte(line, idx);
  b2 = parseHexByte(line, idx + 2);
  return (b1 << 8) | b2;
}

unsigned int HexFileParser::getDataFrom(unsigned int startAddr, unsigned int maxLen, void* pBuffer)
{
  unsigned char* pDestination = reinterpret_cast<unsigned char*>(pBuffer);
  unsigned int offset;
  unsigned int result = 0;
  std::list<Record>::iterator it, old;
  for(it = content_.begin(); it != content_.end(); ++it)
  {
    if (it->adress_ + it->size_ >= startAddr) break;
  }
  if (it->adress_ + it->size_ < startAddr ||
      it->adress_ > startAddr + maxLen)
  {
    return result;
  }
  offset = startAddr - it->adress_;
  while(maxLen > 0)
  {
    unsigned int copySize = maxLen;
    if (copySize > it->size_ - offset)
    {
      copySize = it->size_ - offset;
    }
    memcpy(pDestination, it->data_ + offset, copySize);
    pDestination += copySize;
    result += copySize;
    maxLen -= copySize;
    offset = 0;
    old = it;
    it++;
    if (it == content_.end()) break;
    if (it->adress_ != old->adress_ + old->size_) break;
  }
  return result;
}

static unsigned int checksum(const unsigned char* data, size_t len)
{
  unsigned int result = 0;
  while (len)
  {
    result += *data++;
    len--;
  }
  return result;
}

bool HexFileParser::updateSerial(const char* serial)
{
  std::list<Record>::iterator it;
  std::list<Record>::iterator cksum;
  unsigned int crc;
  for(it = content_.begin(); it != content_.end(); ++it)
  {
    if (it->adress_ == 0x90300000 && it->size_ == 2)
    {
      cksum = it;
      break;
    }
  }
  if (cksum != it)
  {
    return false;
  }
  crc = (cksum->data_[0] << 8) | cksum->data_[1];
  for(it = content_.begin(); it != content_.end(); ++it)
  {
    if (it->size_ != 0x40) continue;
    if (it->data_[0] != 'S' ||
        it->data_[1] != '$' ||
        it->data_[2] != 'N' ||
        it->data_[3] != '&') continue;
    crc -= checksum(it->data_, it->size_);
    size_t len = strlen(serial);
    if (len > 0x39)
    {
      len = 0x39;
    }
    memcpy(it->data_, serial, len);
    it->data_[len] = 0;
    crc += checksum(it->data_, it->size_);
    cksum->data_[0] = crc >> 8;
    cksum->data_[1] = crc & 0xff;
    return true;
  }
  return false;
}
