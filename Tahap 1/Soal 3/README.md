# DNS - DeeezNuddS 💔😩

## Panduan Instalasi dan Setup VM Arch Linux dengan archinstall dan Linked Cloning
Dokumen ini menjelaskan cara konfigurasi VM-VM yang akan digunakan dalam pengerjaan soal ini.

---

### Langkah-langkah

1. **Gunakan archinstall:**

   - Jalankan archinstall.
   - Pada bagian **Profile**, pilih opsi `server`, lalu pilih `httpd`.
   - Pada bagian **Additional Packages**, pilih paket-paket berikut:
     ```
     bind python python-requests dhcp nano inetutils iptables-nft
     ```

2. **Matikan VM** setelah proses instalasi selesai.

3. Klik kanan pada VM, buka **Settings**, cari pilihan **Network** dan ubah bagian **Attached to** menjadi **Internal Network**

4. **Clone VM:**
   - Klik kanan pada VM yang sudah diinstall.
   - Pilih opsi **Clone**.
   - Pilih tipe clone: **Linked Clone**.
   - Pilih opsi **Generate new MAC address** untuk setiap clone agar MAC address berbeda.

5. **Buat minimal 4 clone VM** dengan cara di atas, untuk keperluan:
   - DNS Server
   - Web Server
   - Reverse Proxy
   - Client

---

### Catatan

- Pastikan setiap VM memiliki MAC address yang unik agar tidak terjadi konflik jaringan.
- Linked Clone memungkinkan VM baru berbagi disk virtual dengan VM asli sehingga menghemat ruang penyimpanan.

---

### Bonus dan Dokumentasi

Semua spesifikasi bonus dikerjakan, dapat dilihat dari video berikut: [18223079_DNS](https://youtu.be/6omf11FAiMg)  

---
