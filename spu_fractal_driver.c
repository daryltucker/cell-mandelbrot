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

    // Odotetaan l�ht�lupaa
    message = spu_readch(SPU_RdInMbox);

    // Ladataan parametrit:
    mfc_write_tag_mask(1<<0);  //DMA-tunniste = 0
    mfc_get(&p, fractal_parameter_ea, sizeof(fractal_parameters), 0, 0, 0);
    spu_mfcstat(MFC_TAG_UPDATE_ALL); // Ja odotellaan niit�.

    printf("SPU %u: area_width: %u, area_height: %u, "
           "area_x: %u, area_y: %u\n", my_id,
	   p.area_width, p.area_heigth,
           p.area_x, p.area_y);

    /*
     * TJ:
     * 
     * Jos oma siivu ei sovi puskuriin, niin piirret��n kerrallaan
     * niin paljon kuin sopii, ja siirret��n piirretty osuus
     * p��muistiin.
     *
     * Tietty ongelman voisi my�s hoitaa PPU:sta k�sin, s�ikeiden
     * kanssa vekslaamalla. Mutta varmaan olis tehokkaampi jos vain
     * yksi s�ie hoitaa kokonaan oman osuutensa niin ei tule turhia
     * s�ikeiden k�ynnistyksi�. Ja p��ohjelmassa jaettaisiin kuva niin
     * moneen osaan kuin on s�ikeit�.
     *
     * Tulipa mieleen ett� voisiko t�ss� olla jonkinmoinen
     * kaksoispuskurointi?  Eli onkin kaksi puolet pienemp�� image
     * bufferia. Kun toinen on pistetty menem��n p��muistiin niin
     * toiseen ruvettais heti piirt�m��n. Luulisi ett� se olisi
     * yksinkertainen ja tehokas ratkaisu. Tietysti edelleen pit��
     * olla tarkistus ett� joko puskuri on kopioitu p��muistiin.
     *
     * Ja eri puskurien siirroilla pit�is olla oma tagi, ett�
     * tiedet��n odottaa oikeeta siirtoa.
     */

    // LOOP:

    //     Odotellaan edellinen siirto valmiiksi

    //     Piirret��n fraktaali puskuriin...

    //     Siirret��n data p��muistiin...


    //Piirret��n yksi osa kuvasta noin kokeeksi:
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
