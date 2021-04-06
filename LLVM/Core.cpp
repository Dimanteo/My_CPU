#include "Core.hpp"

Core::Core() : m_memory(new word_t [RAM_SIZE]), m_running(false) {};

Core::~Core() {
    delete [] (m_memory);
}
