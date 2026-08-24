#pragma once
#include "C:/glm/glm.hpp"
#include "C:/glm/gtc/matrix_transform.hpp"
#include <cmath>

namespace MathUtils {
    glm::mat2 RotationMatrix(double Radians) {
        double CosR = cos(Radians), SinR = sin(Radians);
        
        return glm::mat2(
            CosR, SinR,
        -SinR, CosR
    );
    }
    
    glm::mat2 RotationMatrixDeg(double Degrees) {
        double Radians = glm::radians(Degrees);
        double CosR = cos(Radians), SinR = sin(Radians);
        
        return glm::mat2(
            CosR, SinR,
            -SinR, CosR
        );
    }
}