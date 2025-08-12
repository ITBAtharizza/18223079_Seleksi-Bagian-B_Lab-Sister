# Client

## Panduan Konfigurasi Client

### Konfigurasi Dasar
Ganti hostname mesin
```bash
sudo hostnamectl set-hostname pc
```

Pindahkan file 20-manual.network ke lokasi yang benar
```bash
sudo mv 20-manual.network /etc/systemd/network/
```

Pindahkan file 10-dhcp.network ke lokasi yang benar
```bash
sudo mv 10-dhcp.network /etc/systemd/network/
```

Rename file 10-dhcp.network menjadi 10-dhcp.network.bak
```bash
sudo mv /etc/systemd/network/10-dhcp.network /etc/systemd/network/10-dhcp.network.bak
```

Restart layanan jaringan agar konfigurasi diterapkan
```bash
sudo systemctl restart systemd-networkd
```
---
### Konfigurasi DNS
Arahkan nameserver ke 192.168.10.1
```bash
echo "nameserver 192.168.10.1" | sudo tee /etc/resolv.conf
```
---
### Konfigurasi Kode
Pindahkan file get_web.py ke direktori yang diinginkan
```bash
sudo mv get_web.py /home/test/
```

Jalankan kode dengan sudo
```bash
sudo python /home/test/get_web.py
```
---
### Catatan untuk Bonus
Lakukan "Linked Clone" dari mesin ini untuk mensimulasikan client lain.

Ubah isi file 20-manual.network untuk mengganti IP (misal: 192.168.10.5/24)
```bash
sudo nano /etc/systemd/network/20-manual.network
```