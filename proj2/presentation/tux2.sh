#!/bin/bash

bancada=2

ifconfig eth0 down
ifconfig eth0 up 172.16."$bancada"1.1/24

ip route add 172.16."$bancada"0.0/24 via 172.16."$bancada"1.253

ip route add default via 172.16."$bancada"1.254