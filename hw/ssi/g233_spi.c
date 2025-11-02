#include "qemu/osdep.h"
#include "hw/irq.h"
#include "hw/qdev-properties.h"
#include "hw/sysbus.h"
#include "hw/ssi/ssi.h"
#include "qemu/fifo8.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "hw/ssi/g233_spi.h"


static uint64_t g233_spi_read(void *opaque, hwaddr addr, unsigned int size)
{
    printf("g233_spi_read called\n");
    return 0;
}

static void g233_spi_write(void *opaque, hwaddr addr,
                             uint64_t val64, unsigned int size)
{
    printf("g233_spi_write called\n");
    // G233SPIState *s = opaque;
    // uint32_t value = val64;

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

    // device_class_set_legacy_reset(dc, g233_spi_reset);
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
