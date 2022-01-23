#!/bin/bash

ifconfig eth0 down
ifconfig eth0 up 172.16.21.1/24

ip route add 172.16.20.0/24 via 172.16.21.253

ip route add default via 172.16.21.254