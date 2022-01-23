ifconfig eth0 down
ifconfig eth0 up 172.16.20.254/24
ifconfig eth1 down
ifconfig eth1 up 172.16.21.253/24


echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/piv4/icmp_echo_ignore_broadcasts

ip route add default via 172.16.21.254

