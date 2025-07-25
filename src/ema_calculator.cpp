#include "ema_calculator.h"
#include <stdexcept>

EMACalculator::EMACalculator(double smoothing_factor) 
    : alpha(smoothing_factor), current_ema(0.0), initialized(false) {
    if (alpha <= 0.0 || alpha > 1.0) {
        throw std::invalid_argument("Smoothing factor must be between 0 and 1");
    }
}

double EMACalculator::update(double new_value) {
    if (!initialized) { //if not initialized
        current_ema = new_value;
        initialized = true;
    } else {
        current_ema = (new_value * alpha) + (current_ema * (1.0 - alpha));
    }
    return current_ema;
}

double EMACalculator::getCurrentEMA() const {
    return current_ema;
}

bool EMACalculator::isInitialized() const {
    return initialized;
}

void EMACalculator::reset() {
    initialized = false;
    current_ema = 0.0;
}