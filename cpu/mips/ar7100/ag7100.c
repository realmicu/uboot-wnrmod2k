#include <config.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <command.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include "ar7100_soc.h"
#include "ag7100.h"
#include "ag7100_phy.h"

#if (CONFIG_COMMANDS & CFG_CMD_MII)
#include <miiphy.h>
#endif
char *mii_str[2][4] = {
    {"GMii", "Mii", "RGMii", "RMii"},
    {"RGMii", "RMii", "INVL1", "INVL2"}
};
char *spd_str[] = {"10Mbps", "100Mbps", "1000Mbps"};
char *dup_str[] = {"half duplex", "full duplex"};
#define ag7100_name2mac(name)	   (strcmp(name,"eth0") ? ag7100_unit2mac(1) : ag7100_unit2mac(0))

int ag7100_miiphy_read(char *devname, uint32_t phaddr,
		       uint8_t reg, uint16_t *value);
int ag7100_miiphy_write(char *devname, uint32_t phaddr,
			uint8_t reg, uint16_t data);

ag7100_mac_t *ag7100_macs[CFG_AG7100_NMACS];

#ifdef CFG_RTL8366S_PHY
extern void rtl8366s_phy_setup(int mac);
#endif

ag7100_mac_t *ag7100_unit2mac(int unit)
{
    return (unit ? ag7100_macs[1] : ag7100_macs[0]);
}

static int
ag7100_send(struct eth_device *dev, volatile void *packet, int length)
{
    int i;
    ag7100_mac_t *mac = (ag7100_mac_t *)dev->priv;

    ag7100_desc_t *f = mac->fifo_tx[mac->next_tx];

#if defined(CFG_ATHRS26_PHY) && defined(CFG_ATHRHDR_EN)
    uint8_t *pkt_buf;

    pkt_buf = (uint8_t *) packet;

    if ((pkt_buf[1] & 0xf) != 0x5) {
        length = length + ATHRHDR_LEN;
        pkt_buf = (uint8_t *) packet - ATHRHDR_LEN;
        pkt_buf[0] = 0x10;  /* broadcast = 0; from_cpu = 0; reserved = 1; port_num = 0 */
        pkt_buf[1] = 0x80;  /* reserved = 0b10; priority = 0; type = 0 (normal) */
    }
    f->pkt_size = length;
    f->pkt_start_addr = virt_to_phys(pkt_buf);
#else
    f->pkt_size = length;
    f->pkt_start_addr = virt_to_phys(packet);
#endif
    ag7100_tx_give_to_dma(f);
    flush_cache((u32) packet, length);
    ag7100_reg_wr(mac, AG7100_DMA_TX_DESC, virt_to_phys(f));
    ag7100_reg_wr(mac, AG7100_DMA_TX_CTRL, AG7100_TXE);

    for (i = 0; i < MAX_WAIT; i++) {
        udelay(10);
        if (!ag7100_tx_owned_by_dma(f))
            break;
    }
    if (i == MAX_WAIT)
        printf("Tx Timed out\n");

    f->pkt_start_addr = 0;
    f->pkt_size = 0;

    if (++mac->next_tx >= NO_OF_TX_FIFOS)
        mac->next_tx = 0;

    return (0);
}

static int ag7100_recv(struct eth_device *dev)
{
    int length;
    ag7100_desc_t *f;
    ag7100_mac_t *mac;
 
    mac = (ag7100_mac_t *)dev->priv;

    for (;;) {
        f = mac->fifo_rx[mac->next_rx];
        if (ag7100_rx_owned_by_dma(f))
            break;

        length = f->pkt_size;

        NetReceive(NetRxPackets[mac->next_rx] , length - 4);
        flush_cache((u32) NetRxPackets[mac->next_rx] , PKTSIZE_ALIGN);

        ag7100_rx_give_to_dma(f);

        if (++mac->next_rx >= NO_OF_RX_FIFOS)
            mac->next_rx = 0;
    }

    if (!(ag7100_reg_rd(mac, AG7100_DMA_RX_CTRL))) {
        ag7100_reg_wr(mac, AG7100_DMA_RX_DESC, virt_to_phys(f));
        ag7100_reg_wr(mac, AG7100_DMA_RX_CTRL, 1);
    }

    return (0);
}

static void ag7100_hw_start(ag7100_mac_t *mac)
{

    ag7100_reg_wr(mac, AG7100_MAC_CFG1, (AG7100_MAC_CFG1_RX_EN |
		    AG7100_MAC_CFG1_TX_EN));

    ag7100_reg_rmw_set(mac, AG7100_MAC_CFG2, (AG7100_MAC_CFG2_PAD_CRC_EN |
        AG7100_MAC_CFG2_LEN_CHECK));

    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_0, 0x1f00);
    /*
    * set the mii if type - NB reg not in the gigE space
    */
    printf("CHH:mac: %d if: %x\n",mac->mac_unit,mii_if(mac));
    ar7100_reg_wr(mii_reg(mac), mii_if(mac));
    ag7100_reg_wr(mac, AG7100_MAC_MII_MGMT_CFG, AG7100_MGMT_CFG_CLK_DIV_20);
    printf("CHH:mac:verify: %d if: %8.8x\n",mac->mac_unit,((unsigned int)ar7100_reg_rd(mii_reg(mac))));

#ifdef CONFIG_AR7100_EMULATION
    ag7100_reg_rmw_set(mac, AG7100_MAC_FIFO_CFG_4, 0x3ffff);
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_1, 0xfff0000);
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_2, 0x1fff);
#else
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_1, 0xfff0000);
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_2, 0x1fff);
    /*
    * Weed out junk frames (CRC errored, short collision'ed frames etc.)
    */
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_4, 0xffff);
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_5, 0x7ffef);
#endif

    printf(": cfg1 %#x cfg2 %#x\n", ag7100_reg_rd(mac, AG7100_MAC_CFG1),
        ag7100_reg_rd(mac, AG7100_MAC_CFG2));
}


#ifdef CONFIG_AR9100
#define ag7100_pll_shift(_mac)      (((_mac)->mac_unit) ? 22: 20)
#define ag7100_pll_offset(_mac)     \
    (((_mac)->mac_unit) ? AR9100_ETH_INT1_CLK : \
                          AR9100_ETH_INT0_CLK)
#else
#define ag7100_pll_shift(_mac)      (((_mac)->mac_unit) ? 19: 17)
#define ag7100_pll_offset(_mac)     \
    (((_mac)->mac_unit) ? AR7100_USB_PLL_GE1_OFFSET : \
                          AR7100_USB_PLL_GE0_OFFSET)
#endif

static void
ag7100_set_pll(ag7100_mac_t *mac, unsigned int pll)
{
#ifdef CONFIG_AR9100
#define ETH_PLL_CONFIG AR9100_ETH_PLL_CONFIG
#else
#define ETH_PLL_CONFIG AR7100_USB_PLL_CONFIG
#endif 
    uint32_t shift, reg, val;

    shift = ag7100_pll_shift(mac);
    reg   = ag7100_pll_offset(mac);

    val  = ar7100_reg_rd(ETH_PLL_CONFIG);
    val &= ~(3 << shift);
    val |=  (2 << shift);
    ar7100_reg_wr(ETH_PLL_CONFIG, val);
    udelay(100);

    ar7100_reg_wr(reg, pll);

    val |=  (3 << shift);
    ar7100_reg_wr(ETH_PLL_CONFIG, val);
    udelay(100);

    val &= ~(3 << shift);
    ar7100_reg_wr(ETH_PLL_CONFIG, val);
    udelay(100);

    printf(": pll reg %#x: %#x  \n", reg, ar7100_reg_rd(reg));
}

#ifdef CONFIG_AR9100
int fifo_3 = 0x780008;
#else
int fifo_3 = 0x8001ff;
#endif

//int mii0_if = AG7100_MII0_INTERFACE;

#ifndef CONFIG_AR9100
int gige_pll = 0x11110000;
#else
#define SW_PLL 0x1f000000ul
int gige_pll = 0x1a000000;
#endif

/*
* Cfg 5 settings
* Weed out junk frames (CRC errored, short collision'ed frames etc.)
*/
int fifo_5 = 0x7ffef;
static void ag7100_set_mac_from_link(ag7100_mac_t *mac, int speed, int fdx)
{
#ifdef CONFIG_ATHRS26_PHY
    int change_flag = 0;

    if(mac->mac_speed !=  speed)
        change_flag = 1;

    if(change_flag)
    {
        athrs26_phy_off(mac);
        athrs26_mac_speed_set(mac, speed);
    }
#endif

    mac->speed =  speed;
    mac->duplex   =  fdx;

    ag7100_set_mii_ctrl_speed(mac, speed);
    ag7100_set_mac_duplex(mac, fdx);
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_3, fifo_3);
#ifndef CONFIG_AR9100
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_5, fifo_5);
#endif

    switch (speed)
    {
    case AG7100_PHY_SPEED_1000T:
#ifdef CONFIG_AR9100
        ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_3, 0x780fff);
#endif
        ag7100_set_mac_if(mac, 1);
#ifdef CONFIG_AR9100
        if (mac->mac_unit == 0)
        { /* eth0 */
            ag7100_set_pll(mac, gige_pll);
        }
        else
        {
            ag7100_set_pll(mac, SW_PLL);
        }
#else
        printf( "#%d:%s\n", __LINE__, __FUNCTION__);
        ag7100_set_pll(mac, gige_pll);
#endif
        ag7100_reg_rmw_set(mac, AG7100_MAC_FIFO_CFG_5, (1 << 19));
        break;

    case AG7100_PHY_SPEED_100TX:
        ag7100_set_mac_if(mac, 0);
        ag7100_set_mac_speed(mac, 1);
#ifndef CONFIG_AR7100_EMULATION
#ifdef CONFIG_AR9100
        if (mac->mac_unit == 0)
        { /* eth0 */
            ag7100_set_pll(mac, 0x13000a44);
        }
        else
        {
            ag7100_set_pll(mac, SW_PLL);
        }
#else
        ag7100_set_pll(mac, 0x0001099);
#endif
#endif
        ag7100_reg_rmw_clear(mac, AG7100_MAC_FIFO_CFG_5, (1 << 19));
        break;

    case AG7100_PHY_SPEED_10T:
        ag7100_set_mac_if(mac, 0);
        ag7100_set_mac_speed(mac, 0);
#ifdef CONFIG_AR9100
        if (mac->mac_unit == 0)
        { /* eth0 */
            ag7100_set_pll(mac, 0x00441099);
        }
        else
        {
            ag7100_set_pll(mac, SW_PLL);
        }
#else
        ag7100_set_pll(mac, 0x00991099);
#endif
        ag7100_reg_rmw_clear(mac, AG7100_MAC_FIFO_CFG_5, (1 << 19));
        break;

    default:
        assert(0);
    }

#ifdef CONFIG_ATHRS26_PHY
    if(change_flag) 
        athrs26_phy_on(mac);
#endif

    printf(": cfg_1: %#x\n", ag7100_reg_rd(mac, AG7100_MAC_FIFO_CFG_1));
    printf( ": cfg_2: %#x\n", ag7100_reg_rd(mac, AG7100_MAC_FIFO_CFG_2));
    printf( ": cfg_3: %#x\n", ag7100_reg_rd(mac, AG7100_MAC_FIFO_CFG_3));
    printf( ": cfg_4: %#x\n", ag7100_reg_rd(mac, AG7100_MAC_FIFO_CFG_4));
    printf( ": cfg_5: %#x\n", ag7100_reg_rd(mac, AG7100_MAC_FIFO_CFG_5));
}

static int ag7100_check_link(ag7100_mac_t *mac)
{
    int                 fdx, phy_up;
#ifdef CFG_RTL8366S_PHY
    int speed;
#else
    ag7100_phy_speed_t  speed;
#endif
    int                 rc;

    /* The vitesse switch uses an indirect method to communicate phy status
    * so it is best to limit the number of calls to what is necessary.
    * However a single call returns all three pieces of status information.
    * 
    * This is a trivial change to the other PHYs ergo this change.
    *
    */
    
    /*
    ** If this is not connected, let's just jump out
    */
    
    if(mii_if(mac) > 3)
        return 0;

    rc = ag7100_get_link_status(mac->mac_unit, &phy_up, &fdx, &speed);
    if (rc < 0)
        return 0;

    if (!phy_up)
    {
         return 0;
    }

    /*
    * phy is up. Either nothing changed or phy setttings changed while we 
    * were sleeping.
    */

    if ((fdx < 0) || (speed < 0))
    {
         return 0;
    }
    printf( ": unit %d phy is up...", mac->mac_unit);
    printf("%s %s %s\n", mii_str[mac->mac_unit][mii_if(mac)], 
        spd_str[speed], dup_str[fdx]);

    ag7100_set_mac_from_link(mac, speed, fdx);

    printf(": done cfg2 %#x ifctl %#x miictrl %#x \n", 
        ag7100_reg_rd(mac, AG7100_MAC_CFG2), 
        ag7100_reg_rd(mac, AG7100_MAC_IFCTL),
        ar7100_reg_rd(mii_reg(mac)));
   return 1;
}

/*
 * For every command we re-setup the ring and start with clean h/w rx state
 */
static int ag7100_clean_rx(struct eth_device *dev, bd_t * bd)
{

    int i;
    ag7100_desc_t *fr;
    ag7100_mac_t *mac = (ag7100_mac_t*)dev->priv;

    if (!ag7100_check_link(mac))
        return 0;

    mac->next_rx = 0;
    for (i = 0; i < NO_OF_RX_FIFOS; i++) {
        fr = mac->fifo_rx[i];
        fr->pkt_start_addr = virt_to_phys(NetRxPackets[i]);
        flush_cache((u32) NetRxPackets[i], PKTSIZE_ALIGN);
        ag7100_rx_give_to_dma(fr);
    }

    ag7100_reg_wr(mac, AG7100_DMA_RX_DESC, virt_to_phys(mac->fifo_rx[0]));
    ag7100_reg_wr(mac, AG7100_DMA_RX_CTRL, AG7100_RXE);	/* rx start */
    udelay(1000 * 1000);


    return 1;

}

static int ag7100_alloc_fifo(int ndesc, ag7100_desc_t ** fifo)
{
    int i;
    u32 size;
    uchar *p = NULL;

    size = sizeof(ag7100_desc_t) * ndesc;
    size += CFG_CACHELINE_SIZE - 1;

    if ((p = malloc(size)) == NULL) {
        printf("Cant allocate fifos\n");
        return -1;
    }

    p = (uchar *) (((u32) p + CFG_CACHELINE_SIZE - 1) &
	   ~(CFG_CACHELINE_SIZE - 1));
    p = UNCACHED_SDRAM(p);

    for (i = 0; i < ndesc; i++)
        fifo[i] = (ag7100_desc_t *) p + i;

    return 0;
}

static int ag7100_setup_fifos(ag7100_mac_t *mac)
{
    int i;

    if (ag7100_alloc_fifo(NO_OF_TX_FIFOS, mac->fifo_tx))
        return 1;

    for (i = 0; i < NO_OF_TX_FIFOS; i++) {
        mac->fifo_tx[i]->next_desc = (i == NO_OF_TX_FIFOS - 1) ?
            virt_to_phys(mac->fifo_tx[0]) : virt_to_phys(mac->fifo_tx[i + 1]);
        ag7100_tx_own(mac->fifo_tx[i]);
    }

    if (ag7100_alloc_fifo(NO_OF_RX_FIFOS, mac->fifo_rx))
        return 1;

    for (i = 0; i < NO_OF_RX_FIFOS; i++) {
        mac->fifo_rx[i]->next_desc = (i == NO_OF_RX_FIFOS - 1) ?
            virt_to_phys(mac->fifo_rx[0]) : virt_to_phys(mac->fifo_rx[i + 1]);
    }

    return (1);
}

static void ag7100_halt(struct eth_device *dev)
{
    ag7100_mac_t *mac = (ag7100_mac_t *)dev->priv;
    ag7100_reg_wr(mac, AG7100_DMA_RX_CTRL, 0);
    while (ag7100_reg_rd(mac, AG7100_DMA_RX_CTRL));
}

unsigned char *
ag7100_mac_addr_loc(void)
{

#ifdef BOARDCAL
    /*
    ** BOARDCAL environmental variable has the address of the cal sector
    */
    
    return ((unsigned char *)BOARDCAL);
    
#else
	extern flash_info_t flash_info[];

	/* MAC address is store in the 2nd 4k of last sector */
	return ((unsigned char *)
		(KSEG1ADDR(AR7100_SPI_BASE) + (4 * 1024) +
		flash_info[0].size - (64 * 1024) /* sector_size */ ));
#endif

}

static void ag7100_get_ethaddr(struct eth_device *dev)
{
    unsigned char *eeprom;
    unsigned char *mac = dev->enetaddr;

    eeprom = ag7100_mac_addr_loc();

    if (strcmp(dev->name, "eth0") == 0) {
        memcpy(mac, eeprom, 6);
    } else if (strcmp(dev->name, "eth1") == 0) {
        eeprom += 6;
        memcpy(mac, eeprom, 6);
    } else {
        printf("%s: unknown ethernet device %s\n", __func__, dev->name);
        return;
    }

    /* Use fixed address if the above address is invalid */
    if (mac[0] == 0xff && mac[5] == 0xff) {
        mac[0] = 0x00;
        mac[1] = 0x03;
        mac[2] = 0x7f;
        mac[3] = 0x09;
        mac[4] = 0x0b;
        mac[5] = 0xad;
        printf("No valid address in Flash. Using fixed address\n");
    }
}

#ifdef CONFIG_AR9100_MDIO_DEBUG
int
ag7100_dump_vsc_regs(ag7100_mac_t *mac)
{

	unsigned i;
	unsigned short v;
	char *fmt[] = {"\t", "\n"};

	printf("IEEE & Standard registers\n");
	for (i = 0; i < 0x20; i++) {
		v = 0;
		ag7100_miiphy_read(mac->dev->name, 0, (uint8_t)i, (uint16_t*)&v);
		printf("0x%02x: 0x%04x%s", i, v, fmt[i & 1]);
	}

	printf("Extended registers\n");

	/* Enable extended register set access */
	ag7100_miiphy_write(mac->dev->name, 0, (uint8_t)0x1f,  (uint16_t)0x1);
	for (i = 16; i <= 30; i++) {
		v = 0;
		ag7100_miiphy_read(mac->dev->name, 0, (uint8_t)i, (uint16_t*)&v);
		printf("0x%02x: 0x%04x%s", i, v, fmt[i & 1]);
	}
	ag7100_miiphy_write(mac->dev->name, 0, (uint8_t)0x1f,  (uint16_t)0x0);
	printf("\n");
}
#endif

int ag7100_enet_initialize(bd_t * bis)
{
    struct eth_device *dev[CFG_AG7100_NMACS];
    u32 mask, mac_h, mac_l;
    int i;
#if defined(CFG_ATHRS26_PHY) && defined(CFG_SWITCH_FREQ)
    u32 pll_value;
#endif

    printf("ag7100_enet_initialize...\n");

#ifdef AR9100
   /* Workaround to bring the TX_EN to low */

    i = *(volatile int *) 0xb806001c ;
    *(volatile int *) 0xb806001c = (i | 0x3300);
    udelay(10 * 1000);
     i = *(volatile int *) 0xb806001c ;
    *(volatile int *) 0xb806001c = (i & 0xffffccff);
    udelay(10 * 1000);
    *(volatile int *) 0xb8070000 = 0x13;
    *(volatile int *) 0xb8070004 = 0x11;
    udelay(10 * 1000);
    *(volatile int *) 0xb9000000 = 0x0;
    *(volatile int *) 0xba000000 = 0x0;
     i = *(volatile int *) 0xb806001c ;
    *(volatile int *) 0xb806001c = (i | 0x3300);
    udelay(10 * 1000);
#endif

    for (i = 0;i < CFG_AG7100_NMACS;i++) {

    if ((dev[i] = (struct eth_device *) malloc(sizeof (struct eth_device))) == NULL) {
        puts("malloc failed\n");
        return 0;
    }
	
    if ((ag7100_macs[i] = (ag7100_mac_t *) malloc(sizeof (ag7100_mac_t))) == NULL) {
        puts("malloc failed\n");
        return 0;
    }

    memset(ag7100_macs[i], 0, sizeof(ag7100_macs[i]));
    memset(dev[i], 0, sizeof(dev[i]));

    sprintf(dev[i]->name, "eth%d", i);
    ag7100_get_ethaddr(dev[i]);
    
    ag7100_macs[i]->mac_unit = i;
    ag7100_macs[i]->mac_base = i ? AR7100_GE1_BASE : AR7100_GE0_BASE ;
    ag7100_macs[i]->dev = dev[i];

    dev[i]->iobase = 0;
    dev[i]->init = ag7100_clean_rx;
    dev[i]->halt = ag7100_halt;
    dev[i]->send = ag7100_send;
    dev[i]->recv = ag7100_recv;
    dev[i]->priv = (void *)ag7100_macs[i];	

    eth_register(dev[i]);

#if defined(CFG_ATHRS26_PHY) && defined(CFG_ATHRHDR_EN)
    athrs26_reg_dev(dev[i]);
#endif
#if (CONFIG_COMMANDS & CFG_CMD_MII)
    miiphy_register(dev[i]->name, ag7100_miiphy_read, ag7100_miiphy_write);
#endif
        /*
        ** This is the actual reset sequence
        */
        
#if defined(CONFIG_WNDR3700U) || defined(CONFIG_WNDR3700V1H2)
        mask = i ?(AR7100_RESET_GE1_MAC) :
                  (AR7100_RESET_GE0_MAC | AR7100_RESET_GE0_PHY);
#else
        mask = i ?(AR7100_RESET_GE1_MAC | AR7100_RESET_GE1_PHY) :
                  (AR7100_RESET_GE0_MAC | AR7100_RESET_GE0_PHY);
#endif

        ar7100_reg_rmw_set(AR7100_RESET, mask);
        udelay(1000 * 1000);

        ar7100_reg_rmw_clear(AR7100_RESET, mask);
        udelay(1000 * 1000);

    ag7100_hw_start(ag7100_macs[i]);
    ag7100_setup_fifos(ag7100_macs[i]);

#if defined(CFG_ATHRS26_PHY) && defined(CFG_SWITCH_FREQ)
    pll_value = ar7100_reg_rd(AR7100_CPU_PLL_CONFIG);
    mask = pll_value & ~(PLL_CONFIG_PLL_FB_MASK | PLL_CONFIG_REF_DIV_MASK);
    mask = mask | (0x64 << PLL_CONFIG_PLL_FB_SHIFT) |
        (0x5 << PLL_CONFIG_REF_DIV_SHIFT) | (1 << PLL_CONFIG_AHB_DIV_SHIFT);

    ar7100_reg_wr_nf(AR7100_CPU_PLL_CONFIG, mask);
    udelay(100 * 1000);
#endif

    ag7100_phy_setup(ag7100_macs[i]->mac_unit);
    udelay(100 * 1000);

#if defined(CFG_ATHRS26_PHY) && defined(CFG_SWITCH_FREQ)
    ar7100_reg_wr_nf(AR7100_CPU_PLL_CONFIG, pll_value);
    udelay(100 * 1000);
#endif
    {
        unsigned char *mac = dev[i]->enetaddr;

        printf("%s: %02x:%02x:%02x:%02x:%02x:%02x\n", dev[i]->name,
               mac[0] & 0xff, mac[1] & 0xff, mac[2] & 0xff,
               mac[3] & 0xff, mac[4] & 0xff, mac[5] & 0xff);
    }
    mac_l = (dev[i]->enetaddr[4] << 8) | (dev[i]->enetaddr[5]);
    mac_h = (dev[i]->enetaddr[0] << 24) | (dev[i]->enetaddr[1] << 16) |
        (dev[i]->enetaddr[2] << 8) | (dev[i]->enetaddr[3] << 0);

    ag7100_reg_wr(ag7100_macs[i], AG7100_GE_MAC_ADDR1, mac_l);
    ag7100_reg_wr(ag7100_macs[i], AG7100_GE_MAC_ADDR2, mac_h);

#if defined(CFG_ATHRS26_PHY) && defined(CFG_ATHRHDR_EN)
    /* if using header for register configuration, we have to     */
    /* configure s26 register after frame transmission is enabled */
    	athrs26_reg_init();
#endif

    printf("%s up\n",dev[i]->name);
    }

#ifdef CONFIG_AR9100_MDIO_DEBUG
    ag7100_dump_vsc_regs(ag7100_macs[i]);
#endif

    return 1;
}

#if (CONFIG_COMMANDS & CFG_CMD_MII)
int ag7100_miiphy_read(char *devname, uint32_t phaddr,
		       uint8_t reg, uint16_t *value)
{
    uint16_t addr = (phaddr << AG7100_ADDR_SHIFT) | reg;
    volatile int rddata;
    ag7100_mac_t *mac = ag7100_name2mac(devname);

    ag7100_reg_wr(mac, AG7100_MII_MGMT_CMD, 0x0);
    ag7100_reg_wr(mac, AG7100_MII_MGMT_ADDRESS, addr);
    ag7100_reg_wr(mac, AG7100_MII_MGMT_CMD, AG7100_MGMT_CMD_READ);

    rddata = ag7100_reg_rd(mac, AG7100_MII_MGMT_IND) & 0x1;
    while (rddata) {
        rddata = ag7100_reg_rd(mac, AG7100_MII_MGMT_IND) & 0x1;
    }

    *value = ag7100_reg_rd(mac, AG7100_MII_MGMT_STATUS);
    ag7100_reg_wr(mac, AG7100_MII_MGMT_CMD, 0x0);

    return 0;
}

int ag7100_miiphy_write(char *devname, uint32_t phaddr,
			uint8_t reg, uint16_t data)
{
    uint16_t addr = (phaddr << AG7100_ADDR_SHIFT) | reg;
    volatile int rddata;
    ag7100_mac_t *mac = ag7100_name2mac(devname);


    ag7100_reg_wr(mac, AG7100_MII_MGMT_ADDRESS, addr);
    ag7100_reg_wr(mac, AG7100_MII_MGMT_CTRL, data);

    rddata = ag7100_reg_rd(mac, AG7100_MII_MGMT_IND) & 0x1;
    while (rddata) {
        rddata = ag7100_reg_rd(mac, AG7100_MII_MGMT_IND) & 0x1;
    }
    return 0;
}

#if defined(CFG_ATHRS26_PHY) && defined(CFG_ATHRHDR_EN)
/***********************************************************************
 * command line interface:To do individual phy reset
 */

int do_miiphyreset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int phyUnit;
	char *p;

	if (argc == 1) {
		printf("Usage:\n    miiphyreset phyUnit\n");
		return -1;
	}

	*p = *argv[1];
	phyUnit = *p - '0';

	if (phyUnit > 4) {
		printf("phyUnit to big!\n");
		return -1;
	}
	individual_phyreset(phyUnit);
	printf("Completed!\n");
	return 0;
}

U_BOOT_CMD(
    miiphyreset ,CFG_MAXARGS, 1, do_miiphyreset,
    "miiphyreset - Reset phy\n",
    " port no\n"
    );
#endif

#endif		/* CONFIG_COMMANDS & CFG_CMD_MII */

