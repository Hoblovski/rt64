/* acpi.c
 *
 * Copyright (c) 2013 Brian Swetland
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "rt64.h"

// References: ACPI 5.0 Errata A
// http://acpi.info/spec.htm

// 5.2.5.3
#define SIG_RDSP "RSD PTR "
struct acpi_rdsp {
	u8 signature[8];
	u8 checksum;
	u8 oem_id[6];
	u8 revision;
	u32 rsdt_addr_phys;
	u32 length;
	u64 xsdt_addr_phys;
	u8 xchecksum;
	u8 reserved[3];
} __attribute__((__packed__));

// 5.2.6
struct acpi_desc_header {
	u8 signature[4];
	u32 length;
	u8 revision;
	u8 checksum;
	u8 oem_id[6];
	u8 oem_tableid[8];
	u32 oem_revision;
	u8 creator_id[4];
	u32 creator_revision;
} __attribute__((__packed__));

// 5.2.7
struct acpi_rsdt {
	struct acpi_desc_header header;
	u32 entry[0];
} __attribute__((__packed__));

#define TYPE_LAPIC 0
#define TYPE_IOAPIC 1
#define TYPE_INT_SRC_OVERRIDE 2
#define TYPE_NMI_INT_SRC 3
#define TYPE_LAPIC_NMI 4

// 5.2.12 Multiple APIC Description Table
#define SIG_MADT "APIC"
struct acpi_madt {
	struct acpi_desc_header header;
	u32 addr;
	u32 flags;
	u8 table[0];
} __attribute__((__packed__));

// 5.2.12.2
#define APIC_LAPIC_ENABLED 1
struct madt_lapic {
	u8 type;
	u8 length;
	u8 acpi_id;
	u8 apic_id;
	u32 flags;
} __attribute__((__packed__));

// 5.2.12.3
struct madt_ioapic {
	u8 type;
	u8 length;
	u8 id;
	u8 reserved;
	u32 addr;
	u32 interrupt_base;
} __attribute__((__packed__));

static struct acpi_rdsp *scan_rdsp(usize base, usize len)
{
	u8 *p;
	for (p = P2V(base); len >= sizeof(struct acpi_rdsp); len -= 4, p += 4) {
		if (memcmp(p, SIG_RDSP, 8) == 0) {
			usize sum, n;
			for (sum = 0, n = 0; n < 20; n++)
				sum += p[n];
			if ((sum & 0xff) == 0)
				return (struct acpi_rdsp *)p;
		}
	}
	return (struct acpi_rdsp *)0;
}

static struct acpi_rdsp *find_rdsp(void)
{
	struct acpi_rdsp *rdsp;
	usize pa;
	pa = *((u16 *)P2V(0x40E)) << 4; // EBDA
	if (pa && (rdsp = scan_rdsp(pa, 1024)))
		return rdsp;
	return scan_rdsp(0xE0000, 0x20000);
}

static void acpi_config_smp(struct acpi_madt *madt)
{
	usize nioapic = 0;
	u8 *p, *e;

	if (!madt)
		panic("acpi: cannot find madt");
	if (madt->header.length < sizeof(struct acpi_madt))
		panic("acpi: invalid madt");

	if (madt->addr != DEFAULT_LAPIC_PADDR)
		panic("got unexpected lapic address %p", madt->addr);

	p = madt->table;
	e = p + madt->header.length - sizeof(struct acpi_madt);

	while (p < e) {
		usize len;
		if ((e - p) < 2)
			break;
		len = p[1];
		if ((e - p) < len)
			break;
		switch (p[0]) {
		case TYPE_LAPIC: {
			struct madt_lapic *lapic = (void *)p;
			if (len < sizeof(*lapic))
				break;
			if (!(lapic->flags & APIC_LAPIC_ENABLED))
				break;
			cprintf("acpi: cpu#%d apicid %d\n", ncpu,
				lapic->apic_id);
			cpus[ncpu].index = ncpu;
			cpus[ncpu].lapicid = lapic->apic_id;
			ncpu++;
			break;
		}
		case TYPE_IOAPIC: {
			struct madt_ioapic *ioapic = (void *)p;
			if (len < sizeof(*ioapic))
				break;
			cprintf("acpi: ioapic#%d @%x id=%d base=%d\n", nioapic,
				ioapic->addr, ioapic->id,
				ioapic->interrupt_base);
			if (ioapic->addr != DEFAULT_IOAPIC_PADDR)
				panic("got unexpected ioapic address");
			if (nioapic) {
				panic("multiple ioapics are not supported");
			} else {
				ioapicid = ioapic->id;
			}
			nioapic++;
			break;
		}
		}
		p += len;
	}

	if (!ncpu)
		panic("acpi: cannot detect lapic");
}

void acpiinit(void)
{
	unsigned n, count;
	struct acpi_rdsp *rdsp;
	struct acpi_rsdt *rsdt;
	struct acpi_madt *madt = 0;

	rdsp = find_rdsp();
	if (rdsp->rsdt_addr_phys > (u32)MAX_PHYS_MEM)
		goto notmapped;
	rsdt = P2V(rdsp->rsdt_addr_phys);
	count = (rsdt->header.length - sizeof(*rsdt)) / 4;
	for (n = 0; n < count; n++) {
		struct acpi_desc_header *hdr = P2V(rsdt->entry[n]);
		if (rsdt->entry[n] > MAX_PHYS_MEM)
			goto notmapped;
		u8 sig[5], id[7], tableid[9], creator[5];
		memmove(sig, hdr->signature, 4);
		sig[4] = 0;
		memmove(id, hdr->oem_id, 6);
		id[6] = 0;
		memmove(tableid, hdr->oem_tableid, 8);
		tableid[8] = 0;
		memmove(creator, hdr->creator_id, 4);
		creator[4] = 0;
		cprintf("acpi: sig=%s id=%s tableid=%s oemrev=%x creat=%s creatrev=%x\n",
			sig, id, tableid, hdr->oem_revision, creator,
			hdr->creator_revision);
		if (!memcmp(hdr->signature, SIG_MADT, 4))
			madt = (void *)hdr;
	}

	acpi_config_smp(madt);
	return;

notmapped:
	cprintf("acpi: too much physical memory\n");
	panic("acpi: tables not mapped");
}

struct percpu cpus[MAX_CPU];
int ncpu;
u8 ioapicid;
