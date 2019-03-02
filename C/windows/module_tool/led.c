#define PERIPH_BASE           ((unsigned int)0x40000000)

#define APB2PERIPH_BASE       (PERIPH_BASE + 0x10000)

#define AHBPERIPH_BASE        (PERIPH_BASE + 0x20000)

#define GPIOB_BASE            (APB2PERIPH_BASE + 0x0C00)

#define GPIOB_CRL                       *(unsigned int*)(GPIOB_BASE+0x00)
#define GPIOB_CRH                       *(unsigned int*)(GPIOB_BASE+0x04)
#define GPIOB_IDR                       *(unsigned int*)(GPIOB_BASE+0x08)
#define GPIOB_ODR                       *(unsigned int*)(GPIOB_BASE+0x0C)
#define GPIOB_BSRR        *(unsigned int*)(GPIOB_BASE+0x10)
#define GPIOB_BRR                       *(unsigned int*)(GPIOB_BASE+0x14)
#define GPIOB_LCKR              *(unsigned int*)(GPIOB_BASE+0x18)

#define RCC_BASE      (AHBPERIPH_BASE + 0x1000)
#define RCC_APB2ENR              *(unsigned int*)(RCC_BASE+0x18)



void led()
{
        RCC_APB2ENR |= (1<<3);

        GPIOB_CRL &= ~( 0x0F<< (4*0));
        GPIOB_CRL |= (1<<4*0);
        GPIOB_ODR &= ~(1<<0);
}


