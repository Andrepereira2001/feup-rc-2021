#!/bin/bash

ifconfig eth0 down
ifconfig eth0 up 172.16.20.1/24

ip route add 172.16.21.0/24 via 172.16.20.254

ip route add default via 172.16.20.254