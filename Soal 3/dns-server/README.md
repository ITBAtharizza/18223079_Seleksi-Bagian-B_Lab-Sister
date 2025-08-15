# DNS Server

## Panduan Konfigurasi DNS Server

### Konfigurasi Dasar
Ganti hostname mesin
```bash
sudo hostnamectl set-hostname dns-server
```

Pindahkan file konfigurasi jaringan ke lokasi yang benar
```bash
sudo mv 20-wired.network /etc/systemd/network/
```

Restart layanan jaringan agar konfigurasi baru diterapkan
```bash
sudo systemctl restart systemd-networkd
```
---
### Konfigurasi DNS
Pindahkan file named.conf ke direktori /etc
```bash
sudo mv named.conf /etc/
```

Pindahkan file shwibzka.shirahata.zone ke direktori /etc/named
```bash
sudo mv shwibzka.shirahata.zone /etc/named/
```

Aktivasi dan jalankan layanan DNS
```bash
sudo systemctl enable --now named.service
```
---
### Konfigurasi DHCP
Pindahkan file dhcpd.conf ke direktori /etc
```bash
sudo mv dhcpd.conf /etc/
```

Aktivasi dan jalankan layanan DHCP
```bash
sudo systemctl enable --now dhcpd4.service
```
---
### Konfigurasi Firewall

Izinkan koneksi DNS
```bash
sudo iptables -A INPUT -p udp --dport 53 -j ACCEPT
```

Izinkan koneksi DHCP
```bash
sudo iptables -A INPUT -p udp --dport 67 -j ACCEPT
```

Izinkan ping
```bash
sudo iptables -A INPUT -p icmp -j ACCEPT
```

Simpan aturan iptables dan aktifkan layanan iptables
```bash
sudo iptables-save | sudo tee /etc/iptables/iptables.rules
sudo systemctl enable iptables.service
```