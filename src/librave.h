#ifndef VOL2BIRD_LIBRAVE_H
#define VOL2BIRD_LIBRAVE_H

#include <string>
#include <ostream>
extern "C" {
#include <math.h>
#include "rave_object.h"
#include "rave_io.h"
#include "polarvolume.h"
#include "hlhdf.h"
#include "libvol2bird/libvol2bird.h"
}
namespace vol2birdR {
namespace librave {
#include <stdint.h>
#ifndef LIBRAVE_CPP
#define LIBRAVE_EXTERN extern
#else
#define LIBRAVE_EXTERN
#endif

}
}

#endif
