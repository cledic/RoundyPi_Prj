/*
 * Programma per visualizzare Emoji.
 * I file emoji sono PNG e li ho presi dal sito: "https://sensa.co/emoji/".
 * La lista dei nomi è in un file header "emoji_list.h". La lettura dei file è da SDCard: sono in totale 1.3MB
 * e non entrano tutti in flash. Per adesso li voglio vedere tutti. Poi sceglierà quali usare.
 * 
 * Questa versione usa le due MCU per far girare i due JOB principali: la MCU0 esgue la lettura e decodifica dei file PNG
 * mentre la MCU1 ne esegue la visualizzazione.
 * Nell'insieme non è proprio utilizzabile. Utile solo per studiare il multitask.
 * Per un risultato decente vedere "RoundyPi_ImgViewerMjpeg"
 */
#define FIRMWARE_VERSION "0.01"
