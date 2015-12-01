#include "gpio.h"
#include "asm_tools.h"
#include "uart.h"
#include "config.h"

//
// Constante des puissances de 10
static const int TEN_POW[10] = {	1,
									10,
									100,
									1000,
									10000,
									100000,
							        1000000,
							        10000000,
							        100000000,
							        1000000000 };
// Variable publique d'erreur
int uart_error;

//
// Initialisation
void uart_init(void)
{
	// On désactive UART
	Set32(UART_CR, 0u);

	// Initialisation GPIO (14 et 15)
	// On règle la fonction sur ALT0 pour les bons pins
	Set32(GPIO_FSEL1, Get32(GPIO_FSEL1) | (GPIO_ALT0 << GPIO_TDX0_OFF));
	Set32(GPIO_FSEL1, Get32(GPIO_FSEL1) | (GPIO_ALT0 << GPIO_RDX0_OFF));
	// On règle le pull down
	Set32(GPIO_PUD, 0u);
	delay(150u);
	Set32(GPIO_PUDCLK0, (1u << UART_TXD0_PIN) | (1u << UART_RDX0_PIN));
	delay(150u);
	Set32(GPIO_PUD, 0u);
	Set32(GPIO_PUDCLK0, 0u);

	// On initialisation UART
	// On clear la line
	Set32(UART_LCRH, 0u);
	// On clear les intéruptions
	Set32(UART_ICR, 0u);
	// On règles le baud rate à 115200 baud
	Set32(UART_IBRD, 1u);
	Set32(UART_FBRD, 40u);
	// Line control :
	// - break disable
	// - parity disable
	// - stop bits disable
	// - fifo disabled
	// - word length = 8 bits
	Set32(UART_LCRH, (3u << 5u));
	// Seuil FIFO
	// - On n'utilise pas la FIFO
	Set32(UART_IFLS, 0u);
	// Interruption
	//  On masque tout
	Set32(UART_IMSC, (1u << 1u) | (1u << 4u) | (1u << 5u)
						| (1u << 6u) | (1u << 7u) | (1u << 8u)
						| (1u << 9u) | (1u << 10u));
	// Controle :
	// - on active l'UART
	// - pas de loopback
	// - activation de la réception/émission
	// - pas de RTS
	// - pas de flowcontrol
	Set32(UART_CR, (1u << 0u) | (1u << 8u) | (1u << 9u));
}

//
// Permet d'envoyer des chaine de caractère. Celle-ci doit se terminer
//	par le caractère nul.
void uart_send_str(const char *data)
{
	if (*data == 0)
	{
		return;
	}

	do
	{
		// On attend que l'UART soit disponible
		while ((Get32(UART_FR) & (1u << 5u)) != 0u);
		// On écrit la donnée
		Set32(UART_DR, (unsigned int)*(data++));

	} while (*data != 0);

	// On attend que l'UART soit disponible
	while ((Get32(UART_FR) & (1u << 5u)) != 0u);
	// Puis on envoie le caractère nul
#if RPI
	Set32(UART_DR, 0u);
#endif
}

//
// Permet d'envoyer un entier signé
void uart_send_int(int n)
{
	int ten_pow;
	char str[16];
	int str_i = 0;
	int start_conv = 0;

	// Cas du 0
	if (n == 0)
	{
		str[0] = '0';
		str[1] = 0;
		uart_send_str(str);
		return;
	}

	// Négatif ?
	if ((n & 0x80000000) != 0)
	{
		str[str_i++] = '-';
		n -= 1;
		n = ~n;
	}

	for (ten_pow = 9; ten_pow >= 0; ten_pow--)
	{
		int digit = 0;

		while (n >= TEN_POW[ten_pow])
		{
			n -= TEN_POW[ten_pow];
			digit++;
		}
		if (digit > 0)
		{
			str[str_i++] = (char)(digit + 48);

			if (start_conv == 0)
			{
				int zero_pad_i;

				for (zero_pad_i = 0; zero_pad_i < ten_pow; zero_pad_i++)
				{
					str[str_i + zero_pad_i] = '0';
				}
				str[str_i + zero_pad_i] = 0;
				
				start_conv = 1;
			}
		}
		else if (start_conv == 1)
		{
			str_i++;
		}
	}

	uart_send_str(str);
}

//
// Permet de recevoir des caractères
// Block jusqu'a la reception de n-1 caractères, ou la reception
//	du caractère nul.
// NOTE : Un caractère nul est placé automatiquement en fin de
//	chaine.
int uart_receive_str(char *buffer, unsigned int n)
{
	unsigned int byte;
	int i;

	if (n == 0u)
	{
		return 0;
	}

	i = 0;
	n--;
	do
	{
		// On attend que se ne soit pas vide
		while ((Get32(UART_FR) & (1u << 4u)) != 0);
		// Lecture du byte
		byte = Get32(UART_DR) & 0xFFu;

		// On vérifique que se n'est pas la fin
		if (byte == 0u)
		{
			break;
		}

		*(buffer++) = (char)byte;
		i++;
	}
	while (--n != 0u);

	*buffer = 0; // Caractère de fin

	return i;
}

//
// Permet de lire un entier signé.
// Renvoie la valeur lue
// NOTE : en cas d'erreur, uart_error est mis à -1
int uart_receive_int(void)
{
	char buffer[16];
	char *data = buffer;
	int neg;
	int str_c, digit, n;

	uart_error = 0;

	// On lit la chaine de caractère correspondant au nombre
	str_c = uart_receive_str(buffer, 16u);

	// Si il est négatif...
	if (*data == '-')
	{
		neg = 1;
		data++;
		str_c--;
	}
	else
	{
		neg = 0;
	}

	// On enlève les 0 de tête, si il y en a
	while ((str_c > 0) && (*data == 48))
	{
		data++;
		str_c--;
	}

	// Si il ne reste plus rien, c'est que on avait un 0
	if (str_c <= 0)
	{
		return 0;
	}
	
	// On vérifie que le nombre n'est pas trop grand
	if (str_c > 10)
	{
		uart_error = -1;
		return 0;
	}

	n = 0;
	do
	{
		digit = (int)*(data++);
		if ((digit < 48) || (digit > 57))
		{
			uart_error = -1;
			return 0;
		}
		else if (digit == 48)
		{
			str_c--;
		}
		else
		{
			n += (digit - 48) * TEN_POW[--str_c];
		}
	}
	while (str_c > 0);

	if (neg == 1)
	{
		n = -(n);
	}

	return n;
}

//
// Permet de savoir si la receive fifo est vide
int uart_is_receive_fifo_empty(void)
{
	if ((Get32(UART_FR) & (1 << 4)) != 0)
	{
		return 0;
	}

	return 1;
}
