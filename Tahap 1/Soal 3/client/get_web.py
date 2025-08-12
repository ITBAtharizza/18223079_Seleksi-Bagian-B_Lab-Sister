import requests
import sys
import subprocess
import os
import socket
import time

def get_local_ip():
    s = None
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("192.168.10.1", 80))
        ip_address = s.getsockname()[0]
    except Exception:
        ip_address = "Tidak dapat mendeteksi IP"
    finally:
        if s:
            s.close()
    return ip_address

def run_command(command):
    subprocess.run(command, shell=True, check=True)

def configure_dhcp():
    if os.path.exists("/etc/systemd/network/10-dhcp.network.bak"):
        run_command("sudo mv /etc/systemd/network/10-dhcp.network.bak /etc/systemd/network/10-dhcp.network")
    if os.path.exists("/etc/systemd/network/20-manual.network"):
        run_command("sudo mv /etc/systemd/network/20-manual.network /etc/systemd/network/20-manual.network.bak")
    
    run_command("sudo systemctl restart systemd-networkd")
    print("\nKonfigurasi DHCP diterapkan")

def configure_manual():
    if os.path.exists("/etc/systemd/network/20-manual.network.bak"):
        run_command("sudo mv /etc/systemd/network/20-manual.network.bak /etc/systemd/network/20-manual.network")
    if os.path.exists("/etc/systemd/network/10-dhcp.network"):
        run_command("sudo mv /etc/systemd/network/10-dhcp.network /etc/systemd/network/10-dhcp.network.bak")
   
    run_command("sudo systemctl restart systemd-networkd")
    print("\nKonfigurasi IP Statis diterapkan.")

def get_website_content(url):
    print(f"\n--> Mencoba mengakses {url} dari IP: {get_local_ip()}")
    try:
        if not url.startswith('http'):
            url = 'http://' + url
        response = requests.get(url, timeout=10)
        response.raise_for_status()
        print("\n[V] Berhasil! Kode Status:", response.status_code)
        print("-" * 30, "\nKonten Halaman Web:\n", response.text, "\n", "-" * 30)
    except Exception as e:
        print(f"\n[X] Gagal! Error: {e}")

if __name__ == '__main__':
    if os.geteuid() != 0:
        print("Error: Only sudo privileged users that can run this code")
        sys.exit(1)

    print("Pilih mode konfigurasi jaringan:")
    print("1. DHCP\n2. IP Statis")
    choice = input("Masukkan pilihan (1 atau 2): ")

    if choice == '1':
        configure_dhcp()
    elif choice == '2':
        configure_manual()
    else:
        print("Pilihan tidak valid. Keluar dari kode...")
        sys.exit(1)

    time.sleep(5)
    get_website_content("www.shwibzka.shirahata")