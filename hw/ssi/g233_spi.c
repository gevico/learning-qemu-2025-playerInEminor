#include "qemu/osdep.h"
#include "hw/irq.h"
#include "hw/qdev-properties.h"
#include "hw/sysbus.h"
#include "hw/ssi/ssi.h"
#include "qemu/fifo8.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "hw/ssi/g233_spi.h"

#define SPI_CR1     0x00
#define SPI_CR2     0x04
#define SPI_SR      0x08
#define SPI_DR      0x0C
#define SPI_CSCTRL  0x10

#define CSi_EN(sr, i) \
    ( ( (sr) >> (i) ) & 1 )
#define CSi_ACT(sr, i) \
    ( ( (sr) >> ( (i) + 4 ) ) & 1)

static void g233_spi_reset(DeviceState *dev)
{
    G233SPIState *s = G233_SPI(dev);

    s->cr1 = 0;
    s->cr2 = 0;
    s->sr = 0x00000002;
    s->dr = 0x0000000c;
    s->csctrl = 0;

    // sifive_spi_txfifo_reset(s);
    // sifive_spi_rxfifo_reset(s);
}

static void g233_spi_update_cs_lines(G233SPIState *s)
{
    int i;
    for (i = 0; i < G233_SPI_MAX_CS; i++) {
        if (!CSi_EN(s->csctrl, i)) {
            qemu_set_irq(s->cs_lines[i], 1);
            continue;
        }
        if (CSi_ACT(s->csctrl, i)) {
            qemu_set_irq(s->cs_lines[i], 0);
        } else {
            qemu_set_irq(s->cs_lines[i], 1);
        }
    }
}

static uint64_t g233_spi_read(void *opaque, hwaddr addr, unsigned int size)
{
    G233SPIState *s = opaque;

    switch (addr) {
    case SPI_CR1:
        return s->cr1;
        break;
    case SPI_CR2:
        return s->cr2;
        break;
    case SPI_SR:
        return s->sr;
        break;
    case SPI_DR:
        return s->dr;
        break;
    case SPI_CSCTRL:
        return s->csctrl;
        break;
    default:
        qemu_log_mask(LOG_UNIMP, "%s: unimplemented register 0x%08" PRIx64 "\n",
                      __func__, addr);
    }
    return 0;
}

static void g233_spi_write(void *opaque, hwaddr addr,
                             uint64_t val64, unsigned int size)
{
    G233SPIState *s = opaque;
    uint32_t value = val64;

    switch (addr) {
    case SPI_CR1:
        s->cr1 = value;
        break;
    case SPI_CR2:
        s->cr2 = value;
        g233_spi_update_cs_lines(s);
        break;
    case SPI_SR:
        s->sr = value;
        break;
    case SPI_DR:
        s->dr = value;
        break;
    case SPI_CSCTRL:
        s->csctrl = value;
        break;
    default:
        qemu_log_mask(LOG_UNIMP, "%s: unimplemented register 0x%08" PRIx64 "\n",
                      __func__, addr);
        return;
    }
    // g233_spi_update_irq(s);
}

static const MemoryRegionOps g233_spi_ops = {
    .read = g233_spi_read,
    .write = g233_spi_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4
    }
};

static void g233_spi_realize(DeviceState *dev, Error **errp)
{
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);
    G233SPIState *s = G233_SPI(dev);
    int i;

    s->ssi = ssi_create_bus(dev, "ssi");
    sysbus_init_irq(sbd, &s->irq);

    for (i = 0; i < G233_SPI_MAX_CS; i++) {
        sysbus_init_irq(sbd, &s->cs_lines[i]);
    }


    memory_region_init_io(&s->mmio, OBJECT(s), &g233_spi_ops, s,
                          TYPE_G233_SPI, 0x1000);
    sysbus_init_mmio(sbd, &s->mmio);

//     fifo8_create(&s->tx_fifo, FIFO_CAPACITY);
//     fifo8_create(&s->rx_fifo, FIFO_CAPACITY);
}

static void g233_spi_class_init(ObjectClass *klass, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    device_class_set_legacy_reset(dc, g233_spi_reset);
    dc->realize = g233_spi_realize;
}

static const TypeInfo g233_spi_info = {
    .name           = TYPE_G233_SPI,
    .parent         = TYPE_SYS_BUS_DEVICE,
    .instance_size  = sizeof(G233SPIState),
    .class_init     = g233_spi_class_init,
};

static void g233_spi_register_types(void)
{
    type_register_static(&g233_spi_info);
}

type_init(g233_spi_register_types)
