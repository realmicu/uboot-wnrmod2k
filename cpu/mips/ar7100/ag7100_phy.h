#ifndef _AG7100_PHY_H
#define _AG7100_PHY_H

typedef enum {
    AG7100_PHY_SPEED_10T,
    AG7100_PHY_SPEED_100TX,
    AG7100_PHY_SPEED_1000T,
}ag7100_phy_speed_t;

#ifdef CFG_ATHRS26_PHY
#ifndef AR9100
#include "../board/ar7100/ap94/athrs26_phy.h"

#define ag7100_phy_setup(unit)          athrs26_phy_setup (unit)
#define ag7100_phy_is_up(unit)          athrs26_phy_is_up (unit)
#define ag7100_phy_speed(unit)          athrs26_phy_speed (unit)
#define ag7100_phy_is_fdx(unit)         athrs26_phy_is_fdx (unit)

static inline unsigned int 
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
  *link=ag7100_phy_is_up(unit);
  *fdx=ag7100_phy_is_fdx(unit);
  *speed=ag7100_phy_speed(unit);
  return 0;
}

#else
#define ag7100_phy_setup(unit) do { \
if(!unit) \
        athrs26_phy_setup(unit); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(!unit) \
        link=miiphy_link("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(!unit) \
        duplex = miiphy_duplex("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(!unit) \
        speed = miiphy_speed("eth0", CFG_PHY_ADDR); \
} while (0);

#endif // Hydra or Howl
#endif

#ifdef CFG_RTL8366S_PHY
#define ag7100_phy_setup(unit)          rtl8366s_phy_setup (unit)
#define ag7100_rtl_get_tx(Port)	      rtl8366s_get_mib(Port)

// {"down" = 0, "up" = 1}
// {"half duplex" = 0, "full duplex" = 1}
// {"10Mbps" = 0, "100Mbps" = 1, "1000Mbps" = 2}
int ag7100_get_link_status(int unit, int *link, int *fdx, int *speed)
{
  if (1==unit)  // WAN is Giga PHY 4
  {
      *link = 0; // link should be always down
      *fdx = 0; // {"half duplex" = 0, "full duplex" = 1}
      *speed = 0; // {"10Mbps" = 0, "100Mbps" = 1, "1000Mbps" = 2}
      return 0;
  }
  else if (0==unit) {
      *link = 1; // link should be always up
      *fdx = 1; // {"half duplex" = 0, "full duplex" = 1}
      *speed = 2; // {"10Mbps" = 0, "100Mbps" = 1, "1000Mbps" = 2}
      return 0;
  }
  return -1;
}

#endif

#ifdef CFG_VSC8201_PHY

#define ag7100_phy_setup(unit) do { \
if(!unit) \
        vsc_phy_setup(unit); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(!unit) \
        link=miiphy_link("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(!unit) \
        duplex = miiphy_duplex("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(!unit) \
        speed = miiphy_speed("eth0", CFG_PHY_ADDR); \
} while (0);

#endif

#ifdef CFG_VSC8601_PHY

#define ag7100_phy_setup(unit) do { \
if(!unit) \
        vsc8601_phy_setup(unit); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(!unit) \
        link=miiphy_link("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(!unit) \
        duplex = miiphy_duplex("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(!unit) \
        speed = miiphy_speed("eth0", CFG_PHY_ADDR); \
} while (0);

#endif

#ifdef CFG_VITESSE_8601_7395_PHY

#define ag7100_phy_setup(unit) do { \
if(unit) \
	vsc73xx_setup(unit); \
else \
	vsc8601_phy_setup(unit); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(unit) \
	vsc73xx_get_link_status(unit, &link, &fdx, &speed,0); \
else \
        link=miiphy_link("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(unit) \
	vsc73xx_get_link_status(unit, 0, &duplex, 0,0); \
else \
	duplex = miiphy_duplex("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(unit) \
	vsc73xx_get_link_status(unit, 0, 0, &speed,0); \
else \
	speed = miiphy_speed("eth0", CFG_PHY_ADDR); \
} while (0);

#endif


#ifdef CFG_IP175B_PHY

#define ag7100_phy_setup(unit) do { \
if(!unit) \
        ip_phySetup(unit); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(!unit) \
        link=ip_phyIsUp(unit); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(!unit) \
        duplex = ip_phyIsFullDuplex(unit); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(!unit) \
        speed = ip_phySpeed(unit); \
} while (0);

#endif

#ifdef CONFIG_ADMTEK_PHY

#define ag7100_phy_setup(unit) do { \
if(!unit) \
        miiphy_reset("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(!unit) \
        link=miiphy_link("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(!unit) \
        duplex = miiphy_duplex("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(!unit) \
        speed = miiphy_speed("eth0", CFG_PHY_ADDR); \
} while (0);

#endif
#endif /*_AG7100_PHY_H*/
