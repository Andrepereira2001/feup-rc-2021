#!/bin/bash

bancada=2

ifconfig eth0 down
ifconfig eth0 up 172.16."$bancada"0.1/24

ip route add 172.16."$bancada"1.0/24 via 172.16."$bancada"0.254

ip route add default via 172.16."$bancada"0.254