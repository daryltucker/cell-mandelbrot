#include "fractal.h"
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <stdio.h>

typedef unsigned long long uint64;
typedef unsigned int uint32;


int main(uint64 spe_id, uint64 fractal_parameter_ea)
{
    fractal_parameters parameters;

    // Ladataan parametrit...
    mfc_write_tag_mask(1<<0);  //DMA-tunniste = 0
    mfc_get(&parameters, fractal_parameter_ea, sizeof(fractal_parameters), 0, 0, 0);
    spu_mfcstat(MFC_TAG_UPDATE_ALL); // Ja odotellaan niitä.
    
    printf("Saatiin mm. width: %u ja area_x: %u\n", parameters.width, parameters.area_x);
    return 0;
}
