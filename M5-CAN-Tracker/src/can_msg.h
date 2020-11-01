#pragma once
#include <Arduino.h>

enum canCode {
  NIET        = 0x00,
  REMOTEFRAME = 0x01,
  EMPTYFRAME  = 0x02,
  MEASURE     = 0xA0
};


class CanMessage {
    public:
        // Sender 8bit constructor 
        CanMessage(uint8_t id, canCode code, uint8_t value=0) 
            : _uid(id), _code(code) {
            
            this->_length = 1;
            this->_data[0] = value & 0xff;
        }

        // Sender 16bit constructor 
        CanMessage(uint8_t id, canCode code, uint16_t value=0) 
            : _uid(id), _code(code) {
            
            this->_length = 2;
            this->_data[0] = (value >> 8) & 0xff;
            this->_data[1] = value & 0xff;
        }

        // Sender 32bit constructor 
        CanMessage(uint8_t id, canCode code, uint32_t value=0) 
            : _uid(id), _code(code) {
            
            this->_length = 4;
            this->_data[0] = (value >> 24) & 0xff;
            this->_data[1] = (value >> 16) & 0xff;
            this->_data[2] = (value >> 8) & 0xff;
            this->_data[3] = value & 0xff;
        }

        // Receiver constructor
        CanMessage(uint8_t id, uint8_t len, uint8_t raw_data[8])
            : _uid(id), _length(len) {
            
            // Remote Request Frame
            if((_uid & 0x40000000) == 0x40000000) this->_code = REMOTEFRAME;

            // Copy data
            if (_length > 0) {
                this->_code = (canCode) raw_data[0];
                this->_length = this->_length-1;
                for(byte k=0; k<this->_length; k++) this->_data[k] = raw_data[k+1];
            }

            // No Data
            else this->_code = EMPTYFRAME;
        }

        uint8_t uid() { 
            return this->_uid; 
        }

        uint8_t frameSize() { 
            return this->_length+1; 
        }

        uint8_t* frame() {
            this->_frame[0] = this->_code;
            for(uint8_t k=0; k<this->_length; k++) this->_frame[k+1] = this->_data[k];
            return this->_frame;
        }

        uint32_t value() {
            uint32_t v = _data[0];
            for(uint8_t k=1; k<this->_length; k++)
                v = v<<8 | _data[k];
            return v;
        }

        canCode type() {
            return this->_code; 
        }


    private:
        uint8_t _uid;
        canCode _code;
        uint8_t _length;
        uint8_t _data[8];
        uint8_t _frame[9];
};