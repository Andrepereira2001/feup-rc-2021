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



------------------

Tux3: 
Terminal: 
    ifconfig eth0 up 172.16.20.1
    ifconfig (se quiseres confirmar)

Tux4:
Terminal: 
    ifconfig eth0 up 172.16.20.254
    ifconfig eth1 up 172.16.21.253

Tux2: 
Terminal: 
    ifconfig eth0 up 172.16.21.1

-- VLAN

TuxX: 
GKTerm: 
    - creating ethernet vlan
    configure terminal
    vlan 20 
    end
    show vlan

    - add port to vlan 
    configure terminal 
    interface fastethernet 0/4 (adicionar a porta 4 - que é a que está ligada ao tux4 à vlan 20)
    switchport mode acess 
    switchport access vlan 20
    end
    show vlan (porta 4 foi adicionada a vlan 20)~

    - add port to vlan 
    configure terminal 
    interface fastethernet 0/3 (adicionar a porta 3 - que é a que está ligada ao tux3 à vlan 20)
    switchport mode acess 
    switchport access vlan 20
    end
    show vlan (porta 3 foi adicionada a vlan 20)

    - creating ethernet vlan
    configure terminal
    vlan 21 
    end
    show vlan

    - add port to vlan 
    configure terminal 
    interface fastethernet 0/5 (adicionar a porta 5 - que é a que está ligada ao tux2 à vlan 21)
    switchport mode acess 
    switchport access vlan 21
    end
    show vlan (porta 5 foi adicionada a vlan 21)

No tux3: 
Terminal: 
    ping 172.16.20.254 - do tux3 para o tux4 (e funciona!)
    ping 172.16.21.1 - do tux3 para o tux4 (não funciona porque não estão na mesma vlan)

    - add port 6 to vlan 21
    configure terminal 
    interface fastethernet 0/6 (adicionar a porta 6 - que é a que está ligada ao tux4 pela eth1- à vlan 21)
    switchport mode acess 
    switchport access vlan 21
    end
    show vlan (porta 6 foi adicionada a vlan 21)


No tux4: 
Terminal: 
    ifconfig eth1 up 172.16.21.253

    echo 1 > /proc/sys/net/ipv4/ip_forward (enabling ip forwarding)
    echo 0 > /proc/sys/net/piv4/icmp_echo_ignore_broadcasts (disable icmp echo ignore broadcast)

    ifconfig (to check the Mac Adress and IP)
    Results:
    eth0 ip 172.16.20.254 Mac: 00:22:64:a7:26:a2
    eth1 ip 172.16.21.253 Mac: 00:e0:7d:c8:76:55


No tux3: 
Terminal: 
    ip route add 172.16.21.0/24 via 172.16.20.254
    ip route add 172.16.Y0.0/24 via 172.16.Y1.253

    - iniciar o wireshark
    ping 172.16.20.254 (do tux3 para tux4)
    ping 172.16.21.253 (do tux3 para tux4 eth1)
    ping 172.16.21.1 (do tux3 para tux2)



Cisco router


adicionar porta do cisco router 0/9 á vlan 41

No tux3:
Terminal: 
    - add port to vlan 
    configure terminal 
    interface fastethernet 0/9 (adicionar a porta 9 - que é a que está ligada ao tux4 à vlan 20)
    switchport mode acess 
    switchport access vlan 20
    end
    show vlan (porta 4 foi adicionada a vlan 20)
