#include "utils.h"
#include "gamelib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <iso646.h>


void nuke_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void terminal_redimensioner(int width, int height) {
    // Sequenza ANSI per ridimensionare la finestra
    printf("\e[8;%d;%dt", height, width);
}

void terminal_cleaner() {
    // Sposta il cursore in alto a sinistra e pulisce tutto
    printf("\033[H\033[J");
}

void game_title_printer() {
    terminal_cleaner();     // Pulizia del terminale

    FILE *f = fopen("title_img.txt", "r");  // Apertura del file in lettura
    char bufferDIM[1024];   // Array per memorizzare temp. una riga del file.

    // Controllo errori se il file non esiste o non può essere aperto
    if (f == NULL) {
        printf("ERRORE: Impossibile caricare title_img.txt\n");
        // Fallback: Viene stampato il nome del gioco
        printf(ROSSO "\nCOSESTRANE\n\n" RESET);
        return;
    }

    printf(ROSSO);  // Impostazione color del titolo

    // Ciclo di lettura e stampa del titolo.
    //      fgets() legge dal file e mette i caratteri in bufferDIM.
    //      Il ciclo di ferma se legge (sizeof(bufferDIM) - 1) o "\n".
    //      Restituisce NULL quando arriva all'EOF.
    while (fgets(bufferDIM, sizeof(bufferDIM), f) != NULL) {
        printf("%s", bufferDIM);
    }

    printf(RESET "\n");

    fclose(f);
}

void printf_centered(const char* color, const char* ftext, ...) {
    char temp_ftext[1024];
    va_list args;
    va_start(args, ftext);

    vsnprintf(temp_ftext, sizeof(temp_ftext), ftext, args);

    va_end(args);

    const char* text = temp_ftext;


    int total_length = t_length;
    int text_length = strlen(text);
    int spaces = (total_length - text_length) / 2;

    // Se il testo è più lungo della riga, non stampiamo spazi negativi
    if (spaces < 0) spaces = 0;

    // Stampa gli spazi di padding a sinistra
    for (int i = 0; i < spaces; i++) {
        printf(" ");
    }

    // Imposta il colore
    if (color != NULL) {
        printf("%s", color);
    }

    // Stampa il testo
    printf("%s", text);

    // Resetta il colore e va a capo
    if (color != NULL) {
        printf(RESET);
    }
    printf("\n");
}

void wait_4click(const char* text) {
    printf_centered(ROSSO, text);
    nuke_buffer();
}

void player_abilities(giocatore* player) {
    printf("LE TUE ABILITA':\n");
    printf("[%d] Attacco Psichico.\n", player->attacco_psichico);
    printf("[%d] Difesa Psichica.\n", player->difesa_psichica);
    printf("[%d] Fortuna.\n\n", player->fortuna);
}


char* order_color(const uint8_t p_order, const uint8_t p_id) {
    if (p_order == p_id) {
        return H_YELLOW;
    }
    else return RESET;
}



const char* get_nome_zona(uint8_t zona) {
    switch(zona) {
        case bosco: return "Bosco";
        case scuola: return "Scuola";
        case laboratorio: return "Laboratorio";
        case caverna: return "Caverna";
        case strada: return "Strada";
        case giardino: return "Giardino";
        case supermercato: return "Supermercato";
        case centrale_elettrica: return "Centrale Elettrica";
        case deposito_abbandonato: return "Deposito";
        case stazione_polizia: return "Stazione Polizia";
        default: return "Sconosciuto";
    }
}

const char* get_nome_nemico(uint8_t nemico) {
    switch(nemico) {
        case nessun_nemico: return "Nessuno";
        case billi: return "Billi";
        case democane: return "Democane";
        case demotorzone: return "Demotorzone";
        default: return "?";
    }
}

const char* get_nome_oggetto(uint8_t oggetto) {
    switch(oggetto) {
        case nessun_oggetto: return "Nessuno";
        case bicicletta: return "Bicicletta";
        case maglietta_fuocoinferno: return "Maglietta";
        case bussola: return "Bussola";
        case schitarrata_metallica: return "Schitarrata";
        default: return "?";
    }
}