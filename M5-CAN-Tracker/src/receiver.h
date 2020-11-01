#pragma once
#include <Arduino.h>
#include "can_msg.h"

class MeasureNode {
    public:
        MeasureNode(uint8_t id) : _uid(id) {} 
        
        void set(uint16_t val) { this->_value = val; }
        
        uint16_t value() { return this->_value; }
        float value_m() { return (this->_value/1000.); }

    private:
        uint8_t _uid;
        uint16_t _value = 0;
};


class MeasureBook {
    public:
        MeasureBook() {}

        void process(CanMessage* msg) 
        {   
            if (this->_nodes[msg->uid()] == nullptr) 
                this->_nodes[msg->uid()] = new MeasureNode(msg->uid());

            if (msg->type() == MEASURE)
                this->_nodes[msg->uid()]->set(msg->value());
        }

        int value(uint8_t id) 
        { 
            if (this->_nodes[id] == nullptr) return -1;
            return this->_nodes[id]->value(); 
        }
        
        float value_m(uint8_t id) 
        { 
            if (this->_nodes[id] == nullptr) return -1;
            return this->_nodes[id]->value_m(); 
        }

    private:
        MeasureNode* _nodes[255] = {nullptr};

};