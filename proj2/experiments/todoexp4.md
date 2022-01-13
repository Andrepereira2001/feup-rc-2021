2. fazer experiencia 2 - tux4 eth1 e tux2 tem que estar na mesma vlan e tux4 eth0 e tux3 tb
3. No tux4: if config eth1 down
            if config up 172.16.X1.253/24

5. check the mac and ip adresses in tux4 for eth0 e eth1
6. copiar os códigos
7. route -n (nos três tux)
8. wireshark on tux3
9. ping para os 3
10. guardar logs
11. no tux4 on both eth0 e eth1 
12. arp -d