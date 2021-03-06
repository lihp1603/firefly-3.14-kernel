/* SandyBridge-EP/IvyTown uncore support */
#include "perf_event_intel_uncore.h"


/* SNB-EP Box level control */
#define SNBEP_PMON_BOX_CTL_RST_CTRL	(1 << 0)
#define SNBEP_PMON_BOX_CTL_RST_CTRS	(1 << 1)
#define SNBEP_PMON_BOX_CTL_FRZ		(1 << 8)
#define SNBEP_PMON_BOX_CTL_FRZ_EN	(1 << 16)
#define SNBEP_PMON_BOX_CTL_INT		(SNBEP_PMON_BOX_CTL_RST_CTRL | \
					 SNBEP_PMON_BOX_CTL_RST_CTRS | \
					 SNBEP_PMON_BOX_CTL_FRZ_EN)
/* SNB-EP event control */
#define SNBEP_PMON_CTL_EV_SEL_MASK	0x000000ff
#define SNBEP_PMON_CTL_UMASK_MASK	0x0000ff00
#define SNBEP_PMON_CTL_RST		(1 << 17)
#define SNBEP_PMON_CTL_EDGE_DET		(1 << 18)
#define SNBEP_PMON_CTL_EV_SEL_EXT	(1 << 21)
#define SNBEP_PMON_CTL_EN		(1 << 22)
#define SNBEP_PMON_CTL_INVERT		(1 << 23)
#define SNBEP_PMON_CTL_TRESH_MASK	0xff000000
#define SNBEP_PMON_RAW_EVENT_MASK	(SNBEP_PMON_CTL_EV_SEL_MASK | \
					 SNBEP_PMON_CTL_UMASK_MASK | \
					 SNBEP_PMON_CTL_EDGE_DET | \
					 SNBEP_PMON_CTL_INVERT | \
					 SNBEP_PMON_CTL_TRESH_MASK)

/* SNB-EP Ubox event control */
#define SNBEP_U_MSR_PMON_CTL_TRESH_MASK		0x1f000000
#define SNBEP_U_MSR_PMON_RAW_EVENT_MASK		\
				(SNBEP_PMON_CTL_EV_SEL_MASK | \
				 SNBEP_PMON_CTL_UMASK_MASK | \
				 SNBEP_PMON_CTL_EDGE_DET | \
				 SNBEP_PMON_CTL_INVERT | \
				 SNBEP_U_MSR_PMON_CTL_TRESH_MASK)

#define SNBEP_CBO_PMON_CTL_TID_EN		(1 << 19)
#define SNBEP_CBO_MSR_PMON_RAW_EVENT_MASK	(SNBEP_PMON_RAW_EVENT_MASK | \
						 SNBEP_CBO_PMON_CTL_TID_EN)

/* SNB-EP PCU event control */
#define SNBEP_PCU_MSR_PMON_CTL_OCC_SEL_MASK	0x0000c000
#define SNBEP_PCU_MSR_PMON_CTL_TRESH_MASK	0x1f000000
#define SNBEP_PCU_MSR_PMON_CTL_OCC_INVERT	(1 << 30)
#define SNBEP_PCU_MSR_PMON_CTL_OCC_EDGE_DET	(1 << 31)
#define SNBEP_PCU_MSR_PMON_RAW_EVENT_MASK	\
				(SNBEP_PMON_CTL_EV_SEL_MASK | \
				 SNBEP_PCU_MSR_PMON_CTL_OCC_SEL_MASK | \
				 SNBEP_PMON_CTL_EDGE_DET | \
				 SNBEP_PMON_CTL_EV_SEL_EXT | \
				 SNBEP_PMON_CTL_INVERT | \
				 SNBEP_PCU_MSR_PMON_CTL_TRESH_MASK | \
				 SNBEP_PCU_MSR_PMON_CTL_OCC_INVERT | \
				 SNBEP_PCU_MSR_PMON_CTL_OCC_EDGE_DET)

#define SNBEP_QPI_PCI_PMON_RAW_EVENT_MASK	\
				(SNBEP_PMON_RAW_EVENT_MASK | \
				 SNBEP_PMON_CTL_EV_SEL_EXT)

/* SNB-EP pci control register */
#define SNBEP_PCI_PMON_BOX_CTL			0xf4
#define SNBEP_PCI_PMON_CTL0			0xd8
/* SNB-EP pci counter register */
#define SNBEP_PCI_PMON_CTR0			0xa0

/* SNB-EP home agent register */
#define SNBEP_HA_PCI_PMON_BOX_ADDRMATCH0	0x40
#define SNBEP_HA_PCI_PMON_BOX_ADDRMATCH1	0x44
#define SNBEP_HA_PCI_PMON_BOX_OPCODEMATCH	0x48
/* SNB-EP memory controller register */
#define SNBEP_MC_CHy_PCI_PMON_FIXED_CTL		0xf0
#define SNBEP_MC_CHy_PCI_PMON_FIXED_CTR		0xd0
/* SNB-EP QPI register */
#define SNBEP_Q_Py_PCI_PMON_PKT_MATCH0		0x228
#define SNBEP_Q_Py_PCI_PMON_PKT_MATCH1		0x22c
#define SNBEP_Q_Py_PCI_PMON_PKT_MASK0		0x238
#define SNBEP_Q_Py_PCI_PMON_PKT_MASK1		0x23c

/* SNB-EP Ubox register */
#define SNBEP_U_MSR_PMON_CTR0			0xc16
#define SNBEP_U_MSR_PMON_CTL0			0xc10

#define SNBEP_U_MSR_PMON_UCLK_FIXED_CTL		0xc08
#define SNBEP_U_MSR_PMON_UCLK_FIXED_CTR		0xc09

/* SNB-EP Cbo register */
#define SNBEP_C0_MSR_PMON_CTR0			0xd16
#define SNBEP_C0_MSR_PMON_CTL0			0xd10
#define SNBEP_C0_MSR_PMON_BOX_CTL		0xd04
#define SNBEP_C0_MSR_PMON_BOX_FILTER		0xd14
#define SNBEP_CBO_MSR_OFFSET			0x20

#define SNBEP_CB0_MSR_PMON_BOX_FILTER_TID	0x1f
#define SNBEP_CB0_MSR_PMON_BOX_FILTER_NID	0x3fc00
#define SNBEP_CB0_MSR_PMON_BOX_FILTER_STATE	0x7c0000
#define SNBEP_CB0_MSR_PMON_BOX_FILTER_OPC	0xff800000

#define SNBEP_CBO_EVENT_EXTRA_REG(e, m, i) {	\
	.event = (e),				\
	.msr = SNBEP_C0_MSR_PMON_BOX_FILTER,	\
	.config_mask = (m),			\
	.idx = (i)				\
}

/* SNB-EP PCU register */
#define SNBEP_PCU_MSR_PMON_CTR0			0xc36
#define SNBEP_PCU_MSR_PMON_CTL0			0xc30
#define SNBEP_PCU_MSR_PMON_BOX_CTL		0xc24
#define SNBEP_PCU_MSR_PMON_BOX_FILTER		0xc34
#define SNBEP_PCU_MSR_PMON_BOX_FILTER_MASK	0xffffffff
#define SNBEP_PCU_MSR_CORE_C3_CTR		0x3fc
#define SNBEP_PCU_MSR_CORE_C6_CTR		0x3fd

/* IVT event control */
#define IVT_PMON_BOX_CTL_INT		(SNBEP_PMON_BOX_CTL_RST_CTRL | \
					 SNBEP_PMON_BOX_CTL_RST_CTRS)
#define IVT_PMON_RAW_EVENT_MASK		(SNBEP_PMON_CTL_EV_SEL_MASK | \
					 SNBEP_PMON_CTL_UMASK_MASK | \
					 SNBEP_PMON_CTL_EDGE_DET | \
					 SNBEP_PMON_CTL_TRESH_MASK)
/* IVT Ubox */
#define IVT_U_MSR_PMON_GLOBAL_CTL		0xc00
#define IVT_U_PMON_GLOBAL_FRZ_ALL		(1 << 31)
#define IVT_U_PMON_GLOBAL_UNFRZ_ALL		(1 << 29)

#define IVT_U_MSR_PMON_RAW_EVENT_MASK	\
				(SNBEP_PMON_CTL_EV_SEL_MASK | \
				 SNBEP_PMON_CTL_UMASK_MASK | \
				 SNBEP_PMON_CTL_EDGE_DET | \
				 SNBEP_U_MSR_PMON_CTL_TRESH_MASK)
/* IVT Cbo */
#define IVT_CBO_MSR_PMON_RAW_EVENT_MASK		(IVT_PMON_RAW_EVENT_MASK | \
						 SNBEP_CBO_PMON_CTL_TID_EN)

#define IVT_CB0_MSR_PMON_BOX_FILTER_TID		(0x1fULL << 0)
#define IVT_CB0_MSR_PMON_BOX_FILTER_LINK	(0xfULL << 5)
#define IVT_CB0_MSR_PMON_BOX_FILTER_STATE	(0x3fULL << 17)
#define IVT_CB0_MSR_PMON_BOX_FILTER_NID		(0xffffULL << 32)
#define IVT_CB0_MSR_PMON_BOX_FILTER_OPC		(0x1ffULL << 52)
#define IVT_CB0_MSR_PMON_BOX_FILTER_C6		(0x1ULL << 61)
#define IVT_CB0_MSR_PMON_BOX_FILTER_NC		(0x1ULL << 62)
#define IVT_CB0_MSR_PMON_BOX_FILTER_IOSC	(0x1ULL << 63)

/* IVT home agent */
#define IVT_HA_PCI_PMON_CTL_Q_OCC_RST		(1 << 16)
#define IVT_HA_PCI_PMON_RAW_EVENT_MASK		\
				(IVT_PMON_RAW_EVENT_MASK | \
				 IVT_HA_PCI_PMON_CTL_Q_OCC_RST)
/* IVT PCU */
#define IVT_PCU_MSR_PMON_RAW_EVENT_MASK	\
				(SNBEP_PMON_CTL_EV_SEL_MASK | \
				 SNBEP_PMON_CTL_EV_SEL_EXT | \
				 SNBEP_PCU_MSR_PMON_CTL_OCC_SEL_MASK | \
				 SNBEP_PMON_CTL_EDGE_DET | \
				 SNBEP_PCU_MSR_PMON_CTL_TRESH_MASK | \
				 SNBEP_PCU_MSR_PMON_CTL_OCC_INVERT | \
				 SNBEP_PCU_MSR_PMON_CTL_OCC_EDGE_DET)
/* IVT QPI */
#define IVT_QPI_PCI_PMON_RAW_EVENT_MASK	\
				(IVT_PMON_RAW_EVENT_MASK | \
				 SNBEP_PMON_CTL_EV_SEL_EXT)

#define __BITS_VALUE(x, i, n)  ((typeof(x))(((x) >> ((i) * (n))) & \
				((1ULL << (n)) - 1)))

DEFINE_UNCORE_FORMAT_ATTR(event, event, "config:0-7");
DEFINE_UNCORE_FORMAT_ATTR(event_ext, event, "config:0-7,21");
DEFINE_UNCORE_FORMAT_ATTR(umask, umask, "config:8-15");
DEFINE_UNCORE_FORMAT_ATTR(edge, edge, "config:18");
DEFINE_UNCORE_FORMAT_ATTR(tid_en, tid_en, "config:19");
DEFINE_UNCORE_FORMAT_ATTR(inv, inv, "config:23");
DEFINE_UNCORE_FORMAT_ATTR(thresh8, thresh, "config:24-31");
DEFINE_UNCORE_FORMAT_ATTR(thresh5, thresh, "config:24-28");
DEFINE_UNCORE_FORMAT_ATTR(occ_sel, occ_sel, "config:14-15");
DEFINE_UNCORE_FORMAT_ATTR(occ_invert, occ_invert, "config:30");
DEFINE_UNCORE_FORMAT_ATTR(occ_edge, occ_edge, "config:14-51");
DEFINE_UNCORE_FORMAT_ATTR(filter_tid, filter_tid, "config1:0-4");
DEFINE_UNCORE_FORMAT_ATTR(filter_link, filter_link, "config1:5-8");
DEFINE_UNCORE_FORMAT_ATTR(filter_nid, filter_nid, "config1:10-17");
DEFINE_UNCORE_FORMAT_ATTR(filter_nid2, filter_nid, "config1:32-47");
DEFINE_UNCORE_FORMAT_ATTR(filter_state, filter_state, "config1:18-22");
DEFINE_UNCORE_FORMAT_ATTR(filter_state2, filter_state, "config1:17-22");
DEFINE_UNCORE_FORMAT_ATTR(filter_opc, filter_opc, "config1:23-31");
DEFINE_UNCORE_FORMAT_ATTR(filter_opc2, filter_opc, "config1:52-60");
DEFINE_UNCORE_FORMAT_ATTR(filter_band0, filter_band0, "config1:0-7");
DEFINE_UNCORE_FORMAT_ATTR(filter_band1, filter_band1, "config1:8-15");
DEFINE_UNCORE_FORMAT_ATTR(filter_band2, filter_band2, "config1:16-23");
DEFINE_UNCORE_FORMAT_ATTR(filter_band3, filter_band3, "config1:24-31");
DEFINE_UNCORE_FORMAT_ATTR(match_rds, match_rds, "config1:48-51");
DEFINE_UNCORE_FORMAT_ATTR(match_rnid30, match_rnid30, "config1:32-35");
DEFINE_UNCORE_FORMAT_ATTR(match_rnid4, match_rnid4, "config1:31");
DEFINE_UNCORE_FORMAT_ATTR(match_dnid, match_dnid, "config1:13-17");
DEFINE_UNCORE_FORMAT_ATTR(match_mc, match_mc, "config1:9-12");
DEFINE_UNCORE_FORMAT_ATTR(match_opc, match_opc, "config1:5-8");
DEFINE_UNCORE_FORMAT_ATTR(match_vnw, match_vnw, "config1:3-4");
DEFINE_UNCORE_FORMAT_ATTR(match0, match0, "config1:0-31");
DEFINE_UNCORE_FORMAT_ATTR(match1, match1, "config1:32-63");
DEFINE_UNCORE_FORMAT_ATTR(mask_rds, mask_rds, "config2:48-51");
DEFINE_UNCORE_FORMAT_ATTR(mask_rnid30, mask_rnid30, "config2:32-35");
DEFINE_UNCORE_FORMAT_ATTR(mask_rnid4, mask_rnid4, "config2:31");
DEFINE_UNCORE_FORMAT_ATTR(mask_dnid, mask_dnid, "config2:13-17");
DEFINE_UNCORE_FORMAT_ATTR(mask_mc, mask_mc, "config2:9-12");
DEFINE_UNCORE_FORMAT_ATTR(mask_opc, mask_opc, "config2:5-8");
DEFINE_UNCORE_FORMAT_ATTR(mask_vnw, mask_vnw, "config2:3-4");
DEFINE_UNCORE_FORMAT_ATTR(mask0, mask0, "config2:0-31");
DEFINE_UNCORE_FORMAT_ATTR(mask1, mask1, "config2:32-63");

static void snbep_uncore_pci_disable_box(struct intel_uncore_box *box)
{
	struct pci_dev *pdev = box->pci_dev;
	int box_ctl = uncore_pci_box_ctl(box);
	u32 config = 0;

	if (!pci_read_config_dword(pdev, box_ctl, &config)) {
		config |= SNBEP_PMON_BOX_CTL_FRZ;
		pci_write_config_dword(pdev, box_ctl, config);
	}
}

static void snbep_uncore_pci_enable_box(struct intel_uncore_box *box)
{
	struct pci_dev *pdev = box->pci_dev;
	int box_ctl = uncore_pci_box_ctl(box);
	u32 config = 0;

	if (!pci_read_config_dword(pdev, box_ctl, &config)) {
		config &= ~SNBEP_PMON_BOX_CTL_FRZ;
		pci_write_config_dword(pdev, box_ctl, config);
	}
}

static void snbep_uncore_pci_enable_event(struct intel_uncore_box *box, struct perf_event *event)
{
	struct pci_dev *pdev = box->pci_dev;
	struct hw_perf_event *hwc = &event->hw;

	pci_write_config_dword(pdev, hwc->config_base, hwc->config | SNBEP_PMON_CTL_EN);
}

static void snbep_uncore_pci_disable_event(struct intel_uncore_box *box, struct perf_event *event)
{
	struct pci_dev *pdev = box->pci_dev;
	struct hw_perf_event *hwc = &event->hw;

	pci_write_config_dword(pdev, hwc->config_base, hwc->config);
}

static u64 snbep_uncore_pci_read_counter(struct intel_uncore_box *box, struct perf_event *event)
{
	struct pci_dev *pdev = box->pci_dev;
	struct hw_perf_event *hwc = &event->hw;
	u64 count = 0;

	pci_read_config_dword(pdev, hwc->event_base, (u32 *)&count);
	pci_read_config_dword(pdev, hwc->event_base + 4, (u32 *)&count + 1);

	return count;
}

static void snbep_uncore_pci_init_box(struct intel_uncore_box *box)
{
	struct pci_dev *pdev = box->pci_dev;

	pci_write_config_dword(pdev, SNBEP_PCI_PMON_BOX_CTL, SNBEP_PMON_BOX_CTL_INT);
}

static void snbep_uncore_msr_disable_box(struct intel_uncore_box *box)
{
	u64 config;
	unsigned msr;

	msr = uncore_msr_box_ctl(box);
	if (msr) {
		rdmsrl(msr, config);
		config |= SNBEP_PMON_BOX_CTL_FRZ;
		wrmsrl(msr, config);
	}
}

static void snbep_uncore_msr_enable_box(struct intel_uncore_box *box)
{
	u64 config;
	unsigned msr;

	msr = uncore_msr_box_ctl(box);
	if (msr) {
		rdmsrl(msr, config);
		config &= ~SNBEP_PMON_BOX_CTL_FRZ;
		wrmsrl(msr, config);
	}
}

static void snbep_uncore_msr_enable_event(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hw_perf_event_extra *reg1 = &hwc->extra_reg;

	if (reg1->idx != EXTRA_REG_NONE)
		wrmsrl(reg1->reg, uncore_shared_reg_config(box, 0));

	wrmsrl(hwc->config_base, hwc->config | SNBEP_PMON_CTL_EN);
}

static void snbep_uncore_msr_disable_event(struct intel_uncore_box *box,
					struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;

	wrmsrl(hwc->config_base, hwc->config);
}

static void snbep_uncore_msr_init_box(struct intel_uncore_box *box)
{
	unsigned msr = uncore_msr_box_ctl(box);

	if (msr)
		wrmsrl(msr, SNBEP_PMON_BOX_CTL_INT);
}

static struct attribute *snbep_uncore_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_inv.attr,
	&format_attr_thresh8.attr,
	NULL,
};

static struct attribute *snbep_uncore_ubox_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_inv.attr,
	&format_attr_thresh5.attr,
	NULL,
};

static struct attribute *snbep_uncore_cbox_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_tid_en.attr,
	&format_attr_inv.attr,
	&format_attr_thresh8.attr,
	&format_attr_filter_tid.attr,
	&format_attr_filter_nid.attr,
	&format_attr_filter_state.attr,
	&format_attr_filter_opc.attr,
	NULL,
};

static struct attribute *snbep_uncore_pcu_formats_attr[] = {
	&format_attr_event_ext.attr,
	&format_attr_occ_sel.attr,
	&format_attr_edge.attr,
	&format_attr_inv.attr,
	&format_attr_thresh5.attr,
	&format_attr_occ_invert.attr,
	&format_attr_occ_edge.attr,
	&format_attr_filter_band0.attr,
	&format_attr_filter_band1.attr,
	&format_attr_filter_band2.attr,
	&format_attr_filter_band3.attr,
	NULL,
};

static struct attribute *snbep_uncore_qpi_formats_attr[] = {
	&format_attr_event_ext.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_inv.attr,
	&format_attr_thresh8.attr,
	&format_attr_match_rds.attr,
	&format_attr_match_rnid30.attr,
	&format_attr_match_rnid4.attr,
	&format_attr_match_dnid.attr,
	&format_attr_match_mc.attr,
	&format_attr_match_opc.attr,
	&format_attr_match_vnw.attr,
	&format_attr_match0.attr,
	&format_attr_match1.attr,
	&format_attr_mask_rds.attr,
	&format_attr_mask_rnid30.attr,
	&format_attr_mask_rnid4.attr,
	&format_attr_mask_dnid.attr,
	&format_attr_mask_mc.attr,
	&format_attr_mask_opc.attr,
	&format_attr_mask_vnw.attr,
	&format_attr_mask0.attr,
	&format_attr_mask1.attr,
	NULL,
};

static struct uncore_event_desc snbep_uncore_imc_events[] = {
	INTEL_UNCORE_EVENT_DESC(clockticks,      "event=0xff,umask=0x00"),
	INTEL_UNCORE_EVENT_DESC(cas_count_read,  "event=0x04,umask=0x03"),
	INTEL_UNCORE_EVENT_DESC(cas_count_write, "event=0x04,umask=0x0c"),
	{ /* end: all zeroes */ },
};

static struct uncore_event_desc snbep_uncore_qpi_events[] = {
	INTEL_UNCORE_EVENT_DESC(clockticks,       "event=0x14"),
	INTEL_UNCORE_EVENT_DESC(txl_flits_active, "event=0x00,umask=0x06"),
	INTEL_UNCORE_EVENT_DESC(drs_data,         "event=0x102,umask=0x08"),
	INTEL_UNCORE_EVENT_DESC(ncb_data,         "event=0x103,umask=0x04"),
	{ /* end: all zeroes */ },
};

static struct attribute_group snbep_uncore_format_group = {
	.name = "format",
	.attrs = snbep_uncore_formats_attr,
};

static struct attribute_group snbep_uncore_ubox_format_group = {
	.name = "format",
	.attrs = snbep_uncore_ubox_formats_attr,
};

static struct attribute_group snbep_uncore_cbox_format_group = {
	.name = "format",
	.attrs = snbep_uncore_cbox_formats_attr,
};

static struct attribute_group snbep_uncore_pcu_format_group = {
	.name = "format",
	.attrs = snbep_uncore_pcu_formats_attr,
};

static struct attribute_group snbep_uncore_qpi_format_group = {
	.name = "format",
	.attrs = snbep_uncore_qpi_formats_attr,
};

#define SNBEP_UNCORE_MSR_OPS_COMMON_INIT()			\
	.init_box	= snbep_uncore_msr_init_box,		\
	.disable_box	= snbep_uncore_msr_disable_box,		\
	.enable_box	= snbep_uncore_msr_enable_box,		\
	.disable_event	= snbep_uncore_msr_disable_event,	\
	.enable_event	= snbep_uncore_msr_enable_event,	\
	.read_counter	= uncore_msr_read_counter

static struct intel_uncore_ops snbep_uncore_msr_ops = {
	SNBEP_UNCORE_MSR_OPS_COMMON_INIT(),
};

#define SNBEP_UNCORE_PCI_OPS_COMMON_INIT()			\
	.init_box	= snbep_uncore_pci_init_box,		\
	.disable_box	= snbep_uncore_pci_disable_box,		\
	.enable_box	= snbep_uncore_pci_enable_box,		\
	.disable_event	= snbep_uncore_pci_disable_event,	\
	.read_counter	= snbep_uncore_pci_read_counter

static struct intel_uncore_ops snbep_uncore_pci_ops = {
	SNBEP_UNCORE_PCI_OPS_COMMON_INIT(),
	.enable_event	= snbep_uncore_pci_enable_event,	\
};

static struct event_constraint snbep_uncore_cbox_constraints[] = {
	UNCORE_EVENT_CONSTRAINT(0x01, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x02, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x04, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x05, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x07, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x09, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x11, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x12, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x13, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x1b, 0xc),
	UNCORE_EVENT_CONSTRAINT(0x1c, 0xc),
	UNCORE_EVENT_CONSTRAINT(0x1d, 0xc),
	UNCORE_EVENT_CONSTRAINT(0x1e, 0xc),
	EVENT_CONSTRAINT_OVERLAP(0x1f, 0xe, 0xff),
	UNCORE_EVENT_CONSTRAINT(0x21, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x23, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x31, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x32, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x33, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x34, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x35, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x36, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x37, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x38, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x39, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x3b, 0x1),
	EVENT_CONSTRAINT_END
};

static struct event_constraint snbep_uncore_r2pcie_constraints[] = {
	UNCORE_EVENT_CONSTRAINT(0x10, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x11, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x12, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x23, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x24, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x25, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x26, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x32, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x33, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x34, 0x3),
	EVENT_CONSTRAINT_END
};

static struct event_constraint snbep_uncore_r3qpi_constraints[] = {
	UNCORE_EVENT_CONSTRAINT(0x10, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x11, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x12, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x13, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x20, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x21, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x22, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x23, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x24, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x25, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x26, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x28, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x29, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2a, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2b, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2c, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2d, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2e, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2f, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x30, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x31, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x32, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x33, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x34, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x36, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x37, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x38, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x39, 0x3),
	EVENT_CONSTRAINT_END
};

static struct intel_uncore_type snbep_uncore_ubox = {
	.name		= "ubox",
	.num_counters   = 2,
	.num_boxes	= 1,
	.perf_ctr_bits	= 44,
	.fixed_ctr_bits	= 48,
	.perf_ctr	= SNBEP_U_MSR_PMON_CTR0,
	.event_ctl	= SNBEP_U_MSR_PMON_CTL0,
	.event_mask	= SNBEP_U_MSR_PMON_RAW_EVENT_MASK,
	.fixed_ctr	= SNBEP_U_MSR_PMON_UCLK_FIXED_CTR,
	.fixed_ctl	= SNBEP_U_MSR_PMON_UCLK_FIXED_CTL,
	.ops		= &snbep_uncore_msr_ops,
	.format_group	= &snbep_uncore_ubox_format_group,
};

static struct extra_reg snbep_uncore_cbox_extra_regs[] = {
	SNBEP_CBO_EVENT_EXTRA_REG(SNBEP_CBO_PMON_CTL_TID_EN,
				  SNBEP_CBO_PMON_CTL_TID_EN, 0x1),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0334, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4334, 0xffff, 0x6),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0534, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4534, 0xffff, 0x6),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0934, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4934, 0xffff, 0x6),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4134, 0xffff, 0x6),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0135, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0335, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4135, 0xffff, 0xa),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4335, 0xffff, 0xa),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4435, 0xffff, 0x2),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4835, 0xffff, 0x2),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4a35, 0xffff, 0x2),
	SNBEP_CBO_EVENT_EXTRA_REG(0x5035, 0xffff, 0x2),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0136, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0336, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4136, 0xffff, 0xa),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4336, 0xffff, 0xa),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4436, 0xffff, 0x2),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4836, 0xffff, 0x2),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4a36, 0xffff, 0x2),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4037, 0x40ff, 0x2),
	EVENT_EXTRA_END
};

static void snbep_cbox_put_constraint(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event_extra *reg1 = &event->hw.extra_reg;
	struct intel_uncore_extra_reg *er = &box->shared_regs[0];
	int i;

	if (uncore_box_is_fake(box))
		return;

	for (i = 0; i < 5; i++) {
		if (reg1->alloc & (0x1 << i))
			atomic_sub(1 << (i * 6), &er->ref);
	}
	reg1->alloc = 0;
}

static struct event_constraint *
__snbep_cbox_get_constraint(struct intel_uncore_box *box, struct perf_event *event,
			    u64 (*cbox_filter_mask)(int fields))
{
	struct hw_perf_event_extra *reg1 = &event->hw.extra_reg;
	struct intel_uncore_extra_reg *er = &box->shared_regs[0];
	int i, alloc = 0;
	unsigned long flags;
	u64 mask;

	if (reg1->idx == EXTRA_REG_NONE)
		return NULL;

	raw_spin_lock_irqsave(&er->lock, flags);
	for (i = 0; i < 5; i++) {
		if (!(reg1->idx & (0x1 << i)))
			continue;
		if (!uncore_box_is_fake(box) && (reg1->alloc & (0x1 << i)))
			continue;

		mask = cbox_filter_mask(0x1 << i);
		if (!__BITS_VALUE(atomic_read(&er->ref), i, 6) ||
		    !((reg1->config ^ er->config) & mask)) {
			atomic_add(1 << (i * 6), &er->ref);
			er->config &= ~mask;
			er->config |= reg1->config & mask;
			alloc |= (0x1 << i);
		} else {
			break;
		}
	}
	raw_spin_unlock_irqrestore(&er->lock, flags);
	if (i < 5)
		goto fail;

	if (!uncore_box_is_fake(box))
		reg1->alloc |= alloc;

	return NULL;
fail:
	for (; i >= 0; i--) {
		if (alloc & (0x1 << i))
			atomic_sub(1 << (i * 6), &er->ref);
	}
	return &uncore_constraint_empty;
}

static u64 snbep_cbox_filter_mask(int fields)
{
	u64 mask = 0;

	if (fields & 0x1)
		mask |= SNBEP_CB0_MSR_PMON_BOX_FILTER_TID;
	if (fields & 0x2)
		mask |= SNBEP_CB0_MSR_PMON_BOX_FILTER_NID;
	if (fields & 0x4)
		mask |= SNBEP_CB0_MSR_PMON_BOX_FILTER_STATE;
	if (fields & 0x8)
		mask |= SNBEP_CB0_MSR_PMON_BOX_FILTER_OPC;

	return mask;
}

static struct event_constraint *
snbep_cbox_get_constraint(struct intel_uncore_box *box, struct perf_event *event)
{
	return __snbep_cbox_get_constraint(box, event, snbep_cbox_filter_mask);
}

static int snbep_cbox_hw_config(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event_extra *reg1 = &event->hw.extra_reg;
	struct extra_reg *er;
	int idx = 0;

	for (er = snbep_uncore_cbox_extra_regs; er->msr; er++) {
		if (er->event != (event->hw.config & er->config_mask))
			continue;
		idx |= er->idx;
	}

	if (idx) {
		reg1->reg = SNBEP_C0_MSR_PMON_BOX_FILTER +
			SNBEP_CBO_MSR_OFFSET * box->pmu->pmu_idx;
		reg1->config = event->attr.config1 & snbep_cbox_filter_mask(idx);
		reg1->idx = idx;
	}
	return 0;
}

static struct intel_uncore_ops snbep_uncore_cbox_ops = {
	SNBEP_UNCORE_MSR_OPS_COMMON_INIT(),
	.hw_config		= snbep_cbox_hw_config,
	.get_constraint		= snbep_cbox_get_constraint,
	.put_constraint		= snbep_cbox_put_constraint,
};

static struct intel_uncore_type snbep_uncore_cbox = {
	.name			= "cbox",
	.num_counters		= 4,
	.num_boxes		= 8,
	.perf_ctr_bits		= 44,
	.event_ctl		= SNBEP_C0_MSR_PMON_CTL0,
	.perf_ctr		= SNBEP_C0_MSR_PMON_CTR0,
	.event_mask		= SNBEP_CBO_MSR_PMON_RAW_EVENT_MASK,
	.box_ctl		= SNBEP_C0_MSR_PMON_BOX_CTL,
	.msr_offset		= SNBEP_CBO_MSR_OFFSET,
	.num_shared_regs	= 1,
	.constraints		= snbep_uncore_cbox_constraints,
	.ops			= &snbep_uncore_cbox_ops,
	.format_group		= &snbep_uncore_cbox_format_group,
};

static u64 snbep_pcu_alter_er(struct perf_event *event, int new_idx, bool modify)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hw_perf_event_extra *reg1 = &hwc->extra_reg;
	u64 config = reg1->config;

	if (new_idx > reg1->idx)
		config <<= 8 * (new_idx - reg1->idx);
	else
		config >>= 8 * (reg1->idx - new_idx);

	if (modify) {
		hwc->config += new_idx - reg1->idx;
		reg1->config = config;
		reg1->idx = new_idx;
	}
	return config;
}

static struct event_constraint *
snbep_pcu_get_constraint(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event_extra *reg1 = &event->hw.extra_reg;
	struct intel_uncore_extra_reg *er = &box->shared_regs[0];
	unsigned long flags;
	int idx = reg1->idx;
	u64 mask, config1 = reg1->config;
	bool ok = false;

	if (reg1->idx == EXTRA_REG_NONE ||
	    (!uncore_box_is_fake(box) && reg1->alloc))
		return NULL;
again:
	mask = 0xffULL << (idx * 8);
	raw_spin_lock_irqsave(&er->lock, flags);
	if (!__BITS_VALUE(atomic_read(&er->ref), idx, 8) ||
	    !((config1 ^ er->config) & mask)) {
		atomic_add(1 << (idx * 8), &er->ref);
		er->config &= ~mask;
		er->config |= config1 & mask;
		ok = true;
	}
	raw_spin_unlock_irqrestore(&er->lock, flags);

	if (!ok) {
		idx = (idx + 1) % 4;
		if (idx != reg1->idx) {
			config1 = snbep_pcu_alter_er(event, idx, false);
			goto again;
		}
		return &uncore_constraint_empty;
	}

	if (!uncore_box_is_fake(box)) {
		if (idx != reg1->idx)
			snbep_pcu_alter_er(event, idx, true);
		reg1->alloc = 1;
	}
	return NULL;
}

static void snbep_pcu_put_constraint(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event_extra *reg1 = &event->hw.extra_reg;
	struct intel_uncore_extra_reg *er = &box->shared_regs[0];

	if (uncore_box_is_fake(box) || !reg1->alloc)
		return;

	atomic_sub(1 << (reg1->idx * 8), &er->ref);
	reg1->alloc = 0;
}

static int snbep_pcu_hw_config(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hw_perf_event_extra *reg1 = &hwc->extra_reg;
	int ev_sel = hwc->config & SNBEP_PMON_CTL_EV_SEL_MASK;

	if (ev_sel >= 0xb && ev_sel <= 0xe) {
		reg1->reg = SNBEP_PCU_MSR_PMON_BOX_FILTER;
		reg1->idx = ev_sel - 0xb;
		reg1->config = event->attr.config1 & (0xff << reg1->idx);
	}
	return 0;
}

static struct intel_uncore_ops snbep_uncore_pcu_ops = {
	SNBEP_UNCORE_MSR_OPS_COMMON_INIT(),
	.hw_config		= snbep_pcu_hw_config,
	.get_constraint		= snbep_pcu_get_constraint,
	.put_constraint		= snbep_pcu_put_constraint,
};

static struct intel_uncore_type snbep_uncore_pcu = {
	.name			= "pcu",
	.num_counters		= 4,
	.num_boxes		= 1,
	.perf_ctr_bits		= 48,
	.perf_ctr		= SNBEP_PCU_MSR_PMON_CTR0,
	.event_ctl		= SNBEP_PCU_MSR_PMON_CTL0,
	.event_mask		= SNBEP_PCU_MSR_PMON_RAW_EVENT_MASK,
	.box_ctl		= SNBEP_PCU_MSR_PMON_BOX_CTL,
	.num_shared_regs	= 1,
	.ops			= &snbep_uncore_pcu_ops,
	.format_group		= &snbep_uncore_pcu_format_group,
};

static struct intel_uncore_type *snbep_msr_uncores[] = {
	&snbep_uncore_ubox,
	&snbep_uncore_cbox,
	&snbep_uncore_pcu,
	NULL,
};

void snbep_uncore_cpu_init(void)
{
	if (snbep_uncore_cbox.num_boxes > boot_cpu_data.x86_max_cores)
		snbep_uncore_cbox.num_boxes = boot_cpu_data.x86_max_cores;
	uncore_msr_uncores = snbep_msr_uncores;
}

enum {
	SNBEP_PCI_QPI_PORT0_FILTER,
	SNBEP_PCI_QPI_PORT1_FILTER,
};

static int snbep_qpi_hw_config(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hw_perf_event_extra *reg1 = &hwc->extra_reg;
	struct hw_perf_event_extra *reg2 = &hwc->branch_reg;

	if ((hwc->config & SNBEP_PMON_CTL_EV_SEL_MASK) == 0x38) {
		reg1->idx = 0;
		reg1->reg = SNBEP_Q_Py_PCI_PMON_PKT_MATCH0;
		reg1->config = event->attr.config1;
		reg2->reg = SNBEP_Q_Py_PCI_PMON_PKT_MASK0;
		reg2->config = event->attr.config2;
	}
	return 0;
}

static void snbep_qpi_enable_event(struct intel_uncore_box *box, struct perf_event *event)
{
	struct pci_dev *pdev = box->pci_dev;
	struct hw_perf_event *hwc = &event->hw;
	struct hw_perf_event_extra *reg1 = &hwc->extra_reg;
	struct hw_perf_event_extra *reg2 = &hwc->branch_reg;

	if (reg1->idx != EXTRA_REG_NONE) {
		int idx = box->pmu->pmu_idx + SNBEP_PCI_QPI_PORT0_FILTER;
		struct pci_dev *filter_pdev = uncore_extra_pci_dev[box->phys_id][idx];
		WARN_ON_ONCE(!filter_pdev);
		if (filter_pdev) {
			pci_write_config_dword(filter_pdev, reg1->reg,
						(u32)reg1->config);
			pci_write_config_dword(filter_pdev, reg1->reg + 4,
						(u32)(reg1->config >> 32));
			pci_write_config_dword(filter_pdev, reg2->reg,
						(u32)reg2->config);
			pci_write_config_dword(filter_pdev, reg2->reg + 4,
						(u32)(reg2->config >> 32));
		}
	}

	pci_write_config_dword(pdev, hwc->config_base, hwc->config | SNBEP_PMON_CTL_EN);
}

static struct intel_uncore_ops snbep_uncore_qpi_ops = {
	SNBEP_UNCORE_PCI_OPS_COMMON_INIT(),
	.enable_event		= snbep_qpi_enable_event,
	.hw_config		= snbep_qpi_hw_config,
	.get_constraint		= uncore_get_constraint,
	.put_constraint		= uncore_put_constraint,
};

#define SNBEP_UNCORE_PCI_COMMON_INIT()				\
	.perf_ctr	= SNBEP_PCI_PMON_CTR0,			\
	.event_ctl	= SNBEP_PCI_PMON_CTL0,			\
	.event_mask	= SNBEP_PMON_RAW_EVENT_MASK,		\
	.box_ctl	= SNBEP_PCI_PMON_BOX_CTL,		\
	.ops		= &snbep_uncore_pci_ops,		\
	.format_group	= &snbep_uncore_format_group

static struct intel_uncore_type snbep_uncore_ha = {
	.name		= "ha",
	.num_counters   = 4,
	.num_boxes	= 1,
	.perf_ctr_bits	= 48,
	SNBEP_UNCORE_PCI_COMMON_INIT(),
};

static struct intel_uncore_type snbep_uncore_imc = {
	.name		= "imc",
	.num_counters   = 4,
	.num_boxes	= 4,
	.perf_ctr_bits	= 48,
	.fixed_ctr_bits	= 48,
	.fixed_ctr	= SNBEP_MC_CHy_PCI_PMON_FIXED_CTR,
	.fixed_ctl	= SNBEP_MC_CHy_PCI_PMON_FIXED_CTL,
	.event_descs	= snbep_uncore_imc_events,
	SNBEP_UNCORE_PCI_COMMON_INIT(),
};

static struct intel_uncore_type snbep_uncore_qpi = {
	.name			= "qpi",
	.num_counters		= 4,
	.num_boxes		= 2,
	.perf_ctr_bits		= 48,
	.perf_ctr		= SNBEP_PCI_PMON_CTR0,
	.event_ctl		= SNBEP_PCI_PMON_CTL0,
	.event_mask		= SNBEP_QPI_PCI_PMON_RAW_EVENT_MASK,
	.box_ctl		= SNBEP_PCI_PMON_BOX_CTL,
	.num_shared_regs	= 1,
	.ops			= &snbep_uncore_qpi_ops,
	.event_descs		= snbep_uncore_qpi_events,
	.format_group		= &snbep_uncore_qpi_format_group,
};


static struct intel_uncore_type snbep_uncore_r2pcie = {
	.name		= "r2pcie",
	.num_counters   = 4,
	.num_boxes	= 1,
	.perf_ctr_bits	= 44,
	.constraints	= snbep_uncore_r2pcie_constraints,
	SNBEP_UNCORE_PCI_COMMON_INIT(),
};

static struct intel_uncore_type snbep_uncore_r3qpi = {
	.name		= "r3qpi",
	.num_counters   = 3,
	.num_boxes	= 2,
	.perf_ctr_bits	= 44,
	.constraints	= snbep_uncore_r3qpi_constraints,
	SNBEP_UNCORE_PCI_COMMON_INIT(),
};

enum {
	SNBEP_PCI_UNCORE_HA,
	SNBEP_PCI_UNCORE_IMC,
	SNBEP_PCI_UNCORE_QPI,
	SNBEP_PCI_UNCORE_R2PCIE,
	SNBEP_PCI_UNCORE_R3QPI,
};

static struct intel_uncore_type *snbep_pci_uncores[] = {
	[SNBEP_PCI_UNCORE_HA]		= &snbep_uncore_ha,
	[SNBEP_PCI_UNCORE_IMC]		= &snbep_uncore_imc,
	[SNBEP_PCI_UNCORE_QPI]		= &snbep_uncore_qpi,
	[SNBEP_PCI_UNCORE_R2PCIE]	= &snbep_uncore_r2pcie,
	[SNBEP_PCI_UNCORE_R3QPI]	= &snbep_uncore_r3qpi,
	NULL,
};

static DEFINE_PCI_DEVICE_TABLE(snbep_uncore_pci_ids) = {
	{ /* Home Agent */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_UNC_HA),
		.driver_data = UNCORE_PCI_DEV_DATA(SNBEP_PCI_UNCORE_HA, 0),
	},
	{ /* MC Channel 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_UNC_IMC0),
		.driver_data = UNCORE_PCI_DEV_DATA(SNBEP_PCI_UNCORE_IMC, 0),
	},
	{ /* MC Channel 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_UNC_IMC1),
		.driver_data = UNCORE_PCI_DEV_DATA(SNBEP_PCI_UNCORE_IMC, 1),
	},
	{ /* MC Channel 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_UNC_IMC2),
		.driver_data = UNCORE_PCI_DEV_DATA(SNBEP_PCI_UNCORE_IMC, 2),
	},
	{ /* MC Channel 3 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_UNC_IMC3),
		.driver_data = UNCORE_PCI_DEV_DATA(SNBEP_PCI_UNCORE_IMC, 3),
	},
	{ /* QPI Port 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_UNC_QPI0),
		.driver_data = UNCORE_PCI_DEV_DATA(SNBEP_PCI_UNCORE_QPI, 0),
	},
	{ /* QPI Port 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_UNC_QPI1),
		.driver_data = UNCORE_PCI_DEV_DATA(SNBEP_PCI_UNCORE_QPI, 1),
	},
	{ /* R2PCIe */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_UNC_R2PCIE),
		.driver_data = UNCORE_PCI_DEV_DATA(SNBEP_PCI_UNCORE_R2PCIE, 0),
	},
	{ /* R3QPI Link 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_UNC_R3QPI0),
		.driver_data = UNCORE_PCI_DEV_DATA(SNBEP_PCI_UNCORE_R3QPI, 0),
	},
	{ /* R3QPI Link 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_UNC_R3QPI1),
		.driver_data = UNCORE_PCI_DEV_DATA(SNBEP_PCI_UNCORE_R3QPI, 1),
	},
	{ /* QPI Port 0 filter  */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x3c86),
		.driver_data = UNCORE_PCI_DEV_DATA(UNCORE_EXTRA_PCI_DEV,
						   SNBEP_PCI_QPI_PORT0_FILTER),
	},
	{ /* QPI Port 0 filter  */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x3c96),
		.driver_data = UNCORE_PCI_DEV_DATA(UNCORE_EXTRA_PCI_DEV,
						   SNBEP_PCI_QPI_PORT1_FILTER),
	},
	{ /* end: all zeroes */ }
};

static struct pci_driver snbep_uncore_pci_driver = {
	.name		= "snbep_uncore",
	.id_table	= snbep_uncore_pci_ids,
};

/*
 * build pci bus to socket mapping
 */
static int snbep_pci2phy_map_init(int devid)
{
	struct pci_dev *ubox_dev = NULL;
	int i, bus, nodeid;
	int err = 0;
	u32 config = 0;

	while (1) {
		/* find the UBOX device */
		ubox_dev = pci_get_device(PCI_VENDOR_ID_INTEL, devid, ubox_dev);
		if (!ubox_dev)
			break;
		bus = ubox_dev->bus->number;
		/* get the Node ID of the local register */
		err = pci_read_config_dword(ubox_dev, 0x40, &config);
		if (err)
			break;
		nodeid = config;
		/* get the Node ID mapping */
		err = pci_read_config_dword(ubox_dev, 0x54, &config);
		if (err)
			break;
		/*
		 * every three bits in the Node ID mapping register maps
		 * to a particular node.
		 */
		for (i = 0; i < 8; i++) {
			if (nodeid == ((config >> (3 * i)) & 0x7)) {
				uncore_pcibus_to_physid[bus] = i;
				break;
			}
		}
	}

	if (!err) {
		/*
		 * For PCI bus with no UBOX device, find the next bus
		 * that has UBOX device and use its mapping.
		 */
		i = -1;
		for (bus = 255; bus >= 0; bus--) {
			if (uncore_pcibus_to_physid[bus] >= 0)
				i = uncore_pcibus_to_physid[bus];
			else
				uncore_pcibus_to_physid[bus] = i;
		}
	}

	if (ubox_dev)
		pci_dev_put(ubox_dev);

	return err ? pcibios_err_to_errno(err) : 0;
}

int snbep_uncore_pci_init(void)
{
	int ret = snbep_pci2phy_map_init(0x3ce0);
	if (ret)
		return ret;
	uncore_pci_uncores = snbep_pci_uncores;
	uncore_pci_driver = &snbep_uncore_pci_driver;
	return 0;
}
/* end of Sandy Bridge-EP uncore support */

/* IvyTown uncore support */
static void ivt_uncore_msr_init_box(struct intel_uncore_box *box)
{
	unsigned msr = uncore_msr_box_ctl(box);
	if (msr)
		wrmsrl(msr, IVT_PMON_BOX_CTL_INT);
}

static void ivt_uncore_pci_init_box(struct intel_uncore_box *box)
{
	struct pci_dev *pdev = box->pci_dev;

	pci_write_config_dword(pdev, SNBEP_PCI_PMON_BOX_CTL, IVT_PMON_BOX_CTL_INT);
}

#define IVT_UNCORE_MSR_OPS_COMMON_INIT()			\
	.init_box	= ivt_uncore_msr_init_box,		\
	.disable_box	= snbep_uncore_msr_disable_box,		\
	.enable_box	= snbep_uncore_msr_enable_box,		\
	.disable_event	= snbep_uncore_msr_disable_event,	\
	.enable_event	= snbep_uncore_msr_enable_event,	\
	.read_counter	= uncore_msr_read_counter

static struct intel_uncore_ops ivt_uncore_msr_ops = {
	IVT_UNCORE_MSR_OPS_COMMON_INIT(),
};

static struct intel_uncore_ops ivt_uncore_pci_ops = {
	.init_box	= ivt_uncore_pci_init_box,
	.disable_box	= snbep_uncore_pci_disable_box,
	.enable_box	= snbep_uncore_pci_enable_box,
	.disable_event	= snbep_uncore_pci_disable_event,
	.enable_event	= snbep_uncore_pci_enable_event,
	.read_counter	= snbep_uncore_pci_read_counter,
};

#define IVT_UNCORE_PCI_COMMON_INIT()				\
	.perf_ctr	= SNBEP_PCI_PMON_CTR0,			\
	.event_ctl	= SNBEP_PCI_PMON_CTL0,			\
	.event_mask	= IVT_PMON_RAW_EVENT_MASK,		\
	.box_ctl	= SNBEP_PCI_PMON_BOX_CTL,		\
	.ops		= &ivt_uncore_pci_ops,			\
	.format_group	= &ivt_uncore_format_group

static struct attribute *ivt_uncore_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_inv.attr,
	&format_attr_thresh8.attr,
	NULL,
};

static struct attribute *ivt_uncore_ubox_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_inv.attr,
	&format_attr_thresh5.attr,
	NULL,
};

static struct attribute *ivt_uncore_cbox_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_tid_en.attr,
	&format_attr_thresh8.attr,
	&format_attr_filter_tid.attr,
	&format_attr_filter_link.attr,
	&format_attr_filter_state2.attr,
	&format_attr_filter_nid2.attr,
	&format_attr_filter_opc2.attr,
	NULL,
};

static struct attribute *ivt_uncore_pcu_formats_attr[] = {
	&format_attr_event_ext.attr,
	&format_attr_occ_sel.attr,
	&format_attr_edge.attr,
	&format_attr_thresh5.attr,
	&format_attr_occ_invert.attr,
	&format_attr_occ_edge.attr,
	&format_attr_filter_band0.attr,
	&format_attr_filter_band1.attr,
	&format_attr_filter_band2.attr,
	&format_attr_filter_band3.attr,
	NULL,
};

static struct attribute *ivt_uncore_qpi_formats_attr[] = {
	&format_attr_event_ext.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_thresh8.attr,
	&format_attr_match_rds.attr,
	&format_attr_match_rnid30.attr,
	&format_attr_match_rnid4.attr,
	&format_attr_match_dnid.attr,
	&format_attr_match_mc.attr,
	&format_attr_match_opc.attr,
	&format_attr_match_vnw.attr,
	&format_attr_match0.attr,
	&format_attr_match1.attr,
	&format_attr_mask_rds.attr,
	&format_attr_mask_rnid30.attr,
	&format_attr_mask_rnid4.attr,
	&format_attr_mask_dnid.attr,
	&format_attr_mask_mc.attr,
	&format_attr_mask_opc.attr,
	&format_attr_mask_vnw.attr,
	&format_attr_mask0.attr,
	&format_attr_mask1.attr,
	NULL,
};

static struct attribute_group ivt_uncore_format_group = {
	.name = "format",
	.attrs = ivt_uncore_formats_attr,
};

static struct attribute_group ivt_uncore_ubox_format_group = {
	.name = "format",
	.attrs = ivt_uncore_ubox_formats_attr,
};

static struct attribute_group ivt_uncore_cbox_format_group = {
	.name = "format",
	.attrs = ivt_uncore_cbox_formats_attr,
};

static struct attribute_group ivt_uncore_pcu_format_group = {
	.name = "format",
	.attrs = ivt_uncore_pcu_formats_attr,
};

static struct attribute_group ivt_uncore_qpi_format_group = {
	.name = "format",
	.attrs = ivt_uncore_qpi_formats_attr,
};

static struct intel_uncore_type ivt_uncore_ubox = {
	.name		= "ubox",
	.num_counters   = 2,
	.num_boxes	= 1,
	.perf_ctr_bits	= 44,
	.fixed_ctr_bits	= 48,
	.perf_ctr	= SNBEP_U_MSR_PMON_CTR0,
	.event_ctl	= SNBEP_U_MSR_PMON_CTL0,
	.event_mask	= IVT_U_MSR_PMON_RAW_EVENT_MASK,
	.fixed_ctr	= SNBEP_U_MSR_PMON_UCLK_FIXED_CTR,
	.fixed_ctl	= SNBEP_U_MSR_PMON_UCLK_FIXED_CTL,
	.ops		= &ivt_uncore_msr_ops,
	.format_group	= &ivt_uncore_ubox_format_group,
};

static struct extra_reg ivt_uncore_cbox_extra_regs[] = {
	SNBEP_CBO_EVENT_EXTRA_REG(SNBEP_CBO_PMON_CTL_TID_EN,
				  SNBEP_CBO_PMON_CTL_TID_EN, 0x1),
	SNBEP_CBO_EVENT_EXTRA_REG(0x1031, 0x10ff, 0x2),
	SNBEP_CBO_EVENT_EXTRA_REG(0x1134, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4134, 0xffff, 0xc),
	SNBEP_CBO_EVENT_EXTRA_REG(0x5134, 0xffff, 0xc),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0334, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4334, 0xffff, 0xc),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0534, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4534, 0xffff, 0xc),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0934, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4934, 0xffff, 0xc),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0135, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0335, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x2135, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x2335, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4135, 0xffff, 0x18),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4335, 0xffff, 0x18),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4435, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4835, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4a35, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x5035, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x8135, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x8335, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0136, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0336, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x2136, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x2336, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4136, 0xffff, 0x18),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4336, 0xffff, 0x18),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4436, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4836, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4a36, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x5036, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x8136, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x8336, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4037, 0x40ff, 0x8),
	EVENT_EXTRA_END
};

static u64 ivt_cbox_filter_mask(int fields)
{
	u64 mask = 0;

	if (fields & 0x1)
		mask |= IVT_CB0_MSR_PMON_BOX_FILTER_TID;
	if (fields & 0x2)
		mask |= IVT_CB0_MSR_PMON_BOX_FILTER_LINK;
	if (fields & 0x4)
		mask |= IVT_CB0_MSR_PMON_BOX_FILTER_STATE;
	if (fields & 0x8)
		mask |= IVT_CB0_MSR_PMON_BOX_FILTER_NID;
	if (fields & 0x10)
		mask |= IVT_CB0_MSR_PMON_BOX_FILTER_OPC;

	return mask;
}

static struct event_constraint *
ivt_cbox_get_constraint(struct intel_uncore_box *box, struct perf_event *event)
{
	return __snbep_cbox_get_constraint(box, event, ivt_cbox_filter_mask);
}

static int ivt_cbox_hw_config(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event_extra *reg1 = &event->hw.extra_reg;
	struct extra_reg *er;
	int idx = 0;

	for (er = ivt_uncore_cbox_extra_regs; er->msr; er++) {
		if (er->event != (event->hw.config & er->config_mask))
			continue;
		idx |= er->idx;
	}

	if (idx) {
		reg1->reg = SNBEP_C0_MSR_PMON_BOX_FILTER +
			SNBEP_CBO_MSR_OFFSET * box->pmu->pmu_idx;
		reg1->config = event->attr.config1 & ivt_cbox_filter_mask(idx);
		reg1->idx = idx;
	}
	return 0;
}

static void ivt_cbox_enable_event(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hw_perf_event_extra *reg1 = &hwc->extra_reg;

	if (reg1->idx != EXTRA_REG_NONE) {
		u64 filter = uncore_shared_reg_config(box, 0);
		wrmsrl(reg1->reg, filter & 0xffffffff);
		wrmsrl(reg1->reg + 6, filter >> 32);
	}

	wrmsrl(hwc->config_base, hwc->config | SNBEP_PMON_CTL_EN);
}

static struct intel_uncore_ops ivt_uncore_cbox_ops = {
	.init_box		= ivt_uncore_msr_init_box,
	.disable_box		= snbep_uncore_msr_disable_box,
	.enable_box		= snbep_uncore_msr_enable_box,
	.disable_event		= snbep_uncore_msr_disable_event,
	.enable_event		= ivt_cbox_enable_event,
	.read_counter		= uncore_msr_read_counter,
	.hw_config		= ivt_cbox_hw_config,
	.get_constraint		= ivt_cbox_get_constraint,
	.put_constraint		= snbep_cbox_put_constraint,
};

static struct intel_uncore_type ivt_uncore_cbox = {
	.name			= "cbox",
	.num_counters		= 4,
	.num_boxes		= 15,
	.perf_ctr_bits		= 44,
	.event_ctl		= SNBEP_C0_MSR_PMON_CTL0,
	.perf_ctr		= SNBEP_C0_MSR_PMON_CTR0,
	.event_mask		= IVT_CBO_MSR_PMON_RAW_EVENT_MASK,
	.box_ctl		= SNBEP_C0_MSR_PMON_BOX_CTL,
	.msr_offset		= SNBEP_CBO_MSR_OFFSET,
	.num_shared_regs	= 1,
	.constraints		= snbep_uncore_cbox_constraints,
	.ops			= &ivt_uncore_cbox_ops,
	.format_group		= &ivt_uncore_cbox_format_group,
};

static struct intel_uncore_ops ivt_uncore_pcu_ops = {
	IVT_UNCORE_MSR_OPS_COMMON_INIT(),
	.hw_config		= snbep_pcu_hw_config,
	.get_constraint		= snbep_pcu_get_constraint,
	.put_constraint		= snbep_pcu_put_constraint,
};

static struct intel_uncore_type ivt_uncore_pcu = {
	.name			= "pcu",
	.num_counters		= 4,
	.num_boxes		= 1,
	.perf_ctr_bits		= 48,
	.perf_ctr		= SNBEP_PCU_MSR_PMON_CTR0,
	.event_ctl		= SNBEP_PCU_MSR_PMON_CTL0,
	.event_mask		= IVT_PCU_MSR_PMON_RAW_EVENT_MASK,
	.box_ctl		= SNBEP_PCU_MSR_PMON_BOX_CTL,
	.num_shared_regs	= 1,
	.ops			= &ivt_uncore_pcu_ops,
	.format_group		= &ivt_uncore_pcu_format_group,
};

static struct intel_uncore_type *ivt_msr_uncores[] = {
	&ivt_uncore_ubox,
	&ivt_uncore_cbox,
	&ivt_uncore_pcu,
	NULL,
};

void ivt_uncore_cpu_init(void)
{
	if (ivt_uncore_cbox.num_boxes > boot_cpu_data.x86_max_cores)
		ivt_uncore_cbox.num_boxes = boot_cpu_data.x86_max_cores;
	uncore_msr_uncores = ivt_msr_uncores;
}

static struct intel_uncore_type ivt_uncore_ha = {
	.name		= "ha",
	.num_counters   = 4,
	.num_boxes	= 2,
	.perf_ctr_bits	= 48,
	IVT_UNCORE_PCI_COMMON_INIT(),
};

static struct intel_uncore_type ivt_uncore_imc = {
	.name		= "imc",
	.num_counters   = 4,
	.num_boxes	= 8,
	.perf_ctr_bits	= 48,
	.fixed_ctr_bits	= 48,
	.fixed_ctr	= SNBEP_MC_CHy_PCI_PMON_FIXED_CTR,
	.fixed_ctl	= SNBEP_MC_CHy_PCI_PMON_FIXED_CTL,
	IVT_UNCORE_PCI_COMMON_INIT(),
};

/* registers in IRP boxes are not properly aligned */
static unsigned ivt_uncore_irp_ctls[] = {0xd8, 0xdc, 0xe0, 0xe4};
static unsigned ivt_uncore_irp_ctrs[] = {0xa0, 0xb0, 0xb8, 0xc0};

static void ivt_uncore_irp_enable_event(struct intel_uncore_box *box, struct perf_event *event)
{
	struct pci_dev *pdev = box->pci_dev;
	struct hw_perf_event *hwc = &event->hw;

	pci_write_config_dword(pdev, ivt_uncore_irp_ctls[hwc->idx],
			       hwc->config | SNBEP_PMON_CTL_EN);
}

static void ivt_uncore_irp_disable_event(struct intel_uncore_box *box, struct perf_event *event)
{
	struct pci_dev *pdev = box->pci_dev;
	struct hw_perf_event *hwc = &event->hw;

	pci_write_config_dword(pdev, ivt_uncore_irp_ctls[hwc->idx], hwc->config);
}

static u64 ivt_uncore_irp_read_counter(struct intel_uncore_box *box, struct perf_event *event)
{
	struct pci_dev *pdev = box->pci_dev;
	struct hw_perf_event *hwc = &event->hw;
	u64 count = 0;

	pci_read_config_dword(pdev, ivt_uncore_irp_ctrs[hwc->idx], (u32 *)&count);
	pci_read_config_dword(pdev, ivt_uncore_irp_ctrs[hwc->idx] + 4, (u32 *)&count + 1);

	return count;
}

static struct intel_uncore_ops ivt_uncore_irp_ops = {
	.init_box	= ivt_uncore_pci_init_box,
	.disable_box	= snbep_uncore_pci_disable_box,
	.enable_box	= snbep_uncore_pci_enable_box,
	.disable_event	= ivt_uncore_irp_disable_event,
	.enable_event	= ivt_uncore_irp_enable_event,
	.read_counter	= ivt_uncore_irp_read_counter,
};

static struct intel_uncore_type ivt_uncore_irp = {
	.name			= "irp",
	.num_counters		= 4,
	.num_boxes		= 1,
	.perf_ctr_bits		= 48,
	.event_mask		= IVT_PMON_RAW_EVENT_MASK,
	.box_ctl		= SNBEP_PCI_PMON_BOX_CTL,
	.ops			= &ivt_uncore_irp_ops,
	.format_group		= &ivt_uncore_format_group,
};

static struct intel_uncore_ops ivt_uncore_qpi_ops = {
	.init_box	= ivt_uncore_pci_init_box,
	.disable_box	= snbep_uncore_pci_disable_box,
	.enable_box	= snbep_uncore_pci_enable_box,
	.disable_event	= snbep_uncore_pci_disable_event,
	.enable_event	= snbep_qpi_enable_event,
	.read_counter	= snbep_uncore_pci_read_counter,
	.hw_config	= snbep_qpi_hw_config,
	.get_constraint	= uncore_get_constraint,
	.put_constraint	= uncore_put_constraint,
};

static struct intel_uncore_type ivt_uncore_qpi = {
	.name			= "qpi",
	.num_counters		= 4,
	.num_boxes		= 3,
	.perf_ctr_bits		= 48,
	.perf_ctr		= SNBEP_PCI_PMON_CTR0,
	.event_ctl		= SNBEP_PCI_PMON_CTL0,
	.event_mask		= IVT_QPI_PCI_PMON_RAW_EVENT_MASK,
	.box_ctl		= SNBEP_PCI_PMON_BOX_CTL,
	.num_shared_regs	= 1,
	.ops			= &ivt_uncore_qpi_ops,
	.format_group		= &ivt_uncore_qpi_format_group,
};

static struct intel_uncore_type ivt_uncore_r2pcie = {
	.name		= "r2pcie",
	.num_counters   = 4,
	.num_boxes	= 1,
	.perf_ctr_bits	= 44,
	.constraints	= snbep_uncore_r2pcie_constraints,
	IVT_UNCORE_PCI_COMMON_INIT(),
};

static struct intel_uncore_type ivt_uncore_r3qpi = {
	.name		= "r3qpi",
	.num_counters   = 3,
	.num_boxes	= 2,
	.perf_ctr_bits	= 44,
	.constraints	= snbep_uncore_r3qpi_constraints,
	IVT_UNCORE_PCI_COMMON_INIT(),
};

enum {
	IVT_PCI_UNCORE_HA,
	IVT_PCI_UNCORE_IMC,
	IVT_PCI_UNCORE_IRP,
	IVT_PCI_UNCORE_QPI,
	IVT_PCI_UNCORE_R2PCIE,
	IVT_PCI_UNCORE_R3QPI,
};

static struct intel_uncore_type *ivt_pci_uncores[] = {
	[IVT_PCI_UNCORE_HA]	= &ivt_uncore_ha,
	[IVT_PCI_UNCORE_IMC]	= &ivt_uncore_imc,
	[IVT_PCI_UNCORE_IRP]	= &ivt_uncore_irp,
	[IVT_PCI_UNCORE_QPI]	= &ivt_uncore_qpi,
	[IVT_PCI_UNCORE_R2PCIE]	= &ivt_uncore_r2pcie,
	[IVT_PCI_UNCORE_R3QPI]	= &ivt_uncore_r3qpi,
	NULL,
};

static DEFINE_PCI_DEVICE_TABLE(ivt_uncore_pci_ids) = {
	{ /* Home Agent 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe30),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_HA, 0),
	},
	{ /* Home Agent 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe38),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_HA, 1),
	},
	{ /* MC0 Channel 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xeb4),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_IMC, 0),
	},
	{ /* MC0 Channel 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xeb5),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_IMC, 1),
	},
	{ /* MC0 Channel 3 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xeb0),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_IMC, 2),
	},
	{ /* MC0 Channel 4 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xeb1),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_IMC, 3),
	},
	{ /* MC1 Channel 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xef4),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_IMC, 4),
	},
	{ /* MC1 Channel 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xef5),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_IMC, 5),
	},
	{ /* MC1 Channel 3 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xef0),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_IMC, 6),
	},
	{ /* MC1 Channel 4 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xef1),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_IMC, 7),
	},
	{ /* IRP */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe39),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_IRP, 0),
	},
	{ /* QPI0 Port 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe32),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_QPI, 0),
	},
	{ /* QPI0 Port 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe33),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_QPI, 1),
	},
	{ /* QPI1 Port 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe3a),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_QPI, 2),
	},
	{ /* R2PCIe */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe34),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_R2PCIE, 0),
	},
	{ /* R3QPI0 Link 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe36),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_R3QPI, 0),
	},
	{ /* R3QPI0 Link 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe37),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_R3QPI, 1),
	},
	{ /* R3QPI1 Link 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe3e),
		.driver_data = UNCORE_PCI_DEV_DATA(IVT_PCI_UNCORE_R3QPI, 2),
	},
	{ /* QPI Port 0 filter  */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe86),
		.driver_data = UNCORE_PCI_DEV_DATA(UNCORE_EXTRA_PCI_DEV,
						   SNBEP_PCI_QPI_PORT0_FILTER),
	},
	{ /* QPI Port 0 filter  */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe96),
		.driver_data = UNCORE_PCI_DEV_DATA(UNCORE_EXTRA_PCI_DEV,
						   SNBEP_PCI_QPI_PORT1_FILTER),
	},
	{ /* end: all zeroes */ }
};

static struct pci_driver ivt_uncore_pci_driver = {
	.name		= "ivt_uncore",
	.id_table	= ivt_uncore_pci_ids,
};

int ivt_uncore_pci_init(void)
{
	int ret = snbep_pci2phy_map_init(0x0e1e);
	if (ret)
		return ret;
	uncore_pci_uncores = ivt_pci_uncores;
	uncore_pci_driver = &ivt_uncore_pci_driver;
	return 0;
}
/* end of IvyTown uncore support */
