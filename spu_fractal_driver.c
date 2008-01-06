#include "fractal.h"
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <stdio.h>

typedef unsigned long long uint64;
typedef unsigned int uint32;

#define MAX_TRANSFER_SIZE 16384


int main(uint64 spe_id, uint64 fractal_parameter_ea)
{
    fractal_parameters parameters;
    char image_buffer[MAX_TRANSFER_SIZE];

    // Ladataan parametrit:
    mfc_write_tag_mask(1<<0);  //DMA-tunniste = 0
    mfc_get(&parameters, fractal_parameter_ea, sizeof(fractal_parameters), 0, 0, 0);
    spu_mfcstat(MFC_TAG_UPDATE_ALL); // Ja odotellaan niit�.
    
    printf("Saatiin mm. width: %u ja area_x: %u\n", parameters.width, parameters.area_x);

    /*
     * Jos oma siivu ei sovi puskuriin, niin piirret��n kerrallaan
     * niin paljon kuin sopii, ja siirret��n piirretty osuus
     * p��muistiin.
     *
     * Tietty ongelman voisi my�s hoitaa PPU:sta k�sin, s�ikeiden
     * kanssa vekslaamalla. Mutta varmaan olis tehokkaampi jos vain
     * yksi s�ie tekee oman osuutensa niin ei tule turhia s�ikeiden
     * k�ynnistyksi�.
     */

    // LOOP:

    //     Piirret��n fraktaali puskuriin...

    //     Siirret��n data p��muistiin...

    // Odotellaan kaikki siirrot valmiiksi, varmuuden vuoksi.
    spu_mfcstat(MFC_TAG_UPDATE_ALL);
    return 0;
}
