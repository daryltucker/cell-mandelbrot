#include "fractal.h"
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_TRANSFER_SIZE 16384


int main(uint64 spe_id, uint64 fractal_parameter_ea)
{
    fractal_parameters parameters;
    char image_buffer[MAX_TRANSFER_SIZE];
    uint32 message;

    message = spu_readch(SPU_RdInMbox);

    // Ladataan parametrit:
    mfc_write_tag_mask(1<<0);  //DMA-tunniste = 0
    mfc_get(&parameters, fractal_parameter_ea, sizeof(fractal_parameters), 0, 0, 0);
    spu_mfcstat(MFC_TAG_UPDATE_ALL); // Ja odotellaan niitä.

    printf("Saatiin mm. width: %u, height: %u ja area_x: %u\n",
	   parameters.width, parameters.height, parameters.area_x);

    /*
     * TJ:
     * 
     * Jos oma siivu ei sovi puskuriin, niin piirretään kerrallaan
     * niin paljon kuin sopii, ja siirretään piirretty osuus
     * päämuistiin.
     *
     * Tietty ongelman voisi myös hoitaa PPU:sta käsin, säikeiden
     * kanssa vekslaamalla. Mutta varmaan olis tehokkaampi jos vain
     * yksi säie hoitaa kokonaan oman osuutensa niin ei tule turhia
     * säikeiden käynnistyksiä. Ja pääohjelmassa jaettaisiin kuva niin
     * moneen osaan kuin on säikeitä.
     *
     * Tulipa mieleen että voisiko tässä olla jonkinmoinen
     * kaksoispuskurointi?  Eli onkin kaksi puolet pienempää image
     * bufferia. Kun toinen on pistetty menemään päämuistiin niin
     * toiseen ruvettais heti piirtämään. Luulisi että se olisi
     * yksinkertainen ja tehokas ratkaisu. Tietysti edelleen pitää
     * olla tarkistus että joko puskuri on kopioitu päämuistiin.
     *
     * Ja eri puskurien siirroilla pitäis olla oma tagi, että
     * tiedetään odottaa oikeeta siirtoa.
     */

    // LOOP:

    //     Odotellaan edellinen siirto valmiiksi

    //     Piirretään fraktaali puskuriin...

    //     Siirretään data päämuistiin...


    //Piirretään yksi osa kuvasta noin kokeeksi:
    drawMandelbrotArea( 50, 50,
                        0.0f, 0.0f, 1.0f, 100, image_buffer,
                        0, 0, 50, 50,
                        (uint) parameters.bytes_per_pixel );

    /* Kovasti pukkaa varoitusta image_buffer-parametrista, ei auta
     * vaikka castaa void*:ksi.
     */
    mfc_put(image_buffer,
	    parameters.image,
	    //parameters.width*parameters.height*parameters.bytes_per_pixel,
	    MAX_TRANSFER_SIZE,
	    0, 0, 0);

    // Odotellaan kaikki siirrot valmiiksi, varmuuden vuoksi.
    spu_mfcstat(MFC_TAG_UPDATE_ALL);
    return 0;
}
