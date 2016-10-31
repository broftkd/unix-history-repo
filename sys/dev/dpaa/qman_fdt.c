/*-
 * Copyright (c) 2011-2012 Semihalf.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "opt_platform.h"
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/bus.h>
#include <sys/module.h>

#include <machine/bus.h>

#include <dev/fdt/fdt_common.h>

#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>
#include <dev/ofw/ofw_subr.h>

#include "qman.h"
#include "portals.h"

#define	FBMAN_DEVSTR	"Freescale Queue Manager"

static int qman_fdt_probe(device_t);

static device_method_t qman_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		qman_fdt_probe),
	DEVMETHOD(device_attach,	qman_attach),
	DEVMETHOD(device_detach,	qman_detach),

	DEVMETHOD(device_suspend,	qman_suspend),
	DEVMETHOD(device_resume,	qman_resume),
	DEVMETHOD(device_shutdown,	qman_shutdown),

	{ 0, 0 }
};

static driver_t qman_driver = {
	"qman",
	qman_methods,
	sizeof(struct qman_softc),
};

static devclass_t qman_devclass;
DRIVER_MODULE(qman, simplebus, qman_driver, qman_devclass, 0, 0);

static int
qman_fdt_probe(device_t dev)
{

	if (!ofw_bus_is_compatible(dev, "fsl,qman"))
		return (ENXIO);

	device_set_desc(dev, FBMAN_DEVSTR);

	return (BUS_PROBE_DEFAULT);
}

/*
 * BMAN Portals
 */
#define	BMAN_PORT_DEVSTR	"Freescale Queue Manager - Portals"

static device_probe_t qman_portals_fdt_probe;
static device_attach_t qman_portals_fdt_attach;

static device_method_t qm_portals_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		qman_portals_fdt_probe),
	DEVMETHOD(device_attach,	qman_portals_fdt_attach),
	DEVMETHOD(device_detach,	qman_portals_detach),

	{ 0, 0 }
};

static driver_t qm_portals_driver = {
	"qman-portals",
	qm_portals_methods,
	sizeof(struct dpaa_portals_softc),
};

static devclass_t qm_portals_devclass;
DRIVER_MODULE(qman_portals, ofwbus, qm_portals_driver, qm_portals_devclass, 0, 0);

static void
get_addr_props(phandle_t node, uint32_t *addrp, uint32_t *sizep)
{

	*addrp = 2;
	*sizep = 1;
	OF_getencprop(node, "#address-cells", addrp, sizeof(*addrp));
	OF_getencprop(node, "#size-cells", sizep, sizeof(*sizep));
}

static int
qman_portals_fdt_probe(device_t dev)
{

	if (!ofw_bus_is_compatible(dev, "qman-portals"))
		return (ENXIO);

	device_set_desc(dev, BMAN_PORT_DEVSTR);

	return (BUS_PROBE_DEFAULT);
}

static int
qman_portals_fdt_attach(device_t dev)
{
	struct dpaa_portals_softc *sc;
	struct resource_list_entry *rle;
	phandle_t node, child, cpu_node;
	vm_paddr_t portal_pa;
	vm_size_t portal_size;
	uint32_t addr, size;
	ihandle_t cpu;
	int cpu_num, cpus, intr_rid;
	struct dpaa_portals_devinfo di;
	struct ofw_bus_devinfo ofw_di = {};

	cpus = 0;
	sc = device_get_softc(dev);
	sc->sc_dev = dev;

	node = ofw_bus_get_node(dev);
	get_addr_props(node, &addr, &size);

	/* Find portals tied to CPUs */
	for (child = OF_child(node); child != 0; child = OF_peer(child)) {
		if (!fdt_is_compatible(child, "fsl,qman-portal")) {
			continue;
		}
		/* Checkout related cpu */
		if (OF_getprop(child, "cpu-handle", (void *)&cpu,
		    sizeof(cpu)) <= 0) {
			continue;
		}
		/* Acquire cpu number */
		cpu_node = OF_instance_to_package(cpu);
		if (OF_getencprop(cpu_node, "reg", &cpu_num, sizeof(cpu_num)) <= 0) {
			device_printf(dev, "Could not retrieve CPU number.\n");
			return (ENXIO);
		}

		cpus++;

		if (cpus > MAXCPU)
			break;

		if (ofw_bus_gen_setup_devinfo(&ofw_di, child) != 0) {
			device_printf(dev, "could not set up devinfo\n");
			continue;
		}

		resource_list_init(&di.di_res);
		if (ofw_bus_reg_to_rl(dev, child, addr, size, &di.di_res)) {
			device_printf(dev, "%s: could not process 'reg' "
			    "property\n", ofw_di.obd_name);
			ofw_bus_gen_destroy_devinfo(&ofw_di);
			continue;
		}
		if (ofw_bus_intr_to_rl(dev, child, &di.di_res, &intr_rid)) {
			device_printf(dev, "%s: could not process "
			    "'interrupts' property\n", ofw_di.obd_name);
			resource_list_free(&di.di_res);
			ofw_bus_gen_destroy_devinfo(&ofw_di);
			continue;
		}
		di.di_intr_rid = intr_rid;
		
		ofw_reg_to_paddr(child, 0, &portal_pa, &portal_size, NULL);
		rle = resource_list_find(&di.di_res, SYS_RES_MEMORY, 0);

		if (sc->sc_dp_pa == 0)
			sc->sc_dp_pa = portal_pa - rle->start;

		portal_size = rle->end + 1;
		rle = resource_list_find(&di.di_res, SYS_RES_MEMORY, 1);
		portal_size = ulmax(rle->end + 1, portal_size);
		sc->sc_dp_size = ulmax(sc->sc_dp_size, portal_size);

		if (dpaa_portal_alloc_res(dev, &di, cpu_num))
			goto err;
	}

	ofw_bus_gen_destroy_devinfo(&ofw_di);

	return (qman_portals_attach(dev));
err:
	resource_list_free(&di.di_res);
	ofw_bus_gen_destroy_devinfo(&ofw_di);
	qman_portals_detach(dev);
	return (ENXIO);
}
