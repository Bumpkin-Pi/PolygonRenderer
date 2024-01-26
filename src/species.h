//
// Created by holly on 25/01/24.
//

#ifndef TWINSTICKSDL_SPECIES_H
#define TWINSTICKSDL_SPECIES_H
#include <SDL2/SDL.h>

namespace species{
    namespace position{
        struct TwoD{
            float x;
            float y;
        };
        struct ThreeD{
            float x;
            float y;
            float z;
        };
    }
    struct Polygon{
        position::ThreeD a;
        position::ThreeD b;
        position::ThreeD c;
        SDL_Color color;
    };
    struct Triangle{
        position::TwoD a;
        position::TwoD b;
        position::TwoD c;
        SDL_Color color;
    };
}


#endif //TWINSTICKSDL_SPECIES_H
