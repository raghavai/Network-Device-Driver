
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ip.h>
#include <linux/etherdevice.h>

MODULE_LICENSE("GPL");

struct os_packet {
      struct net_device *dev;
      int datalen;
      u8 data[ETH_DATA_LEN];
};

struct os_priv {
      struct net_device_stats stats;
      struct sk_buff *skb;
      struct os_packet *pkt;
      struct net_device *dev;
};

struct net_device *os0;
struct net_device *os1;
/*OS _priv*/
struct os_priv *priv0;
struct os_priv *priv1;
//struct os_priv *priv;




static void os_tx_i_handler(struct net_device *dev) {
      /* normally stats are kept - you do not need to do much
         here if you do not want to except resume the queue 
         if it is not accepting packets.
       */
	struct os_priv *priv = netdev_priv(dev);
	if (netif_queue_stopped(priv->pkt->dev)) 
		netif_wake_queue(priv->pkt->dev);
   }

static void os_rx_i_handler(struct net_device *dev) {
	/* allocate space for a socket buffer
	add two bytes of space to align on 16 byte boundary
	copy the packet from the private part of dev to the socket buffer
	set the protocol field of the socket buffer using 'eth_type_trans'
	set the dev field of the socket buffer to dev (argument)
	invoke 'netif_rx()' on the socket buffer
	resume the network queue if it is not accepting packets

	*/

	struct sk_buff *skb;
	/*Extract the data and length */
	char *data;
	int len;
	struct os_priv *priv = netdev_priv(dev);
	data = priv->skb->data;
	len = priv->skb->len;

	skb = dev_alloc_skb(len + 2); 
	skb_reserve(skb, 2);  /* to align on a 16B boundary */
	memcpy(skb_put(skb, len), data, len);

	skb->dev = dev;
	skb->protocol = eth_type_trans(skb, dev);
	netif_rx(skb);
	if (netif_queue_stopped(priv->pkt->dev))
	netif_wake_queue(priv->pkt->dev);



}





/*Open , stop , start_xmit,  net_dev_stats */
 int os_open(struct net_device *dev) {
	printk(KERN_INFO "inside os_open!\n");
	netif_start_queue(dev); 
			return 0; 
		}
 int os_stop(struct net_device *dev) {
	printk(KERN_INFO "inside os_Stop!\n");
		 netif_stop_queue(dev); 
			return 0; 
		}



int os_start_xmit(struct sk_buff *skb, struct net_device *dev) {
   char *data = skb->data;
   int len = skb->len;
   struct sk_buff *skb_priv;

   struct iphdr *ih = (struct iphdr *)(data+sizeof(struct ethhdr));
   u32 *saddr = &ih->saddr;
   u32 *daddr = &ih->daddr;
   ((u8*)saddr)[2] ^= 1;
   ((u8*)daddr)[2] ^= 1;
   ih->check = 0;
   ih->check = ip_fast_csum((unsigned char *)ih, ih->ihl);
   skb_priv = dev_alloc_skb(len + 2);
   skb_reserve(skb_priv, 2);
   memcpy(skb_put(skb_priv, len), data, len);
   skb_priv->dev = dev;
   skb_priv->protocol = eth_type_trans(skb_priv, dev);
   netif_rx(skb_priv);
   dev_kfree_skb(skb);
   if (netif_queue_stopped(priv0->pkt->dev)) netif_wake_queue(priv0->pkt->dev);
   if (netif_queue_stopped(priv1->pkt->dev)) netif_wake_queue(priv1->pkt->dev);
   return 0;
}


	struct net_device_stats *os_stats(struct net_device *dev) {

		return &(((struct os_priv*)netdev_priv(dev))->stats);
	}

	/*net_device_ops structure refers to the above one */
	static const struct net_device_ops os_device_ops = {
	      .ndo_open = os_open,
	      .ndo_stop = os_stop,
	      .ndo_start_xmit = os_start_xmit,
	      .ndo_get_stats = os_stats,
	   };

	int os_header(struct sk_buff *skb, struct net_device *dev,
	unsigned short type, const void *daddr, const void *saddr,
	unsigned int len) {

		/* make room for the header in the socket buffer (from argument 'skb') - done 
		set the protocol field (from argument 'type')- done 
		copy the address given by the device to both source and done 
		destination fields (from argument 'dev')- done 
		reverse the LSB on the destination address- done 
		*/

		/*sk_buff sk whenn packet is pushed into one of the interfaces os0 , os1*/

		struct ethhdr *eth = (struct ethhdr*)skb_push(skb,ETH_HLEN);
		eth->h_proto = htons(type);
		/*filling up the destination and source address*/ 

		memcpy(eth->h_source, dev->dev_addr, dev->addr_len);
		memcpy(eth->h_dest, eth->h_source, dev->addr_len);
		eth->h_dest[ETH_ALEN-1] = (eth->h_dest[ETH_ALEN-1] == 5) ? 6 : 5;

		/*Protocol is filled like this */
		return dev->hard_header_len;

		return 0;
		

	


	}


	static const struct header_ops os_header_ops = {
		.create  = os_header,
	};

 


int init_module (void) {


		/* allocate two ethernet devices - done
		set MAC addresses and broadcast values - done 
		set device names- done 
		set network device operations - done 
		set network header creation operation - done
		set NOARP flags - done 
		kmalloc space for a packet - done
		register both network devices - done 
		*/
		int i;

		/*Allocate space */
		os0 = alloc_etherdev(sizeof(struct os_priv));
		os1 = alloc_etherdev(sizeof(struct os_priv));

		//priv0 kmalloc?
		priv0 = kmalloc(sizeof(struct os_priv), GFP_KERNEL);
		//priv1 kmalloc?
		priv1 = kmalloc(sizeof(struct os_priv), GFP_KERNEL);

		/*setting the MAC Address , header length and boradcast for both os0 and os1*/	
		for (i=0 ; i < 6 ; i++) os0->dev_addr[i] = (unsigned char)i;
		for (i=0 ; i < 6 ; i++) os0->broadcast[i] = (unsigned char)15;
		os0->hard_header_len = 14;

		for (i=0 ; i < 6 ; i++) os1->dev_addr[i] = (unsigned char)i;
		for (i=0 ; i < 6 ; i++) os1->broadcast[i] = (unsigned char)15;
		os1->hard_header_len = 14;
		os1->dev_addr[5]++;

		/*Names of the address can be filled for os1 and os0*/
		memcpy(os0->name, "os0\0", 4);
		memcpy(os1->name, "os1\0", 4);

		/* defined in the init module to instantiate the os_device_ops - open , stop, start_xmit*/
		os0->netdev_ops = &os_device_ops;
		os1->netdev_ops = &os_device_ops;

		/*Header_ops instantiation for os_header_ops*/
		os0->header_ops = &os_header_ops;
		os1->header_ops = &os_header_ops;

		/*Allowing no ARP*/
		os0->flags |= IFF_NOARP;
		os1->flags |= IFF_NOARP;

		/*Allocate the private memory for both os1 and os0*/	
		priv0= netdev_priv(os0);
		memset(priv0, 0, sizeof(struct os_priv));
		

		priv1= netdev_priv(os1);
		memset(priv1, 0, sizeof(struct os_priv));

		priv0->pkt = kmalloc (sizeof (struct os_packet), GFP_KERNEL);
		priv0->pkt->dev = os0;

		priv1->pkt = kmalloc (sizeof (struct os_packet), GFP_KERNEL);
		priv1->pkt->dev = os1;


		/*Register the devices */
		register_netdev(os0);
		register_netdev(os1);
		printk(KERN_INFO "The function is running successfully ");

	return 0;


}

void cleanup_module(void) {

/* free the packet space
         unregister the network devices
       */
   struct os_priv *priv;
   if (os0) {
	printk(KERN_INFO "The function is unloaded successfully ");
      priv = netdev_priv(os0);
      kfree(priv->pkt);
      unregister_netdev(os0);
      kfree(os0);
   }
   if (os1) {
	printk(KERN_INFO "The function is unloaded successfully ");
      priv = netdev_priv(os1);
      kfree(priv->pkt);
      unregister_netdev(os1);
      kfree(os1);
   }
}
