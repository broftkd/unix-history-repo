/*-
 * Copyright (c) 2003 OGAWA Takaya <t-ogawa@triaez.kaisei.org>
 * Copyright (c) 2004 Yoshihiro TAKAHASHI <nyan@FreeBSD.org>
 * All rights Reserved.
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
 *
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_acpi.h"
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/bus.h>

#include "acpi.h"
#include <dev/acpica/acpivar.h>

/* Debug */
#undef	ACPI_PANASONIC_DEBUG

/* Operations */
#define	HKEY_SET	0
#define	HKEY_GET	1

/* Functions */
#define	HKEY_REG_LCD_BRIGHTNESS		0x04
#define	HKEY_REG_SOUND_MUTE		0x08

/* Field definitions */
#define	HKEY_LCD_BRIGHTNESS_BITS	4
#define	HKEY_LCD_BRIGHTNESS_DIV		((1 << HKEY_LCD_BRIGHTNESS_BITS) - 1)

struct acpi_panasonic_softc {
	device_t	dev;
	ACPI_HANDLE	handle;

	struct sysctl_ctx_list	sysctl_ctx;
	struct sysctl_oid	*sysctl_tree;
};

/* Prototype for HKEY functions for getting/setting a value. */
typedef int hkey_fn_t(ACPI_HANDLE, int, UINT32 *);

static int	acpi_panasonic_probe(device_t dev);
static int	acpi_panasonic_attach(device_t dev);
static int	acpi_panasonic_detach(device_t dev);
static int	acpi_panasonic_sysctl(SYSCTL_HANDLER_ARGS);
static ACPI_INTEGER acpi_panasonic_sinf(ACPI_HANDLE h, ACPI_INTEGER index);
static void	acpi_panasonic_sset(ACPI_HANDLE h, ACPI_INTEGER index,
		    ACPI_INTEGER val);
static int	acpi_panasonic_hkey_event(struct acpi_panasonic_softc *sc,
		    ACPI_HANDLE h, UINT32 *arg);
static void	acpi_panasonic_hkey_action(struct acpi_panasonic_softc *sc,
		    ACPI_HANDLE h, UINT32 key);
static void	acpi_panasonic_notify(ACPI_HANDLE h, UINT32 notify,
		    void *context);

static hkey_fn_t	hkey_lcd_brightness_max;
static hkey_fn_t	hkey_lcd_brightness;
static hkey_fn_t	hkey_sound_mute;
static int		lcd_brightness_max = 255;
ACPI_SERIAL_DECL(panasonic, "ACPI Panasonic extras");

/* Table of sysctl names and HKEY functions to call. */
static struct {
	char		*name;
	hkey_fn_t	*handler;
} sysctl_table[] = {
	/* name,		handler */
	{"lcd_brightness_max",	hkey_lcd_brightness_max},
	{"lcd_brightness",	hkey_lcd_brightness},
	{"sound_mute",		hkey_sound_mute},
	{NULL, NULL}
};

static device_method_t acpi_panasonic_methods[] = {
	DEVMETHOD(device_probe,		acpi_panasonic_probe),
	DEVMETHOD(device_attach,	acpi_panasonic_attach),
	DEVMETHOD(device_detach,	acpi_panasonic_detach),

	{0, 0}
};

static driver_t acpi_panasonic_driver = {
	"acpi_panasonic",
	acpi_panasonic_methods,
	sizeof(struct acpi_panasonic_softc),
};

static devclass_t acpi_panasonic_devclass;

DRIVER_MODULE(acpi_panasonic, acpi, acpi_panasonic_driver,
    acpi_panasonic_devclass, 0, 0);
MODULE_DEPEND(acpi_panasonic, acpi, 1, 1, 1);

static int
acpi_panasonic_probe(device_t dev)
{
	static char *mat_ids[] = { "MAT0019", NULL };

	if (acpi_disabled("panasonic") ||
	    ACPI_ID_PROBE(device_get_parent(dev), dev, mat_ids) == NULL ||
	    device_get_unit(dev) != 0)
		return (ENXIO);

	device_set_desc(dev, "Panasonic Notebook Hotkeys");
	return (0);
}

static int
acpi_panasonic_attach(device_t dev)
{
	struct acpi_panasonic_softc *sc;
	struct acpi_softc *acpi_sc;
	ACPI_STATUS status;
	int i;

	sc = device_get_softc(dev);
	sc->dev = dev;
	sc->handle = acpi_get_handle(dev);

	acpi_sc = acpi_device_get_parent_softc(dev);

	/* Build sysctl tree */
	sysctl_ctx_init(&sc->sysctl_ctx);
	sc->sysctl_tree = SYSCTL_ADD_NODE(&sc->sysctl_ctx,
	    SYSCTL_CHILDREN(acpi_sc->acpi_sysctl_tree), OID_AUTO,
	    "panasonic", CTLFLAG_RD, 0, "");
	for (i = 0; sysctl_table[i].name != NULL; i++) {
		SYSCTL_ADD_PROC(&sc->sysctl_ctx,
		    SYSCTL_CHILDREN(sc->sysctl_tree), OID_AUTO,
		    sysctl_table[i].name,
		    CTLTYPE_INT | CTLFLAG_RW | CTLFLAG_ANYBODY,
		    sc, i, acpi_panasonic_sysctl, "I", "");
	}

#if 0
	/* Activate hotkeys */
	status = AcpiEvaluateObject(sc->handle, "", NULL, NULL);
	if (ACPI_FAILURE(status)) {
		device_printf(dev, "enable FN keys failed\n");
		sysctl_ctx_free(&sc->sysctl_ctx);
		return (ENXIO);
	}
#endif

        /* Handle notifies */
	status = AcpiInstallNotifyHandler(sc->handle, ACPI_DEVICE_NOTIFY,
	    acpi_panasonic_notify, sc);
	if (ACPI_FAILURE(status)) {
		device_printf(dev, "couldn't install notify handler - %s\n",
		    AcpiFormatException(status));
		sysctl_ctx_free(&sc->sysctl_ctx);
		return (ENXIO);
	}

	return (0);
}

static int
acpi_panasonic_detach(device_t dev)
{
	struct acpi_panasonic_softc *sc;

	sc = device_get_softc(dev);

	/* Remove notify handler */
	AcpiRemoveNotifyHandler(sc->handle, ACPI_DEVICE_NOTIFY,
	    acpi_panasonic_notify);

	/* Free sysctl tree */
	sysctl_ctx_free(&sc->sysctl_ctx);

	return (0);
}

static int
acpi_panasonic_sysctl(SYSCTL_HANDLER_ARGS)
{
	struct acpi_panasonic_softc *sc;
	UINT32 arg;
	int function, error;
	hkey_fn_t *handler;

	sc = (struct acpi_panasonic_softc *)oidp->oid_arg1;
	function = oidp->oid_arg2;
	handler = sysctl_table[function].handler;

        /* Get the current value from the appropriate function. */
	ACPI_SERIAL_BEGIN(panasonic);
	error = handler(sc->handle, HKEY_GET, &arg);
	if (error != 0)
		goto out;

	/* Send the current value to the user and return if no new value. */
	error = sysctl_handle_int(oidp, &arg, 0, req);
	if (error != 0 || req->newptr == NULL)
		goto out;

	/* Set the new value via the appropriate function. */
	error = handler(sc->handle, HKEY_SET, &arg);

out:
	ACPI_SERIAL_END(panasonic);
	return (error);
}

static ACPI_INTEGER
acpi_panasonic_sinf(ACPI_HANDLE h, ACPI_INTEGER index)
{
	ACPI_BUFFER buf;
	ACPI_OBJECT *res;
	ACPI_INTEGER ret;

	ACPI_SERIAL_ASSERT(panasonic);
	ret = -1;
	buf.Length = ACPI_ALLOCATE_BUFFER;
	buf.Pointer = NULL;
	AcpiEvaluateObject(h, "SINF", NULL, &buf);
	res = (ACPI_OBJECT *)buf.Pointer;
	if (res->Type == ACPI_TYPE_PACKAGE)
		ret = res->Package.Elements[index].Integer.Value;
	AcpiOsFree(buf.Pointer);

	return (ret);
}

static void
acpi_panasonic_sset(ACPI_HANDLE h, ACPI_INTEGER index, ACPI_INTEGER val)
{
	ACPI_OBJECT_LIST args;
	ACPI_OBJECT obj[2];

	ACPI_SERIAL_ASSERT(panasonic);
	obj[0].Type = ACPI_TYPE_INTEGER;
	obj[0].Integer.Value = index;
	obj[1].Type = ACPI_TYPE_INTEGER;
	obj[1].Integer.Value = val;
	args.Count = 2;
	args.Pointer = obj;
	AcpiEvaluateObject(h, "SSET", &args, NULL);
}

static int
hkey_lcd_brightness_max(ACPI_HANDLE h, int op, UINT32 *val)
{

	ACPI_SERIAL_ASSERT(panasonic);
	switch (op) {
	case HKEY_SET:
		if (*val < 0 || *val > 255)
			return (EINVAL);
		lcd_brightness_max = *val;
		break;
	case HKEY_GET:
		*val = lcd_brightness_max;
		break;
	}

	return (0);
}

static int
hkey_lcd_brightness(ACPI_HANDLE h, int op, UINT32 *val)
{

	ACPI_SERIAL_ASSERT(panasonic);
	switch (op) {
	case HKEY_SET:
		if (*val < 0 || *val > lcd_brightness_max)
			return (EINVAL);
		acpi_panasonic_sset(h, HKEY_REG_LCD_BRIGHTNESS, *val);
		break;
	case HKEY_GET:
		*val = acpi_panasonic_sinf(h, HKEY_REG_LCD_BRIGHTNESS);
		break;
	}

	return (0);
}

static int
hkey_sound_mute(ACPI_HANDLE h, int op, UINT32 *val)
{

	ACPI_SERIAL_ASSERT(panasonic);
	switch (op) {
	case HKEY_SET:
		if (*val != 0 && *val != 1)
			return (EINVAL);
		acpi_panasonic_sset(h, HKEY_REG_SOUND_MUTE, *val);
		break;
	case HKEY_GET:
		*val = acpi_panasonic_sinf(h, HKEY_REG_SOUND_MUTE);
		break;
	}

	return (0);
}

static int
acpi_panasonic_hkey_event(struct acpi_panasonic_softc *sc, ACPI_HANDLE h,
    UINT32 *arg)
{
	ACPI_BUFFER buf;
	ACPI_OBJECT *res;
	ACPI_INTEGER val;
	int status;

	ACPI_SERIAL_ASSERT(panasonic);
	status = ENXIO;

	buf.Length = ACPI_ALLOCATE_BUFFER;
	buf.Pointer = NULL;
	AcpiEvaluateObject(h, "HINF", NULL, &buf);
	res = (ACPI_OBJECT *)buf.Pointer;
	if (res->Type != ACPI_TYPE_INTEGER) {
		device_printf(sc->dev, "HINF returned non-integer\n");
		goto end;
	}
	val = res->Integer.Value;
#ifdef ACPI_PANASONIC_DEBUG
	device_printf(sc->dev, "%s button Fn+F%d\n",
		      (val & 0x80) ? "Pressed" : "Released",
		      (int)(val & 0x7f));
#endif
	if ((val & 0x7f) > 0 && (val & 0x7f) < 11) {
		*arg = val;
		status = 0;
	}
end:
	if (buf.Pointer)
		AcpiOsFree(buf.Pointer);

	return (status);
}

static void
acpi_panasonic_hkey_action(struct acpi_panasonic_softc *sc, ACPI_HANDLE h,
    UINT32 key)
{
	int arg;

	ACPI_SERIAL_ASSERT(panasonic);
	switch (key) {
	case 1:
		/* Decrease LCD brightness. */
		hkey_lcd_brightness(h, HKEY_GET, &arg);
		arg -= lcd_brightness_max / HKEY_LCD_BRIGHTNESS_DIV;
		if (arg < 0)
			arg = 0;
		else if (arg > lcd_brightness_max)
			arg = lcd_brightness_max;
		hkey_lcd_brightness(h, HKEY_SET, &arg);
		break;
	case 2:
		/* Increase LCD brightness. */
		hkey_lcd_brightness(h, HKEY_GET, &arg);
		arg += lcd_brightness_max / HKEY_LCD_BRIGHTNESS_DIV;
		if (arg < 0)
			arg = 0;
		else if (arg > lcd_brightness_max)
			arg = lcd_brightness_max;
		hkey_lcd_brightness(h, HKEY_SET, &arg);
		break;
	case 4:
		/* Toggle sound mute. */
		hkey_sound_mute(h, HKEY_GET, &arg);
		if (arg)
			arg = 0;
		else
			arg = 1;
		hkey_sound_mute(h, HKEY_SET, &arg);
		break;
	}
}

static void
acpi_panasonic_notify(ACPI_HANDLE h, UINT32 notify, void *context)
{
	struct acpi_panasonic_softc *sc;
	UINT32 key;

	sc = (struct acpi_panasonic_softc *)context;

	switch (notify) {
	case 0x80:
		ACPI_SERIAL_BEGIN(panasonic);
		if (acpi_panasonic_hkey_event(sc, h, &key) == 0) {
			acpi_panasonic_hkey_action(sc, h, key);
			acpi_UserNotify("Panasonic", h, (uint8_t)key);
		}
		ACPI_SERIAL_END(panasonic);
		break;
	default:
		device_printf(sc->dev, "unknown notify: %#x\n", notify);
		break;
	}
}
