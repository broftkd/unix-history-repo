/*-
 * Copyright (c) 2001 Tsubai Masanari.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 *	NetBSD: ki2c.c,v 1.11 2007/12/06 17:00:33 ad Exp
 *	Id: ki2c.c,v 1.7 2002/10/05 09:56:05 tsubai Exp
 */

/*
 * 	Support routines for the Keywest I2C controller.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <machine/resource.h>
#include <machine/bus.h>
#include <sys/rman.h>

#include <dev/iicbus/iicbus.h>
#include <dev/iicbus/iiconf.h>
#include <dev/ofw/ofw_bus.h>
#include "iicbus_if.h"

/* Keywest I2C Register offsets */
#define MODE	0
#define CONTROL	1
#define STATUS	2
#define ISR	3
#define IER	4
#define ADDR	5
#define SUBADDR	6
#define DATA	7

/* MODE */
#define I2C_SPEED	0x03	/* Speed mask */
#define  I2C_100kHz	0x00
#define  I2C_50kHz	0x01
#define  I2C_25kHz	0x02
#define I2C_MODE	0x0c	/* Mode mask */
#define  I2C_DUMBMODE	0x00	/*  Dumb mode */
#define  I2C_STDMODE	0x04	/*  Standard mode */
#define  I2C_STDSUBMODE	0x08	/*  Standard mode + sub address */
#define  I2C_COMBMODE	0x0c	/*  Combined mode */
#define I2C_PORT	0xf0	/* Port mask */

/* CONTROL */
#define I2C_CT_AAK	0x01	/* Send AAK */
#define I2C_CT_ADDR	0x02	/* Send address(es) */
#define I2C_CT_STOP	0x04	/* Send STOP */
#define I2C_CT_START	0x08	/* Send START */

/* STATUS */
#define I2C_ST_BUSY	0x01	/* Busy */
#define I2C_ST_LASTAAK	0x02	/* Last AAK */
#define I2C_ST_LASTRW	0x04	/* Last R/W */
#define I2C_ST_SDA	0x08	/* SDA */
#define I2C_ST_SCL	0x10	/* SCL */

/* ISR/IER */
#define I2C_INT_DATA	0x01	/* Data byte sent/received */
#define I2C_INT_ADDR	0x02	/* Address sent */
#define I2C_INT_STOP	0x04	/* STOP condition sent */
#define I2C_INT_START	0x08	/* START condition sent */

/* I2C flags */
#define I2C_BUSY	0x01
#define I2C_READING	0x02
#define I2C_ERROR	0x04
#define I2C_SELECTED	0x08

struct kiic_softc {
	device_t 		 sc_dev;
	phandle_t		 sc_node;
	struct mtx 		 sc_mutex;
	struct resource		*sc_reg;
	int			 sc_irqrid;
	struct resource		*sc_irq;
	void			*sc_ih;
	u_int 			 sc_regstep;
	u_int 			 sc_flags;
	u_char			*sc_data;
	int 			 sc_resid;
	device_t 		 sc_iicbus;
};

static int 	kiic_probe(device_t dev);
static int 	kiic_attach(device_t dev);
static void 	kiic_writereg(struct kiic_softc *sc, u_int, u_int);
static u_int 	kiic_readreg(struct kiic_softc *, u_int);
static void 	kiic_setmode(struct kiic_softc *, u_int);
static void 	kiic_setspeed(struct kiic_softc *, u_int);
static void 	kiic_intr(void *xsc);
static int	kiic_transfer(device_t dev, struct iic_msg *msgs,
		    uint32_t nmsgs);
static phandle_t kiic_get_node(device_t bus, device_t dev);

static device_method_t kiic_methods[] = {
	/* device interface */
	DEVMETHOD(device_probe, 	kiic_probe),
	DEVMETHOD(device_attach, 	kiic_attach),

	/* iicbus interface */
	DEVMETHOD(iicbus_callback,	iicbus_null_callback),
	DEVMETHOD(iicbus_transfer,	kiic_transfer),

	/* ofw_bus interface */
	DEVMETHOD(ofw_bus_get_node,	kiic_get_node),

	{ 0, 0 }
};

static driver_t kiic_driver = {
	"iichb",
	kiic_methods,
	sizeof(struct kiic_softc)
};
static devclass_t kiic_devclass;

DRIVER_MODULE(kiic, macio, kiic_driver, kiic_devclass, 0, 0);

static int
kiic_probe(device_t self)
{
	const char *name;

	name = ofw_bus_get_name(self);
	if (name && strcmp(name, "i2c") == 0) {
		device_set_desc(self, "Keywest I2C controller");
		return (0);
	}

	return (ENXIO);
}

static int
kiic_attach(device_t self)
{
	struct kiic_softc *sc = device_get_softc(self);
	int rid, rate;
	phandle_t node;
	char name[64];

	bzero(sc, sizeof(*sc));
	sc->sc_dev = self;
	
	node = ofw_bus_get_node(self);
	if (node == 0 || node == -1) {
		return (EINVAL);
	}

	rid = 0;
	sc->sc_reg = bus_alloc_resource_any(self, SYS_RES_MEMORY,
			&rid, RF_ACTIVE);
	if (sc->sc_reg == NULL) {
		return (ENOMEM);
	}

	if (OF_getprop(node, "AAPL,i2c-rate", &rate, 4) != 4) {
		device_printf(self, "cannot get i2c-rate\n");
		return (ENXIO);
	}
	if (OF_getprop(node, "AAPL,address-step", &sc->sc_regstep, 4) != 4) {
		device_printf(self, "unable to find i2c address step\n");
		return (ENXIO);
	}

	/*
	 * Some Keywest I2C devices have their children attached directly
	 * underneath them.  Some have a single 'iicbus' child with the
	 * devices underneath that.  Sort this out, and make sure that the
	 * OFW I2C layer has the correct node.
	 */

	sc->sc_node = OF_child(node);
	if (OF_getprop(sc->sc_node,"name",name,sizeof(name)) > 0) {
		if (strcmp(name,"i2c-bus") != 0)
			sc->sc_node = node;
	}

	mtx_init(&sc->sc_mutex, "kiic", NULL, MTX_DEF);

	sc->sc_irq = bus_alloc_resource_any(self, SYS_RES_IRQ, &sc->sc_irqrid, 
	    RF_ACTIVE);
	bus_setup_intr(self, sc->sc_irq, INTR_TYPE_MISC | INTR_MPSAFE, NULL,
	    kiic_intr, sc, &sc->sc_ih);

	kiic_writereg(sc, STATUS, 0);
	kiic_writereg(sc, ISR, 0);
	kiic_writereg(sc, IER, 0);

	kiic_setmode(sc, I2C_STDMODE);
	kiic_setspeed(sc, I2C_100kHz);		/* XXX rate */
	
	kiic_writereg(sc, IER, I2C_INT_DATA | I2C_INT_ADDR | I2C_INT_STOP);

	/* Add the IIC bus layer */
	sc->sc_iicbus = device_add_child(self, "iicbus", -1);

	return (bus_generic_attach(self));
}

static void
kiic_writereg(struct kiic_softc *sc, u_int reg, u_int val)
{
	bus_write_1(sc->sc_reg, sc->sc_regstep * reg, val);
	DELAY(10); /* XXX why? */
}

static u_int
kiic_readreg(struct kiic_softc *sc, u_int reg)
{
	return bus_read_1(sc->sc_reg, sc->sc_regstep * reg);
}

static void
kiic_setmode(struct kiic_softc *sc, u_int mode)
{
	u_int x;

	KASSERT((mode & ~I2C_MODE) == 0, ("bad mode"));
	x = kiic_readreg(sc, MODE);
	x &= ~I2C_MODE;
	x |= mode;
	kiic_writereg(sc, MODE, x);
}

static void
kiic_setspeed(struct kiic_softc *sc, u_int speed)
{
	u_int x;

	KASSERT((speed & ~I2C_SPEED) == 0, ("bad speed"));
	x = kiic_readreg(sc, MODE);
	x &= ~I2C_SPEED;
	x |= speed;
	kiic_writereg(sc, MODE, x);
}

static void
kiic_intr(void *xsc)
{
	struct kiic_softc *sc = xsc;
	u_int isr;
	uint32_t x;

	mtx_lock(&sc->sc_mutex);
	isr = kiic_readreg(sc, ISR);

	if (isr & I2C_INT_ADDR) {
		sc->sc_flags |= I2C_SELECTED;

		if (sc->sc_flags & I2C_READING) {
			if (sc->sc_resid > 1) {
				x = kiic_readreg(sc, CONTROL);
				x |= I2C_CT_AAK;
				kiic_writereg(sc, CONTROL, x);
			}
		} else {
			kiic_writereg(sc, DATA, *sc->sc_data++);
			sc->sc_resid--;
		}
	}

	if (isr & I2C_INT_DATA) {
		if (sc->sc_resid > 0) {
			if (sc->sc_flags & I2C_READING) {
				*sc->sc_data++ = kiic_readreg(sc, DATA);
				sc->sc_resid--;
			} else {
				kiic_writereg(sc, DATA, *sc->sc_data++);
				sc->sc_resid--;
			}
		}

		if (sc->sc_resid == 0)
			wakeup(sc->sc_dev);
	}

	if (isr & I2C_INT_STOP) {
		kiic_writereg(sc, CONTROL, 0);
		sc->sc_flags &= ~I2C_SELECTED;
		wakeup(sc->sc_dev);
	}

	kiic_writereg(sc, ISR, isr);
	mtx_unlock(&sc->sc_mutex);
}

static int
kiic_transfer(device_t dev, struct iic_msg *msgs, uint32_t nmsgs)
{
	struct kiic_softc *sc;
	int i, x, timo;
	uint8_t addr;

	sc = device_get_softc(dev);
	timo = 100;

	mtx_lock(&sc->sc_mutex);

	if (sc->sc_flags & I2C_BUSY)
		mtx_sleep(dev, &sc->sc_mutex, 0, "kiic", timo);

	if (sc->sc_flags & I2C_BUSY) {
		mtx_unlock(&sc->sc_mutex);
		return (ETIMEDOUT);
	}
		
	sc->sc_flags = I2C_BUSY;

	for (i = 0; i < nmsgs; i++) {
		sc->sc_data = msgs[i].buf;
		sc->sc_resid = msgs[i].len;
		sc->sc_flags = I2C_BUSY;
		addr = msgs[i].slave;
		timo = 1000 + sc->sc_resid * 200;

		if (msgs[i].flags & IIC_M_RD) {
			sc->sc_flags |= I2C_READING;
			addr |= 1;
		}

		kiic_writereg(sc, ADDR, addr);
		kiic_writereg(sc, SUBADDR, 0x04);

		x = kiic_readreg(sc, CONTROL) | I2C_CT_ADDR;
		kiic_writereg(sc, CONTROL, x);

		mtx_sleep(dev, &sc->sc_mutex, 0, "kiic", timo);

		if (!(sc->sc_flags & I2C_READING)) {
			x = kiic_readreg(sc, CONTROL) | I2C_CT_STOP;
			kiic_writereg(sc, CONTROL, x);
		}

		mtx_sleep(dev, &sc->sc_mutex, 0, "kiic", timo);

		msgs[i].len -= sc->sc_resid;

		if (sc->sc_flags & I2C_ERROR) {
			device_printf(sc->sc_dev, "I2C_ERROR\n");
			mtx_unlock(&sc->sc_mutex);
			return (-1);
		}
	}

	sc->sc_flags = 0;

	mtx_unlock(&sc->sc_mutex);

	return (0);
}

static phandle_t
kiic_get_node(device_t bus, device_t dev)
{
	struct kiic_softc *sc;

	sc = device_get_softc(bus);
	/* We only have one child, the I2C bus, which needs our own node. */
		
	return sc->sc_node;
}

