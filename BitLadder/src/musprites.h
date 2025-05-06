typedef struct sprStatus
{
	uint8_t s; //sprite index in Vicky
	int16_t x,y,z; //position
	bool rightOrLeft; //last facing
	bool isDashing;
	uint32_t addr; //base address
	uint8_t frame; //frame into view
	uint16_t sx, sy; //speed
	int8_t vx, vy; //velocity
	int8_t ax, ay; //acceleration
	struct timer_t timer; //animation timer;
	uint8_t cookie; //cookie for timer
	uint8_t state; //which state: 0 idle, 1 walk right, 2 walk left, etc
	uint8_t *minIndexForState; //minimum index for given state
	uint8_t *maxIndexForState; //maximum index for given state
} sprStatus;
