#include "gamelib.h"
#include "utils.h"

#include <time.h>   // Per generazione di numeri casuali.
#include <unistd.h>     // Per la funzione sleep()
#include <iso646.h>
#include <stdio.h>

// Per i Giocatori.
static giocatore* giocatori[4];
static uint8_t n_giocatori;

// Per la Mappa di gioco | Inizializzazione a NULL per lasciare dopo la 
static struct zona_mondoreale* prima_zona_mondoreale = NULL;
static struct zona_soprasotto* prima_zona_soprasotto = NULL;

// Variabile come flag per verificare se la mappa è stata chiusa correttamente.
static uint8_t mappa_chiusa = 0;    // Inizializzata su no

// Contatore di zone utile alle condizioni per l'input utente
static uint8_t n_zone;

// Flag per verificare se il giocatore ha combattuto durante il turno
static uint8_t combattuto = 0;  // Inizializzata su No per debugging, tuttavia se non da problemi potrebbe anche rimanere inizializzata qui

static uint8_t movimento = 0;

// Prototipi funzioni interne a gamelib.c (non pubbliche)

// Prototipi generalmente per imposta_gioco()
static void genera_mappa();
static void inserisci_zona();
static void cancella_zona();
static void stampa_mappa();
static void stampa_zona();
static void chiudi_mappa();
static void dealloca_mappe(); // Utile per pulire prima di rigenerare

static void stampa_zona_giocatore();
static void avanza();
static void indietreggia();
static void cambia_mondo();
/*
static void combatti();
static void stampa_giocatore();
static void raccogli_oggetto();
static void utilizza_oggetto();
static void passa();
*/


void imposta_gioco() {

    // Deallocazione giocatori precedenti (se ce n'erano)
    if (giocatori[0] != NULL) {
        for (int i = 0; i < 4; i++) {
            if (giocatori[i] != NULL) {
                free(giocatori[i]->nome);
                free(giocatori[i]);
                giocatori[i] = NULL;
            }
        }
    }

    // Deallocazione mappa precedente (se ce n'erano, il controllo è dentro la funzione)
    dealloca_mappe();
    mappa_chiusa = 0;

    
    do{
        n_giocatori = 0;
        terminal_cleaner();
        printf_centered(RESET, "Quante persone giocano?\n");
        printf("| ");
        scanf("%hhu", &n_giocatori);
        nuke_buffer();
        switch(n_giocatori) {
            case 1: case 2: case 3: case 4:
                break;

            default:
                printf_centered(RESET, "L'opzione scelta non è valida o non esiste.\n");
                printf_centered(RESET, "Riprovare.\n");
                sleep(1);
                break;
        }
    } while((n_giocatori < 1) or (n_giocatori > 4));


    char temp_playername[player_name_length];
    temp_playername[0] = '\0';  // Inizializzazione sicura

    for(uint8_t i = 0; i < n_giocatori; i++) {
        giocatori[i] = (giocatore*) calloc(1, sizeof(giocatore));


        do{
            terminal_cleaner();
            printf_centered(RESET, "Giocatore %d inserisci il tuo nickname:", i + 1);
            printf("\n| ");

            if (fgets(temp_playername, sizeof(temp_playername), stdin) != NULL) {
                
                // Rimuovo il '\n' finale che fgets cattura
                temp_playername[strcspn(temp_playername, "\n")] = 0;
            }
            
            if (strlen(temp_playername) == 0 or temp_playername[0] == ' ') {
                printf_centered(RESET, "Devi inserire almeno un carattere e non iniziare con uno spazio.\n");
                printf_centered(RESET, "Riprovare.\n");
                sleep(1);
            }

        } while(strlen(temp_playername) == 0 or temp_playername[0] == ' ');

        giocatori[i]->nome = (char*) calloc(strlen(temp_playername) + 1, 1);
        strcpy(giocatori[i]->nome, temp_playername);


        printf_centered(RESET, "È il momento di sorteggiare le tue abilità.\n");
        printf_centered(RESET, "%s, premi INVIO per lanciare 3 dadi!\n", giocatori[i]->nome);
        wait_4click("[Lancia!]");

        uint8_t dadi[3];
        for(uint8_t i = 0; i < len(dadi); i++) {
            dadi[i] = (rand() % 20) + 1;
        }

        giocatori[i]->attacco_psichico = dadi[0];
        giocatori[i]->difesa_psichica = dadi[1];
        giocatori[i]->fortuna = dadi[2];


        uint8_t option;
        do{
            option = 99;
            terminal_cleaner();
            uint8_t break_flag = 0;
            if (i > 0) {
                for (int j = 0; j < i; j++) {
                    if (strcmp(giocatori[j]->nome, "UndiciVirgolaCinque") == 0) {
                        break_flag = 1;
                        break;
                    }
                }
            }
            if (break_flag == 1) {
                printf_centered(RESET, "Il ruolo \"UndiciVirgolaCinque\" è stato già preso...\n");
                wait_4click("Premi INVIO per continuare.");
                break;
            }
            
            
            player_abilities(giocatori[i]);
            wait_4click(RESET "Premi INVIO");

            printf_centered(RESET, "\nPERSONALIZZA LE TUE ABILITA' (1/2):\n\n");
            printf("Lo Status \"UndiciVirgolaCinque\" ti conferirebbe:\n");
            printf("    [+4] Attacco Psichico.\n");
            printf("    [+4] Difesa Psichica.\n");
            printf("    [-7] Fortuna.\n\n");

            printf_centered(RESET, "Questo status può essere scelto da un solo giocatore e cambierà il suo nome.\n");
            printf_centered(RESET, "Vuoi essere tu ad attivarlo?\n\n");
            printf_centered(RESET, "(0) No.\n");
            printf_centered(RESET, "(1) Si.\n");
            
            printf("\n| ");
            scanf("%hhu", &option);
            nuke_buffer();

            switch (option) {
                case 0:
                    break;
                
                case 1:
                    giocatori[i]->nome = realloc(giocatori[i]->nome, player_name_length);
                    strcpy(giocatori[i]->nome, "UndiciVirgolaCinque");

                    giocatori[i]->attacco_psichico += 4;
                    giocatori[i]->difesa_psichica += 4;
                    giocatori[i]->fortuna -= 7;
                    break;

                default:
                    printf_centered(RESET, "L'opzione scelta non è valida o non esiste.\n");
                    printf_centered(RESET, "Riprovare.\n");
                    break;
            }

        } while (option > 1);

        do{
            terminal_cleaner();
            option = 99;

            player_abilities(giocatori[i]);

            printf("PERSONALIZZA LE TUE ABILITA':\n\n");
            printf_centered(RESET, "(OPZIONE 0) [+3] Attacco Psichico » [-3] Difesa Psichica.\n");
            printf_centered(RESET, "(OPZIONE 1) [+3] Difesa Psichica » [-3] Attacco Psichico.\n");

            printf("| ");
            scanf("%hhu", &option);
            nuke_buffer();

            switch (option) {
            case 0:
                giocatori[i]->attacco_psichico += 3;
                giocatori[i]->difesa_psichica -= 3;
                break;

            case 1:
                giocatori[i]->difesa_psichica += 3;
                giocatori[i]->attacco_psichico -= 3;
                break;
            
            default:
                printf_centered(RESET, "L'opzione scelta non è valida o non esiste.\n");
                printf_centered(RESET, "Riprovare.\n");            
                break;
            }

        } while (option > 1); 

        for (int j = 0; j < 3; j++) {
            giocatori[i]->zaino[j] = nessun_oggetto;
        }
        
        
        
    }
    

    uint8_t gm_ticket;
    do {
        gm_ticket = 99;
        terminal_cleaner();

        printf_centered(RESET, "Volete accedere al Game Master Menu'?\n");
        printf_centered(RESET, "(0) No.\n");
        printf_centered(RESET, "(1) Si.\n");
        printf("| ");
        scanf("%hhu", &gm_ticket);
        nuke_buffer();  
        
        switch (gm_ticket) {
            case 0:
                genera_mappa();
                stampa_mappa();
                chiudi_mappa();
                break;
            
            case 1:
                uint8_t choice_gmo;

                do {
                    choice_gmo = 99;
                    terminal_cleaner();
                    
                    printf_centered(RESET, "Benvenuti nel Game Master Menu'!\n");
                    printf_centered(RESET, "Qui potete personalizzare la mappa di gioco come desiderate!\n");
                    printf_centered(RESET, "============================================================\n");
                    printf_centered(RESET, "Scegli un'opzione:\n");
                    printf_centered(RESET, "(1) Genera Mappa.\n");
                    printf_centered(RESET, "(2) Inserisci nuova Zona.\n");
                    printf_centered(RESET, "(3) Cancella una Zona\n");
                    printf_centered(RESET, "(4) Visualizza Mappa completa.\n");
                    printf_centered(RESET, "(5) Visualizza i dettagli di una Zona specifica\n");
                    printf_centered(RESET, "(6) Salva le impostazioni ed esci dal Game Master Menu'.\n");
                    printf("\n| ");
                    scanf("%hhu", &choice_gmo);
                    nuke_buffer();
                    printf("\n");

                    switch (choice_gmo) {
                    case 1:
                        genera_mappa();
                        break;

                    case 2:
                        inserisci_zona();
                        break;

                    case 3:
                        cancella_zona();
                        break;

                    case 4:
                        stampa_mappa();
                        break;

                    case 5:
                        stampa_zona();
                        break;

                    case 6:
                        chiudi_mappa();
                        break;

                    default:
                        printf_centered(RESET, "L'opzione scelta non è valida o non esiste.\n");
                        printf_centered(RESET, "Riprovare.\n");
                        break;
                    }

                } while (mappa_chiusa != 1);
                break;

            default:
                printf_centered(RESET, "L'opzione scelta non è valida o non esiste.\n");
                printf_centered(RESET, "Riprovare.\n");
                break;
        }
    } while (gm_ticket > 1);
    


    // Posizionamento dei giocatori nella prima zona del mondo reale
    if (mappa_chiusa == 1) {
        for (uint8_t i = 0; i < n_giocatori; i++) {
            giocatori[i]->mondo = 0; // 0 = Mondo Reale (Default) | 1 = Soprasotto
            
            // Posizionamento del giocatore nella prima zona del mondo reale.
            giocatori[i]->pos_mondoreale = prima_zona_mondoreale;
            
            /*
            // Per sicurezza colleghiamo anche il soprasotto, anche se sono nel reale
            if (prima_zona_mondoreale != NULL) {
                giocatori[i]->pos_soprasotto = prima_zona_mondoreale->link_soprasotto;
            }
            */
        }
        printf_centered(VERDE, "Giocatori posizionati all'ingresso di Occhinz!\n");
    } else {
        // Se l'utente esce dal menu senza validare la mappa (es. se forza l'uscita o errore)
        printf_centered(ROSSO, "Attenzione: Mappa non validata. Impossibile posizionare i giocatori nella mappa.\n");
    }
    sleep(1);
    terminal_cleaner();
}


static void dealloca_mappe() {
    // Scorre le liste e libera la memoria (da fare per evitare memory leak)
    struct zona_mondoreale* current_r = prima_zona_mondoreale;
    struct zona_soprasotto* current_s = prima_zona_soprasotto;
    
    while (current_r != NULL) {
        struct zona_mondoreale* next = current_r->avanti;
        free(current_r);
        current_r = next;
    }
    while (current_s != NULL) {
        struct zona_soprasotto* next = current_s->avanti;
        free(current_s);
        current_s = next;
    }
    prima_zona_mondoreale = NULL;
    prima_zona_soprasotto = NULL;

    n_zone = 0;
}


static void genera_mappa() {
    //TODO: Aggiornare con la macro lenum()
    
    if (prima_zona_mondoreale != NULL) {
        printf("Cancellazione mappa precedente...\n");
        dealloca_mappe();
        mappa_chiusa = 0;
    }

    struct zona_mondoreale* precedente_zona_mondoreale = NULL;
    struct zona_soprasotto* precedente_zona_soprasotto = NULL;

    uint8_t pos_demotorzone = rand() % 15;  
    
    for (uint8_t i = 0; i < 15; i++) {
        struct zona_mondoreale* nuova_zona_mondoreale = (struct zona_mondoreale*) calloc(1, sizeof(struct zona_mondoreale));
        struct zona_soprasotto* nuova_zona_soprasotto = (struct zona_soprasotto*) calloc(1, sizeof(struct zona_soprasotto));
        
        // Setting del tipo di zona a ogni iterazione (il tipo deve essere lo stesso tra mondoreale e soprasotto corrispondenti)
        tipo_zona zona = (tipo_zona) (rand() % 10);
        nuova_zona_mondoreale->zona = zona;
        nuova_zona_soprasotto->zona = zona;

        // Setting dei link "verticali"
        nuova_zona_mondoreale->link_soprasotto = nuova_zona_soprasotto;
        nuova_zona_soprasotto->link_mondoreale = nuova_zona_mondoreale;
        
        // Setting degli oggetti (solo nel mondoreale)
        tipo_oggetto oggetto = (tipo_oggetto) (rand() % 5);
        nuova_zona_mondoreale->oggetto = oggetto;

        // Setting del demotorzone all'indice corrispondente a al numero generato casualmente prima del ciclo
        if(i == pos_demotorzone) {
            nuova_zona_soprasotto->nemico = demotorzone;
        }
        else {  // Setting degli altri nemici (billi sono nel mondoreale)
            tipo_nemico nemico = (tipo_nemico) (rand() % 3);
            if(nemico == billi) {
                nuova_zona_mondoreale->nemico = nemico;
                nuova_zona_soprasotto->nemico = nessun_nemico;
            }
            else {
                nuova_zona_mondoreale->nemico = nemico;
                nuova_zona_soprasotto->nemico = nemico;
            }
        }


        // Setting della prima zona in entrambi i mondi al primo indice della lista
        if(i == 0) {
            prima_zona_mondoreale = nuova_zona_mondoreale;
            prima_zona_soprasotto = nuova_zona_soprasotto;

            nuova_zona_mondoreale->indietro = NULL;
            nuova_zona_soprasotto->indietro = NULL;
        }
        else {  // Ossia i > 0 in quanto il ciclo parte da zero in senso crescente
            precedente_zona_mondoreale->avanti = nuova_zona_mondoreale;     // Collegamento del nuovo davanti al vecchio
            nuova_zona_mondoreale->indietro = precedente_zona_mondoreale;   // Collegamento del nuovo, dietro al vecchio

            precedente_zona_soprasotto->avanti = nuova_zona_soprasotto;     // Collegamento del nuovo, davanti al vecchio
            nuova_zona_soprasotto->indietro = precedente_zona_soprasotto;   // Collegamento del nuovo, dietro al vecchio
        }

        // Aggiornamento della zona precedente prima dell'iterazione successiva
        precedente_zona_mondoreale = nuova_zona_mondoreale;
        precedente_zona_soprasotto = nuova_zona_soprasotto;
    }
    n_zone = 15;
    
    printf_centered(RESET, "Mappa generata con successo");
    wait_4click("Premi INVIO per continuare.");
}


// Permette l'inserimento a piacere da parte dell'utente di una nuova zona e le sue caratteristiche
static void inserisci_zona() {
    if(prima_zona_mondoreale == NULL) {
        printf_centered(ROSSO, "Errore: La mappa di gioco non esiste.\n");
        printf_centered(ROSSO, "Generala dal Game Master Menu'.\n");
        sleep(1);
        return;
    }

    uint8_t choice = 0;
    do {
        terminal_cleaner();
        printf_centered(RESET, "Quale vuoi sia il numero della zona? (1 - %hhu)\n", n_zone + 1);
        printf("| ");
        scanf("%hhu", &choice);
        nuke_buffer();
    
        if (choice > 0 and choice <= (n_zone + 1)) {
    
            // Allocazione in memoria dinamica/heap delle nuove zone.
            struct zona_mondoreale* new_z = (struct zona_mondoreale*) calloc(1, sizeof(struct zona_mondoreale));
            struct zona_soprasotto* new_zs = (struct zona_soprasotto*) calloc(1, sizeof(struct zona_soprasotto));
    
            // Link verticale subito
            new_z->link_soprasotto = new_zs;
            new_zs->link_mondoreale = new_z;
    
            // Verifica della presenza del demotorzone
            uint8_t demotorzone_flag = 0;
            struct zona_soprasotto* zs_check = prima_zona_soprasotto;
    
            while(zs_check != NULL) {
                if (zs_check->nemico == demotorzone) {
                    demotorzone_flag = 1;
                    break;
                }
                zs_check = zs_check->avanti;
            }
    
    
            uint8_t choice_zona, choice_nemico_z, choice_nemico_zs, choice_oggetto;
            
            // Input TIPO ZONA
            do {
                choice_zona = 0;
                terminal_cleaner();
                printf_centered(RESET, "Decidi il Tipo di Zona:\n");
                for(uint8_t i = 0; i < lenum(tipo_zona); i++) {
                    printf_centered(RESET, "[%hhu] %s\n", i + 1, get_nome_zona(i));
                }
                printf("| ");
                scanf("%hhu", &choice_zona);
                nuke_buffer();
                printf("\n");
        
                if (choice_zona > 0 and choice_zona <= lenum(tipo_zona)) {
                    new_z->zona = (tipo_zona) choice_zona - 1;
                    new_zs->zona = (tipo_zona) choice_zona - 1;
                }
                else {
                    printf_centered(RESET, "L'opzione scelta non è valida o non esiste.\n");
                    printf_centered(RESET, "Riprovare.\n");
                    sleep(1);
                }
            } while (choice_zona == 0 or choice_zona > lenum(tipo_zona));
            
    
            
            // Input TIPO_NEMICO | MONDO REALE
            do {
                choice_nemico_z = 0;
                terminal_cleaner();
                printf_centered(RESET, "Decidi il Nemico presente nella Zona %hhu del Mondo Reale:\n", choice);
                for(uint8_t i = 0; i < lenum(tipo_nemico); i++) {
                    if (i == 1) {
                        printf_centered(RESET, "[%hhu] %s (Solo nel Mondo Reale)\n", i + 1, get_nome_nemico(i));
                    }
                    else if (i < 3) {
                        printf_centered(RESET, "[%hhu] %s\n", i + 1, get_nome_nemico(i));
                    }
                }
                printf("| ");
                scanf("%hhu", &choice_nemico_z);
                nuke_buffer();
                printf("\n");
        
                if (choice_nemico_z > 0 and choice_nemico_z <= (lenum(tipo_nemico) - 1)) {
                    new_z->nemico = (tipo_nemico) choice_nemico_z - 1;
                }
                else {
                    printf_centered(RESET, "L'opzione scelta non è valida o non esiste.\n");
                    printf_centered(RESET, "Riprovare.\n");
                    sleep(1);
                }
            } while (choice_nemico_z == 0 or choice_nemico_z > (lenum(tipo_nemico) - 1));
            
    
    
            // Input  TIPO_NEMICO | SOPRASOTTO
            do {
                choice_nemico_zs = 0;
                terminal_cleaner();
                uint8_t count = 0;
                printf_centered(RESET, "Decidi il Nemico presente nella Zona %hhu del Sopra-Sotto:\n", choice);
                for(uint8_t i = 0; i < lenum(tipo_nemico); i++) {
                    count++;
                    if (i == 1) continue;
                    if (i == 3) {
                        if (demotorzone_flag == 0) {
                            printf_centered(RESET, "[%hhu] %s (SOLO 1 PER MAPPA, Non è in altre zone)\n", i + 1, get_nome_nemico(i));
                        }
                        else {
                            printf_centered(ROSSO, "Il %s è già presente in mappa. Se vuoi generarlo elimina quello attuale.\n", get_nome_nemico(i));
                        }
                    }
                    if (i != 1 and i != 3) {
                        printf_centered(RESET, "[%hhu] %s\n", i + 1, get_nome_nemico(i));
                    }
                }
                printf("| ");
                scanf("%hhu", &choice_nemico_zs);
                nuke_buffer();
                printf("\n");
        
                if (choice_nemico_zs > 0 and choice_nemico_zs <= (lenum(tipo_nemico) - demotorzone_flag)) {
                    if (choice_nemico_zs == 2) choice_nemico_zs++;
                    if (choice_nemico_zs == 4 and demotorzone_flag == 0) {
                        new_zs->nemico = (tipo_nemico) demotorzone;
                    }
                    if (choice_nemico_zs != 2 and choice_nemico_zs != 4) {
                        new_zs->nemico = (tipo_nemico) choice_nemico_zs - 1;
                    }
                }
                else {
                    printf_centered(RESET, "L'opzione scelta non è valida o non esiste.\n");
                    printf_centered(RESET, "Riprovare.\n");
                    sleep(1);
                }
            } while (choice_nemico_zs == 0 or choice_nemico_zs > (lenum(tipo_nemico) - demotorzone_flag));
            
    
    
            // Input TIPO_OGGETTO
            do {
                choice_oggetto = 0;
                terminal_cleaner();
                printf_centered(RESET, "Decidi l'Oggetto della Zona (Solo Mondo Reale):\n");
                for(uint8_t i = 0; i < lenum(tipo_oggetto); i++) {
                    printf_centered(RESET, "[%hhu] %s\n", i + 1, get_nome_oggetto(i));
                }
                printf("| ");
                scanf("%hhu", &choice_oggetto);
                nuke_buffer();
                printf("\n");
        
                if (choice_oggetto > 0 and choice_oggetto <= lenum(tipo_oggetto)) {
                    new_z->oggetto = (tipo_oggetto) choice_oggetto - 1;
                }
                else {
                    printf_centered(RESET, "L'opzione scelta non è valida o non esiste.\n");
                    printf_centered(RESET, "Riprovare.\n");
                    sleep(1);
                }
            } while (choice_oggetto == 0 or choice_oggetto > lenum(tipo_oggetto));
            
    
            
    
            // Inserimento della nuova zona nella linked-list.
            if (choice == 1) {
                new_z->avanti = prima_zona_mondoreale;
                new_zs->avanti = prima_zona_soprasotto;
    
                new_z->indietro = NULL;
                new_zs->indietro = NULL;
    
                prima_zona_mondoreale->indietro = new_z;
                prima_zona_soprasotto->indietro = new_zs;
    
                prima_zona_mondoreale = new_z;
                prima_zona_soprasotto = new_zs;
            }
            else {
                struct zona_mondoreale* nav_z = prima_zona_mondoreale;
                struct zona_soprasotto* nav_zs = prima_zona_soprasotto;
    
                //! Tenere d'occhio questo ciclo e in particolare (choice - 2)
                // Questo ciclo fa puntare nav_z a (choice - 1)
                for(uint8_t i = 0; i < (choice - 2) and nav_z != NULL; i++) {
                    nav_z = nav_z->avanti;
                    nav_zs = nav_zs->avanti;
                }
    
                // Controllo per sicurezza.
                if (nav_z != NULL) {
                    // nav_z utilizzato come supporto essendo nella stessa posizione che deve diventare new_z
                    new_z->avanti = nav_z->avanti;
                    new_zs->avanti = nav_zs->avanti;
    
                    // Se la zona davanti a quella che deve diventare new_z esiste, allora la facciamo puntare indietro a new_z
                    if (nav_z->avanti != NULL) {
                        nav_z->avanti->indietro = new_z;
                        nav_zs->avanti->indietro = new_zs;
                    }
    
                    // Se la zona davanti a quella che deve diventare new_z non esiste, semplicemente vuol dire che new_z diventa l'ultima zona della mappa
                    // Ovviamente questo snippet lavora anche nei casi in cui la zona avanti esiste
                    nav_z->avanti = new_z;
                    nav_zs->avanti = new_zs;
    
                    // Ricucitura della nuova zona creata con quella che deve stargli dietro
                    new_z->indietro = nav_z;
                    new_zs->indietro = nav_zs;
                }
            }
            n_zone++;
    
            printf_centered(VERDE, "Zona inserita con successo!\n");
            sleep(1);
    
        }
        else {
            printf_centered(RESET, "L'opzione scelta non è valida o non esiste.\n");
            printf_centered(RESET, "Riprovare.\n");
            sleep(1);
        }


    } while (choice == 0 or choice > (n_zone + 1));
    
    

}

// Elimina una zona a scelta dall'utente
static void cancella_zona() {
    if(prima_zona_mondoreale == NULL) {
        printf_centered(ROSSO, "Errore: La mappa di gioco non esiste.\n");
        printf_centered(ROSSO, "Generala dal Game Master Menu'.\n");
        sleep(1);
        return;
    }

    uint8_t choice;
    do {
        if (n_zone <= 1) {
            printf_centered(RESET, "Hai eliminato il massimo di zone possibili!");
            terminal_cleaner();
            break;
        }
        
        choice = 0;
        terminal_cleaner();

        printf_centered(RESET, "Quale zona vuoi cancellare? (1 - %hhu)\n", n_zone);
        printf("| ");
        scanf("%hhu", &choice);
        nuke_buffer();
        printf("\n");
    
        if(choice > 0 and choice <= n_zone) {
            struct zona_mondoreale* del_z = prima_zona_mondoreale;
            struct zona_soprasotto* del_zs = prima_zona_soprasotto;
            
            // Il for fa raggiungere a del_z e del_zs la zona scelta dall'utente
            for (uint8_t i = 0; i < (choice - 1) and del_z != NULL; i++) {
                del_z = del_z->avanti;
                del_zs = del_zs->avanti;
            }
    
            if (del_z == NULL) {
                printf_centered(ROSSO, "Errore: La zona non esiste.\n");
                printf_centered(ROSSO, "Prova a rigenerare la mappa dal Game Master Menu'.\n");
                sleep(1);
                return;
            }
    
            // Ricucitura delle linked list
    
            if (del_z == prima_zona_mondoreale) {
                prima_zona_mondoreale = del_z->avanti;
                prima_zona_soprasotto = del_zs->avanti;
    
                if (prima_zona_mondoreale != NULL) {    // Controllo in caso sia l'ultima zona rimasta
                    prima_zona_mondoreale->indietro = NULL;
                    prima_zona_soprasotto->indietro = NULL;
                }
            }
            else {
                if (del_z->avanti == NULL) {
                    del_z->indietro->avanti = NULL;
                    del_zs->indietro->avanti = NULL;
                }
                else {
                    del_z->indietro->avanti = del_z->avanti;
                    del_zs->indietro->avanti = del_zs->avanti;
    
                    del_z->avanti->indietro = del_z->indietro;
                    del_zs->avanti->indietro = del_zs->indietro;
                }
            }
            free(del_z);
            free(del_zs);
            n_zone -= 1;
    
            printf_centered(VERDE, "La Zona %hhu cancellata con successo!\n", choice);
        }
        else {
            printf_centered(RESET, "L'opzione scelta non è valida o non esiste.\n");
            printf_centered(RESET, "Riprovare.\n");
        }
        sleep(1);
    } while ((choice == 0 or choice > n_zone) and n_zone > 1);
    
}

// Stampa di una mappa a scelta tra mondoreale o soprasotto
static void stampa_mappa() {
    if(prima_zona_mondoreale == NULL) {
        printf_centered(ROSSO, "Errore: La mappa di gioco non esiste.\n");
        printf_centered(ROSSO, "Generala dal Game Master Menu'.\n");
        sleep(1);
        return;
    }

    uint8_t choice;
    do {
        choice = 99;
        terminal_cleaner();
        
        printf_centered(RESET, "Quale mappa vuoi visualizzare?\n");
        printf_centered(RESET, "[0] Mondo Reale.\n");
        printf_centered(RESET, "[1] Soprasotto.\n");
        printf("| ");
        scanf("%hhu", &choice);
        nuke_buffer();
        printf("\n");
    
        switch(choice) {
        case 0:
            struct zona_mondoreale* p = prima_zona_mondoreale;
            printf_centered(VERDE, "===============< MAPPA MONDO REALE >===============\n");
            for (uint8_t i = 0; p != NULL; i++){
                printf_centered(RESET, "Zona [%hhu]: %s | Nemico: %s | Oggetto: %s\n",
                i + 1, get_nome_zona(p->zona), get_nome_nemico(p->nemico), get_nome_oggetto(p->oggetto));
                
                p = p->avanti;
            }
            break;
        
        case 1:
            struct zona_soprasotto* s = prima_zona_soprasotto;
            printf_centered(ROSSO, "===============< MAPPA SOPRA-SOTTO >===============\n");
            for (uint8_t i = 0; s != NULL; i++) {
                printf_centered(RESET, "Zona [%hhu]: %s | Nemico: %s\n",
                i + 1, get_nome_zona(s->zona), get_nome_nemico(s->nemico));
                
                s = s->avanti;
            }
            break;
            
        default:
            printf_centered(RESET, "L'opzione scelta non è valida o non esiste.\n");
            printf_centered(RESET, "Riprovare.\n");            
            break;
        }

    } while (choice > 1);
    
    wait_4click("\nPremi INVIO per continuare.\n");
}

// Stampa di una zona a scelta sia quella del mondoreale che la corrispondente nel soprasotto
static void stampa_zona() {
    if(prima_zona_mondoreale == NULL) {
        printf_centered(ROSSO, "Errore: La mappa di gioco non esiste.\n");
        printf_centered(ROSSO, "Generala dal Game Master Menu'.\n");
        sleep(1);
        return;
    }

    uint8_t choice;
    do {
        choice = 0;
        terminal_cleaner();

        printf_centered(RESET, "Quale zona vuoi ispezionare? (1 - %hhu)\n", n_zone);
        printf_centered(RESET, "Vedrai sia la zona del Mondo Reale che la sua corrispondente nel Sopra-Sotto.\n");
        printf("| ");
        scanf("%hhu", &choice);
        nuke_buffer();
        printf("\n");
    
        if(choice > 0 and choice <= n_zone) {
            struct zona_mondoreale* z = prima_zona_mondoreale;
            for (uint8_t i = 0; z != NULL and i < (choice - 1); i++) {
                z = z->avanti;
            }
    
            if (z == NULL) {
                printf_centered(ROSSO, "Errore: La zona non esiste.\n");
                printf_centered(ROSSO, "Prova a rigenerare la mappa dal Game Master Menu'.\n");
                sleep(1);
                return;
            }
            else {      // Stampa della zona
                // Recupero anche della zona corrispondente nel soprasotto
                struct zona_soprasotto* zs = z->link_soprasotto;
    
                printf_centered(VERDE, "============< ZONA %hhu | MONDO REALE >============\n", choice);
                printf_centered(RESET, "Tipo:       %s\n", get_nome_zona(z->zona));
                printf_centered(RESET, "Nemico:     %s\n", get_nome_nemico(z->nemico));
                printf_centered(RESET, "Oggetto:    %s\n", get_nome_oggetto(z->oggetto));
                printf_centered(RESET, "\n");
                printf_centered(ROSSO, "============< ZONA %hhu | SOPRA-SOTTO >============\n", choice);
                printf_centered(RESET, "Tipo:       %s\n", get_nome_zona(zs->zona));
                printf_centered(RESET, "Nemico:     %s\n", get_nome_nemico(zs->nemico));
            }
        }
        else {
            printf_centered(RESET, "L'opzione scelta non è valida o non esiste.\n");
            printf_centered(RESET, "Riprovare.\n");
            sleep(1);
        }
    } while (choice == 0 or choice > n_zone);
    
    wait_4click("\nPremi INVIO per continuare.\n");
}


static void chiudi_mappa() {
    if(prima_zona_mondoreale == NULL or prima_zona_soprasotto == NULL) {
        printf_centered(ROSSO, "Errore: La mappa di gioco non esiste.\n");
        printf_centered(ROSSO, "Generala dal Game Master Menu'.\n");
        sleep(1);
        return;
    }

    uint8_t count_zone = 0;
    uint8_t count_demotorzone = 0;
    struct zona_soprasotto* iter_zone_zs = prima_zona_soprasotto;

    while(iter_zone_zs != NULL) {
        count_zone++;
        if(iter_zone_zs->nemico == demotorzone) {
            count_demotorzone++;
        }
        iter_zone_zs = iter_zone_zs->avanti;
    }
    

    if(count_zone < 15) {
        printf_centered(ROSSO, "Errore: La mappa ha meno di 15 zone.\n");
        sleep(1);
        return;
    } else if(count_demotorzone != 1) {
        printf_centered(ROSSO, "Errore: Deve esserci esattamento un Demotorzone nella mappa.\n");
        printf_centered(ROSSO, "Attualmente ce ne sono: %d.\n", count_demotorzone);
        return;
    } else {

        printf("PALLE QUADRATE\n");
        // Aggiunta del numero della zona a tutte le zone
        struct zona_mondoreale* iter_zone_z = prima_zona_mondoreale;
        iter_zone_zs = prima_zona_soprasotto;

        count_zone = 0;
        while(iter_zone_z != NULL or iter_zone_zs != NULL) {
            count_zone++;
            iter_zone_z->id_zona = count_zone;
            iter_zone_zs->id_zona = count_zone;

            iter_zone_z = iter_zone_z->avanti;
            iter_zone_zs = iter_zone_zs->avanti;
        }


        mappa_chiusa = 1;
        printf(VERDE "Mappa validata.\n");
        wait_4click("Premi INVIO.\n");
        terminal_cleaner();
        return;
    }

    
}



void gioca() {
    // Controllo se la mappa esiste o se è chiusa
    if (mappa_chiusa != 1 or prima_zona_mondoreale == NULL or prima_zona_soprasotto == NULL) {
        terminal_cleaner();
        printf_centered(ROSSO, "Errore: La mappa non è stata generata correttamente.\n");
        printf_centered(ROSSO, "Torna alle impostazioni, genera la mappa ed infine chiudila.\n");
        printf(RESET "\n");
        sleep(1);
        terminal_cleaner();
        
        return;
    }

    // Controllo se esistono il numero consentito di giocatori
    if (n_giocatori < 1 or n_giocatori > 4) {
        terminal_cleaner();
        printf_centered(ROSSO, "Errore: Nessun giocatore presente.\n");
        printf_centered(ROSSO, "Torna alle impostazioni, genera la mappa ed infine chiudila.\n");
        printf(RESET "\n");
        sleep(1);
        terminal_cleaner();

        return;
    }

    uint8_t stato_partita = 0;   // 0 = In corso | 1 = Terminata (Tutti i giocatori sono morti)
    uint8_t ordine[n_giocatori];
    uint16_t ciclo = 0;
    uint8_t turno = n_giocatori + 1;


    while (stato_partita == 0) {
        

        if (turno > n_giocatori) {
            for(uint8_t i = 0; i < n_giocatori; i++) {
                ordine[i] = i + 1;
            }

            for (uint8_t i = 0; i < n_giocatori; i++) {
                // Estrazione casuale DI UN INDICE per ordine
                uint8_t j = randint(n_giocatori);

                // Scambio del "giocatore" nella posizione corrente con una posizione a caso
                uint8_t temp = ordine[i];
                ordine[i] = ordine[j];
                ordine[j] = temp;
            }

            turno = 1;
            ciclo++;
        }
        
        char choice;
        do{
            choice = '0';
            terminal_cleaner();

            combattuto = 0;
            movimento = 0;

            printf("<< Ciclo %hu >>\n", ciclo);
            printf("[");
            for (uint8_t i = 1; i <= len(ordine); i++){
                printf("%s%s" RESET, order_color(turno, i), giocatori[ordine[i - 1] - 1]->nome);
    
                if (i < n_giocatori) {
                    printf(" > ");
                }
                
            }
            printf("]\n");
            
            //TODO: Implementare la TUI, quindi le opzioni del turno con le chiamate alle funzioni

            printf("\n\n");
            stampa_zona_giocatore(giocatori[ordine[turno - 1] - 1]);

            char spa = 32;
            printf_centered(CIANO, "< SCEGLI UN OPZIONE >");
            printf("\n\n"),

            printf_centered(RESET, "Inventario");
            printf_centered(RESET, "[I]");
            printf_centered(RESET, "\t%cCombatti!  [J]   [L]  Raccogli Oggetto\t\t", spa);
            printf_centered(RESET, "[K]");
            printf_centered(RESET, "Utilizza Oggetto");
            printf("\n\n\n");

            // controllare il mondo del giocatore e switchare tra W per andare nel mondo reale e con S per andare nel soprasotto

            if (giocatori[ordine[turno - 1] - 1]->mondo == 1) {
                printf_centered(RESET, "Vai nel Mondo Reale");
                printf_centered(RESET, "[W]");
            }
            printf_centered(RESET, "Indietreggia  [A]   [D]  Avanza%c%c%c%c%c%c", spa, spa, spa, spa, spa, spa);
            if (giocatori[ordine[turno - 1] - 1]->mondo == 0) {
                printf_centered(RESET, "[S]");
                printf_centered(RESET, "Vai nel Sopra-Sotto");
                printf("\n");
                
            }
            
            printf(ROSA "\n\n\n\n\n\n\n[Q] Passa il turno\n" RESET);
            printf("\n| ");
            
            scanf("%c", choice);

            if (choice == 'i' or choice == 'I') {
                // stampa_giocatore();
                printf("palle1\n");

            }
            else if (choice == 'j' or choice == 'J') {
                // combatti();
                printf("palle2\n");
            } else if (choice == 'l' or choice == 'L') {
                // raccogli_oggetto();
                printf("palle3\n");
            } 
            else if (choice == 'k' or choice == 'K') {
                // utlizza_oggetto();
                printf("palle4\n");
            } 
            else if (choice == 'a' or choice == 'A') {
                indietreggia(giocatori[ordine[turno - 1] - 1]);
                printf("palle5\n");
            } 
            else if (choice == 'd' or choice == 'D') {
                avanza(giocatori[ordine[turno - 1] - 1]);
                printf("palle6\n");
            } 
            else if (choice == 'q' or choice == 'Q') {
                // passa();
                printf("palle7\n");
            } 
            else if (giocatori[ordine[turno - 1] - 1]->mondo == 0 and (choice == 's' or choice == 'S')) {
                cambia_mondo(giocatori[ordine[turno - 1] - 1]);
                printf("palle8\n");
            } 
            else if (giocatori[ordine[turno - 1] - 1]->mondo == 1 and (choice == 'w' or choice == 'W')) {
                cambia_mondo(giocatori[ordine[turno - 1] - 1]);
                printf("palle9\n");
            }
            else {
                printf_centered(RESET, "L'opzione scelta non è valida o non esiste.\n");
                printf_centered(RESET, "Riprovare.\n");
                sleep(1);
            }

            
            




        } while (choice != '0');
        
        
        
        //TODO: Rimuovere questa condizione per effettuare i beta test finali
        // Condizione per testing 
        if (ciclo == 10 and turno == n_giocatori) {
            stato_partita = 1;
            exit(0);
        }
        
        turno++;
    }
    
}

static void stampa_zona_giocatore(giocatore* giocatore) {

    if (giocatore->mondo == 0) {
        zona_mondoreale* z = giocatore->pos_mondoreale;

        printf_centered(VERDE, "============< ZONA %hhu | MONDO REALE >============\n", z->id_zona);
        printf_centered(RESET, "Tipo: %s\n", get_nome_zona(z->zona));
        printf_centered(RESET, "Nemico: %s\n", get_nome_nemico(z->nemico));
        printf_centered(RESET, "Oggetto: %s\n", get_nome_oggetto(z->oggetto));
        printf_centered(RESET, "___________________________________________________\n\n");
    }
    else {
        zona_soprasotto* zs = giocatore->pos_soprasotto;

        printf_centered(ROSSO, "============< ZONA %hhu | SOPRA-SOTTO >============\n", zs->id_zona);
        printf_centered(RESET, "Tipo: %s\n", get_nome_zona(zs->zona));
        printf_centered(RESET, "Nemico: %s\n", get_nome_nemico(zs->nemico));
        printf_centered(RESET, "___________________________________________________\n\n");
    }
    
}

static void avanza(giocatore* giocatore) {
    if (combattuto == 0) {
        printf_centered(ROSSO, "Non puoi: Devi prima combattere il nemico.\n");
        terminal_cleaner();
        return;
    }

    if (giocatore->mondo == 0 and giocatore->pos_mondoreale->avanti != NULL) {
        giocatore->pos_mondoreale = giocatore->pos_mondoreale->avanti;
        printf_centered(VERDE, "Sei avanzato alla Zona %hhu! (Mondo Reale)\n", giocatore->pos_mondoreale->id_zona);
    }
    if (giocatore->mondo > 0 and giocatore->pos_soprasotto->avanti != NULL){
        giocatore->pos_soprasotto = giocatore->pos_soprasotto->avanti;
        printf_centered(VERDE, "Sei avanzato alla Zona %hhu! (Sopra-Sotto)\n", giocatore->pos_soprasotto->id_zona);
    }

    movimento = 1;
}

static void indietreggia(giocatore* giocatore) {
    if (combattuto == 0) {
        printf_centered(ROSSO, "Non puoi: Devi prima combattere il nemico.\n");
        terminal_cleaner();
        return;
    }

    if (giocatore->mondo == 0 and giocatore->pos_mondoreale->indietro != NULL) {
        giocatore->pos_mondoreale = giocatore->pos_mondoreale->indietro;
        printf_centered(VERDE, "Sei avanzaato alla Zona %hhu! (Mondo Reale)\n", giocatore->pos_mondoreale->id_zona);
    }
    if (giocatore->mondo > 0 and giocatore->pos_soprasotto->indietro != NULL) {
        giocatore->pos_soprasotto = giocatore->pos_soprasotto->indietro;
        printf_centered(VERDE, "Sei avanzaato alla Zona %hhu! (Sopra-Sotto)\n", giocatore->pos_soprasotto->id_zona);
    }
    
    movimento = 1;
}

static void cambia_mondo(giocatore* giocatore) {
    if (combattuto = 0) {
        printf_centered(ROSSO, "Non puoi: Devi prima combattere il nemico.\n");
        terminal_cleaner();
        return;
    }

    if (movimento != 0) {
        
    }
    
}