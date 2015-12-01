#ifndef __HEADER_UART
#define __HEADER_UART

//
// Registres UART
#define UART_DR		0x20201000u	// Data register
#define UART_RSRECR	0x20201004u
#define UART_FR		0x20201018u	// Flag register
#define UART_IBRD	0x20201024u	// Integer Daud rate divisor
#define UART_FBRD	0x20201028u	// Fractional Baud rate divisor
#define UART_LCRH	0x2020102Cu	// Line control register
#define UART_CR 	0x20201030u	// Control register
#define UART_IFLS	0x20201034u	// Interupt FIFO level select register
#define UART_IMSC	0x20201038u	// Interupt mask set clear register
#define UART_RIS	0x2020103Cu	// Raw Interupt Status register
#define UART_MIS	0x20201040u	// Masked Interupt Status Register
#define UART_ICR	0x20201044u	// Interupt clear register
#define UART_ITCR	0x20201080u	// Test Control register
#define UART_ITIP	0x20201084u	// Integration test input reg
#define UART_ITOP	0x20201088u	// Integration test output reg
#define UART_TDR	0x2020108Cu	// Test data reg
// Constantes GPIO utiles aux UART
#define UART_TXD0_PIN	14u	// UART TDX0 pin sur ALT0
#define UART_RDX0_PIN	15u // UART RDX0 pin sur ALT0
#define GPIO_ALT0		4u	// GPIO fonction ALT0
#define GPIO_TDX0_OFF	((14u % 10u) * 3u)
#define GPIO_RDX0_OFF	((15u % 10u) * 3u)

//
// Variable d'erreur
extern int uart_error;

// Initialise l'UART
void uart_init(void);

// Envoie un caractère
void uart_send_char(const char c);

// Permet d'envoyer des chaine de caractère. Celle-ci doit se terminer
//	par le caractère nul.
void uart_send_str(const char *data);

// Permet d'envoyer un entier signé
void uart_send_int(int n);

// Permet de savoir si la receive fifo est vide
int uart_is_receive_fifo_empty(void);

// Permet de recevoir une chaine de caractère.
// Bloque jusqu'a la reception de n-1 caractères, ou la reception
//	du caractère nul.
// NOTE : Un caractère nul est placé automatiquement en fin de
//	chaine contenu dans le buffer.
int uart_receive_str(char *buffer, unsigned int n);

// Permet de lire un entier signé.
// Renvoie la valeur lue
// NOTE : en cas d'erreur, uart_error est mis à -1
int uart_receive_int(void);

#endif
