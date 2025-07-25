#pragma once

class EMACalculator {
private:
    double alpha;
    double current_ema;
    bool initialized;

public:
    EMACalculator(double smoothing_factor = 0.2);
    
    double update(double new_value);
    double getCurrentEMA() const;
    bool isInitialized() const;
    void reset();
    
    // For testing
    double getAlpha() const { return alpha; }
};
