#include "fractal.h"
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_TRANSFER_SIZE 16384


unsigned int my_id;
char image_buffer[MAX_TRANSFER_SIZE];


int main(uint64 spe_id, uint64 fractal_parameter_ea)
{
    fractal_parameters p;
    uint32 message;

    my_id = (unsigned int) spe_id;

    // Odotetaan lähtölupaa
    message = spu_readch(SPU_RdInMbox);

    // Ladataan parametrit:
    mfc_write_tag_mask(1<<0);  //DMA-tunniste = 0
    mfc_get(&p, fractal_parameter_ea, sizeof(fractal_parameters), 0, 0, 0);
    spu_mfcstat(MFC_TAG_UPDATE_ALL); // Ja odotellaan niitä.

    printf("SPU %u: area_width: %u, area_height: %u, "
           "area_x: %u, area_y: %u\n", my_id,
	   p.area_width, p.area_heigth,
           p.area_x, p.area_y);

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
    drawMandelbrotArea( p.width, p.height,
                        0.0f, 0.0f, 1.0f, 100, image_buffer,
                        p.area_x, p.area_y,
                        p.area_width, p.area_heigth,
                        (uint) p.bytes_per_pixel );

    uint32 my_area = p.area_width*p.area_heigth*p.bytes_per_pixel;

    /* Huom! tassa oletetaan etta oma osa kuvasta on koko kuvan
     * levyinen.
     */ 
    mfc_put( image_buffer,
             p.image + p.area_y*p.width*p.bytes_per_pixel,
             MIN(my_area, MAX_TRANSFER_SIZE),
             0, 0, 0 );

    // Odotellaan kaikki siirrot valmiiksi, varmuuden vuoksi.
    spu_mfcstat(MFC_TAG_UPDATE_ALL);
    return 0;
}
