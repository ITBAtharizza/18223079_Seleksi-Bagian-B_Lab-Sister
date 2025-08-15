# Web Server

## Panduan Konfigurasi Web Server

### Konfigurasi Dasar
Ganti hostname mesin
```bash
sudo hostnamectl set-hostname web-server
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
### Konfigurasi Web Server
Pindahkan file konfigurasi Apache ke direktori yang sesuai
```bash
sudo mv httpd.conf /etc/httpd/conf/
```

Pindahkan file halaman web ke direktori web root
```bash
sudo mv index.html /srv/http/
```

Enable dan jalankan layanan web server Apache
```bash
sudo systemctl enable --now httpd.service
```