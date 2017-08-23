DEVICETOUCH=$(sudo lsinput | grep -B 5 FlatFrog | grep event)
sudo ./mtdev2tuio "$DEVICETOUCH" osc.udp://127.0.0.1:3333
