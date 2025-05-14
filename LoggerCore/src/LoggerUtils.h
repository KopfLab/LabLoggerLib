#pragma once

#include "Particle.h"

namespace LoggerUtils {
 
    /**
     * @brief This checks numbers for whether they are not a number and returns the correct Variant.
     * @param x number
     * @return Variant object representing NULL (i.e. coded as null in JSON later) if x is NaN, or a double Variant with x if x is a valid number
     */
    Variant checkNaN(double x) {
        if (std::isnan(x)) return(Variant());
        return(Variant(x));
    };

}