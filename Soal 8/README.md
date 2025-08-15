# Se mettre Requiem
## Kompilasi
```bash
gcc -O3 -o kali kali.c
```
---
## Penggunaan
```bash
./kali
<input angka pertama>
<input angka kedua>
<output hasil>
```
---
## Implementasi Algoritma
Source code menggunakan algoritma NTT (Number Theoretic Transform) dan Barret Reduction.

---
## Benchmark Perkalian n x n
| Jumlah Digit n | Waktu (s)          |
|----------------|--------------------|
| 1              | 0.025              |
| 10             | 0.036              |
| 100            | 0.044              |
| 1.000          | 0.131              |
| 10.000         | 2.014              |
| 100.000        | 22.156             |
| 250.000        | 49.352             |
| 500.000        | 96.229             |
| 750.000        | 182.466            |
| 1.000.000      | 226.364            |
