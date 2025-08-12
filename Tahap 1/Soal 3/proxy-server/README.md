# Proxy Server

## Panduan Konfigurasi Proxy Server

### Konfigurasi Dasar
Ganti hostname mesin
```bash
sudo hostnamectl set-hostname proxy-server
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
Arahkan nameserver ke 192.168.10.1
```bash
echo "nameserver 192.168.10.1" | sudo tee /etc/resolv.conf
```

### Konfigurasi Reverse Proxy
Pindahkan file reverse_proxy.py ke direktori home
```bash
sudo mv reverse_proxy.py /home/test/
```

Pindahkan file reverse-proxy.service ke direktori systemd
```bash
sudo mv reverse-proxy.service /etc/systemd/system/
```

Aktivasi dan jalankan layanan reverse-proxy
```bash
sudo systemctl enable --now reverse-proxy.service
```
---
### Konfigurasi Firewall

Flush aturan iptables
```bash
sudo iptables -F INPUT
sudo iptables -F OUTPUT
sudo iptables -F FORWARD
```

Konfigurasi dasar iptables
```bash
sudo iptables -A INPUT -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
sudo iptables -A OUTPUT -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
sudo iptables -A INPUT -i lo -j ACCEPT
sudo iptables -A OUTPUT -o lo -j ACCEPT
```

Blokir IP tertentu
```bash
sudo iptables -A INPUT -s 192.168.10.101 -j DROP
```

Izinkan trafik HTTP masuk
```bash
sudo iptables -A INPUT -p tcp --dport 80 -j ACCEPT
```

Izinkan *traffic* keluar ke port 8080
```bash
sudo iptables -A OUTPUT -p tcp -d 192.168.10.2 --dport 8080 -j ACCEPT
```

Set default policy DROP untuk semua chain
```bash
sudo iptables -P INPUT DROP
sudo iptables -P OUTPUT DROP
sudo iptables -P FORWARD DROP
```

Simpan dan aktifkan layanan iptables
```bash
sudo iptables-save | sudo tee /etc/iptables/iptables.rules
sudo systemctl enable iptables.service
```