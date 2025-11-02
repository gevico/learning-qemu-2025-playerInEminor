#ifndef HW_G233_SPI_H
#define HW_G233_SPI_H

// #include "qemu/fifo8.h"
#include "hw/sysbus.h"

#define G233_SPI_MAX_CS 4

#define TYPE_G233_SPI "g233.spi"
#define G233_SPI(obj) OBJECT_CHECK(G233SPIState, (obj), TYPE_G233_SPI)

typedef struct G233SPIState {
    SysBusDevice parent_obj;

    MemoryRegion mmio;
    qemu_irq irq;

    uint32_t num_cs;
    uint32_t cr1;
    uint32_t cr2;
    uint32_t sr;
    uint32_t dr;
    uint32_t csctrl;

    qemu_irq cs_lines[G233_SPI_MAX_CS];

    // Fifo8 tx_fifo;
    // Fifo8 rx_fifo;
} G233SPIState;

#endif /* HW_G233_SPI_H */
