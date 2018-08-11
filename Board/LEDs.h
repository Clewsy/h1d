#define LED_GREEN		(1 << PD5)	//Orange LED
#define LED_ORANGE		(1 << PC7)	//Green LED

#define LED_GREEN_DDR		DDRD
#define LED_ORANGE_DDR		DDRC

#define LED_GREEN_PORT		PORTD
#define LED_ORANGE_PORT		PORTC

#define LED_GREEN_OFF		LED_GREEN_PORT |= LED_GREEN
#define LED_ORANGE_OFF		LED_ORANGE_PORT &= ~LED_ORANGE

#define LED_GREEN_ON		LED_GREEN_PORT &= ~LED_GREEN
#define LED_ORANGE_ON		LED_ORANGE_PORT |= LED_ORANGE

static inline void LEDS_Init(void)
{
	LED_GREEN_DDR |= LED_GREEN;
	LED_ORANGE_DDR |= LED_ORANGE;

	LED_GREEN_OFF;
	LED_ORANGE_OFF;
}

static inline void LEDS_Disable(void)
{
	LED_GREEN_DDR &= ~LED_GREEN;
	LED_ORANGE_DDR &= ~LED_ORANGE;

	LED_GREEN_PORT &= ~LED_GREEN;
	LED_ORANGE_PORT &= ~LED_ORANGE;
}

static inline void LEDs_TurnOnLEDS(const uint8_t LEDMask)
{
	LED_GREEN_PORT &= ~(LEDMask & LED_GREEN);
	LED_ORANGE_PORT |= (LEDMask & LED_ORANGE);
}

static inline void LEDs_TurnOffLEDS(const uint8_t LEDMask)
{
	LED_GREEN_PORT |= (LEDMask & LED_GREEN);
	LED_ORANGE_PORT &= ~(LEDMask & LED_ORANGE);
}

static inline void LEDS_SetAllLEDS(const uint8_t LEDMask)
{
	LED_GREEN_PORT = ((LED_GREEN_PORT | LED_GREEN) & ~LEDMask);
	LED_ORANGE_PORT = ((LED_ORANGE_PORT & ~LED_ORANGE) | (LEDMask & LED_ORANGE));
}
